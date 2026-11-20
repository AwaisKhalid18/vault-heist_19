#include "TestRunner.h"
#include "game/SaveManager.h"
#include "model/BinaryTree.h"
#include "model/MaxHeap.h"
#include "core/Random.h"
#include "core/TextUtil.h"

// SaveManager tests: campaign progress, the leaderboard table, the settings,
// and the two serialisation formats section 8 of the spec specifies.

static void testProgress(TestRunner& t)
{
    SaveManager save;
    t.check(save.totalStars() == 0, "a fresh save has no stars");
    t.check(save.starsFor("1.1") == 0, "an unplayed level has no stars");
    t.check(!save.isCleared("1.1"), "an unplayed level is not cleared");

    save.recordLevel("1.1", 2, 800);
    t.check(save.starsFor("1.1") == 2 && save.bestScoreFor("1.1") == 800,
            "a finished level records its stars and score");
    t.check(save.isCleared("1.1"), "a level with stars counts as cleared");

    // a worse replay must not take the record away
    save.recordLevel("1.1", 1, 400);
    t.check(save.starsFor("1.1") == 2 && save.bestScoreFor("1.1") == 800,
            "a worse replay does not lower the recorded best");

    save.recordLevel("1.1", 3, 950);
    t.check(save.starsFor("1.1") == 3 && save.bestScoreFor("1.1") == 950,
            "a better replay raises the recorded best");

    save.recordLevel("4.5", 3, 700);
    t.check(save.totalStars() == 6, "total stars adds every level up");

    save.clear();
    t.check(save.totalStars() == 0 && save.starsFor("1.1") == 0,
            "clear() wipes the progress");
}

static void testLeaderboard(TestRunner& t)
{
    SaveManager save;
    t.check(save.scoreCount() == 0, "the leaderboard starts empty");

    save.addScore("Ali", 500);
    save.addScore("Awais", 900);
    save.addScore("Guard", 100);
    t.check(save.scoreCount() == 3, "three runs are recorded");
    t.check(save.scoreName(0) == "Awais" && save.scoreValue(0) == 900,
            "the leaderboard is sorted highest first");
    t.check(save.scoreName(2) == "Guard", "the lowest run sits last");

    // the table holds ten entries and keeps only the best of them
    for (int i = 0; i < 20; i++)
    {
        save.addScore("Run" + TextUtil::intToText(i), i * 10);
    }
    t.check(save.scoreCount() == SaveManager::MAX_SCORES,
            "the leaderboard never grows past its limit");
    bool descending = true;
    for (int i = 1; i < save.scoreCount(); i++)
    {
        if (save.scoreValue(i) > save.scoreValue(i - 1))
        {
            descending = false;
        }
    }
    t.check(descending, "the leaderboard stays sorted after many entries");
    t.check(save.scoreValue(0) == 900, "the best run of all is still on top");

    save.addScore("", 1);
    t.check(save.scoreCount() == SaveManager::MAX_SCORES,
            "a run too slow for the table is dropped");
}

static void testSaveAndLoad(TestRunner& t)
{
    SaveManager written;
    written.recordLevel("1.1", 3, 910);
    written.recordLevel("1.4", 2, 640);
    written.recordLevel("4.5", 1, 300);
    written.addScore("Ali", 1234);
    written.addScore("Awais", 777);
    written.animationSpeed = 2.5f;
    written.validateAfterEachStep = true;
    written.showTooltips = false;
    written.endlessSeed = 4242;
    string savePath = "vault_heist_test_save.txt";   // a local, never a global
    t.check(written.save(savePath), "the save file is written");

    SaveManager read;
    t.check(read.load(savePath), "the save file is read back");
    t.check(read.starsFor("1.1") == 3 && read.bestScoreFor("1.1") == 910,
            "level progress survives a save and load");
    t.check(read.totalStars() == 6, "every level's stars survive");
    t.check(read.scoreCount() == 2 && read.scoreName(0) == "Ali"
            && read.scoreValue(0) == 1234, "the leaderboard survives");
    t.check(read.animationSpeed > 2.4f && read.animationSpeed < 2.6f,
            "the animation speed setting survives");
    t.check(read.validateAfterEachStep, "the validator setting survives");
    t.check(!read.showTooltips, "the tooltip setting survives");
    t.check(read.endlessSeed == 4242, "the endless seed survives");

    SaveManager missing;
    t.check(!missing.load("no_such_save_file_at_all.txt"),
            "loading a missing save file fails cleanly instead of crashing");
    t.check(missing.totalStars() == 0, "a failed load leaves an empty save");
}

static void testTreeSerialisation(TestRunner& t)
{
    // the spec's own worked example: 50 30 # # 70 60 # # #
    BinaryTree spec;
    int pre[4] = { 50, 30, 70, 60 };
    int in[4] = { 30, 50, 60, 70 };
    spec.buildPreIn(pre, in, 4);
    string text = SaveManager::serialiseTree(spec.getRoot());
    t.check(text == "50 30 # # 70 60 # # #",
            "a tree serialises as preorder with null markers (" + text + ")");

    BinaryTree rebuilt;
    t.check(SaveManager::deserialiseTree(text, rebuilt), "that text loads back");
    t.check(spec.isSameTree(rebuilt), "the reloaded tree is identical");

    BinaryTree empty;
    string emptyText = SaveManager::serialiseTree(empty.getRoot());
    t.check(emptyText == "#", "an empty tree serialises as a single marker");

    BinaryTree single;
    int preOne[1] = { 7 };
    int inOne[1] = { 7 };
    single.buildPreIn(preOne, inOne, 1);
    t.check(SaveManager::serialiseTree(single.getRoot()) == "7 # #",
            "a single room serialises with both children missing");

    BinaryTree badInput;
    t.check(!SaveManager::deserialiseTree("50 30 #", badInput),
            "a truncated save string is rejected");
    t.check(!SaveManager::deserialiseTree("50 banana # #", badInput),
            "a save string with rubbish in it is rejected");
    t.check(!SaveManager::deserialiseTree("50 # # 99", badInput),
            "a save string with trailing rubbish is rejected");

    // round trip over many random shapes
    Random rng(5150);
    bool allRoundTrip = true;
    for (int round = 0; round < 200; round++)
    {
        int n = rng.nextInt(1, 15);
        int values[16];
        for (int i = 0; i < n; i++)
        {
            values[i] = i + 1;
        }
        rng.shuffle(values, n);

        int randomPre[16];
        int written = 0;
        int stackLo[34];
        int stackHi[34];
        int top = 0;
        stackLo[0] = 0;
        stackHi[0] = n - 1;
        top = 1;
        while (top > 0)
        {
            top--;
            int lo = stackLo[top];
            int hi = stackHi[top];
            if (lo > hi)
            {
                continue;
            }
            int split = rng.nextInt(lo, hi);
            randomPre[written] = values[split];
            written++;
            stackLo[top] = split + 1;
            stackHi[top] = hi;
            top++;
            stackLo[top] = lo;
            stackHi[top] = split - 1;
            top++;
        }

        BinaryTree original;
        original.buildPreIn(randomPre, values, n);
        string saved = SaveManager::serialiseTree(original.getRoot());
        BinaryTree loaded;
        if (!SaveManager::deserialiseTree(saved, loaded) || !original.isSameTree(loaded))
        {
            allRoundTrip = false;
        }
    }
    t.check(allRoundTrip, "200 random vaults survive a save and load unchanged");
}

static void testHeapSerialisation(TestRunner& t)
{
    MaxHeap heap;
    int values[6] = { 90, 70, 80, 10, 20, 30 };
    heap.buildMaxHeap(values, 6);
    string text = SaveManager::serialiseHeap(heap);
    t.check(text == "90 70 80 10 20 30 | 6",
            "a heap serialises as its array plus the active size (" + text + ")");

    MaxHeap loaded;
    t.check(SaveManager::deserialiseHeap(text, loaded), "that text loads back");
    bool identical = loaded.totalCount() == heap.totalCount()
                  && loaded.getHeapSize() == heap.getHeapSize();
    for (int i = 0; i < heap.totalCount() && identical; i++)
    {
        if (loaded.get(i) != heap.get(i))
        {
            identical = false;
        }
    }
    t.check(identical, "the reloaded heap matches value for value");
    t.check(loaded.isMaxHeap(), "the reloaded heap is still a valid Max Heap");

    // a half-sorted heap: the active size is the part that matters
    int result = 0;
    heap.extractMax(result);
    heap.extractMax(result);
    string midSort = SaveManager::serialiseHeap(heap);
    MaxHeap resumed;
    t.check(SaveManager::deserialiseHeap(midSort, resumed), "a half-sorted heap loads back");
    t.check(resumed.getHeapSize() == heap.getHeapSize() && resumed.totalCount() == 6,
            "the active heap size and the sorted region both survive");
    t.check(resumed.isMaxHeap(), "the resumed active heap is still valid");

    MaxHeap emptyHeap;
    string emptyText = SaveManager::serialiseHeap(emptyHeap);
    MaxHeap emptyLoaded;
    t.check(SaveManager::deserialiseHeap(emptyText, emptyLoaded)
            && emptyLoaded.totalCount() == 0, "an empty heap round trips");

    MaxHeap bad;
    t.check(!SaveManager::deserialiseHeap("90 oops 80 | 3", bad),
            "a heap save string with rubbish in it is rejected");
}

void runSaveTests(TestRunner& t)
{
    t.line("");
    t.line("-- SaveManager --");
    testProgress(t);
    testLeaderboard(t);
    testSaveAndLoad(t);
    testTreeSerialisation(t);
    testHeapSerialisation(t);
}
