#include "model/BinaryTree.h"
#include "core/NodeStack.h"
#include "core/NodeQueue.h"
#include "core/TextUtil.h"

// The shared header fixes this class's member list, so the recursive helpers
// live here as plain file-scope functions that take the node they work on.
// They are functions, not variables, so no global state is created.

static TreeNode* findNodeById(TreeNode* root, int id)
{
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty())
    {
        TreeNode* current = stack.pop();
        if (current->id == id)
        {
            return current;
        }
        if (current->right != nullptr)
        {
            stack.push(current->right);
        }
        if (current->left != nullptr)
        {
            stack.push(current->left);
        }
    }
    return nullptr;
}

static void deleteSubtree(TreeNode* root)
{
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty())
    {
        TreeNode* current = stack.pop();
        if (current->left != nullptr)
        {
            stack.push(current->left);
        }
        if (current->right != nullptr)
        {
            stack.push(current->right);
        }
        delete current;
    }
}

static void preorderWalk(TreeNode* node, int outIds[], int maxCount, int& count)
{
    if (node == nullptr || count >= maxCount)
    {
        return;
    }
    outIds[count] = node->id;
    count++;
    preorderWalk(node->left, outIds, maxCount, count);
    preorderWalk(node->right, outIds, maxCount, count);
}

static void inorderWalk(TreeNode* node, int outIds[], int maxCount, int& count)
{
    if (node == nullptr)
    {
        return;
    }
    inorderWalk(node->left, outIds, maxCount, count);
    if (count < maxCount)
    {
        outIds[count] = node->id;
        count++;
    }
    inorderWalk(node->right, outIds, maxCount, count);
}

static void postorderWalk(TreeNode* node, int outIds[], int maxCount, int& count)
{
    if (node == nullptr)
    {
        return;
    }
    postorderWalk(node->left, outIds, maxCount, count);
    postorderWalk(node->right, outIds, maxCount, count);
    if (count < maxCount)
    {
        outIds[count] = node->id;
        count++;
    }
}

// Sets parent links and recomputes height (leaf = 1) and subtree size from
// the bottom up. Called after anything that changes the shape of the tree.
static void refreshNode(TreeNode* node, TreeNode* parent)
{
    if (node == nullptr)
    {
        return;
    }
    node->parent = parent;
    refreshNode(node->left, node);
    refreshNode(node->right, node);
    int leftHeight = node->left != nullptr ? node->left->height : 0;
    int rightHeight = node->right != nullptr ? node->right->height : 0;
    node->height = (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
    int leftSize = node->left != nullptr ? node->left->size : 0;
    int rightSize = node->right != nullptr ? node->right->size : 0;
    node->size = leftSize + rightSize + 1;
}

// Returns this subtree's height and keeps the longest path found so far.
static int diameterWalk(TreeNode* node, int& best)
{
    if (node == nullptr)
    {
        return 0;
    }
    int left = diameterWalk(node->left, best);
    int right = diameterWalk(node->right, best);
    int through = left + right + 1;      // rooms on the path that bends here
    if (through > best)
    {
        best = through;
    }
    return (left > right ? left : right) + 1;
}

static void mirrorWalk(TreeNode* node)
{
    if (node == nullptr)
    {
        return;
    }
    TreeNode* hold = node->left;
    node->left = node->right;
    node->right = hold;
    mirrorWalk(node->left);
    mirrorWalk(node->right);
}

static bool sameWalk(TreeNode* a, TreeNode* b)
{
    if (a == nullptr && b == nullptr)
    {
        return true;
    }
    if (a == nullptr || b == nullptr)
    {
        return false;
    }
    if (a->key != b->key)
    {
        return false;
    }
    return sameWalk(a->left, b->left) && sameWalk(a->right, b->right);
}

// Like sameWalk, but each side is compared against the other tree's opposite
// side, which is exactly what "is a mirror image of" means.
static bool mirrorCompareWalk(TreeNode* a, TreeNode* b)
{
    if (a == nullptr && b == nullptr)
    {
        return true;
    }
    if (a == nullptr || b == nullptr)
    {
        return false;
    }
    if (a->key != b->key)
    {
        return false;
    }
    return mirrorCompareWalk(a->left, b->right) && mirrorCompareWalk(a->right, b->left);
}

static int indexOfValue(const int values[], int lo, int hi, int target)
{
    for (int i = lo; i <= hi; i++)
    {
        if (values[i] == target)
        {
            return i;
        }
    }
    return -1;
}

// Preorder names the root first, so the next unused preorder entry is the
// root of this range; inorder then says how many rooms lie on each side.
static TreeNode* buildPreInWalk(const int pre[], const int in[], int& preIndex,
                                int inLo, int inHi, int& nextId)
{
    if (inLo > inHi)
    {
        return nullptr;
    }
    int rootValue = pre[preIndex];
    preIndex++;
    TreeNode* node = new TreeNode(nextId, rootValue);
    nextId++;
    int split = indexOfValue(in, inLo, inHi, rootValue);
    if (split == -1)
    {
        return node;      // inconsistent input: stop rather than recurse forever
    }
    node->left = buildPreInWalk(pre, in, preIndex, inLo, split - 1, nextId);
    node->right = buildPreInWalk(pre, in, preIndex, split + 1, inHi, nextId);
    return node;
}

// Postorder read backwards gives the root, then the RIGHT subtree, then the
// left one, so here the right child must be built before the left child.
static TreeNode* buildPostInWalk(const int post[], const int in[], int& postIndex,
                                 int inLo, int inHi, int& nextId)
{
    if (inLo > inHi)
    {
        return nullptr;
    }
    int rootValue = post[postIndex];
    postIndex--;
    TreeNode* node = new TreeNode(nextId, rootValue);
    nextId++;
    int split = indexOfValue(in, inLo, inHi, rootValue);
    if (split == -1)
    {
        return node;
    }
    node->right = buildPostInWalk(post, in, postIndex, split + 1, inHi, nextId);
    node->left = buildPostInWalk(post, in, postIndex, inLo, split - 1, nextId);
    return node;
}

// Every traversal produces the same kind of event stream: one VISIT per room
// in the order it was reached, then the DONE event the animator waits for.
static void logTraversal(EventLog* log, TreeNode* root, const int ids[], int count,
                         const string& name)
{
    if (log == nullptr)
    {
        return;
    }
    for (int i = 0; i < count; i++)
    {
        TreeNode* node = findNodeById(root, ids[i]);
        Event e;
        e.type = Event::VISIT;
        e.nodeId = ids[i];
        e.key = node != nullptr ? node->key : 0;
        e.text = "Visit room " + TextUtil::intToText(e.key);
        log->add(e);
    }
    Event done;
    done.type = Event::DONE;
    done.text = name + ": " + TextUtil::intToText(count) + " rooms";
    log->add(done);
}

// ---- BinaryTree ----

BinaryTree::BinaryTree()
{
    root = nullptr;
    nextId = 1;
    log = nullptr;
}

BinaryTree::~BinaryTree()
{
    deleteSubtree(root);
    root = nullptr;
}

void BinaryTree::setEventLog(EventLog* eventLog)
{
    log = eventLog;
}

TreeNode* BinaryTree::getRoot()
{
    return root;
}

int BinaryTree::preorderRec(int outIds[], int maxCount)
{
    int count = 0;
    preorderWalk(root, outIds, maxCount, count);
    logTraversal(log, root, outIds, count, "Preorder");
    return count;
}

int BinaryTree::preorderIter(int outIds[], int maxCount)
{
    int count = 0;
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty() && count < maxCount)
    {
        TreeNode* node = stack.pop();
        outIds[count] = node->id;
        count++;
        // push the right child first so the left child is popped next
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
    }
    logTraversal(log, root, outIds, count, "Preorder");
    return count;
}

int BinaryTree::inorderRec(int outIds[], int maxCount)
{
    int count = 0;
    inorderWalk(root, outIds, maxCount, count);
    logTraversal(log, root, outIds, count, "Inorder");
    return count;
}

int BinaryTree::inorderIter(int outIds[], int maxCount)
{
    int count = 0;
    NodeStack stack;
    TreeNode* current = root;
    while ((current != nullptr || !stack.isEmpty()) && count < maxCount)
    {
        // run as far left as possible, remembering the way back
        while (current != nullptr)
        {
            stack.push(current);
            current = current->left;
        }
        TreeNode* node = stack.pop();
        outIds[count] = node->id;
        count++;
        current = node->right;
    }
    logTraversal(log, root, outIds, count, "Inorder");
    return count;
}

int BinaryTree::postorderRec(int outIds[], int maxCount)
{
    int count = 0;
    postorderWalk(root, outIds, maxCount, count);
    logTraversal(log, root, outIds, count, "Postorder");
    return count;
}

int BinaryTree::postorderIter(int outIds[], int maxCount)
{
    // Visiting root, right, left is preorder with the two children swapped,
    // and reversing that order gives postorder with only one stack.
    int count = 0;
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty() && count < maxCount)
    {
        TreeNode* node = stack.pop();
        outIds[count] = node->id;
        count++;
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
    }
    for (int i = 0; i < count / 2; i++)
    {
        int hold = outIds[i];
        outIds[i] = outIds[count - 1 - i];
        outIds[count - 1 - i] = hold;
    }
    logTraversal(log, root, outIds, count, "Postorder");
    return count;
}

int BinaryTree::levelOrder(int outIds[], int maxCount)
{
    int count = 0;
    NodeQueue queue;
    if (root != nullptr)
    {
        queue.enqueue(root);
    }
    while (!queue.isEmpty() && count < maxCount)
    {
        TreeNode* node = queue.dequeue();
        outIds[count] = node->id;
        count++;
        if (node->left != nullptr)
        {
            queue.enqueue(node->left);
        }
        if (node->right != nullptr)
        {
            queue.enqueue(node->right);
        }
    }
    logTraversal(log, root, outIds, count, "Level order");
    return count;
}

void BinaryTree::buildPreIn(const int pre[], const int in[], int n)
{
    deleteSubtree(root);
    root = nullptr;
    nextId = 1;
    if (n <= 0)
    {
        return;
    }
    int preIndex = 0;
    root = buildPreInWalk(pre, in, preIndex, 0, n - 1, nextId);
    refreshNode(root, nullptr);
}

void BinaryTree::buildPostIn(const int post[], const int in[], int n)
{
    deleteSubtree(root);
    root = nullptr;
    nextId = 1;
    if (n <= 0)
    {
        return;
    }
    int postIndex = n - 1;
    root = buildPostInWalk(post, in, postIndex, 0, n - 1, nextId);
    refreshNode(root, nullptr);
}

int BinaryTree::height()
{
    return root != nullptr ? root->height : 0;
}

int BinaryTree::diameter()
{
    int best = 0;
    diameterWalk(root, best);
    return best;
}

bool BinaryTree::lca(int idA, int idB, int& result)
{
    int pathA[Limits::MAX_NODES_GUI];
    int pathB[Limits::MAX_NODES_GUI];
    int lengthA = pathTo(idA, pathA, Limits::MAX_NODES_GUI);
    int lengthB = pathTo(idB, pathB, Limits::MAX_NODES_GUI);
    if (lengthA == 0 || lengthB == 0)
    {
        return false;
    }
    // both paths start at the root, so the last room they share is the LCA
    int shared = 0;
    while (shared < lengthA && shared < lengthB && pathA[shared] == pathB[shared])
    {
        shared++;
    }
    if (shared == 0)
    {
        return false;
    }
    result = pathA[shared - 1];
    return true;
}

int BinaryTree::pathTo(int id, int out[], int maxCount)
{
    TreeNode* node = findNodeById(root, id);
    if (node == nullptr)
    {
        return 0;
    }
    // climb to the root collecting ids, then flip them into root-first order
    int count = 0;
    TreeNode* current = node;
    while (current != nullptr && count < maxCount)
    {
        out[count] = current->id;
        count++;
        current = current->parent;
    }
    for (int i = 0; i < count / 2; i++)
    {
        int hold = out[i];
        out[i] = out[count - 1 - i];
        out[count - 1 - i] = hold;
    }
    return count;
}

void BinaryTree::mirror()
{
    mirrorWalk(root);
    refreshNode(root, nullptr);
    if (log != nullptr)
    {
        // one shape change: the animator slides every room by id from the
        // snapshot it already has to the mirrored snapshot saved here
        Event e;
        e.type = Event::ROTATE;
        e.nodeId = root != nullptr ? root->id : -1;
        e.otherId = e.nodeId;
        e.text = "Mirror the whole vault";
        e.snapshotIndex = log->saveTreeShape(root);
        log->add(e);
        Event done;
        done.type = Event::DONE;
        done.text = "Vault mirrored";
        log->add(done);
    }
}

bool BinaryTree::isSameTree(BinaryTree& other)
{
    return sameWalk(root, other.getRoot());
}

bool BinaryTree::isSymmetric()
{
    if (root == nullptr)
    {
        return true;
    }
    return mirrorCompareWalk(root->left, root->right);
}
