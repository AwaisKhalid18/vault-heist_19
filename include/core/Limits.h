#pragma once

// Fixed sizes shared by every module, so no file needs a growing container.
class Limits
{
public:
    static const int MAX_NODES_GUI  = 256;   // largest tree/heap shown on screen
    static const int MAX_EVENTS     = 4000;  // events stored per operation
    static const int MAX_SNAPSHOTS  = 400;   // tree-shape / array snapshots per operation
    static const int MAX_LEVEL_KEYS = 64;
    static const int MAX_HINTS      = 5;
    static const int MAX_SCREENS    = 10;
};
