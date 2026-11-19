#include "screens/Level12Rebuild.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelBuilderB.h"
#include "core/TextUtil.h"
#include "core/NodeStack.h"

// Reads a subtree's values in postorder. The blueprint needs the VALUES, and
// BinaryTree's traversals return node ids, so this walks the nodes directly.
static void collectPostorderKeys(TreeNode* node, int out[], int maxCount, int& written)
{
    if (node == nullptr)
    {
        return;
    }
    collectPostorderKeys(node->left, out, maxCount, written);
    collectPostorderKeys(node->right, out, maxCount, written);
    if (written < maxCount)
    {
        out[written] = node->key;
        written++;
    }
}

Level12Rebuild::Level12Rebuild(App& a, const LevelDef& d) : TreeLevelBase(a, d)
{
    roomCount = 0;
    placedCount = 0;
    usePostorder = false;
    fontReady = false;
    for (int i = 0; i < Limits::MAX_LEVEL_KEYS; i++)
    {
        preList[i] = 0;
        inList[i] = 0;
        tokenUsed[i] = false;
    }
}

void Level12Rebuild::onLevelStart()
{
    roomCount = LevelBuilderB::buildTree(def, hidden, preList, inList);

    // `k` picks which pair of traversals the blueprint shows:
    // 0 means preorder + inorder, 1 means postorder + inorder
    usePostorder = def.k == 1;
    if (usePostorder)
    {
        // swap the preorder list for the hidden vault's postorder values
        int written = 0;
        collectPostorderKeys(hidden.getRoot(), preList, roomCount, written);
    }

    // the player's tree has the right SHAPE but no values yet
    if (usePostorder)
    {
        tree.buildPostIn(preList, inList, roomCount);
    }
    else
    {
        tree.buildPreIn(preList, inList, roomCount);
    }
    NodeStack stack;
    if (tree.getRoot() != nullptr)
    {
        stack.push(tree.getRoot());
    }
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        node->key = BLANK;
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
    }

    for (int i = 0; i < Limits::MAX_LEVEL_KEYS; i++)
    {
        tokenUsed[i] = false;
    }
    placedCount = 0;
    optimalSteps = roomCount;
    selectedId = -1;
    showTooltips = false;         // the tooltip would give the shape away

    opPanel.clearButtons();
    opPanel.addButton("Check the blueprint");
    opPanel.addButton("Clear every room");
    opPanel.setFieldCount(0);
    opPanel.relayout();

    setObjective("Place every value into the right room, using the two "
                 "traversal lists on the blueprint.");
    showTreeStatic(true);
}

float Level12Rebuild::tokenRowY()
{
    return (float)app.window.getSize().y - 96.f - 54.f;
}

float Level12Rebuild::tokenWidth()
{
    if (roomCount <= 0)
    {
        return 48.f;
    }
    float available = rightX - 32.f;
    float width = available / (float)roomCount;
    if (width > 52.f)
    {
        width = 52.f;
    }
    if (width < 26.f)
    {
        width = 26.f;
    }
    return width;
}

float Level12Rebuild::tokenRowLeft()
{
    return 16.f + (rightX - 32.f - tokenWidth() * (float)roomCount) / 2.f;
}

int Level12Rebuild::tokenAt(float pixelX, float pixelY)
{
    float y = tokenRowY();
    if (pixelY < y || pixelY > y + 36.f)
    {
        return -1;
    }
    float width = tokenWidth();
    float left = tokenRowLeft();
    for (int i = 0; i < roomCount; i++)
    {
        float x = left + width * (float)i;
        if (pixelX >= x && pixelX <= x + width - 4.f)
        {
            return i;
        }
    }
    return -1;
}

void Level12Rebuild::onNodeClicked(int nodeId)
{
    TreeNode* node = nodeById(nodeId);
    if (node == nullptr)
    {
        return;
    }
    if (node->key != BLANK)
    {
        clearRoom(nodeId);
        return;
    }
    selectedId = nodeId;
}

void Level12Rebuild::clearRoom(int nodeId)
{
    TreeNode* node = nodeById(nodeId);
    if (node == nullptr || node->key == BLANK)
    {
        return;
    }
    // give the token back to the row
    for (int i = 0; i < roomCount; i++)
    {
        if (tokenUsed[i] && inList[i] == node->key)
        {
            tokenUsed[i] = false;
            break;
        }
    }
    node->key = BLANK;
    placedCount--;
    selectedId = nodeId;
    showTreeStatic(false);
}

void Level12Rebuild::placeValue(int tokenIndex)
{
    if (tokenIndex < 0 || tokenIndex >= roomCount || tokenUsed[tokenIndex])
    {
        return;
    }
    if (selectedId == -1)
    {
        flash("Pick a room first, then choose the value that belongs in it.");
        return;
    }
    TreeNode* node = nodeById(selectedId);
    if (node == nullptr || node->key != BLANK)
    {
        return;
    }
    node->key = inList[tokenIndex];
    tokenUsed[tokenIndex] = true;
    placedCount++;
    selectedId = -1;
    addAlarm(1);
    showTreeStatic(false);
    if (placedCount >= roomCount)
    {
        checkAnswer();
    }
}

void Level12Rebuild::checkAnswer()
{
    if (placedCount < roomCount)
    {
        flash("Every room needs a value before the blueprint can be checked.");
        return;
    }
    if (tree.isSameTree(hidden))
    {
        win("The blueprint matches the vault exactly.");
        return;
    }
    penalty(1, 0, "That is not the vault these two traversals describe.");
}

void Level12Rebuild::onOperation(int buttonIndex)
{
    if (buttonIndex == OP_CHECK)
    {
        checkAnswer();
        return;
    }
    if (buttonIndex == OP_CLEAR)
    {
        NodeStack stack;
        if (tree.getRoot() != nullptr)
        {
            stack.push(tree.getRoot());
        }
        while (!stack.isEmpty())
        {
            TreeNode* node = stack.pop();
            node->key = BLANK;
            if (node->right != nullptr)
            {
                stack.push(node->right);
            }
            if (node->left != nullptr)
            {
                stack.push(node->left);
            }
        }
        for (int i = 0; i < roomCount; i++)
        {
            tokenUsed[i] = false;
        }
        placedCount = 0;
        selectedId = -1;
        showTreeStatic(false);
    }
}

void Level12Rebuild::onLevelEvent(const sf::Event& e)
{
    if (e.type != sf::Event::MouseButtonPressed || e.mouseButton.button != sf::Mouse::Left)
    {
        return;
    }
    int token = tokenAt((float)e.mouseButton.x, (float)e.mouseButton.y);
    if (token != -1)
    {
        placeValue(token);
    }
}

string Level12Rebuild::tooltipFor(int nodeId)
{
    return "";
}

// An unfilled room still carries the BLANK sentinel as its key, and the shared
// renderer prints whatever key it is given. So the sentinel is painted over
// here with a question mark, which is what an empty slot should look like.
void Level12Rebuild::drawCanvasOverlay(sf::RenderWindow& window)
{
    if (!fontReady)
    {
        lineText.setFont(app.monoFont);
        lineText.setCharacterSize(Theme::monoSize());
        fontReady = true;
    }

    NodeStack stack;
    if (tree.getRoot() != nullptr)
    {
        stack.push(tree.getRoot());
    }
    sf::CircleShape cover;
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
        if (node->key != BLANK)
        {
            continue;
        }
        NodeVisual* v = anim.findById(node->id);
        if (v == nullptr)
        {
            continue;
        }
        float r = Theme::nodeRadius() * 0.76f;
        cover.setRadius(r);
        cover.setOrigin(r, r);
        cover.setPosition(v->x, v->y);
        cover.setFillColor(node->id == selectedId ? Theme::nodeDiscLit() : Theme::nodeDisc());
        window.draw(cover);

        lineText.setCharacterSize(18);
        lineText.setString("?");
        sf::FloatRect qb = lineText.getLocalBounds();
        lineText.setPosition(v->x - qb.width / 2.f - qb.left, v->y - qb.height / 2.f - qb.top);
        lineText.setFillColor(node->id == selectedId ? Theme::gold() : Theme::dimText());
        window.draw(lineText);
        lineText.setCharacterSize(Theme::monoSize());
    }
}

string Level12Rebuild::listToText(const int values[], int n)
{
    string out = "";
    for (int i = 0; i < n; i++)
    {
        if (i > 0)
        {
            out = out + "  ";
        }
        out = out + TextUtil::intToText(values[i]);
    }
    return out;
}

void Level12Rebuild::drawUiOverlay(sf::RenderWindow& window)
{
    if (!fontReady)
    {
        lineText.setFont(app.monoFont);
        lineText.setCharacterSize(Theme::monoSize());
        fontReady = true;
    }

    lineText.setFillColor(Theme::gold());
    lineText.setString((usePostorder ? string("POSTORDER  ") : string("PREORDER   "))
                       + listToText(preList, roomCount));
    lineText.setPosition(16.f, 16.f);
    window.draw(lineText);

    lineText.setFillColor(Theme::currentVisit());
    lineText.setString("INORDER    " + listToText(inList, roomCount));
    lineText.setPosition(16.f, 38.f);
    window.draw(lineText);

    lineText.setFillColor(Theme::dimText());
    lineText.setString(selectedId == -1
        ? "Click an empty room, then click the value that belongs in it."
        : "Room selected - now click its value below.");
    lineText.setPosition(16.f, 62.f);
    window.draw(lineText);

    // ---- the value tokens ----
    float y = tokenRowY();
    float width = tokenWidth();
    float left = tokenRowLeft();
    for (int i = 0; i < roomCount; i++)
    {
        float x = left + width * (float)i;
        tokenBox.setPosition(x, y);
        tokenBox.setSize(sf::Vector2f(width - 4.f, 36.f));
        tokenBox.setFillColor(tokenUsed[i]
            ? Theme::withAlpha(Theme::panel(), 90)
            : Theme::panelLight());
        tokenBox.setOutlineThickness(1.f);
        tokenBox.setOutlineColor(tokenUsed[i] ? Theme::skipped() : Theme::gold());
        window.draw(tokenBox);

        lineText.setCharacterSize(Theme::textSize());
        lineText.setString(tokenUsed[i] ? string("-") : TextUtil::intToText(inList[i]));
        sf::FloatRect tb = lineText.getLocalBounds();
        lineText.setPosition(x + (width - 4.f - tb.width) / 2.f - tb.left,
                             y + (36.f - tb.height) / 2.f - tb.top);
        lineText.setFillColor(tokenUsed[i] ? Theme::skipped() : Theme::text());
        window.draw(lineText);
        lineText.setCharacterSize(Theme::monoSize());
    }
}
