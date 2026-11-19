#include "game/ScoreSystem.h"

int ScoreSystem::alarmCapacity(int optimal)
{
    // ceil(1.5 * optimal) with integer maths: (3 * optimal + 1) / 2 rounds up
    // exactly when 3 * optimal is odd, which is when optimal is odd.
    return (3 * optimal + 1) / 2 + 2;
}

int ScoreSystem::computeStars(const LevelOutcome& o)
{
    if (!o.completed)
    {
        return 0;
    }
    int stars = 1;   // completed within capacity
    // 1.25 * optimal without floating point: steps * 4 <= optimal * 5
    if (o.alarmSteps * 4 <= o.optimalSteps * 5 && o.penaltyCount <= 1)
    {
        stars = 2;
    }
    if (o.alarmSteps <= o.optimalSteps && o.penaltyCount == 0)
    {
        stars = 3;
    }
    if (o.hintUsed && stars > 2)
    {
        stars = 2;   // using a hint caps the result at 2 stars
    }
    return stars;
}

int ScoreSystem::computeScore(const LevelOutcome& o)
{
    int score = 1000 - 10 * o.alarmSteps - o.penaltyPoints + o.bonuses;
    if (score < 0)
    {
        score = 0;
    }
    return score;
}
