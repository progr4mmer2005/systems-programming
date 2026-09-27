// Всё, что есть в окне лабы: элементы, результаты проходов, разметка.
#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../asm/program.h"
#include "../examples.h"
#include "../ui/grid.h"

// Номера элементов окна (Windows различает элементы по номерам).
const int ID_SOURCE = 100;   // исходный текст
const int ID_OPS = 101;      // ТКО
const int ID_SUPPORT = 102;  // вспомогательная таблица
const int ID_SYMBOLS = 103;  // ТСИ
const int ID_ERRORS1 = 104;  // ошибки первого прохода
const int ID_HEADER = 105;   // заголовок объектного модуля
const int ID_CODE = 106;     // двоичный код
const int ID_ERRORS2 = 107;  // ошибки второго прохода
const int ID_RELOC = 108;    // таблица настройки
const int ID_EXTNAMES = 109; // таблица внешних имён
const int ID_EXTREFS = 110;  // список внешних ссылок
const int ID_PASS1 = 200;
const int ID_PASS2 = 201;
const int ID_RESET = 202;
const int ID_EXAMPLES = 203;
const int ID_ADDRESS = 204;
const int ID_THEME = 205;
const int ID_SAVE = 206;

// Состояние этапа для отображения вверху окна.
const int STEP_WAITING = 0;
const int STEP_OK = 1;
const int STEP_FAILED = 2;

// ---- элементы окна ----
HWND mainWindow = NULL;
Grid sourceGrid, opsGrid;                    // левая карточка - входные данные
Grid supportGrid, symbolsGrid, errors1Grid;  // средняя - первый проход
Grid headerGrid, relocGrid, extNamesGrid, extRefsGrid, codeGrid, errors2Grid;  // правая - второй проход
HWND addressEdit = NULL;                     // поле "Адрес загрузки"
HWND pass1Button = NULL, pass2Button = NULL, resetButton = NULL, themeButton = NULL, saveButton = NULL;
HWND examplesList = NULL;                    // выпадающий список "Пример"

// ---- данные ----
vector<Example> examples;
Pass1Result pass1;
Pass2Result pass2;
bool pass1Done = false;
int lastPass = 0;  // до какого прохода дошёл пользователь: 0, 1 или 2
int step1 = STEP_WAITING;
int step2 = STEP_WAITING;
vector<bool> badSourceRows;  // строки исходного текста с ошибками - красные
vector<bool> badOpsRows;     // строки ТКО с ошибками
string statusText;           // строка состояния внизу
COLORREF statusColor = 0;
bool updatingAddress = false;     // поле адреса меняется программой, а не пользователем

// ---- разметка (считается в layout.h, рисуется в paint.h) ----

// Подписанная таблица на карточке: заголовок, подсказка справа и рамка таблицы.
struct Section {
    RECT title;
    RECT box;
    string caption;
    string hint;
};

vector<RECT> cards;  // три белые карточки
vector<Section> sections;
RECT addressLabelBox, addressBox, keysHintBox, statusBox, exampleLabelBox, stepsBox;
