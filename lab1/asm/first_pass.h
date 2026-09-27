// Первый проход.
//
// Идём по строкам исходного текста со счётчиком адреса (counter):
//   - метку записываем в ТСИ с текущим адресом;
//   - команду сразу переводим в код: первый байт, регистры, числа.
//     Не хватает только адресов меток - вместо них "····";
//   - константы WORD / BYTE тоже сразу переводим в байты;
//   - counter увеличиваем на длину строки.
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

// Метка не может быть зарезервированным словом (регистр, директива, команда из ТКО)
// и не может повторяться. "" - метка правильная, иначе текст ошибки.
string checkLabel(string label, vector<OpCode> ops, Pass1Result result) {
    if (!isName(label)) return "Метка «" + label + "»: только латинские буквы, цифры и «_», первая — буква";
    if (len(label) > MAX_NAME_LENGTH) return "Метка «" + label + "» длиннее " + to_string(MAX_NAME_LENGTH) + " символов";
    if (isRegister(label)) return "Метка «" + label + "» совпадает с именем регистра";
    if (isDirective(label) || findOp(ops, label) != -1) return "Метка «" + label + "» совпадает с названием команды или директивы";
    if (result.programName != "" && sameName(label, result.programName)) return "Метка «" + label + "» совпадает с именем программы";
    int i = findSymbol(result.symbols, label);
    if (i != -1) return "Метка «" + label + "» уже определена в строке " + to_string(result.symbols[i].row + 1);
    return "";
}

// WORD n - слово из 2 байт; WORD ? - просто 2 байта места.
string parseWord(Line& line) {
    line.kind = "WORD";
    line.size = 2;
    if (line.op2 != "") return "У WORD только один операнд";
    if (line.op1 == "") return "У WORD нет значения: нужно число или «?»";
    if (line.op1 == "?") {
        line.reserve = true;
        return "";
    }
    Number n = parseDecimal(line.op1);
    if (!n.ok || n.value < -32768 || n.value > 65535) return "WORD «" + line.op1 + "»: нужно целое число от -32768 до 65535 или «?»";
    line.code = hex(n.value & 0xFFFF, 4);  // & 0xFFFF - отрицательное число в дополнительном коде: -1 -> FFFF
    return "";
}

// BYTE "текст" - по байту на символ; BYTE n - один байт 0..255; BYTE ? - байт места.
string parseByte(Line& line) {
    line.kind = "BYTE";
    if (line.op1 == "?") {
        line.reserve = true;
        line.size = 1;
        return line.op2 == "" ? "" : "У BYTE только один операнд";
    }
    TextBytes text = parseText(line.op1);
    if (text.isText) {
        line.size = len(text.bytes);
        line.code = bytesToHex(text.bytes);
        if (text.problem != "") return "BYTE " + line.op1 + ": " + text.problem;
    } else {
        Number n = parseDecimal(line.op1);
        if (!n.ok || n.value < 0 || n.value > 255) return "BYTE «" + line.op1 + "»: нужно \"строка\" в двойных кавычках, число 0..255 или «?»";
        line.size = 1;
        line.code = hex(n.value, 2);
    }
    return line.op2 == "" ? "" : "У BYTE только один операнд";
}

// Команда: формат и код - из ТКО, операнды переводим в байты.
string parseCommand(Line& line, OpCode op) {
    line.kind = "command";
    line.format = op.length;
    line.size = op.length;

    string kind = operandKind(op.length, line.op1, line.op2);
    if (kind == "")
        return op.name + " (формат " + to_string(op.length) + "): ожидается " + expectedOperands(op.length) +
               ", а записано «" + joinOperands(line.op1, line.op2) + "»";

    // Есть ли в операндах метка и какая адресация.
    string labelOperand = "";
    if (kind == "label") labelOperand = line.op1;
    if (kind == "register+label") labelOperand = line.op2;
    int addressing = ADDRESSING_NONE;
    if (labelOperand != "") {
        addressing = isRelativeLabel(labelOperand) ? ADDRESSING_RELATIVE : ADDRESSING_DIRECT;
        line.label = withoutBrackets(labelOperand);
    }
    if (addressing == ADDRESSING_RELATIVE && !ALLOW_RELATIVE)
        return op.name + " " + joinOperands(line.op1, line.op2) + ": [метка] — относительная адресация, она появится в лабе 2";

    // Собираем код: первый байт, потом операнды.
    line.firstByte = makeFirstByte(op.code, addressing);
    line.code = hex(line.firstByte, 2);
    if (kind == "registers") {  // два регистра в одном байте: R1 R2 -> 12
        line.code += " " + hex(registerNumber(line.op1) * 16 + registerNumber(line.op2), 2);
    }
    if (kind == "number") {
        line.code += " " + hex(parseSmallNumber(line.op1).value, 2);
    }
    if (kind == "label") {
        line.code += " " + ADDRESS_GAP;
    }
    if (kind == "register+label") {
        line.code += " " + hex(registerNumber(line.op1), 2) + " " + ADDRESS_GAP;
    }
    return "";
}

Pass1Result firstPass(Table source, Table opTable) {
    Pass1Result result;
    vector<OpCode> ops = readOpTable(opTable, result.errors);
    if (len(result.errors) > 0) return result;  // с ошибочной ТКО исходный текст не разбираем

    int counter = 0;  // счётчик адреса
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

        // ---- Start: первая строка программы. Метка - имя программы, операнд - адрес загрузки.
        if (!started) {
            started = true;
            if (mnemonic == "START") {
                if (label == "")
                    addError(result, row, "У Start нет метки — это имя программы, оно обязательно");
                else if (!isName(label) || len(label) > MAX_NAME_LENGTH)
                    addError(result, row, "Имя программы «" + label + "»: латинские буквы, цифры и «_», до 10 символов");
                else if (findOp(ops, label) != -1 || isDirective(label) || isRegister(label))
                    addError(result, row, "Имя программы «" + label + "» совпадает с командой, директивой или регистром");
                else
                    result.programName = label;

                // Ведущие нули разрешены: 00001000 = 1000.
                Number address = parseHex(op1, 8);
                if (address.ok && address.value >= MEMORY_SIZE) address.ok = false;
                if (op1 == "")
                    addError(result, row, "Не задан адрес загрузки (операнд Start, шестнадцатеричный)");
                else if (!address.ok)
                    addError(result, row, "Адрес загрузки «" + op1 + "»: нужно шестнадцатеричное число от 0001 до FFFF");
                else if (address.value == 0)
                    addError(result, row, "Адрес загрузки абсолютной программы не может быть 0 (с нуля начинается перемещаемая программа)");
                if (op2 != "") addError(result, row, "У Start только один операнд — адрес загрузки");
                counter = address.ok ? (int)address.value : 0;
                result.start = counter;
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

        // ---- End: конец программы.
        if (mnemonic == "END") {
            if (label != "") addError(result, row, "У End не может быть метки");
            if (op1 != "" || op2 != "") addError(result, row, "У End нет операндов");
            ended = true;
            continue;
        }

        // ---- Метка -> ТСИ.
        if (label != "") {
            string problem = checkLabel(label, ops, result);
            if (problem == "")
                result.symbols.push_back({label, counter, row});
            else
                addError(result, row, problem);
        }

        // ---- Команда или данные -> вспомогательная таблица.
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
        } else {
            int i = findOp(ops, mnemonic);
            if (i == -1) {
                addError(result, row, "Команды «" + cell(source[row], 1) + "» нет в таблице кодов операций");
                continue;  // длина неизвестна - счётчик адреса не двигаем
            }
            problem = parseCommand(line, ops[i]);
        }

        if (counter + line.size > MEMORY_SIZE) {
            addError(result, row, "Программа выходит за пределы памяти (последний адрес FFFF)");
            overflow = true;
            break;
        }
        if (problem == "")
            result.lines.push_back(line);
        else
            addError(result, row, problem);
        counter += line.size;  // даже у строки с ошибкой размер известен - адреса дальше не съедут
    }

    if (!started) result.errors.push_back({"program", -1, "Исходный текст пуст — нечего ассемблировать"});
    if (started && !ended && !overflow) result.errors.push_back({"program", -1, "Нет директивы End в конце программы"});

    result.length = counter - result.start;
    result.ok = len(result.errors) == 0;
    return result;
}
