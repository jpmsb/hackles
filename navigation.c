#include "navigation.h"

bool IsAnyKeyDown(int keys[]){
    for (int i = 0; i < 5; i++) {
        if (IsKeyDown(keys[i])) return true;
    }
    return false;
}

ButtonData CalculateButtonData(float x, float y, int maxWidth, int maxHeight, float scale, float selectedButtonScale, Button button, CustomFont font){
    float container_additional_scale = 1.1;
    ButtonData button_geometry;
    button_geometry.button = button;
    button_geometry.scale = scale;
    button_geometry.texturePosition = (Vector2){ x - (maxWidth * scale) / 2, y - (maxHeight * scale) / 2 };
    button_geometry.container.x = button_geometry.texturePosition.x;
    button_geometry.container.y = button_geometry.texturePosition.y;

    // Calculate the button texture to match the max width and height
    float x_scale = (float)maxWidth / (float)button.texture.width;
    float y_scale = (float)maxHeight / (float)button.texture.height;
    button_geometry.textureScale = x_scale < y_scale ? x_scale : y_scale;
    button_geometry.textureScale *= scale;
    button_geometry.selectedButtonScale = selectedButtonScale;
    button_geometry.textureScaleSelected = button_geometry.textureScale * button_geometry.selectedButtonScale;    

    // Text parameters
    int font_size = 10 * scale;
    Vector2 text_size = MeasureTextEx(font.regular[font_size], button.text, font_size, 1);
    button_geometry.textPosition = (Vector2){ button_geometry.container.x + (maxWidth * scale) / 2 - text_size.x / 2, button_geometry.container.y + maxHeight * scale * container_additional_scale };

    button_geometry.fontSize = font_size;
    button_geometry.font = font.regular[font_size];

    // Calculate the button geometry
    float button_y_size = maxHeight * scale + text_size.y;
    float button_x_size = 0;
    if (text_size.x > maxWidth * scale) {
        button_geometry.container.x = button_geometry.texturePosition.x - (text_size.x - maxWidth * scale) / 2;
        button_x_size = text_size.x;
    } else {
        button_x_size = maxWidth * scale;
    }
    button_geometry.container.width = button_x_size;
    button_geometry.container.height = button_y_size;

    // Calculate the scaled selected container dimensions and start coordinates (texture + text)
    float selected_container_width = button_x_size * button_geometry.selectedButtonScale * container_additional_scale;
    float selected_container_height = button_y_size * button_geometry.selectedButtonScale * container_additional_scale;
    button_geometry.scaledContainer = (Rectangle){ 
        button_geometry.container.x - ((selected_container_width - button_x_size) / 2),
        button_geometry.container.y - ((selected_container_height - button_y_size) / 2),
        selected_container_width,
        selected_container_height
    };

    // Calculate the scaled font size
    button_geometry.scaledFontSize = font_size * selectedButtonScale;
    button_geometry.scaledFont = font.regular[button_geometry.scaledFontSize];

    // Calculate the scaled texture position
    button_geometry.scaledTexturePosition = (Vector2){ button_geometry.scaledContainer.x + button_geometry.scaledContainer.width / 2 - button_geometry.textureScaleSelected * button.texture.width / 2, button_geometry.scaledContainer.y + button_geometry.scaledContainer.height / 2 - button_geometry.textureScaleSelected * button.texture.height / 2 - button_geometry.scaledFontSize / 2 };

    // Calculate the scaled text position
    Vector2 scaled_text_size = MeasureTextEx(button_geometry.scaledFont, button.text, button_geometry.scaledFontSize, 1);
    button_geometry.scaledTextPosition = (Vector2){ button_geometry.scaledContainer.x + button_geometry.scaledContainer.width / 2 - scaled_text_size.x/2, button_geometry.container.y + maxHeight * scale * selectedButtonScale };

    return button_geometry;
}

void DrawButton(ButtonData buttonData, bool selected, bool clicked){
    if (selected){
        if (clicked) {
            DrawRectangleRec(buttonData.scaledContainer, GRAY);
        } else {
            DrawRectangleRec(buttonData.scaledContainer, LIGHTGRAY);
        }

        DrawTextureEx(buttonData.button.texture, buttonData.scaledTexturePosition, 0, buttonData.textureScaleSelected, WHITE);
        DrawTextEx(buttonData.scaledFont, buttonData.button.text, buttonData.scaledTextPosition, buttonData.scaledFontSize, 1, BLACK);
    } else {
        DrawRectangleRec(buttonData.container, WHITE);
        DrawTextureEx(buttonData.button.texture, buttonData.texturePosition, 0, buttonData.textureScale, WHITE);
        DrawTextEx(buttonData.font, buttonData.button.text, buttonData.textPosition, buttonData.fontSize, 1, BLACK);
    }
}

NavigationButtonsGeometryArray CalculateNavigationButtonsGeometry(float x, float y, int maxButtonWidth, float maxButtonHeight, float navScale, float selectedButtonScale, Navigation navigation, CustomFont font) {
    int space_between_buttons = maxButtonWidth * 2 * navScale;
    NavigationButtonsGeometryArray buttons_geometry;

    // Calculate the buttons geometry
    buttons_geometry.first = CalculateButtonData(x - space_between_buttons * 2, y, maxButtonWidth, maxButtonHeight, navScale, selectedButtonScale, navigation.first, font);
    buttons_geometry.previous = CalculateButtonData(x - space_between_buttons, y, maxButtonWidth, maxButtonHeight, navScale, selectedButtonScale, navigation.previous, font);
    buttons_geometry.random = CalculateButtonData(x, y, maxButtonWidth, maxButtonHeight, navScale, selectedButtonScale, navigation.random, font);
    buttons_geometry.next = CalculateButtonData(x + space_between_buttons, y, maxButtonWidth, maxButtonHeight, navScale, selectedButtonScale, navigation.next, font);
    buttons_geometry.last = CalculateButtonData(x + space_between_buttons * 2, y, maxButtonWidth, maxButtonHeight, navScale, selectedButtonScale, navigation.last, font);

    buttons_geometry.container.x = buttons_geometry.first.container.x;
    buttons_geometry.container.y = buttons_geometry.first.container.y;

    buttons_geometry.container.height = (buttons_geometry.first.container.height + buttons_geometry.previous.container.height + buttons_geometry.random.container.height + buttons_geometry.next.container.height + buttons_geometry.last.container.height) / 5;
    buttons_geometry.container.width = buttons_geometry.last.container.x - buttons_geometry.first.container.x + buttons_geometry.last.container.width;

    buttons_geometry.scaledContainer = (Rectangle){ 
        buttons_geometry.first.scaledContainer.x,
        buttons_geometry.first.scaledContainer.y,
        buttons_geometry.last.scaledContainer.x - buttons_geometry.first.scaledContainer.x + buttons_geometry.last.scaledContainer.width,
        (buttons_geometry.first.scaledContainer.height + buttons_geometry.previous.scaledContainer.height + buttons_geometry.random.scaledContainer.height + buttons_geometry.next.scaledContainer.height + buttons_geometry.last.scaledContainer.height) / 5
    };

    return buttons_geometry;
}

void DrawNavigationButtons(NavigationButtonsGeometryArray navigationButtons, int buttonID, bool clicked){
    if (buttonID == 1) DrawButton(navigationButtons.first, true, clicked);
    else DrawButton(navigationButtons.first, false, clicked);

    if (buttonID == 2) DrawButton(navigationButtons.previous, true, clicked);
    else DrawButton(navigationButtons.previous, false, clicked);

    if (buttonID == 3) DrawButton(navigationButtons.random, true, clicked);
    else DrawButton(navigationButtons.random, false, clicked);

    if (buttonID == 4) DrawButton(navigationButtons.next, true, clicked);
    else DrawButton(navigationButtons.next, false, clicked);

    if (buttonID == 5) DrawButton(navigationButtons.last, true, clicked);
    else DrawButton(navigationButtons.last, false, clicked);
}

int DrawNavigation(NavigationButtonsGeometryArray navigationButtons, Vector2 mousePosition, int stripAmount, int count, float selectedButtonScale, int navigationKeys[]){
    // Ensure the count is always within the strip amount
    if (count >= stripAmount) count = stripAmount - 1;

    // Mouse navigation
    if (! IsAnyKeyDown(navigationKeys)){
        if (CheckCollisionPointRec(mousePosition, navigationButtons.first.container)){
            DrawNavigationButtons(navigationButtons, 1, false);

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) DrawNavigationButtons(navigationButtons, 1, true);
            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                return 0;
            } 

        } else if (CheckCollisionPointRec(mousePosition, navigationButtons.previous.container)) {
            DrawNavigationButtons(navigationButtons, 2, false);
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) DrawNavigationButtons(navigationButtons, 2, true);
            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                if (count > 0) {
                    return --count;
                } else {
                    return stripAmount - 1;
                }
            }

        } else if (CheckCollisionPointRec(mousePosition, navigationButtons.random.container)) {
            DrawNavigationButtons(navigationButtons, 3, false);

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) DrawNavigationButtons(navigationButtons, 3, true);
            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) return GetRandomValue(0, stripAmount - 1);

        } else if (CheckCollisionPointRec(mousePosition, navigationButtons.next.container)) {
            DrawNavigationButtons(navigationButtons, 4, false);

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) DrawNavigationButtons(navigationButtons, 4, true);
            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                if (count < stripAmount - 1) {
                    return ++count;
                } else {
                    return 0;
                }
            }

        } else if (CheckCollisionPointRec(mousePosition, navigationButtons.last.container)) {
            DrawNavigationButtons(navigationButtons, 5, false);

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) DrawNavigationButtons(navigationButtons, 5, true);
            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) return stripAmount - 1;
        } else {
            DrawNavigationButtons(navigationButtons, 0, false);
        }
    } else {
        // Keyboard navigation
        if (IsKeyDown(navigationKeys[1])) {
            DrawNavigationButtons(navigationButtons, 2, true);
            if (IsKeyPressed(navigationKeys[1])){
                if (count > 0) {
                    return --count;
                } else {
                    return stripAmount - 1;
                }
            }
        } else if (IsKeyDown(navigationKeys[3])) {
            DrawNavigationButtons(navigationButtons, 4, true);
            if (IsKeyPressed(navigationKeys[3])) {
                if (count < stripAmount - 1) {
                    return ++count;
                } else {
                    return 0;
                }
            }
        }
    }

    return count;
}