#include "model/AVLTree.h"
#include "core/NodeStack.h"
#include "core/TextUtil.h"

AVLTree::AVLTree()
{
    autoRebalance = true;
    rotations = 0;
    rotationsThisOp = 0;
}

void AVLTree::setAutoRebalance(bool on)
{
    autoRebalance = on;
}

bool AVLTree::getAutoRebalance()
{
    return autoRebalance;
}

int AVLTree::balanceFactor(TreeNode* node)
{
    if (node == nullptr)
    {
        return 0;
    }
    int leftHeight = node->left != nullptr ? node->left->height : 0;
    int rightHeight = node->right != nullptr ? node->right->height : 0;
    return leftHeight - rightHeight;
}

// ---- primitive rotations ----

void AVLTree::logBalanceAlert(TreeNode* node, int bf)
{
    if (log == nullptr)
    {
        return;
    }
    Event e;
    e.type = Event::BALANCE_ALERT;
    e.nodeId = node->id;
    e.balanceFactor = bf;
    e.text = "Room " + TextUtil::intToText(node->key) + " is unbalanced (BF "
           + TextUtil::intToText(bf) + ")";
    log->add(e);
}

// x's right child rises; x becomes its left child.
void AVLTree::rotateLeftNode(TreeNode* x, int rotCase)
{
    TreeNode* y = x->right;
    TreeNode* p = x->parent;

    x->right = y->left;             // y's left subtree moves under x
    if (y->left != nullptr)
    {
        y->left->parent = x;
    }
    y->left = x;
    x->parent = y;
    y->parent = p;
    if (p == nullptr)
    {
        root = y;
    }
    else if (p->left == x)
    {
        p->left = y;
    }
    else
    {
        p->right = y;
    }

    updateUpwards(x);               // x first, then y and the ancestors above
    rotations++;
    rotationsThisOp++;

    if (log != nullptr)
    {
        Event e;
        e.type = Event::ROTATE;
        e.nodeId = x->id;           // the pivot that went down
        e.otherId = y->id;          // the new subtree root
        e.rotateLeft = true;
        e.rotation = rotCase;
        e.text = "Rotate left at " + TextUtil::intToText(x->key);
        e.snapshotIndex = log->saveTreeShape(root);
        log->add(e);
    }
}

// Mirror image of rotateLeftNode.
void AVLTree::rotateRightNode(TreeNode* x, int rotCase)
{
    TreeNode* y = x->left;
    TreeNode* p = x->parent;

    x->left = y->right;
    if (y->right != nullptr)
    {
        y->right->parent = x;
    }
    y->right = x;
    x->parent = y;
    y->parent = p;
    if (p == nullptr)
    {
        root = y;
    }
    else if (p->left == x)
    {
        p->left = y;
    }
    else
    {
        p->right = y;
    }

    updateUpwards(x);
    rotations++;
    rotationsThisOp++;

    if (log != nullptr)
    {
        Event e;
        e.type = Event::ROTATE;
        e.nodeId = x->id;
        e.otherId = y->id;
        e.rotateLeft = false;
        e.rotation = rotCase;
        e.text = "Rotate right at " + TextUtil::intToText(x->key);
        e.snapshotIndex = log->saveTreeShape(root);
        log->add(e);
    }
}

// ---- case analysis and repair ----

int AVLTree::caseOf(TreeNode* node)
{
    int bf = balanceFactor(node);
    if (bf > 1)
    {
        // left-heavy; the left child decides single or double rotation
        if (balanceFactor(node->left) >= 0)
        {
            return Event::ROT_LL;
        }
        return Event::ROT_LR;
    }
    if (bf < -1)
    {
        if (balanceFactor(node->right) <= 0)
        {
            return Event::ROT_RR;
        }
        return Event::ROT_RL;
    }
    return Event::ROT_NONE;
}

int AVLTree::rebalanceNode(TreeNode* node)
{
    int rotCase = caseOf(node);
    if (rotCase == Event::ROT_NONE)
    {
        return Event::ROT_NONE;
    }
    logBalanceAlert(node, balanceFactor(node));
    if (rotCase == Event::ROT_LL)
    {
        rotateRightNode(node, rotCase);
    }
    else if (rotCase == Event::ROT_RR)
    {
        rotateLeftNode(node, rotCase);
    }
    else if (rotCase == Event::ROT_LR)
    {
        rotateLeftNode(node->left, rotCase);
        rotateRightNode(node, rotCase);
    }
    else   // ROT_RL
    {
        rotateRightNode(node->right, rotCase);
        rotateLeftNode(node, rotCase);
    }
    return rotCase;
}

void AVLTree::rebalanceUpFrom(TreeNode* node)
{
    TreeNode* cur = node;
    while (cur != nullptr)
    {
        TreeNode* next = cur->parent;   // taken before any rotation moves cur
        if (caseOf(cur) != Event::ROT_NONE)
        {
            rebalanceNode(cur);
        }
        cur = next;
    }
}

// ---- public operations ----

bool AVLTree::insert(int key)
{
    rotationsThisOp = 0;
    bool ok = insertCore(key);
    if (!ok)
    {
        logDone("Duplicate " + TextUtil::intToText(key) + " rejected");
        return false;
    }
    if (autoRebalance)
    {
        rebalanceUpFrom(lastChangedParent);
    }
    if (rotationsThisOp > 0)
    {
        logDone("Inserted " + TextUtil::intToText(key) + " with "
                + TextUtil::intToText(rotationsThisOp) + " rotation(s)");
    }
    else
    {
        logDone("Inserted " + TextUtil::intToText(key));
    }
    return true;
}

bool AVLTree::remove(int key)
{
    rotationsThisOp = 0;
    bool ok = removeCore(key);
    if (!ok)
    {
        logDone(TextUtil::intToText(key) + " is not in the vault");
        return false;
    }
    if (autoRebalance)
    {
        rebalanceUpFrom(lastChangedParent);
    }
    if (rotationsThisOp > 0)
    {
        logDone("Deleted " + TextUtil::intToText(key) + " with "
                + TextUtil::intToText(rotationsThisOp) + " rotation(s)");
    }
    else
    {
        logDone("Deleted " + TextUtil::intToText(key));
    }
    return true;
}

bool AVLTree::rotateLeftAt(int nodeId)
{
    TreeNode* node = findById(nodeId);
    if (node == nullptr || node->right == nullptr)
    {
        return false;   // a left rotation needs a right child to promote
    }
    rotationsThisOp = 0;
    rotateLeftNode(node, Event::ROT_NONE);
    logDone("Manual left rotation");
    return true;
}

bool AVLTree::rotateRightAt(int nodeId)
{
    TreeNode* node = findById(nodeId);
    if (node == nullptr || node->left == nullptr)
    {
        return false;
    }
    rotationsThisOp = 0;
    rotateRightNode(node, Event::ROT_NONE);
    logDone("Manual right rotation");
    return true;
}

int AVLTree::rebalanceAt(int nodeId)
{
    TreeNode* node = findById(nodeId);
    if (node == nullptr)
    {
        return Event::ROT_NONE;
    }
    rotationsThisOp = 0;
    int rotCase = rebalanceNode(node);
    if (rotCase != Event::ROT_NONE)
    {
        logDone("Repaired with case " + TextUtil::intToText(rotCase));
    }
    return rotCase;
}

int AVLTree::caseAt(int nodeId)
{
    TreeNode* node = findById(nodeId);
    if (node == nullptr)
    {
        return Event::ROT_NONE;
    }
    return caseOf(node);
}

int AVLTree::predictInsert(int key)
{
    if (contains(key))
    {
        return Event::ROT_NONE;   // a duplicate changes nothing
    }
    // simulate on a silent copy with rotations switched off, then read
    // which case the first unbalanced ancestor of the new node would need
    AVLTree temp;
    temp.copyFrom(*this);
    temp.setEventLog(nullptr);
    temp.setAutoRebalance(false);
    temp.insert(key);
    TreeNode* cur = temp.findByKey(key);
    while (cur != nullptr)
    {
        int rotCase = temp.caseOf(cur);
        if (rotCase != Event::ROT_NONE)
        {
            return rotCase;
        }
        cur = cur->parent;
    }
    return Event::ROT_NONE;
}

// ---- order statistics in O(log n) using subtree sizes ----

bool AVLTree::kth(int k, int& result)
{
    if (k < 1 || k > size())
    {
        logDone("There is no " + TextUtil::intToText(k) + "-th key");
        return false;
    }
    TreeNode* cur = root;
    int remaining = k;
    while (cur != nullptr)
    {
        logVisit(cur);
        int leftSize = cur->left != nullptr ? cur->left->size : 0;
        if (remaining == leftSize + 1)
        {
            result = cur->key;
            logDone("The " + TextUtil::intToText(k) + "-th smallest is " + TextUtil::intToText(result));
            return true;
        }
        if (remaining <= leftSize)
        {
            cur = cur->left;
        }
        else
        {
            remaining = remaining - leftSize - 1;
            cur = cur->right;
        }
    }
    return false;
}

int AVLTree::rank(int key)
{
    int result = 0;
    TreeNode* cur = root;
    while (cur != nullptr)
    {
        if (cur->key <= key)
        {
            int leftSize = cur->left != nullptr ? cur->left->size : 0;
            result = result + leftSize + 1;
            cur = cur->right;
        }
        else
        {
            cur = cur->left;
        }
    }
    return result;
}

int AVLTree::countInRange(int lo, int hi)
{
    return rank(hi) - rank(lo - 1);
}

// ---- balance inspection ----

bool AVLTree::isBalanced()
{
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        int bf = balanceFactor(node);
        if (bf > 1 || bf < -1)
        {
            return false;
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
    return true;
}

int AVLTree::unbalancedIds(int out[], int maxCount)
{
    int n = 0;
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty() && n < maxCount)
    {
        TreeNode* node = stack.pop();
        int bf = balanceFactor(node);
        if (bf > 1 || bf < -1)
        {
            out[n] = node->id;
            n++;
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
    return n;
}

int AVLTree::deepestUnbalancedId()
{
    // repairing the lowest violation first is how AVL insertion fixes
    // the tree with one rotation, so levels flag this node
    int bestId = -1;
    int bestDepth = -1;
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        int bf = balanceFactor(node);
        if (bf > 1 || bf < -1)
        {
            int depth = 0;
            TreeNode* up = node;
            while (up->parent != nullptr)
            {
                depth++;
                up = up->parent;
            }
            if (depth > bestDepth)
            {
                bestDepth = depth;
                bestId = node->id;
            }
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
    return bestId;
}

long long AVLTree::getRotations()
{
    return rotations;
}

int AVLTree::getMaxRotationsLastOp()
{
    return rotationsThisOp;
}
