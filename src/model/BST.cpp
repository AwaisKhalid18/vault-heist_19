#include "model/BST.h"
#include "core/NodeStack.h"
#include "core/TextUtil.h"

BST::BST()
{
    root = nullptr;
    count = 0;
    nextId = 1;
    log = nullptr;
    comparisons = 0;
    lastChangedParent = nullptr;
}

BST::~BST()
{
    clear();
}

void BST::setEventLog(EventLog* eventLog)
{
    log = eventLog;
}

void BST::clear()
{
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
        delete node;
    }
    root = nullptr;
    count = 0;
    lastChangedParent = nullptr;
}

void BST::copyFrom(const BST& other)
{
    clear();
    nextId = other.nextId;        // ids must stay stable across undo copies
    count = other.count;
    comparisons = other.comparisons;
    if (other.root == nullptr)
    {
        return;
    }

    // clone the root, then walk both trees in step with two parallel stacks
    root = new TreeNode(other.root->id, other.root->key);
    root->treasure = other.root->treasure;
    root->height = other.root->height;
    root->size = other.root->size;

    NodeStack sourceStack;
    NodeStack cloneStack;
    sourceStack.push(other.root);
    cloneStack.push(root);
    while (!sourceStack.isEmpty())
    {
        TreeNode* source = sourceStack.pop();
        TreeNode* clone = cloneStack.pop();
        if (source->left != nullptr)
        {
            TreeNode* child = new TreeNode(source->left->id, source->left->key);
            child->treasure = source->left->treasure;
            child->height = source->left->height;
            child->size = source->left->size;
            child->parent = clone;
            clone->left = child;
            sourceStack.push(source->left);
            cloneStack.push(child);
        }
        if (source->right != nullptr)
        {
            TreeNode* child = new TreeNode(source->right->id, source->right->key);
            child->treasure = source->right->treasure;
            child->height = source->right->height;
            child->size = source->right->size;
            child->parent = clone;
            clone->right = child;
            sourceStack.push(source->right);
            cloneStack.push(child);
        }
    }
}

// ---- event helpers ----

void BST::logVisit(TreeNode* node)
{
    if (log == nullptr)
    {
        return;
    }
    Event e;
    e.type = Event::VISIT;
    e.nodeId = node->id;
    e.key = node->key;
    e.text = "Enter room " + TextUtil::intToText(node->key);
    log->add(e);
}

void BST::logCompare(TreeNode* node, int key, int cmpResult)
{
    if (log == nullptr)
    {
        return;
    }
    Event e;
    e.type = Event::COMPARE;
    e.nodeId = node->id;
    e.key = key;
    e.cmp = cmpResult;
    if (cmpResult == Event::CMP_LESS)
    {
        e.side = Event::SIDE_LEFT;
        e.text = "Compare " + TextUtil::intToText(key) + " < " + TextUtil::intToText(node->key) + ", go left";
    }
    else if (cmpResult == Event::CMP_GREATER)
    {
        e.side = Event::SIDE_RIGHT;
        e.text = "Compare " + TextUtil::intToText(key) + " > " + TextUtil::intToText(node->key) + ", go right";
    }
    else
    {
        e.text = "Compare " + TextUtil::intToText(key) + " = " + TextUtil::intToText(node->key) + ", match";
    }
    log->add(e);
}

void BST::logDone(const string& text)
{
    if (log == nullptr)
    {
        return;
    }
    Event e;
    e.type = Event::DONE;
    e.text = text;
    log->add(e);
}

void BST::logHighlight(TreeNode* node, int mode, const string& text)
{
    if (log == nullptr)
    {
        return;
    }
    Event e;
    e.type = Event::HIGHLIGHT_SUBTREE;
    e.nodeId = node->id;
    e.mode = mode;
    e.text = text;
    log->add(e);
}

// ---- height / size maintenance ----

void BST::updateUpwards(TreeNode* node)
{
    TreeNode* cur = node;
    while (cur != nullptr)
    {
        int leftHeight = cur->left != nullptr ? cur->left->height : 0;
        int rightHeight = cur->right != nullptr ? cur->right->height : 0;
        int newHeight = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
        int leftSize = cur->left != nullptr ? cur->left->size : 0;
        int rightSize = cur->right != nullptr ? cur->right->size : 0;
        int newSize = 1 + leftSize + rightSize;

        if (newHeight != cur->height)
        {
            if (log != nullptr)
            {
                Event e;
                e.type = Event::UPDATE_HEIGHT;
                e.nodeId = cur->id;
                e.oldValue = cur->height;
                e.newValue = newHeight;
                e.text = "Height of " + TextUtil::intToText(cur->key) + ": "
                       + TextUtil::intToText(cur->height) + " -> " + TextUtil::intToText(newHeight);
                log->add(e);
            }
            cur->height = newHeight;
        }
        if (newSize != cur->size)
        {
            if (log != nullptr)
            {
                Event e;
                e.type = Event::UPDATE_SIZE;
                e.nodeId = cur->id;
                e.oldValue = cur->size;
                e.newValue = newSize;
                e.text = "Size of " + TextUtil::intToText(cur->key) + ": "
                       + TextUtil::intToText(cur->size) + " -> " + TextUtil::intToText(newSize);
                log->add(e);
            }
            cur->size = newSize;
        }
        cur = cur->parent;
    }
}

// ---- insert ----

bool BST::insertCore(int key)
{
    TreeNode* parent = nullptr;
    TreeNode* cur = root;
    int side = Event::SIDE_NONE;
    while (cur != nullptr)
    {
        logVisit(cur);
        comparisons++;
        if (key == cur->key)
        {
            logCompare(cur, key, Event::CMP_EQUAL);
            return false;   // duplicates are rejected
        }
        parent = cur;
        if (key < cur->key)
        {
            logCompare(cur, key, Event::CMP_LESS);
            cur = cur->left;
            side = Event::SIDE_LEFT;
        }
        else
        {
            logCompare(cur, key, Event::CMP_GREATER);
            cur = cur->right;
            side = Event::SIDE_RIGHT;
        }
    }

    TreeNode* node = new TreeNode(nextId, key);
    nextId++;
    node->parent = parent;
    if (parent == nullptr)
    {
        root = node;
    }
    else if (side == Event::SIDE_LEFT)
    {
        parent->left = node;
    }
    else
    {
        parent->right = node;
    }
    count++;
    lastChangedParent = parent;
    updateUpwards(parent);

    if (log != nullptr)
    {
        Event e;
        e.type = Event::INSERT;
        e.nodeId = node->id;
        e.parentId = parent != nullptr ? parent->id : -1;
        e.side = side;
        e.key = key;
        e.text = "New room " + TextUtil::intToText(key) + " appears";
        e.snapshotIndex = log->saveTreeShape(root);
        log->add(e);
    }
    return true;
}

bool BST::insert(int key)
{
    bool ok = insertCore(key);
    if (ok)
    {
        logDone("Inserted " + TextUtil::intToText(key));
    }
    else
    {
        logDone("Duplicate " + TextUtil::intToText(key) + " rejected");
    }
    return ok;
}

// ---- search ----

bool BST::search(int key)
{
    TreeNode* cur = root;
    while (cur != nullptr)
    {
        logVisit(cur);
        comparisons++;
        if (key == cur->key)
        {
            logCompare(cur, key, Event::CMP_EQUAL);
            logDone("Found " + TextUtil::intToText(key));
            return true;
        }
        if (key < cur->key)
        {
            logCompare(cur, key, Event::CMP_LESS);
            cur = cur->left;
        }
        else
        {
            logCompare(cur, key, Event::CMP_GREATER);
            cur = cur->right;
        }
    }
    logDone(TextUtil::intToText(key) + " is not in the vault");
    return false;
}

bool BST::contains(int key)
{
    TreeNode* cur = root;
    while (cur != nullptr)
    {
        if (key == cur->key)
        {
            return true;
        }
        cur = key < cur->key ? cur->left : cur->right;
    }
    return false;
}

// ---- remove ----

bool BST::removeCore(int key)
{
    TreeNode* cur = root;
    while (cur != nullptr)
    {
        logVisit(cur);
        comparisons++;
        if (key == cur->key)
        {
            logCompare(cur, key, Event::CMP_EQUAL);
            break;
        }
        if (key < cur->key)
        {
            logCompare(cur, key, Event::CMP_LESS);
            cur = cur->left;
        }
        else
        {
            logCompare(cur, key, Event::CMP_GREATER);
            cur = cur->right;
        }
    }
    if (cur == nullptr)
    {
        return false;
    }

    if (cur->left != nullptr && cur->right != nullptr)
    {
        // two children: swap values with the inorder successor
        // (smallest key of the right subtree), then unlink that node
        TreeNode* successorNode = cur->right;
        logVisit(successorNode);
        while (successorNode->left != nullptr)
        {
            successorNode = successorNode->left;
            logVisit(successorNode);
        }

        int oldKey = cur->key;
        cur->key = successorNode->key;
        successorNode->key = oldKey;
        string oldTreasure = cur->treasure;
        cur->treasure = successorNode->treasure;
        successorNode->treasure = oldTreasure;

        if (log != nullptr)
        {
            Event e;
            e.type = Event::SWAP_VALUE;
            e.nodeId = cur->id;
            e.otherId = successorNode->id;
            e.oldValue = oldKey;
            e.newValue = cur->key;
            e.text = "Swap " + TextUtil::intToText(oldKey) + " with successor "
                   + TextUtil::intToText(cur->key);
            e.snapshotIndex = log->saveTreeShape(root);
            log->add(e);
        }
        cur = successorNode;   // this node now holds the key to delete
    }

    // cur has at most one child: bypass it
    TreeNode* child = cur->left != nullptr ? cur->left : cur->right;
    TreeNode* parent = cur->parent;
    int side = Event::SIDE_NONE;
    if (parent != nullptr)
    {
        side = parent->left == cur ? Event::SIDE_LEFT : Event::SIDE_RIGHT;
    }
    if (child != nullptr)
    {
        child->parent = parent;
    }
    if (parent == nullptr)
    {
        root = child;
    }
    else if (parent->left == cur)
    {
        parent->left = child;
    }
    else
    {
        parent->right = child;
    }
    count--;
    lastChangedParent = parent;
    updateUpwards(parent);

    if (log != nullptr)
    {
        Event e;
        e.type = Event::UNLINK;
        e.nodeId = cur->id;
        e.parentId = parent != nullptr ? parent->id : -1;
        e.side = side;
        e.key = cur->key;
        e.text = "Room " + TextUtil::intToText(cur->key) + " collapses";
        e.snapshotIndex = log->saveTreeShape(root);
        log->add(e);
    }
    delete cur;
    return true;
}

bool BST::remove(int key)
{
    bool ok = removeCore(key);
    if (ok)
    {
        logDone("Deleted " + TextUtil::intToText(key));
    }
    else
    {
        logDone(TextUtil::intToText(key) + " is not in the vault");
    }
    return ok;
}

int BST::deleteCaseOf(int key)
{
    TreeNode* cur = root;
    while (cur != nullptr && cur->key != key)
    {
        cur = key < cur->key ? cur->left : cur->right;
    }
    if (cur == nullptr)
    {
        return DEL_NOT_FOUND;
    }
    if (cur->left != nullptr && cur->right != nullptr)
    {
        return DEL_TWO_CHILDREN;
    }
    if (cur->left != nullptr || cur->right != nullptr)
    {
        return DEL_ONE_CHILD;
    }
    return DEL_LEAF;
}

// ---- min / max / neighbours ----

bool BST::findMin(int& result)
{
    if (root == nullptr)
    {
        logDone("The vault is empty");
        return false;
    }
    TreeNode* cur = root;
    logVisit(cur);
    while (cur->left != nullptr)
    {
        cur = cur->left;
        logVisit(cur);
    }
    result = cur->key;
    logDone("Minimum is " + TextUtil::intToText(result));
    return true;
}

bool BST::findMax(int& result)
{
    if (root == nullptr)
    {
        logDone("The vault is empty");
        return false;
    }
    TreeNode* cur = root;
    logVisit(cur);
    while (cur->right != nullptr)
    {
        cur = cur->right;
        logVisit(cur);
    }
    result = cur->key;
    logDone("Maximum is " + TextUtil::intToText(result));
    return true;
}

bool BST::successor(int key, int& result)
{
    // top-down search for the smallest key greater than `key`
    TreeNode* cur = root;
    TreeNode* best = nullptr;
    while (cur != nullptr)
    {
        logVisit(cur);
        if (cur->key > key)
        {
            best = cur;
            cur = cur->left;
        }
        else
        {
            cur = cur->right;
        }
    }
    if (best == nullptr)
    {
        logDone("No successor of " + TextUtil::intToText(key));
        return false;
    }
    result = best->key;
    logDone("Successor of " + TextUtil::intToText(key) + " is " + TextUtil::intToText(result));
    return true;
}

bool BST::predecessor(int key, int& result)
{
    // top-down search for the largest key smaller than `key`
    TreeNode* cur = root;
    TreeNode* best = nullptr;
    while (cur != nullptr)
    {
        logVisit(cur);
        if (cur->key < key)
        {
            best = cur;
            cur = cur->right;
        }
        else
        {
            cur = cur->left;
        }
    }
    if (best == nullptr)
    {
        logDone("No predecessor of " + TextUtil::intToText(key));
        return false;
    }
    result = best->key;
    logDone("Predecessor of " + TextUtil::intToText(key) + " is " + TextUtil::intToText(result));
    return true;
}

// ---- range query ----

int BST::rangeQuery(int lo, int hi, int out[], int maxCount)
{
    int found = 0;
    NodeStack stack;
    TreeNode* cur = root;

    // pruned iterative inorder: never enter a subtree that cannot
    // contain a key inside [lo, hi]
    while (cur != nullptr || !stack.isEmpty())
    {
        while (cur != nullptr)
        {
            logVisit(cur);
            comparisons++;
            stack.push(cur);
            if (cur->key > lo)
            {
                cur = cur->left;
            }
            else
            {
                // everything left of here is < lo, skip that whole subtree
                if (cur->left != nullptr)
                {
                    logHighlight(cur->left, Event::HL_SKIPPED,
                                 "Skip rooms below " + TextUtil::intToText(cur->key));
                }
                cur = nullptr;
            }
        }
        cur = stack.pop();
        if (cur == nullptr)
        {
            break;
        }
        if (cur->key >= lo && cur->key <= hi)
        {
            if (found < maxCount)
            {
                out[found] = cur->key;
                found++;
            }
            logHighlight(cur, Event::HL_RESULT, "Collect " + TextUtil::intToText(cur->key));
        }
        if (cur->key < hi)
        {
            cur = cur->right;
        }
        else
        {
            // everything right of here is > hi, skip that whole subtree
            if (cur->right != nullptr)
            {
                logHighlight(cur->right, Event::HL_SKIPPED,
                             "Skip rooms above " + TextUtil::intToText(cur->key));
            }
            cur = nullptr;
        }
    }
    logDone("Range [" + TextUtil::intToText(lo) + ", " + TextUtil::intToText(hi) + "] holds "
            + TextUtil::intToText(found) + " keys");
    return found;
}

// ---- order statistics (plain BST versions) ----

bool BST::kthSmallest(int k, int& result)
{
    if (k < 1)
    {
        return false;
    }
    // iterative inorder with a counter
    NodeStack stack;
    TreeNode* cur = root;
    int seen = 0;
    while (cur != nullptr || !stack.isEmpty())
    {
        while (cur != nullptr)
        {
            stack.push(cur);
            cur = cur->left;
        }
        cur = stack.pop();
        seen++;
        if (seen == k)
        {
            result = cur->key;
            logDone("The " + TextUtil::intToText(k) + "-th smallest is " + TextUtil::intToText(result));
            return true;
        }
        cur = cur->right;
    }
    logDone("There is no " + TextUtil::intToText(k) + "-th smallest key");
    return false;
}

bool BST::lcaBST(int a, int b, int& result)
{
    if (!contains(a) || !contains(b))
    {
        logDone("Both keys must exist to find their meeting room");
        return false;
    }
    TreeNode* cur = root;
    while (cur != nullptr)
    {
        logVisit(cur);
        if (a < cur->key && b < cur->key)
        {
            cur = cur->left;
        }
        else if (a > cur->key && b > cur->key)
        {
            cur = cur->right;
        }
        else
        {
            result = cur->key;
            logDone("Routes meet at " + TextUtil::intToText(result));
            return true;
        }
    }
    return false;
}

// ---- lookups and helpers ----

int BST::height()
{
    return root != nullptr ? root->height : 0;
}

int BST::size()
{
    return count;
}

bool BST::isEmpty()
{
    return count == 0;
}

TreeNode* BST::getRoot()
{
    return root;
}

TreeNode* BST::findById(int id)
{
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        if (node->id == id)
        {
            return node;
        }
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
    }
    return nullptr;
}

TreeNode* BST::findByKey(int key)
{
    TreeNode* cur = root;
    while (cur != nullptr)
    {
        if (key == cur->key)
        {
            return cur;
        }
        cur = key < cur->key ? cur->left : cur->right;
    }
    return nullptr;
}

int BST::inorderKeys(int out[], int maxCount)
{
    int n = 0;
    NodeStack stack;
    TreeNode* cur = root;
    while ((cur != nullptr || !stack.isEmpty()) && n < maxCount)
    {
        while (cur != nullptr)
        {
            stack.push(cur);
            cur = cur->left;
        }
        cur = stack.pop();
        out[n] = cur->key;
        n++;
        cur = cur->right;
    }
    return n;
}

int BST::searchPathLength(int key)
{
    int steps = 0;
    TreeNode* cur = root;
    while (cur != nullptr)
    {
        steps++;
        if (key == cur->key)
        {
            return steps;
        }
        cur = key < cur->key ? cur->left : cur->right;
    }
    return steps;   // key missing: rooms visited on the way to the empty spot
}

long long BST::getComparisons()
{
    return comparisons;
}

void BST::resetCounters()
{
    comparisons = 0;
}
