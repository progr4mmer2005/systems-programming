#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../config.h"
#include "../util/table.h"
#include "encoding.h"
#include "names.h"
#include "operands.h"
#include "optable.h"
#include "program.h"

void addError(Assembly& a, int row, string text) { a.errors.push_back({"source", row, text}); }

bool isReserved(string name, vector<OpCode> ops) { return isRegister(name) || isDirective(name) || findOp(ops, name) != -1; }

string placeholder(int format) { return string(addressDigits(format), '?'); }

string checkLabel(string label, Assembly& a) {
    if (!isName(label)) return "Метка «" + label + "»: только латинские буквы, цифры и «_», первая - буква";
    if (len(label) > MAX_NAME_LENGTH) return "Метка «" + label + "» длиннее " + to_string(MAX_NAME_LENGTH) + " символов";
    if (isRegister(label)) return "Метка «" + label + "» совпадает с именем регистра";
    if (isReserved(label, a.ops)) return "Метка «" + label + "» совпадает с названием команды или директивы";
    if (a.programName != "" && sameName(label, a.programName)) return "Метка «" + label + "» совпадает с именем программы";
    int i = findSymbol(a.symbols, label);
    if (i != -1 && a.symbols[i].defined) return "Метка «" + label + "» уже определена в строке " + to_string(a.symbols[i].row + 1);
    if (findSymbol(a.extRefs, label) != -1)
        return "Метка «" + label + "» совпадает с внешней ссылкой из EXTREF - эта метка определена в другом модуле";
    return "";
}

string checkExternalName(string name, bool isDef, Assembly& a) {
    if (!isName(name) || len(name) > MAX_NAME_LENGTH)
        return "имя «" + name + "»: латинские буквы, цифры и «_», первая - буква, до 10 символов";
    if (isReserved(name, a.ops)) return "имя «" + name + "» совпадает с регистром, директивой или командой";
    if (a.programName != "" && sameName(name, a.programName)) return "имя «" + name + "» совпадает с именем программы";
    int d = findSymbol(a.extDefs, name);
    int r = findSymbol(a.extRefs, name);
    if (d != -1 && isDef) return "имя «" + name + "» уже есть в EXTDEF (строка " + to_string(a.extDefs[d].row + 1) + ")";
    if (r != -1 && !isDef) return "имя «" + name + "» уже есть в EXTREF (строка " + to_string(a.extRefs[r].row + 1) + ")";
    if (d != -1) return "имя «" + name + "» уже объявлено внешним именем в EXTDEF - внешней ссылкой оно быть не может";
    if (r != -1) return "имя «" + name + "» уже объявлено внешней ссылкой в EXTREF - внешним именем оно быть не может";
    return "";
}

void parseExternals(Assembly& a, int row, string mnemonic, string label, string op1, string op2) {
    bool isDef = mnemonic != "EXTREF";
    string directive = isDef ? "EXTDEF" : "EXTREF";
    if (label != "") addError(a, row, "У " + directive + " не может быть метки");
    if (a.codeStarted) addError(a, row, directive + " должна стоять в начале программы, сразу после Start");
    if (op1 == "" && op2 == "") {
        addError(a, row, "У " + directive + " не указано ни одного имени");
        return;
    }
    string added;
    for (string name : {op1, op2}) {
        if (name == "") continue;
        string problem = checkExternalName(name, isDef, a);
        if (problem != "") {
            addError(a, row, directive + ": " + problem);
            continue;
        }
        Symbol symbol;
        symbol.name = name;
        symbol.row = row;
        if (isDef)
            a.extDefs.push_back(symbol);
        else
            a.extRefs.push_back(symbol);
        added += (added == "" ? "" : ", ") + name;
    }
    if (added == "") return;
    if (isDef)
        a.done = "EXTDEF: внешние имена " + added + " - их адреса появятся, когда встретятся метки";
    else
        a.done = "EXTREF: внешние ссылки " + added + " - они определены в другом модуле";
}

string parseWord(Line& line) {
    line.kind = "WORD";
    line.size = WORD_SIZE;
    if (line.op1 == "") return "У WORD нет значения: нужно число или «?»";
    if (line.op1 == "?") {
        line.reserve = true;
    } else {
        Number n = parseCount(line.op1);
        if (!n.ok || n.value > MAX_WORD) return "WORD «" + line.op1 + "»: нужно целое число от 0 до 16777215 или «?»";
        line.code = hex(n.value, 6);
    }
    return line.op2 == "" ? "" : "У WORD только один операнд";
}

string parseByte(Line& line) {
    line.kind = "BYTE";
    line.size = 1;
    if (line.op1 == "?") {
        line.reserve = true;
        return line.op2 == "" ? "" : "У BYTE только один операнд";
    }
    ByteString text = parseByteString(line.op1);
    if (text.isString) {
        if (text.problem != "") return "BYTE " + line.op1 + ": " + text.problem;
        line.size = text.size;
        line.code = text.code;
    } else {
        Number n = parseSmallNumber(line.op1);
        if (!n.ok) return "BYTE «" + line.op1 + "»: нужно \"строка\", C\"строка\", X\"hex-цифры\", число 0..255 или «?»";
        line.code = hex(n.value, 2);
    }
    return line.op2 == "" ? "" : "У BYTE только один операнд";
}

string parseReserve(Line& line) {
    line.kind = line.mnemonic;
    line.reserve = true;
    int unit = line.mnemonic == "RESW" ? WORD_SIZE : 1;
    Number n = parseCount(line.op1);
    if (!n.ok || n.value == 0 || n.value * unit > MEMORY_SIZE)
        return line.mnemonic + " «" + line.op1 + "»: нужно, сколько " + (unit == 1 ? "байт" : "слов") +
               " зарезервировать - целое число больше 0";
    line.size = n.value * unit;
    return line.op2 == "" ? "" : "У " + line.mnemonic + " только один операнд";
}

string parseCommand(Line& line, OpCode op, vector<OpCode> ops, int mode) {
    line.kind = "command";
    line.format = op.length;
    line.size = op.length;

    string kind = operandKind(op.length, line.op1, line.op2);
    if (kind == "")
        return op.name + " (формат " + to_string(op.length) + "): ожидается " + expectedOperands(op.length) +
               ", а записано «" + joinOperands(line.op1, line.op2) + "»";
    string name = withoutBrackets(line.op1);
    bool withLabel = kind == "label" || kind == "relative";
    if (withLabel && isReserved(name, ops)) return op.name + ": операнд «" + line.op1 + "» - зарезервированное слово, а не метка";
    if (withLabel && len(name) > MAX_NAME_LENGTH)
        return op.name + ": операнд «" + line.op1 + "» длиннее " + to_string(MAX_NAME_LENGTH) + " символов - такой метки быть не может";
    if (kind == "relative" && mode == MODE_DIRECT)
        return op.name + " " + line.op1 + ": относительная адресация не разрешена - выбран режим «" + modeName(mode) + "»";
    if (kind == "label" && mode == MODE_RELATIVE)
        return op.name + " " + line.op1 + ": прямая адресация не разрешена - выбран режим «" + modeName(mode) +
               "», нужно [" + line.op1 + "]";

    line.addressing = ADDRESSING_NONE;
    if (kind == "label") line.addressing = ADDRESSING_DIRECT;
    if (kind == "relative") line.addressing = ADDRESSING_RELATIVE;
    line.firstByte = makeFirstByte(op.code, line.addressing);
    line.code = hex(line.firstByte, 2);
    if (kind == "registers") line.code += hex(registerNumber(line.op1), 1) + hex(registerNumber(line.op2), 1);
    if (kind == "number") line.code += hex(parseSmallNumber(line.op1).value, 2);
    if (withLabel) line.label = name;
    return "";
}

string tooFar(string mnemonic, string label, long long address) {
    return mnemonic + " (формат 3): адрес метки «" + label + "» = " + hexAddress(address) + " не помещается в 2 байта";
}

void parseStart(Assembly& a, int row, string label, string op1, string op2) {
    if (label == "")
        addError(a, row, "У Start нет метки - это имя программы, оно обязательно");
    else if (!isName(label) || len(label) > MAX_NAME_LENGTH)
        addError(a, row, "Имя программы «" + label + "»: латинские буквы, цифры и «_», до 10 символов");
    else if (isReserved(label, a.ops))
        addError(a, row, "Имя программы «" + label + "» совпадает с командой, директивой или регистром");
    else
        a.programName = label;

    Number address = parseHex(op1, 8);
    if (op1 != "" && !address.ok)
        addError(a, row, "Адрес «" + op1 + "» в Start: нужно шестнадцатеричное число, здесь только 0");
    else if (op1 != "" && address.value != 0)
        addError(a, row, "Перемещаемая программа начинается с адреса 0: в Start нужно 0 или пусто, а не «" + op1 + "»");
    if (op2 != "") addError(a, row, "У Start только один операнд - адрес 0");

    a.start = 0;
    a.counter = a.start;
    a.startRow = row;
    a.done = "Start: программа «" + label + "», счётчик адреса = " + hexAddress(a.start);
}

void parseEnd(Assembly& a, int row, string op1, string op2) {
    a.entry = a.start;
    if (op2 != "") addError(a, row, "У End только один операнд - точка входа");
    if (op1 == "") return;

    long long entry = 0;
    int i = findSymbol(a.symbols, op1);
    if (i != -1 && a.symbols[i].defined) {
        entry = a.symbols[i].address;
    } else if (findSymbol(a.extRefs, op1) != -1) {
        addError(a, row, "Точка входа «" + op1 + "» - внешняя ссылка из EXTREF: она определена в другом модуле, "
                         "начать выполнение этой программы с неё нельзя");
        return;
    } else {
        Number address = parseHex(op1, 8);
        if (!address.ok) {
            if (i != -1)
                addError(a, row, "Точка входа «" + op1 + "»: метка не определена");
            else if (isName(op1))
                addError(a, row, "Точка входа «" + op1 + "»: такой метки нет в ТСИ");
            else
                addError(a, row, "Точка входа «" + op1 + "»: нужна метка или шестнадцатеричный адрес");
            return;
        }
        entry = address.value;
    }
    if (entry < a.start || entry > a.counter)
        addError(a, row, "Точка входа " + op1 + " = " + hexAddress(entry) + " вне программы: нужен адрес от " +
                             hexAddress(a.start) + " до " + hexAddress(a.counter));
    else
        a.entry = entry;
}

void dropRelocation(Assembly& a, long long address) {
    for (int i = 0; i < len(a.relocations); i++) {
        if (a.relocations[i].address != address) continue;
        a.relocations.erase(a.relocations.begin() + i);
        return;
    }
}

void fillAddress(Assembly& a, CodeLine& line, string label, long long target) {
    string mnemonic = upper(cell(a.source[line.row], 1));
    int digits = addressDigits(line.format);
    long long limit = digits == 4 ? 0x10000 : 0x1000000;
    string field;
    if (line.addressing == ADDRESSING_RELATIVE) {
        long long offset = target - (line.address + line.size);
        if (offset < -limit / 2 || offset >= limit / 2)
            addError(a, line.row, mnemonic + " [" + label + "]: смещение " + to_string(offset) + " не помещается в " +
                                      to_string(digits / 2) + " байта");
        else
            field = hex((offset + limit) % limit, digits);
    } else if (target >= limit) {
        addError(a, line.row, tooFar(mnemonic, label, target));
    } else {
        field = hex(target, digits);
    }
    line.waiting = false;
    if (field == "") {
        line.unresolved = true;
        dropRelocation(a, line.address);
        return;
    }
    line.code = line.code.substr(0, len(line.code) - digits) + field;
}

void checkUndefined(Assembly& a) {
    for (Symbol& symbol : a.symbols) {
        if (symbol.defined) continue;
        for (Reference reference : symbol.waiting) {
            CodeLine& line = a.code[reference.line];
            line.waiting = false;
            line.unresolved = true;
            dropRelocation(a, line.address);
            addError(a, line.row, "Метка «" + symbol.name + "» не определена: до End она так и не встретилась, адрес в команде " +
                                      hexAddress(reference.address) + " не заполнен");
        }
    }
}

void checkExternalNames(Assembly& a) {
    for (Symbol name : a.extDefs) {
        if (!isDefined(a, name.name))
            addError(a, name.row, "Внешнее имя «" + name.name + "» из EXTDEF не определено в программе");
    }
}

string addressList(vector<Reference> references) {
    string text;
    for (Reference reference : references) text += (text == "" ? "" : ", ") + hexAddress(reference.address);
    return text;
}

void defineLabel(Assembly& a, string label, int row) {
    string problem = checkLabel(label, a);
    if (problem != "") {
        addError(a, row, problem);
        return;
    }
    int i = findSymbol(a.symbols, label);
    if (i == -1) {
        Symbol symbol;
        symbol.name = label;
        a.symbols.push_back(symbol);
        i = len(a.symbols) - 1;
    }
    Symbol& symbol = a.symbols[i];
    symbol.defined = true;
    symbol.address = a.counter;
    symbol.row = row;
    symbol.external = findSymbol(a.extDefs, label) != -1;
    a.done = "Метка " + symbol.name + " = " + hexAddress(a.counter);
    if (symbol.external) a.done += " (внешнее имя)";
    if (len(symbol.waiting) == 0) return;

    string direct, relative;
    for (Reference reference : symbol.waiting) {
        CodeLine& line = a.code[reference.line];
        fillAddress(a, line, symbol.name, a.counter);
        if (line.unresolved) continue;
        string where = hexAddress(reference.address);
        if (line.addressing == ADDRESSING_RELATIVE) {
            int digits = addressDigits(line.format);
            relative += (relative == "" ? "" : ", ") + where + " (" + line.code.substr(len(line.code) - digits) + ")";
        } else {
            direct += (direct == "" ? "" : ", ") + where;
        }
    }
    if (direct != "") a.done += "; адрес вписан в команды " + direct;
    if (relative != "") a.done += "; смещение вписано в команды " + relative;
    symbol.waiting.clear();
}

string useLabel(Assembly& a, CodeLine& out, Line line) {
    bool relative = line.addressing == ADDRESSING_RELATIVE;
    int ref = findSymbol(a.extRefs, line.label);
    if (ref != -1 && relative) {
        addError(a, line.row, "Внешняя ссылка «" + line.label + "» не может быть с относительной адресацией: "
                              "расстояние до другого модуля неизвестно");
        out.code += placeholder(line.format);
        out.unresolved = true;
        return "";
    }
    if (ref != -1) {
        out.code += string(addressDigits(line.format), '0');
        a.relocations.push_back({line.address, line.row, a.extRefs[ref].name});
        return line.label + " - внешняя ссылка: вместо адреса нули, в таблице настройки адрес команды и имя " + a.extRefs[ref].name;
    }
    out.code += placeholder(line.format);
    if (!relative) a.relocations.push_back({line.address, line.row, ""});
    int i = findSymbol(a.symbols, line.label);
    if (i != -1 && a.symbols[i].defined) {
        long long address = a.symbols[i].address;
        fillAddress(a, out, line.label, address);
        if (out.unresolved) return "";
        if (relative)
            return line.label + " = " + hexAddress(address) + " уже известна, смещение от следующей команды " +
                   hexAddress(line.address + line.size);
        return line.label + " = " + hexAddress(address) + " уже известна, команда в таблице настройки";
    }
    if (i == -1) {
        Symbol symbol;
        symbol.name = line.label;
        symbol.row = line.row;
        a.symbols.push_back(symbol);
        i = len(a.symbols) - 1;
    }
    out.waiting = true;
    a.symbols[i].waiting.push_back({line.address, len(a.code)});
    if (relative) return "метка " + a.symbols[i].name + " ещё не встречалась: вместо смещения заглушка, команда ждёт её в ТСИ";
    return "метка " + a.symbols[i].name + " ещё не встречалась: вместо адреса заглушка, команда ждёт её в ТСИ и уже в таблице настройки";
}

void processRow(Assembly& a, int row) {
    Row source = a.source[row];
    string label = cell(source, 0);
    string mnemonic = upper(cell(source, 1));
    string op1 = cell(source, 2);
    string op2 = cell(source, 3);
    a.done = "";

    if (a.ended) {
        addError(a, row, "Строка после End: все строки программы должны быть до End");
        return;
    }
    if (!a.started) {
        a.started = true;
        if (mnemonic == "START") {
            parseStart(a, row, label, op1, op2);
            return;
        }
        addError(a, row, "Программа должна начинаться с директивы Start");
    }
    if (mnemonic == "START") {
        addError(a, row, "Директива Start может быть только в первой строке");
        return;
    }
    if (mnemonic == "") {
        addError(a, row, label == "" ? "Не указана команда" : "Метка «" + label + "» стоит без команды");
        return;
    }
    if (mnemonic == "END") {
        if (label != "") addError(a, row, "У End не может быть метки «" + label + "»: она указывала бы за конец программы");
        parseEnd(a, row, op1, op2);
        a.endRow = row;
        a.ended = true;
        a.length = a.counter - a.start;
        checkUndefined(a);
        checkExternalNames(a);
        a.done = "End: длина программы " + hexAddress(a.length) + ", точка входа " + hexAddress(a.entry);
        return;
    }

    if (mnemonic == "EXTDEF" || mnemonic == "EXTNAME" || mnemonic == "EXTREF") {
        parseExternals(a, row, mnemonic, label, op1, op2);
        return;
    }
    a.codeStarted = true;

    if (label != "") defineLabel(a, label, row);

    Line line;
    line.address = a.counter;
    line.row = row;
    line.mnemonic = mnemonic;
    line.op1 = op1;
    line.op2 = op2;

    string problem;
    if (mnemonic == "WORD") {
        problem = parseWord(line);
    } else if (mnemonic == "BYTE") {
        problem = parseByte(line);
    } else if (mnemonic == "RESB" || mnemonic == "RESW") {
        problem = parseReserve(line);
    } else {
        int i = findOp(a.ops, mnemonic);
        if (i == -1) {
            addError(a, row, "Команды «" + cell(source, 1) + "» нет в таблице кодов операций");
            return;
        }
        problem = parseCommand(line, a.ops[i], a.ops, a.mode);
    }

    if (a.counter + line.size > MEMORY_SIZE) {
        addError(a, row, "Программа выходит за пределы памяти (последний адрес FFFFFF)");
        a.stopped = true;
        return;
    }
    if (problem != "") {
        addError(a, row, problem);
        a.counter += line.size;
        return;
    }

    CodeLine out;
    out.address = line.address;
    out.size = line.size;
    out.code = line.code;
    out.format = line.format;
    out.addressing = line.addressing;
    out.reserve = line.reserve;
    out.row = row;
    string note;
    if (line.label != "") note = useLabel(a, out, line);
    a.code.push_back(out);
    a.counter += line.size;

    string what = hexAddress(out.address) + ": " + (out.reserve ? "резерв " + hexLength(out.size) + " байт" : out.code);
    if (note != "") what += " (" + note + ")";
    a.done = a.done == "" ? what : a.done + "; " + what;
}

void skipEmptyRows(Assembly& a) {
    while (a.nextRow < len(a.source) && isEmptyRow(a.source[a.nextRow])) a.nextRow++;
}

void finishAssembly(Assembly& a) {
    if (!a.started) a.errors.push_back({"program", -1, "Исходный текст пуст - нечего ассемблировать"});
    if (a.started && !a.ended) {
        if (!a.stopped) a.errors.push_back({"program", -1, "Нет директивы End в конце программы"});
        a.length = a.counter - a.start;
        checkUndefined(a);
        checkExternalNames(a);
    }
    a.finished = true;
    a.ok = len(a.errors) == 0;
}

Assembly beginAssembly(Table source, Table opTable, int mode) {
    Assembly a;
    a.source = source;
    a.mode = mode;
    a.ops = readOpTable(opTable, a.errors);
    if (len(a.errors) > 0) {
        a.finished = true;
        return a;
    }
    skipEmptyRows(a);
    return a;
}

void stepAssembly(Assembly& a) {
    if (a.finished) return;
    a.steps++;
    if (a.nextRow >= len(a.source)) {
        a.lastRow = -1;
        a.done = "";
        finishAssembly(a);
        return;
    }
    a.lastRow = a.nextRow;
    processRow(a, a.nextRow);
    a.nextRow++;
    skipEmptyRows(a);
    if (a.nextRow >= len(a.source) || a.stopped) finishAssembly(a);
}

void runAssembly(Assembly& a) {
    while (!a.finished) stepAssembly(a);
}
