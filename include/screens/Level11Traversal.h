#pragma once

#include "screens/TreeLevelBase.h"

// Level 1.1 - Guards patrol in a given order. Predict their route.
// The level names one of the four traversals and the player clicks the rooms
// in the order that traversal visits them. A wrong click costs 1 alarm, as
// the spec's penalty table requires. "Show me" replays the real traversal
// through the animator, which is the same event stream the model produced.
//
// The level JSON's `k` picks the traversal: 0 preorder, 1 inorder,
// 2 postorder, 3 level order.
class Level11Traversal : public TreeLevelBase
{
public:
    Level11Traversal(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onOperation(int buttonIndex);
    void drawUiOverlay(sf::RenderWindow& window);
    string tooltipFor(int nodeId);

private:
    static const int ORDER_PRE = 0, ORDER_IN = 1, ORDER_POST = 2, ORDER_LEVEL = 3;
    static const int OP_SHOW = 0, OP_RESTART_ROUTE = 1;

    int expected[Limits::MAX_LEVEL_KEYS];
    int expectedCount;
    int matched;
    int order;
    sf::Text statusText;
    bool fontReady;

    string orderName();
    void fillExpected();
};
