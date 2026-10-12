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

void addError(Pass1Result& result, int row, string text) { result.errors.push_back({"source", row, text}); }

bool isReserved(string name, vector<OpCode> ops) { return isRegister(name) || isDirective(name) || findOp(ops, name) != -1; }

string checkLabel(string label, vector<OpCode> ops, Pass1Result result) {
    if (!isName(label)) return "Метка «" + label + "»: только латинские буквы, цифры и «_», первая - буква";
    if (len(label) > MAX_NAME_LENGTH) return "Метка «" + label + "» длиннее " + to_string(MAX_NAME_LENGTH) + " символов";
    if (isRegister(label)) return "Метка «" + label + "» совпадает с именем регистра";
    if (isReserved(label, ops)) return "Метка «" + label + "» совпадает с названием команды или директивы";
    if (result.programName != "" && sameName(label, result.programName)) return "Метка «" + label + "» совпадает с именем программы";
    int i = findSymbol(result.symbols, label);
    if (i != -1) return "Метка «" + label + "» уже определена в строке " + to_string(result.symbols[i].row + 1);
    return "";
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
    if ((kind == "label" || kind == "relative") && isReserved(withoutBrackets(line.op1), ops))
        return op.name + ": операнд «" + line.op1 + "» - зарезервированное слово, а не метка";
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
    if (kind == "label" || kind == "relative") line.label = withoutBrackets(line.op1);
    return "";
}

void parseStart(Pass1Result& result, vector<OpCode> ops, int row, string label, string op1, string op2) {
    if (label == "")
        addError(result, row, "У Start нет метки - это имя программы, оно обязательно");
    else if (!isName(label) || len(label) > MAX_NAME_LENGTH)
        addError(result, row, "Имя программы «" + label + "»: латинские буквы, цифры и «_», до 10 символов");
    else if (isReserved(label, ops))
        addError(result, row, "Имя программы «" + label + "» совпадает с командой, директивой или регистром");
    else
        result.programName = label;

    Number address = parseHex(op1, 8);
    if (op1 != "" && !address.ok)
        addError(result, row, "Адрес «" + op1 + "» в Start: нужно шестнадцатеричное число, здесь только 0");
    else if (op1 != "" && address.value != 0)
        addError(result, row, "Перемещаемая программа начинается с адреса 0: в Start нужно 0 или пусто, а не «" + op1 + "»");
    if (op2 != "") addError(result, row, "У Start только один операнд - адрес 0");

    result.start = 0;
    Line line;
    line.kind = "START";
    line.row = row;
    line.mnemonic = "START";
    line.op1 = op1;
    line.op2 = op2;
    result.lines.push_back(line);
}

void parseEnd(Pass1Result& result, int row, string op1, string op2, long long counter) {
    result.entry = result.start;
    if (op2 != "") addError(result, row, "У End только один операнд - точка входа");
    if (op1 == "") return;

    long long entry = 0;
    int i = findSymbol(result.symbols, op1);
    if (i != -1) {
        entry = result.symbols[i].address;
    } else {
        Number address = parseHex(op1, 8);
        if (!address.ok) {
            if (isName(op1))
                addError(result, row, "Точка входа «" + op1 + "»: такой метки нет в ТСИ");
            else
                addError(result, row, "Точка входа «" + op1 + "»: нужна метка или шестнадцатеричный адрес");
            return;
        }
        entry = address.value;
    }
    if (entry < result.start || entry > counter)
        addError(result, row, "Точка входа " + op1 + " = " + hexAddress(entry) + " вне программы: нужен адрес от " +
                                  hexAddress(result.start) + " до " + hexAddress(counter));
    else
        result.entry = entry;
}

Pass1Result firstPass(Table source, Table opTable, int mode) {
    Pass1Result result;
    vector<OpCode> ops = readOpTable(opTable, result.errors);
    if (len(result.errors) > 0) return result;

    long long counter = 0;
    bool started = false, ended = false, overflow = false;

    for (int row = 0; row < len(source); row++) {
        if (isEmptyRow(source[row])) continue;
        string label = cell(source[row], 0);
        string mnemonic = upper(cell(source[row], 1));
        string op1 = cell(source[row], 2);
        string op2 = cell(source[row], 3);

        if (ended) {
            addError(result, row, "Строка после End: все строки программы должны быть до End");
            continue;
        }

        if (!started) {
            started = true;
            if (mnemonic == "START") {
                parseStart(result, ops, row, label, op1, op2);
                counter = result.start;
                continue;
            }
            addError(result, row, "Программа должна начинаться с директивы Start");
        }
        if (mnemonic == "START") {
            addError(result, row, "Директива Start может быть только в первой строке");
            continue;
        }
        if (mnemonic == "") {
            addError(result, row, label == "" ? "Не указана команда" : "Метка «" + label + "» стоит без команды");
            continue;
        }

        if (mnemonic == "END") {
            if (label != "")
                addError(result, row, "У End не может быть метки «" + label + "»: она указывала бы за конец программы");
            parseEnd(result, row, op1, op2, counter);
            result.endRow = row;
            ended = true;
            continue;
        }

        if (label != "") {
            string problem = checkLabel(label, ops, result);
            if (problem == "")
                result.symbols.push_back({label, counter, row});
            else
                addError(result, row, problem);
        }

        Line line;
        line.address = counter;
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
            int i = findOp(ops, mnemonic);
            if (i == -1) {
                addError(result, row, "Команды «" + cell(source[row], 1) + "» нет в таблице кодов операций");
                continue;
            }
            problem = parseCommand(line, ops[i], ops, mode);
        }

        if (counter + line.size > MEMORY_SIZE) {
            addError(result, row, "Программа выходит за пределы памяти (последний адрес FFFFFF)");
            overflow = true;
            break;
        }
        if (problem == "")
            result.lines.push_back(line);
        else
            addError(result, row, problem);
        counter += line.size;
    }

    if (!started) result.errors.push_back({"program", -1, "Исходный текст пуст - нечего ассемблировать"});
    if (started && !ended && !overflow) result.errors.push_back({"program", -1, "Нет директивы End в конце программы"});

    result.length = counter - result.start;
    result.ok = len(result.errors) == 0;
    return result;
}
