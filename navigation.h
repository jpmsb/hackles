#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "raylib/raylib.h"
#include "common_types.h"

#define NAV_FIRST "nav_first_on.png"
#define NAV_LAST "nav_last_on.png"
#define NAV_RANDOM "nav_random_on.png"
#define NAV_NEXT "nav_next_on.png"
#define NAV_PREVIOUS "nav_previous_on.png"

typedef struct Button {
    Texture2D texture;
    char text[16];
} Button;

typedef struct Navigation {
    Button first;
    Button last;
    Button next;
    Button previous;
    Button random;
} Navigation;

typedef struct ButtonData {
    Button button;
    Rectangle container;
    Rectangle scaledContainer;
    Vector2 texturePosition;
    Vector2 textPosition;
    Vector2 scaledTexturePosition;
    Vector2 scaledTextPosition;
    Font font;
    Font scaledFont;
    int fontSize;
    int scaledFontSize;
    float scale;
    float textureScale;
    float textureScaleSelected;
    float selectedButtonScale;
} ButtonData;

typedef struct NavigationButtonsGeometryArray {
    ButtonData first;
    ButtonData last;
    ButtonData next;
    ButtonData previous;
    ButtonData random;
    Rectangle container;
    Rectangle scaledContainer;
} NavigationButtonsGeometryArray;

bool IsAnyKeyDown(int keys[]);

ButtonData CalculateButtonData(float x, float y, int maxWidth, int maxHeight, float scale, float selectedButtonScale, Button button, CustomFont font);

void DrawButton(ButtonData buttonData, bool selected, bool clicked);

NavigationButtonsGeometryArray CalculateNavigationButtonsGeometry(float x, float y, int maxButtonWidth, float maxButtonHeight, float navScale, float selectedButtonScale, Navigation navigation, CustomFont font);

void DrawNavigationButtons(NavigationButtonsGeometryArray navigationButtons, int buttonID, bool clicked);

int DrawNavigation(NavigationButtonsGeometryArray navigationButtons, Vector2 mousePosition, int stripAmount, int count, float selectedButtonScale, int navigationKeys[]);

#endif