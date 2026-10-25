#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../asm/program.h"
#include "../examples.h"
#include "../ui/grid.h"

const int ID_SOURCE = 100;
const int ID_OPS = 101;
const int ID_SYMBOLS = 103;
const int ID_ERRORS = 104;
const int ID_HEADER = 105;
const int ID_CODE = 106;
const int ID_STEP = 200;
const int ID_RUN = 201;
const int ID_RESET = 202;
const int ID_EXAMPLES = 203;
const int ID_ADDRESS = 204;
const int ID_STATUS = 205;

HWND mainWindow = NULL;
Grid sourceGrid, opsGrid;
Grid symbolsGrid, errorsGrid;
Grid headerGrid, codeGrid;
HWND addressEdit = NULL;
HWND stepButton = NULL, runButton = NULL, resetButton = NULL;
HWND examplesList = NULL;
HWND statusBar = NULL;

vector<Example> examples;
Assembly assembly;
bool assembling = false;
int nextSourceRow = 0;
vector<bool> badSourceRows;
vector<bool> badOpsRows;
bool updatingAddress = false;

struct Section {
    RECT title;
    string caption;
};

vector<Section> sections;
RECT addressLabelBox, exampleLabelBox;
