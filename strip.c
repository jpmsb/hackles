#include "strip.h"

float MeasureStripTextHeight(StripText *stripTexts, int maxWidth, CustomFont font, int baseFontSize, float scale){
    int font_size = (baseFontSize * scale) + 0.5;
    Vector2 strip_text_dimensions = { 0, 0 };
    
    // Measure the whole text block height
    for (int i = 0; i < stripTexts->textCount; i++) {
        Vector2 text_dimensions = MeasureTextEx(font.regular[font_size], stripTexts->texts[i]->text, font_size, 1);
        float text_height = font_size;

        if (text_dimensions.x > maxWidth) text_height = ceil(text_dimensions.x / maxWidth) * font_size;
        strip_text_dimensions.y += (!i) ? text_height : (text_height + font_size);
    }

    return strip_text_dimensions.y;
}

float CalculateAdjustedScale(Texture2D strip, float scale){
    if (strip.width * scale > GetScreenWidth()) {
        return (float)GetScreenWidth() / (float)strip.width;
    }
    return scale;
}

void SortStrips(StripData *strips, int stripAmount) {
    for (int i = 0; i < stripAmount; i++) {
        for (int j = i + 1; j < stripAmount; j++) {
            if (strips[i].number > strips[j].number) {
                StripData temp = strips[i];
                strips[i] = strips[j];
                strips[j] = temp;
            }
        }
    }
}

void DrawStrip(float x, float y, Texture2D strip, float scale) {
    float pos_x = x;
    float pos_y = y;
    float adjusted_scale = CalculateAdjustedScale(strip, scale);

    if (adjusted_scale != scale) {
        pos_x = 0;
    }

    Vector2 strip_position = { pos_x, pos_y };
    DrawTextureEx(strip, strip_position, 0, adjusted_scale, WHITE);
}

void DrawStripText(StripText *stripTexts, float x, float y, float maxWidth, float maxHeight, CustomFont font[], int baseFontSize, float scale){
    float x_pos = x;
    float y_pos = y;
    int font_size = (baseFontSize * scale) + 0.5;

    // Draw the text
    float previous_text_height = 0;
    for (int i = 0; i < stripTexts->textCount; i++) {
        Vector2 text_dimensions = MeasureTextEx(font[0].regular[font_size], stripTexts->texts[i]->text, font_size, 1);
        float text_height = 0;
        if (text_dimensions.x > maxWidth) text_height = ceil((text_dimensions.x / (float)maxWidth)) * font_size;
        else text_height = font_size;

        y_pos += previous_text_height;
        Rectangle rec = { x_pos, y_pos + (i * font_size), maxWidth, text_height };
        DrawTextBoxedSelectable(font, stripTexts->texts[i]->text, rec, font_size, 1, true, BLACK, -1, -1, BLACK, BLACK);
        previous_text_height = text_height;
    }
}

void DrawTextBoxedSelectable(CustomFont fontFamily[], const char *stripText, Rectangle rec, int fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint) {
    Font font = fontFamily[0].regular[fontSize];
    bool stylized = false;
    bool underline = false;
    bool monospaced = false;
    int underline_count = 0;
    bool hyperlink_title = false;
    bool hyperlink_url = false;
    int asterisk_count = 0;
    int previous_codepoint = 0;

    int length = TextLength(stripText);  // Total length in bytes of the text, scanned by codepoints in loop
    
    // Temporary solution to handle markdown hyperlinks
    // For now, we'll just draw the text between the brackets
    char *cleanned_text = (char *)malloc(length + 1);
    int cleanned_text_length = 0;
    for (int i = 0; i < length; i++) {
        if (stripText[i] == '[') hyperlink_title = true;
        else if (stripText[i] == ']') hyperlink_url = true;
        else if (stripText[i] == ')' && hyperlink_title) hyperlink_url = false;
        else if (hyperlink_title && hyperlink_url) continue; 
        else {
            cleanned_text[cleanned_text_length] = stripText[i];
            cleanned_text_length++;
        }
    }
    cleanned_text[cleanned_text_length] = '\0';
    
    char *text = (char *)malloc(cleanned_text_length + 1);
    strcpy(text, cleanned_text);
    length = TextLength(text);
    free(cleanned_text);
    /********************************************************************/

    float textOffsetY = 0;          // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize / (float)font.baseSize;     // Character rectangle scaling factor

    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++) {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        float glyphWidth = 0;
        if (codepoint != '\n') {
            glyphWidth = (font.glyphs[index].advanceX == 0) ? font.recs[index].width*scaleFactor : font.glyphs[index].advanceX*scaleFactor;

            if (i + 1 < length) glyphWidth = glyphWidth + spacing;
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE){
            // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // Ref: http://jkorpela.fi/chars/spaces.html
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth) > rec.width) {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            } else if ((i + 1) == length) {
                endLine = i;
                state = !state;
            } else if (codepoint == '\n') state = !state;

            if (state == DRAW_STATE) {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        } else {
            if (codepoint == '\n') {
                if (!wordWrap) {
                    textOffsetY += (font.baseSize) * scaleFactor;
                    textOffsetX = 0;
                }
            } else {
                if (!wordWrap && ((textOffsetX + glyphWidth) > rec.width)) {
                    textOffsetY += (font.baseSize) * scaleFactor;
                    textOffsetX = 0;
                }

                // When text overflows rectangle height limit, just stop drawing
                if ((textOffsetY + font.baseSize*scaleFactor) > rec.height) break;

                // Draw selection background
                bool isGlyphSelected = false;
                if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength))) {
                    DrawRectangleRec((Rectangle){ rec.x + textOffsetX - 1, rec.y + textOffsetY, glyphWidth, (float)font.baseSize*scaleFactor }, selectBackTint);
                    isGlyphSelected = true;
                }

                // Set the font syle based on Markdown standard
                // *italic* -> italic
                // **bold** -> bold
                // ***bold italic*** -> bold italic
                if ((codepoint != ' ') && (codepoint != '\t')) {
                    if (codepoint == '*') {
                        if ((asterisk_count > 0) && (stylized)) {
                            asterisk_count--;
                            continue;
                        } else {
                            asterisk_count++;
                            continue;
                        }
                    } else if (asterisk_count > 0) stylized = true;
                    else stylized = false;

                    if (stylized) {
                        if (asterisk_count == 1) font = fontFamily[0].italic[fontSize];
                        else if (asterisk_count == 2) font = fontFamily[0].bold[fontSize];
                        else if (asterisk_count == 3) font = fontFamily[0].boldItalic[fontSize];
                    } else font = fontFamily[0].regular[fontSize];

                    // Draw a line under the text if the text is __underlined__
                    if (codepoint == '_') {
                        if (previous_codepoint == '_') underline_count++;
                        else underline_count = 0;

                        if (underline_count == 1) {
                            underline = !underline;
                        } 
                        previous_codepoint = codepoint;
                        continue;
                    } else previous_codepoint = codepoint;

                    if (underline) {
                        DrawLine(rec.x + textOffsetX, rec.y + textOffsetY + (font.baseSize*scaleFactor), rec.x + textOffsetX + glyphWidth, rec.y + textOffsetY + (font.baseSize*scaleFactor), tint);
                    }

                    // For Mono Style, for now, we'll just draw the text between the grave accents
                    // `monospace` -> monospace
                    if (codepoint == '`') {
                        if (monospaced) monospaced = false;
                        else monospaced = true;
                        continue;
                    }
                    if (monospaced) font = fontFamily[1].regular[fontSize];

                    // Draw current character glyph
                    DrawTextCodepoint(font, codepoint, (Vector2){ rec.x + textOffsetX, rec.y + textOffsetY }, fontSize, isGlyphSelected? selectTint : tint);
                }
            }

            if (wordWrap && (i == endLine)) {
                textOffsetY += (font.baseSize) * scaleFactor;
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                selectStart += lastk - k;
                k = lastk;

                state = !state;
            }
        }

        if ((textOffsetX != 0) || (codepoint != ' ')) textOffsetX += glyphWidth;  // avoid leading spaces
    }
}

void DrawStripAndText(int x, int y, StripData *stripData, int count, CustomFont font[], int baseFontSize, float baseScale, float maxWidth, float maxHeight) {
    if (stripData->texture.height > stripData->texture.width / 2) {
        // Draw the strip image
        float image_scale = (float)maxHeight / (float)stripData->texture.height;
        float x_strip = x - (stripData->texture.width * image_scale);
        if (x_strip < 0) x_strip = 0;
        DrawStrip(x_strip, y, stripData->texture, image_scale);

        // Draw the strip description
        float text_scale = 1;
        if (baseScale > 1) text_scale += baseScale * 0.2;
        float strip_text_pos_x = x_strip + stripData->texture.width * image_scale;
        float text_max_width = maxWidth - strip_text_pos_x - x_strip;
        float text_max_height = maxHeight;

        float text_height = MeasureStripTextHeight(&stripData->stripText, text_max_width, font[0], baseFontSize, text_scale);
        float y_strip = y + (maxHeight / 2) - (text_height / 2);
        DrawStripText(&stripData->stripText, strip_text_pos_x, y_strip, text_max_width, text_max_height, font, baseFontSize, text_scale);
    } else {
        // Draw the strip image
        float image_max_height = maxHeight * 0.7;
        float total_text_height = MeasureStripTextHeight(&stripData->stripText, maxWidth, font[0], baseFontSize, baseScale);
        float image_scale = (float)image_max_height / (float)stripData->texture.height;

        // Adjust the image scale if the text is too tall
        if (total_text_height + image_max_height > maxHeight) {
            image_max_height = maxHeight - total_text_height * 0.85;
            image_scale = (float)image_max_height / (float)stripData->texture.height;
        }

        float x_strip = x - (stripData->texture.width * image_scale) / 2;
        DrawStrip(x_strip, y, stripData->texture, image_scale);

        // Draw the strip description
        float current_image_scale = CalculateAdjustedScale(stripData->texture, image_scale);
        float max_width = maxWidth - 20;
        float max_height = maxHeight - image_max_height;
        float text_scale = 1;
        if (baseScale > 1) text_scale += baseScale * 0.1;
        float strip_text_pos_y = y + stripData->texture.height * current_image_scale + 10;
        DrawStripText(&stripData->stripText, 10, strip_text_pos_y, max_width, max_height, font, baseFontSize, text_scale);
    }
}