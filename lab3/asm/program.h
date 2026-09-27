// Что получается после проходов: вспомогательная таблица, ТСИ, машинный код.
#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../util/text.h"
#include "errors.h"

// Строка вспомогательной таблицы (результат первого прохода).
// Команда уже переведена в код, кроме адреса метки: вместо него стоит "····",
// а имя метки лежит в label - его адрес подставит второй проход.
struct Line {
    int address = 0;
    int row = 0;           // номер строки в исходном тексте (с нуля)
    string kind;           // "command", "WORD" или "BYTE"
    string mnemonic;       // LD, WORD, BYTE...
    string op1, op2;       // операнды, как записаны в исходном тексте
    int size = 0;          // сколько байт занимает строка
    int format = 0;        // формат команды 1..4
    int firstByte = 0;     // код операции + тип адресации
    int addressing = 0;    // ADDRESSING_NONE / DIRECT / RELATIVE
    string code;           // код после первого прохода: "05 01 ····"
    string label;          // метка в операнде (пусто, если метки нет)
    bool reserve = false;  // WORD ? / BYTE ? - место без значения
};

// Строка ТСИ: метка и её адрес. Так же хранятся имена из EXTDEF и EXTREF.
struct Symbol {
    string name;
    int address;
    int row;
    bool external = false;  // метка объявлена в EXTDEF - внешнее имя
};

struct Pass1Result {
    bool ok = false;
    vector<Error> errors;
    vector<Line> lines;      // вспомогательная таблица
    vector<Symbol> symbols;  // ТСИ
    vector<Symbol> extDefs;  // имена из EXTDEF (адрес не используется)
    vector<Symbol> extRefs;  // имена из EXTREF
    string programName;
    int start = 0;   // адрес загрузки
    int length = 0;  // длина программы
};

// Строка двоичного кода (результат второго прохода).
struct CodeLine {
    int address = 0;
    string code;              // машинный код, байты через пробел
    string source;            // исходная команда - для наглядности
    bool reserve = false;     // резерв памяти без кода
    bool unresolved = false;  // есть неопределённая метка
    int row = 0;
};

// Строка таблицы настройки: команда с прямым адресом, который при загрузке
// программы нужно увеличить на адрес загрузки.
// Для внешней ссылки name - её имя: при загрузке прибавляется адрес этого имени.
// Для обычной метки name пустое: прибавляется адрес загрузки модуля.
struct Relocation {
    int address;    // адрес команды
    string name;    // имя внешней ссылки или ""
    string source;  // сама команда - для наглядности
    int row;        // строка исходного текста
};

struct Pass2Result {
    bool ok = false;
    vector<Error> errors;
    string programName;   // заголовок: имя, длина, адрес загрузки
    int length = 0;
    int start = 0;
    vector<CodeLine> code;
    vector<Relocation> relocations;  // таблица настройки
    vector<Symbol> externalNames;    // таблица внешних имён: имя и адрес
    vector<Symbol> externalRefs;     // список внешних ссылок
};

// Номер метки в ТСИ или -1, если такой нет.
int findSymbol(vector<Symbol> symbols, string name) {
    for (int i = 0; i < len(symbols); i++) {
        if (sameName(symbols[i].name, name)) return i;
    }
    return -1;
}
