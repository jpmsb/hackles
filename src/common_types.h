#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "raylib/raylib.h"

#define MAX_FONT_AMOUNT 64

typedef struct CustomFont {
    Font regular[MAX_FONT_AMOUNT];
    Font bold[MAX_FONT_AMOUNT];
    Font italic[MAX_FONT_AMOUNT];
    Font boldItalic[MAX_FONT_AMOUNT];
} CustomFont;

#endif