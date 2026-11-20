#pragma once

#include <string>
#include "core/Limits.h"

using namespace std;

struct TreeNode;
class BinaryTree;
class MaxHeap;

// Campaign progress, the local leaderboard and the settings, written to one
// small text file with our own reader - the course forbids a JSON library,
// and the format only has to hold what section 8 of the spec lists.
//
// It also holds the two serialisation formats the spec asks for:
//   trees as a preorder walk with '#' for the missing children, e.g.
//     50 30 # # 70 60 # # #
//   heaps as the underlying array plus the current active heap size.
class SaveManager
{
public:
    static const int MAX_LEVELS = 32;
    static const int MAX_SCORES = 10;
    static const int MAX_NAME = 16;

    SaveManager();

    void clear();
    bool load(const string& path);
    bool save(const string& path);

    // ---- campaign progress ----
    // Keeps the BEST result for a level: replaying it can never cost stars.
    void recordLevel(const string& levelId, int stars, int score);
    int starsFor(const string& levelId);
    int bestScoreFor(const string& levelId);
    int totalStars();
    bool isCleared(const string& levelId);

    // ---- endless mode leaderboard ----
    void addScore(const string& name, int score);
    int scoreCount();
    string scoreName(int index);
    int scoreValue(int index);

    // ---- settings ----
    float animationSpeed;      // 0.25 .. 4
    bool validateAfterEachStep;
    bool showTooltips;
    bool unlockAllLevels;      // opens every level for a demo or the viva
    int endlessSeed;

    // ---- serialisation (spec section 8) ----
    static string serialiseTree(TreeNode* root);
    // Rebuilds through BinaryTree::buildPreIn, so the tree is reconstructed
    // by the data structure itself. Act 1 values are distinct, which is what
    // makes that pair of traversals describe exactly one tree.
    static bool deserialiseTree(const string& text, BinaryTree& tree);
    static string serialiseHeap(MaxHeap& heap);
    static bool deserialiseHeap(const string& text, MaxHeap& heap);

private:
    string levelIds[MAX_LEVELS];
    int levelStars[MAX_LEVELS];
    int levelScores[MAX_LEVELS];
    int levelCount;

    string names[MAX_SCORES];
    int values[MAX_SCORES];
    int entryCount;

    int indexOf(const string& levelId);
};
