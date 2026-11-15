#include "gui/AnimationController.h"
#include "gui/Theme.h"

NodeVisual::NodeVisual()
{
    id = -1;
    key = 0;
    parentId = -1;
    depth = 0;
    heightVal = 1;
    sizeVal = 1;
    bf = 0;
    x = 0.f;
    y = 0.f;
    fromX = 0.f;
    fromY = 0.f;
    toX = 0.f;
    toY = 0.f;
    scale = 1.f;
    targetScale = 1.f;
    alpha = 255.f;
    targetAlpha = 255.f;
    state = 0;
    flash = 0.f;
    present = true;
}

AnimationController::AnimationController()
{
    log = nullptr;
    nodeCount = 0;
    eventIndex = 0;
    eventTime = 0.f;
    eventStarted = false;
    playing = false;
    pauseAfterEvent = false;
    speed = 1.f;
    moveT = 1.f;
}

int AnimationController::visualIndexOf(int id)
{
    for (int i = 0; i < nodeCount; i++)
    {
        if (nodes[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

NodeVisual* AnimationController::findById(int id)
{
    int index = visualIndexOf(id);
    return index >= 0 ? &nodes[index] : nullptr;
}

void AnimationController::applyShape(int snapshotIndex, bool snap)
{
    if (log == nullptr)
    {
        return;
    }
    int n = log->getTreeShape(snapshotIndex, shapeBuffer);
    layout.layoutShape(shapeBuffer, n, placeBuffer);

    // Every slot must start false: nodes added below push nodeCount up, and
    // the "vanished" loop at the end reads seen[] for those new slots too.
    bool seen[Limits::MAX_NODES_GUI];
    for (int i = 0; i < Limits::MAX_NODES_GUI; i++)
    {
        seen[i] = false;
    }

    for (int p = 0; p < n; p++)
    {
        int index = visualIndexOf(placeBuffer[p].id);
        if (index >= 0)
        {
            NodeVisual& v = nodes[index];
            seen[index] = true;
            v.fromX = v.x;
            v.fromY = v.y;
            v.toX = placeBuffer[p].x;
            v.toY = placeBuffer[p].y;
            v.key = placeBuffer[p].key;
            v.parentId = placeBuffer[p].parentId;
            v.depth = placeBuffer[p].depth;
            v.heightVal = placeBuffer[p].heightVal;
            v.sizeVal = placeBuffer[p].sizeVal;
            v.bf = placeBuffer[p].bf;
            v.present = true;
            v.targetAlpha = 255.f;
            v.targetScale = 1.f;
            if (snap)
            {
                v.x = v.toX;
                v.y = v.toY;
                v.fromX = v.toX;
                v.fromY = v.toY;
                v.alpha = 255.f;
                v.scale = 1.f;
            }
        }
        else if (nodeCount < Limits::MAX_NODES_GUI)
        {
            NodeVisual& v = nodes[nodeCount];
            v = NodeVisual();
            v.id = placeBuffer[p].id;
            v.key = placeBuffer[p].key;
            v.parentId = placeBuffer[p].parentId;
            v.depth = placeBuffer[p].depth;
            v.heightVal = placeBuffer[p].heightVal;
            v.sizeVal = placeBuffer[p].sizeVal;
            v.bf = placeBuffer[p].bf;
            v.toX = placeBuffer[p].x;
            v.toY = placeBuffer[p].y;
            v.fromX = v.toX;
            v.fromY = v.toY;
            v.x = v.toX;
            v.y = v.toY;
            if (snap)
            {
                v.scale = 1.f;
                v.alpha = 255.f;
            }
            else
            {
                v.scale = 0.f;      // grow-in for freshly inserted rooms
                v.alpha = 0.f;
            }
            v.targetScale = 1.f;
            v.targetAlpha = 255.f;
            seen[nodeCount] = true;   // a brand new room belongs to this shape
            nodeCount++;
        }
    }

    // nodes that vanished from the snapshot fade away
    for (int i = 0; i < nodeCount; i++)
    {
        if (!seen[i] && nodes[i].present)
        {
            nodes[i].present = false;
            nodes[i].targetAlpha = 0.f;
            nodes[i].targetScale = 0.6f;
            if (snap)
            {
                nodes[i].alpha = 0.f;
            }
        }
    }
    if (snap)
    {
        removeFadedNodes();
        moveT = 1.f;
    }
    else
    {
        moveT = 0.f;
    }
}

void AnimationController::removeFadedNodes()
{
    int i = 0;
    while (i < nodeCount)
    {
        if (!nodes[i].present && nodes[i].alpha <= 4.f)
        {
            nodes[i] = nodes[nodeCount - 1];
            nodeCount--;
        }
        else
        {
            i++;
        }
    }
}

void AnimationController::startLog(EventLog* eventLog, bool autoPlay)
{
    log = eventLog;
    eventIndex = 0;
    eventTime = 0.f;
    eventStarted = false;
    playing = autoPlay;
    pauseAfterEvent = false;
    nodeCount = 0;
    if (log != nullptr && log->snapshotCount() > 0)
    {
        applyShape(0, true);
    }
}

void AnimationController::markSubtree(int rootId, int state)
{
    // walk the subtree using parent links stored in the visuals:
    // a node belongs if following parents reaches rootId
    for (int i = 0; i < nodeCount; i++)
    {
        if (!nodes[i].present)
        {
            continue;
        }
        int cur = nodes[i].id;
        int guard = 0;
        while (cur != -1 && guard < Limits::MAX_NODES_GUI)
        {
            if (cur == rootId)
            {
                nodes[i].state = state;
                break;
            }
            NodeVisual* v = findById(cur);
            cur = v != nullptr ? v->parentId : -1;
            guard++;
        }
    }
}

void AnimationController::beginEvent(const Event& e, bool snap)
{
    if (e.type == Event::VISIT)
    {
        // the previous current room becomes a visited room
        for (int i = 0; i < nodeCount; i++)
        {
            if (nodes[i].state == STATE_CURRENT)
            {
                nodes[i].state = STATE_VISITED;
            }
        }
        NodeVisual* v = findById(e.nodeId);
        if (v != nullptr && v->state != STATE_RESULT)
        {
            v->state = STATE_CURRENT;
        }
    }
    else if (e.type == Event::INSERT || e.type == Event::UNLINK || e.type == Event::ROTATE
             || e.type == Event::SWAP_VALUE)
    {
        if (e.snapshotIndex >= 0)
        {
            applyShape(e.snapshotIndex, snap);
        }
        if (e.type == Event::SWAP_VALUE)
        {
            NodeVisual* a = findById(e.nodeId);
            NodeVisual* b = findById(e.otherId);
            if (a != nullptr) { a->flash = 1.f; }
            if (b != nullptr) { b->flash = 1.f; }
        }
        if (e.type == Event::INSERT)
        {
            NodeVisual* v = findById(e.nodeId);
            if (v != nullptr)
            {
                v->state = STATE_RESULT;
            }
        }
    }
    else if (e.type == Event::UPDATE_HEIGHT || e.type == Event::UPDATE_SIZE)
    {
        NodeVisual* v = findById(e.nodeId);
        if (v != nullptr)
        {
            v->flash = 1.f;
        }
    }
    else if (e.type == Event::BALANCE_ALERT)
    {
        NodeVisual* v = findById(e.nodeId);
        if (v != nullptr)
        {
            v->state = STATE_VIOLATION;
        }
    }
    else if (e.type == Event::HIGHLIGHT_SUBTREE)
    {
        int state = STATE_VISITED;
        if (e.mode == Event::HL_SKIPPED)
        {
            state = STATE_SKIPPED;
        }
        else if (e.mode == Event::HL_RESULT)
        {
            state = STATE_RESULT;
        }
        if (e.mode == Event::HL_RESULT)
        {
            NodeVisual* v = findById(e.nodeId);
            if (v != nullptr)
            {
                v->state = state;
            }
        }
        else
        {
            markSubtree(e.nodeId, state);
        }
    }
}

void AnimationController::update(float dt)
{
    float duration = Theme::eventSeconds() / speed;

    // slide every node towards its target position
    if (moveT < 1.f)
    {
        moveT += dt / duration;
        if (moveT > 1.f)
        {
            moveT = 1.f;
        }
    }
    float t = Theme::ease(moveT);
    for (int i = 0; i < nodeCount; i++)
    {
        NodeVisual& v = nodes[i];
        v.x = v.fromX + (v.toX - v.fromX) * t;
        v.y = v.fromY + (v.toY - v.fromY) * t;

        float fade = 6.f * dt;
        if (v.alpha < v.targetAlpha)
        {
            v.alpha += 255.f * fade;
            if (v.alpha > v.targetAlpha) { v.alpha = v.targetAlpha; }
        }
        else if (v.alpha > v.targetAlpha)
        {
            v.alpha -= 255.f * fade;
            if (v.alpha < v.targetAlpha) { v.alpha = v.targetAlpha; }
        }
        if (v.scale < v.targetScale)
        {
            v.scale += fade;
            if (v.scale > v.targetScale) { v.scale = v.targetScale; }
        }
        else if (v.scale > v.targetScale)
        {
            v.scale -= fade;
            if (v.scale < v.targetScale) { v.scale = v.targetScale; }
        }
        if (v.flash > 0.f)
        {
            v.flash -= dt * 2.f;
            if (v.flash < 0.f) { v.flash = 0.f; }
        }
    }
    removeFadedNodes();

    // advance through the event list
    if (!playing || log == nullptr || eventIndex >= log->size())
    {
        return;
    }
    if (!eventStarted)
    {
        beginEvent(log->get(eventIndex), false);
        eventStarted = true;
        eventTime = 0.f;
    }
    eventTime += dt;
    if (eventTime >= duration)
    {
        eventIndex++;
        eventStarted = false;
        if (pauseAfterEvent)
        {
            playing = false;
            pauseAfterEvent = false;
        }
    }
}

void AnimationController::play()
{
    playing = true;
}

void AnimationController::pause()
{
    playing = false;
}

void AnimationController::togglePlay()
{
    playing = !playing;
}

bool AnimationController::isPlaying()
{
    return playing;
}

void AnimationController::stepForward()
{
    if (log == nullptr || eventIndex >= log->size())
    {
        return;
    }
    playing = true;
    pauseAfterEvent = true;
}

void AnimationController::stepBack()
{
    if (eventIndex > 0)
    {
        seek(eventIndex - 1);
    }
}

void AnimationController::seek(int newIndex)
{
    if (log == nullptr)
    {
        return;
    }
    if (newIndex < 0)
    {
        newIndex = 0;
    }
    if (newIndex > log->size())
    {
        newIndex = log->size();
    }
    // rebuild the picture from the start, applying events instantly
    nodeCount = 0;
    if (log->snapshotCount() > 0)
    {
        applyShape(0, true);
    }
    for (int i = 0; i < newIndex; i++)
    {
        beginEvent(log->get(i), true);
    }
    removeFadedNodes();
    eventIndex = newIndex;
    eventTime = 0.f;
    eventStarted = false;
    playing = false;
}

void AnimationController::finishInstantly()
{
    if (log == nullptr)
    {
        return;
    }
    seek(log->size());
}

void AnimationController::setSpeed(float s)
{
    if (s < 0.25f) { s = 0.25f; }
    if (s > 4.f) { s = 4.f; }
    speed = s;
}

float AnimationController::getSpeed()
{
    return speed;
}

bool AnimationController::isFinished()
{
    return log == nullptr || eventIndex >= log->size();
}

int AnimationController::currentIndex()
{
    return eventIndex;
}

int AnimationController::totalEvents()
{
    return log != nullptr ? log->size() : 0;
}

float AnimationController::progress01()
{
    int total = totalEvents();
    if (total == 0)
    {
        return 1.f;
    }
    return (float)eventIndex / (float)total;
}

const Event* AnimationController::getCurrentEvent()
{
    if (log == nullptr || log->size() == 0)
    {
        return nullptr;
    }
    int index = eventIndex < log->size() ? eventIndex : log->size() - 1;
    return &log->get(index);
}

EventLog* AnimationController::getLog()
{
    return log;
}

NodeVisual* AnimationController::getVisuals()
{
    return nodes;
}

int AnimationController::getVisualCount()
{
    return nodeCount;
}

int AnimationController::findIdAt(float worldX, float worldY)
{
    float radius = Theme::nodeRadius() + 6.f;
    for (int i = nodeCount - 1; i >= 0; i--)
    {
        if (!nodes[i].present)
        {
            continue;
        }
        float dx = worldX - nodes[i].x;
        float dy = worldY - nodes[i].y;
        if (dx * dx + dy * dy <= radius * radius)
        {
            return nodes[i].id;
        }
    }
    return -1;
}

void AnimationController::getBounds(float& minX, float& minY, float& maxX, float& maxY)
{
    if (nodeCount == 0)
    {
        minX = -100.f;
        minY = -100.f;
        maxX = 100.f;
        maxY = 100.f;
        return;
    }
    minX = nodes[0].toX;
    maxX = nodes[0].toX;
    minY = nodes[0].toY;
    maxY = nodes[0].toY;
    for (int i = 1; i < nodeCount; i++)
    {
        if (!nodes[i].present)
        {
            continue;
        }
        if (nodes[i].toX < minX) { minX = nodes[i].toX; }
        if (nodes[i].toX > maxX) { maxX = nodes[i].toX; }
        if (nodes[i].toY < minY) { minY = nodes[i].toY; }
        if (nodes[i].toY > maxY) { maxY = nodes[i].toY; }
    }
}

void AnimationController::clearStates()
{
    for (int i = 0; i < nodeCount; i++)
    {
        nodes[i].state = STATE_NORMAL;
    }
}

void AnimationController::setStateById(int id, int state)
{
    NodeVisual* v = findById(id);
    if (v != nullptr)
    {
        v->state = state;
    }
}
