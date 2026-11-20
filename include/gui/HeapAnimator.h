#pragma once

#include "core/EventLog.h"
#include "core/Limits.h"

// Plays one heap operation's EventLog, the way AnimationController plays a
// tree's. A heap has no nodes to track by id: its picture is just the array,
// so this keeps the array the player should be seeing right now, plus which
// cells are highlighted and how far through a swap the animation is.
//
// Both Act 4 views read from here, which is what keeps the tree view and the
// array view synchronised: they never hold their own copy of the data.
//
// Convention, the same as AnimationController: the caller saves array
// snapshot 0 (the state BEFORE the operation), runs the model operation,
// then calls start().
class HeapAnimator
{
public:
    HeapAnimator();

    void start(EventLog* eventLog, bool autoPlay, int startHeapSize);
    void showStatic(const int values[], int n, int activeHeapSize);
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
    EventLog* getLog();

    // ---- the picture the two views draw ----
    int getCount();
    int getValue(int i);
    int getHeapSize();          // indices 0 .. heapSize-1 are the active heap
    int getSortedFrom();        // indices here .. count-1 are the sorted region
    bool isSortComplete();
    int getHighlightA();        // the two cells of the swap being animated, or -1
    int getHighlightB();
    int getActiveIndex();       // the cell a sift or extraction is working on
    bool isSwapping();
    float getSwapProgress();    // 0..1 eased, for sliding the two cells past each other

private:
    EventLog* log;
    int values[Limits::MAX_NODES_GUI];
    int count;
    int heapSize;
    int initialHeapSize;
    bool sortComplete;
    int highlightA, highlightB;
    int activeIndex;

    int eventIndex;
    float eventTime;
    bool eventStarted;
    bool playing;
    bool pauseAfterEvent;
    float speed;
    bool swapping;
    float swapT;
    int pendingSnapshot;

    void loadSnapshot(int snapshotIndex);
    void beginEvent(const Event& e, bool snap);
    void finishEvent();
};
