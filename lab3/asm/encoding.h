// Как команда превращается в байты.
//
// Формат команды = её длина в байтах (1, 2, 3 или 4):
//
//   формат 1:  КОП                    HALT          -> 24
//   формат 2:  КОП  R1R2  / КОП число  ADD R1 R2     -> 0C 12     INT 5 -> 1C 05
//   формат 3:  КОП  адрес              CALL Show     -> 19 0012
//   формат 4:  КОП  регистр  адрес     LD R1 One     -> 05 01 0015
//
// Первый байт (КОП) собирается из битов:
//
//     7 6 5 4 3 2 | 1 0
//     код операции | тип адресации
//
//   тип адресации: 00 - без адреса (форматы 1 и 2)
//                  01 - прямая, операнд "метка": в команде адрес метки
//                  10 - относительная, операнд "[метка]": в команде смещение
//                       до метки от следующей команды
//
// Поэтому код операции в ТКО - от 00 до 3F: он должен влезть в 6 бит.
#pragma once

#include <string>

using namespace std;

#include "../util/text.h"

const int ADDRESS_DIGITS = 4;     // адрес - 4 hex-цифры
const int MEMORY_SIZE = 0x10000;  // память 64 КБ: адреса 0000..FFFF
const int MAX_OPCODE = 0x3F;      // код операции - 6 бит

const int ADDRESSING_NONE = 0;      // биты 00
const int ADDRESSING_DIRECT = 1;    // биты 01
const int ADDRESSING_RELATIVE = 2;  // биты 10

// Место под адрес метки: его заполнит второй проход.
const string ADDRESS_GAP = "····";

// Код операции сдвигаем на 2 бита влево (умножить на 4) и дописываем тип адресации.
// LD (код 01), прямая адресация (01): 1 * 4 + 1 = 5 = 000001|01
int makeFirstByte(int code, int addressing) { return code * 4 + addressing; }

// Первый байт в двоичном виде, код и тип адресации раздельно: 5 -> "000001 01"
string firstByteBits(int byte) {
    string bits = "";
    for (int i = 0; i < 8; i++) {
        bits = to_string(byte % 2) + bits;
        byte = byte / 2;
        if (i == 1) bits = " " + bits;  // пробел между кодом и типом адресации
    }
    return bits;
}

string hexAddress(long long address) { return hex(address, ADDRESS_DIGITS); }
