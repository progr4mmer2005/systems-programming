// Второй проход.
//
// Всё, кроме адресов меток, уже переведено в код на первом проходе.
// Теперь ТСИ заполнена целиком, поэтому адрес любой метки известен.
// На место "····" пишется:
//   - прямая адресация (метка): адрес метки, а команда попадает в таблицу
//     настройки - при загрузке этот адрес надо будет сдвинуть;
//   - относительная адресация ([метка]): смещение = адрес метки - адрес
//     следующей команды. При сдвиге программы оно не меняется, поэтому
//     в таблицу настройки такая команда не попадает.
#pragma once

#include <string>

using namespace std;

#include "encoding.h"
#include "program.h"

// "LD R1 One" - исходная команда для столбца "Команда".
string sourceText(Line line) {
    string s = line.mnemonic;
    if (line.op1 != "") s += " " + line.op1;
    if (line.op2 != "") s += " " + line.op2;
    return s;
}

Pass2Result secondPass(Pass1Result pass1) {
    Pass2Result result;
    result.programName = pass1.programName;
    result.length = pass1.length;
    result.start = pass1.start;

    for (Line line : pass1.lines) {
        CodeLine out;
        out.address = line.address;
        out.size = line.size;
        out.code = line.code;
        out.source = sourceText(line);
        out.reserve = line.reserve;
        out.row = line.row;
        if (line.reserve) out.source += "  (резерв)";

        if (line.label != "") {
            string field = "????";
            int i = findSymbol(pass1.symbols, line.label);
            if (i == -1) {
                result.errors.push_back({"source", line.row, "Метка «" + line.label + "» не определена"});
                out.unresolved = true;
            } else if (line.addressing == ADDRESSING_RELATIVE) {
                int next = line.address + line.size;  // адрес следующей команды
                int offset = pass1.symbols[i].address - next;
                if (offset < -32768 || offset > 32767) {
                    result.errors.push_back({"source", line.row, "Смещение до метки «" + line.label + "» не помещается в 2 байта"});
                    out.unresolved = true;
                } else {
                    field = hex(offset & 0xFFFF, ADDRESS_DIGITS);  // отрицательное - в дополнительном коде: -6 -> FFFA
                }
            } else {
                field = addressField(pass1.symbols[i].address);
                result.relocations.push_back({line.address, sourceText(line), line.row});
            }
            out.code = replaceFirst(out.code, ADDRESS_GAP, field);
        }
        result.code.push_back(out);
    }

    result.ok = len(result.errors) == 0;
    return result;
}
