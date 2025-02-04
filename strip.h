#ifndef STRIP_H
#define STRIP_H

#include "raylib/raylib.h"
#include "common_types.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ELEMENTS 512
#define MAX_TEXT_BLOCKS 8
#define MAX_LANGUAGES 2

typedef struct Text {
    char text[8192];
} Text;

typedef struct StripText {
    int textCount;
    char date[32];
    struct Text *texts[MAX_TEXT_BLOCKS];
} StripText;

typedef struct StripData {
    char name[16];
    int number;
    Texture2D texture;
    struct StripText stripText;
} StripData;

typedef struct StripCollection {
    char language[8];
    int stripAmount;
    struct StripData strips[MAX_ELEMENTS];
} StripCollection;

float MeasureStripTextHeight(StripText *stripTexts, int maxWidth, CustomFont font, int baseFontSize, float scale);

float CalculateAdjustedScale(Texture2D strip, float scale);

void SortStrips(StripData *strips, int stripAmount);

void DrawStrip(float x, float y, Texture2D strip, float scale);

void DrawStripText(StripText *stripTexts, float x, float y, float maxWidth, float maxHeight, CustomFont font[], int baseFontSize, float scale);

// Adaptation from the Raylib example below:
// https://github.com/raysan5/raylib/blob/master/examples/text/text_rectangle_bounds.c
void DrawTextBoxedSelectable(CustomFont fontFamily[], const char *stripText, Rectangle rec, int fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint);

void DrawStripAndText(float x, float y, StripData *stripData, int count, CustomFont font[], int baseFontSize, float baseScale, float maxWidth, float maxHeight);

#endif