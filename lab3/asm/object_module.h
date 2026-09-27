// Объектный модуль в полном перемещаемом формате - текст для файла.
//
// Порядок разделов по методичке: заголовок, таблица внешних имён, список
// внешних ссылок, таблица настройки, тело с кодом. Каждая строка начинается
// с буквы раздела:
//   H имя длина адрес       - заголовок
//   D имя адрес             - внешнее имя (EXTDEF)
//   R имя                   - внешняя ссылка (EXTREF)
//   M адрес [имя]           - строка таблицы настройки
//   T адрес байты           - код (резерв памяти без кода не пишется)
#pragma once

#include <string>

using namespace std;

#include "encoding.h"
#include "program.h"

string objectModuleText(Pass2Result module) {
    string text;
    text += "H " + module.programName + " " + hexAddress(module.length) + " " + hexAddress(module.start) + "\n";
    for (Symbol name : module.externalNames) text += "D " + name.name + " " + hexAddress(name.address) + "\n";
    for (Symbol ref : module.externalRefs) text += "R " + ref.name + "\n";
    for (Relocation relocation : module.relocations) {
        text += "M " + hexAddress(relocation.address);
        if (relocation.name != "") text += " " + relocation.name;
        text += "\n";
    }
    for (CodeLine line : module.code) {
        if (!line.reserve) text += "T " + hexAddress(line.address) + " " + line.code + "\n";
    }
    return text;
}
