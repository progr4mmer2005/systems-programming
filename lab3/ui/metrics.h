// Размеры с учётом масштаба экрана.
// S(10) - это 10 пикселей при масштабе 100%, 15 пикселей при 150% и т. д.
#pragma once

#include "../platform/winapi.h"

int screenDpi = 96;  // 96 - это масштаб 100%

int S(int pixels) { return MulDiv(pixels, screenDpi, 96); }
