#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../asm/program.h"
#include "../platform/settings.h"
#include "../examples.h"
#include "../ui/grid.h"

const int ID_SOURCE = 100;
const int ID_OPS = 101;
const int ID_SUPPORT = 102;
const int ID_SYMBOLS = 103;
const int ID_ERRORS1 = 104;
const int ID_HEADER = 105;
const int ID_CODE = 106;
const int ID_RELOC = 108;
const int ID_ERRORS2 = 107;
const int ID_PASS1 = 200;
const int ID_PASS2 = 201;
const int ID_RESET = 202;
const int ID_EXAMPLES = 203;
const int ID_THEME = 205;
const int ID_MODE = 206;

const int STEP_WAITING = 0;
const int STEP_OK = 1;
const int STEP_FAILED = 2;

HWND mainWindow = NULL;
Grid sourceGrid, opsGrid;
Grid supportGrid, symbolsGrid, errors1Grid;
Grid headerGrid, relocGrid, codeGrid, errors2Grid;
HWND pass1Button = NULL, pass2Button = NULL, resetButton = NULL, themeButton = NULL;
HWND examplesList = NULL;
HWND modeList = NULL;

vector<Example> examples;
Pass1Result pass1;
Pass2Result pass2;
bool pass1Done = false;
int lastPass = 0;
int step1 = STEP_WAITING;
int step2 = STEP_WAITING;
vector<bool> badSourceRows;
vector<bool> badOpsRows;
string statusText;
COLORREF statusColor = 0;
int themeMode = THEME_SYSTEM;
int addressingMode = MODE_BOTH;

struct Section {
    RECT title;
    RECT box;
    string caption;
    string hint;
};

vector<RECT> cards;
vector<Section> sections;
RECT modeLabelBox, keysHintBox, statusBox, exampleLabelBox, stepsBox;
