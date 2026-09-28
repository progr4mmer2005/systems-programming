// Второй проход.
//
// Всё, кроме адресов меток, уже переведено в код на первом проходе.
// Теперь ТСИ заполнена целиком, поэтому адрес любой метки известен:
// подставляем его на место "····".
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
            string address;
            int i = findSymbol(pass1.symbols, line.label);
            if (i != -1) {
                address = addressField(pass1.symbols[i].address);
            } else {
                result.errors.push_back({"source", line.row, "Метка «" + line.label + "» не определена"});
                address = "????";
                out.unresolved = true;
            }
            out.code = replaceFirst(out.code, ADDRESS_GAP, address);
        }
        result.code.push_back(out);
    }

    result.ok = len(result.errors) == 0;
    return result;
}
