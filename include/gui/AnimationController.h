#pragma once

#include "core/EventLog.h"
#include "gui/LayoutEngine.h"

// The animated picture of one node on the canvas.
class NodeVisual
{
public:
    int id, key, parentId, depth, heightVal, sizeVal, bf;
    float x, y;                  // where the node is drawn right now
    float fromX, fromY, toX, toY;
    float scale, targetScale;
    float alpha, targetAlpha;
    int state;                   // an AnimationController::STATE_ constant
    float flash;                 // badge flicker timer
    bool present;                // still part of the current tree shape

    NodeVisual();
};

// Plays an EventLog one event at a time with easing. Nodes are matched by
// id between the saved tree snapshots, so a rotation is just "everyone
// slides to the next snapshot's position": the GUI has no rotation code.
//
// Convention: the caller saves snapshot 0 (the shape before the operation)
// into the log, runs the model operation, then calls startLog.
class AnimationController
{
public:
    static const int STATE_NORMAL = 0, STATE_VISITED = 1, STATE_CURRENT = 2, STATE_RESULT = 3,
                     STATE_SKIPPED = 4, STATE_TARGET = 5, STATE_VIOLATION = 6, STATE_SELECTED = 7,
                     STATE_WARNING = 8;

    AnimationController();

    void startLog(EventLog* eventLog, bool autoPlay);
    void update(float dt);

    void play();
    void pause();
    void togglePlay();
    bool isPlaying();
    void stepForward();
    void stepBack();
    void seek(int eventIndex);
    void finishInstantly();
    void setSpeed(float s);
    float getSpeed();
    bool isFinished();
    int currentIndex();
    int totalEvents();
    float progress01();
    const Event* getCurrentEvent();
    EventLog* getLog();

    NodeVisual* getVisuals();
    int getVisualCount();
    NodeVisual* findById(int id);
    int findIdAt(float worldX, float worldY);          // node id under a point, or -1
    void getBounds(float& minX, float& minY, float& maxX, float& maxY);

    void clearStates();                                 // everything back to NORMAL
    void setStateById(int id, int state);               // screens mark targets etc.

private:
    EventLog* log;
    LayoutEngine layout;
    NodeVisual nodes[Limits::MAX_NODES_GUI];
    int nodeCount;

    NodeState shapeBuffer[Limits::MAX_NODES_GUI];
    NodePlacement placeBuffer[Limits::MAX_NODES_GUI];

    int eventIndex;
    float eventTime;
    bool eventStarted;
    bool playing;
    bool pauseAfterEvent;
    float speed;
    float moveT;                 // 0..1 progress of the current slide

    void applyShape(int snapshotIndex, bool snap);
    void beginEvent(const Event& e, bool snap);
    void markSubtree(int rootId, int state);
    void removeFadedNodes();
    int visualIndexOf(int id);
};
