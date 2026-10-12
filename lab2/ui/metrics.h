#pragma once

#include "../platform/winapi.h"

int screenDpi = 96;

int S(int pixels) { return MulDiv(pixels, screenDpi, 96); }
