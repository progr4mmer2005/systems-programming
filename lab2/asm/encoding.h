#pragma once

#include <string>

using namespace std;

#include "../util/text.h"

const int ADDRESS_DIGITS = 6;
const long long MEMORY_SIZE = 0x1000000;
const int MAX_OPCODE = 0x3F;
const int WORD_SIZE = 3;
const long long MAX_WORD = 0xFFFFFF;

const int ADDRESSING_NONE = 0;
const int ADDRESSING_DIRECT = 1;
const int ADDRESSING_RELATIVE = 2;

const int MODE_BOTH = 0;
const int MODE_DIRECT = 1;
const int MODE_RELATIVE = 2;

string modeName(int mode) {
    if (mode == MODE_DIRECT) return "только прямая";
    if (mode == MODE_RELATIVE) return "только относительная";
    return "прямая и относительная";
}

int makeFirstByte(int code, int addressing) { return code * 4 + addressing; }

int addressDigits(int format) { return format == 3 ? 4 : 6; }

string hexAddress(long long address) { return hex(address, ADDRESS_DIGITS); }

string hexLength(long long size) {
    if (size <= 0xFF) return hex(size, 2);
    if (size <= 0xFFFF) return hex(size, 4);
    return hex(size, 6);
}
