#pragma once

// Everything a finished level reports about itself.
struct LevelOutcome
{
    int alarmSteps, optimalSteps, capacity, penaltyCount, penaltyPoints, bonuses;
    bool hintUsed, completed;

    LevelOutcome()
    {
        alarmSteps = 0;
        optimalSteps = 0;
        capacity = 0;
        penaltyCount = 0;
        penaltyPoints = 0;
        bonuses = 0;
        hintUsed = false;
        completed = false;
    }
};

// Alarm capacity, stars and score. Static functions only: the formulas are
// shared by every level and have no state of their own.
//
// Penalties used by the levels:
//   wrong traversal click  -> +1 alarm
//   wrong rotation choice  -> +2 alarm and -50 points
//   invalid Act 4 swap     -> +1 alarm
class ScoreSystem
{
public:
    static int alarmCapacity(int optimal);          // ceil(1.5 * optimal) + 2
    static int computeStars(const LevelOutcome& o); // 0..3, hint lowers the cap by 1
    static int computeScore(const LevelOutcome& o); // 1000 - 10*alarm - penalties + bonuses, floored at 0
};
