#include "game/SaveManager.h"
#include "model/TreeNode.h"
#include "model/BinaryTree.h"
#include "model/MaxHeap.h"
#include "core/TextUtil.h"
#include "core/NodeStack.h"
#include <fstream>
#include <sstream>

SaveManager::SaveManager()
{
    animationSpeed = 1.f;
    validateAfterEachStep = false;
    showTooltips = true;
    unlockAllLevels = false;
    endlessSeed = 2026;
    levelCount = 0;
    entryCount = 0;
    clear();
}

void SaveManager::clear()
{
    levelCount = 0;
    entryCount = 0;
    for (int i = 0; i < MAX_LEVELS; i++)
    {
        levelIds[i] = "";
        levelStars[i] = 0;
        levelScores[i] = 0;
    }
    for (int i = 0; i < MAX_SCORES; i++)
    {
        names[i] = "";
        values[i] = 0;
    }
}

int SaveManager::indexOf(const string& levelId)
{
    for (int i = 0; i < levelCount; i++)
    {
        if (levelIds[i] == levelId)
        {
            return i;
        }
    }
    return -1;
}

void SaveManager::recordLevel(const string& levelId, int stars, int score)
{
    int index = indexOf(levelId);
    if (index == -1)
    {
        if (levelCount >= MAX_LEVELS)
        {
            return;
        }
        index = levelCount;
        levelIds[index] = levelId;
        levelStars[index] = 0;
        levelScores[index] = 0;
        levelCount++;
    }
    // replaying a level can only ever improve the record
    if (stars > levelStars[index])
    {
        levelStars[index] = stars;
    }
    if (score > levelScores[index])
    {
        levelScores[index] = score;
    }
}

int SaveManager::starsFor(const string& levelId)
{
    int index = indexOf(levelId);
    return index == -1 ? 0 : levelStars[index];
}

int SaveManager::bestScoreFor(const string& levelId)
{
    int index = indexOf(levelId);
    return index == -1 ? 0 : levelScores[index];
}

int SaveManager::totalStars()
{
    int total = 0;
    for (int i = 0; i < levelCount; i++)
    {
        total += levelStars[i];
    }
    return total;
}

bool SaveManager::isCleared(const string& levelId)
{
    return starsFor(levelId) > 0;
}

// Keeps the table sorted highest first with an insertion step, written by
// hand because <algorithm> is off-limits.
void SaveManager::addScore(const string& name, int score)
{
    string trimmed = name;
    if ((int)trimmed.length() > MAX_NAME)
    {
        trimmed = trimmed.substr(0, MAX_NAME);
    }
    if (trimmed.length() == 0)
    {
        trimmed = "Thief";
    }

    int slot = entryCount;
    if (entryCount >= MAX_SCORES)
    {
        if (score <= values[MAX_SCORES - 1])
        {
            return;      // not good enough to make the table
        }
        slot = MAX_SCORES - 1;
    }
    else
    {
        entryCount++;
    }

    names[slot] = trimmed;
    values[slot] = score;
    for (int i = slot; i > 0; i--)
    {
        if (values[i] > values[i - 1])
        {
            int holdValue = values[i];
            values[i] = values[i - 1];
            values[i - 1] = holdValue;
            string holdName = names[i];
            names[i] = names[i - 1];
            names[i - 1] = holdName;
        }
        else
        {
            break;
        }
    }
}

int SaveManager::scoreCount()
{
    return entryCount;
}

string SaveManager::scoreName(int index)
{
    if (index < 0 || index >= entryCount)
    {
        return "";
    }
    return names[index];
}

int SaveManager::scoreValue(int index)
{
    if (index < 0 || index >= entryCount)
    {
        return 0;
    }
    return values[index];
}

bool SaveManager::save(const string& path)
{
    ofstream file(path.c_str());
    if (!file.is_open())
    {
        return false;
    }
    file << "VAULTHEIST 1\n";
    file << "SETTINGS " << (int)(animationSpeed * 100.f) << " "
         << (validateAfterEachStep ? 1 : 0) << " "
         << (showTooltips ? 1 : 0) << " " << endlessSeed << " "
         << (unlockAllLevels ? 1 : 0) << "\n";
    for (int i = 0; i < levelCount; i++)
    {
        file << "LEVEL " << levelIds[i] << " " << levelStars[i] << " " << levelScores[i] << "\n";
    }
    for (int i = 0; i < entryCount; i++)
    {
        file << "SCORE " << names[i] << " " << values[i] << "\n";
    }
    file.close();
    return true;
}

bool SaveManager::load(const string& path)
{
    ifstream file(path.c_str());
    if (!file.is_open())
    {
        return false;
    }
    clear();

    string line;
    bool sawHeader = false;
    while (getline(file, line))
    {
        istringstream parts(line);
        string tag;
        parts >> tag;
        if (tag == "VAULTHEIST")
        {
            sawHeader = true;
        }
        else if (tag == "SETTINGS")
        {
            int speedHundredths = 100;
            int validate = 0;
            int tooltips = 1;
            int seed = 2026;
            int unlockAll = 0;
            parts >> speedHundredths >> validate >> tooltips >> seed >> unlockAll;
            unlockAllLevels = unlockAll != 0;
            animationSpeed = (float)speedHundredths / 100.f;
            if (animationSpeed < 0.25f) { animationSpeed = 0.25f; }
            if (animationSpeed > 4.f) { animationSpeed = 4.f; }
            validateAfterEachStep = validate != 0;
            showTooltips = tooltips != 0;
            endlessSeed = seed;
        }
        else if (tag == "LEVEL")
        {
            string id;
            int stars = 0;
            int score = 0;
            parts >> id >> stars >> score;
            recordLevel(id, stars, score);
        }
        else if (tag == "SCORE")
        {
            string name;
            int score = 0;
            parts >> name >> score;
            addScore(name, score);
        }
    }
    file.close();
    return sawHeader;
}

// ---- serialisation ----

string SaveManager::serialiseTree(TreeNode* root)
{
    // preorder with '#' where a child is missing, done with our own stack so
    // a deep tree cannot overflow the call stack
    string out = "";
    NodeStack stack;
    stack.push(root);
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        if (out.length() > 0)
        {
            out = out + " ";
        }
        if (node == nullptr)
        {
            out = out + "#";
            continue;
        }
        out = out + TextUtil::intToText(node->key);
        stack.push(node->right);
        stack.push(node->left);
    }
    return out;
}

// Reads one subtree from the token stream, collecting the preorder and the
// inorder value lists as it goes: preorder is written on the way in, inorder
// between the two children. BinaryTree then rebuilds the tree from that pair,
// so the reconstruction is done by the data structure itself.
static bool parseSubtree(const int tokens[], const bool isNull[], int tokenCount, int& cursor,
                         int preOut[], int& preWritten, int inOut[], int& inWritten,
                         int maxCount)
{
    if (cursor >= tokenCount)
    {
        return false;
    }
    if (isNull[cursor])
    {
        cursor++;
        return true;             // a missing child contributes nothing
    }
    if (preWritten >= maxCount)
    {
        return false;
    }
    int value = tokens[cursor];
    cursor++;
    preOut[preWritten] = value;
    preWritten++;

    if (!parseSubtree(tokens, isNull, tokenCount, cursor, preOut, preWritten,
                      inOut, inWritten, maxCount))
    {
        return false;
    }
    inOut[inWritten] = value;    // between the two children: that is inorder
    inWritten++;
    return parseSubtree(tokens, isNull, tokenCount, cursor, preOut, preWritten,
                        inOut, inWritten, maxCount);
}

bool SaveManager::deserialiseTree(const string& text, BinaryTree& tree)
{
    int tokens[Limits::MAX_NODES_GUI * 2 + 2];
    bool isNull[Limits::MAX_NODES_GUI * 2 + 2];
    int tokenCount = 0;

    istringstream parts(text);
    string token;
    while (parts >> token)
    {
        if (tokenCount >= Limits::MAX_NODES_GUI * 2 + 2)
        {
            return false;
        }
        if (token == "#")
        {
            tokens[tokenCount] = 0;
            isNull[tokenCount] = true;
        }
        else
        {
            bool ok = false;
            tokens[tokenCount] = TextUtil::textToInt(token, ok);
            if (!ok)
            {
                return false;
            }
            isNull[tokenCount] = false;
        }
        tokenCount++;
    }

    int pre[Limits::MAX_NODES_GUI];
    int in[Limits::MAX_NODES_GUI];
    int preCount = 0;
    int inCount = 0;
    int cursor = 0;
    if (!parseSubtree(tokens, isNull, tokenCount, cursor, pre, preCount,
                      in, inCount, Limits::MAX_NODES_GUI))
    {
        return false;
    }
    if (preCount != inCount || cursor != tokenCount)
    {
        return false;        // trailing rubbish, or the stream ran short
    }
    tree.buildPreIn(pre, in, preCount);
    return true;
}

string SaveManager::serialiseHeap(MaxHeap& heap)
{
    // the array, then the active heap size, which is all a heap needs
    string out = "";
    for (int i = 0; i < heap.totalCount(); i++)
    {
        if (i > 0)
        {
            out = out + " ";
        }
        out = out + TextUtil::intToText(heap.get(i));
    }
    out = out + " | " + TextUtil::intToText(heap.getHeapSize());
    return out;
}

bool SaveManager::deserialiseHeap(const string& text, MaxHeap& heap)
{
    int values[Limits::MAX_NODES_GUI];
    int count = 0;
    int activeSize = -1;
    bool pastBar = false;

    istringstream parts(text);
    string token;
    while (parts >> token)
    {
        if (token == "|")
        {
            pastBar = true;
            continue;
        }
        bool ok = false;
        int value = TextUtil::textToInt(token, ok);
        if (!ok)
        {
            return false;
        }
        if (pastBar)
        {
            activeSize = value;
        }
        else
        {
            if (count >= Limits::MAX_NODES_GUI)
            {
                return false;
            }
            values[count] = value;
            count++;
        }
    }
    if (activeSize < 0)
    {
        activeSize = count;
    }
    heap.load(values, count, activeSize);
    return true;
}
