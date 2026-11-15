#pragma once

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

// Our own tiny test runner: no framework, no globals.
// check() prints [PASS]/[FAIL] and counts results; everything is also
// mirrored into docs/test_output_A.txt for the test report.
class TestRunner
{
public:
    int passed;
    int failed;

    TestRunner()
    {
        passed = 0;
        failed = 0;
        mirror = nullptr;
    }

    void setMirror(ofstream* file)
    {
        mirror = file;
    }

    void line(const string& text)
    {
        cout << text << endl;
        if (mirror != nullptr)
        {
            *mirror << text << endl;
        }
    }

    void check(bool condition, const string& testName)
    {
        if (condition)
        {
            passed++;
            line("[PASS] " + testName);
        }
        else
        {
            failed++;
            line("[FAIL] " + testName);
        }
    }

private:
    ofstream* mirror;
};
