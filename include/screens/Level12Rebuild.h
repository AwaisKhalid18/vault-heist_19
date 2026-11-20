#pragma once

#include "screens/TreeLevelBase.h"

// Level 1.2 - Decode the blueprint.
// The vault's shape is known but its rooms are unlabelled. The player is given
// the hidden vault's preorder and inorder lists and has to put each value in
// the right room: click a room, then click a value token.
//
// The win check is BinaryTree::isSameTree against the tree the model rebuilt
// from the same two traversals, so the level is graded by the data structure,
// not by a comparison written into the screen.
class Level12Rebuild : public TreeLevelBase
{
public:
    Level12Rebuild(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onOperation(int buttonIndex);
    void onLevelEvent(const sf::Event& e);
    void drawCanvasOverlay(sf::RenderWindow& window);
    void drawUiOverlay(sf::RenderWindow& window);
    string tooltipFor(int nodeId);

private:
    static const int OP_CHECK = 0, OP_CLEAR = 1;
    static const int BLANK = -1;        // a room with no value placed in it yet

    BinaryTree hidden;
    int preList[Limits::MAX_LEVEL_KEYS];
    int inList[Limits::MAX_LEVEL_KEYS];
    int roomCount;
    bool tokenUsed[Limits::MAX_LEVEL_KEYS];
    int placedCount;
    bool usePostorder;                  // show postorder + inorder instead

    sf::Text lineText;
    sf::RectangleShape tokenBox;
    bool fontReady;

    float tokenRowY();
    float tokenWidth();
    float tokenRowLeft();
    int tokenAt(float pixelX, float pixelY);
    void placeValue(int value);
    void clearRoom(int nodeId);
    void checkAnswer();
    string listToText(const int values[], int n);
};
