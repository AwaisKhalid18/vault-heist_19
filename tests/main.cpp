#include "TestRunner.h"
#include "core/TextUtil.h"

void runCoreTests(TestRunner& t);
void runBSTTests(TestRunner& t);
void runAVLTests(TestRunner& t);
void runTreeValidatorTests(TestRunner& t);
void runTreeEventTests(TestRunner& t);
void runLevelTests(TestRunner& t);

int main()
{
    ofstream mirror("docs/test_output_A.txt");
    TestRunner t;
    if (mirror.is_open())
    {
        t.setMirror(&mirror);
    }

    t.line("==== Vault Heist - Partner A test suite ====");
    runCoreTests(t);
    runBSTTests(t);
    runAVLTests(t);
    runTreeValidatorTests(t);
    runTreeEventTests(t);
    runLevelTests(t);

    t.line("");
    t.line("==== Summary: " + TextUtil::intToText(t.passed) + " passed, "
           + TextUtil::intToText(t.failed) + " failed ====");
    if (mirror.is_open())
    {
        mirror.close();
    }
    return t.failed == 0 ? 0 : 1;
}
