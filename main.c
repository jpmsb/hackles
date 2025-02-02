#include "raylib/raylib.h"
#include "navigation.h"
#include "strip.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void) {
    char strips_dir[] = "strips";
    char strips_image_dir[] = "images";
    char strips_text_dir[] = "texts";
    char nav_dir[] = "images/navigation";
    char window_icon[] = "images/logohackles.png";

    struct dirent *de;
    DIR *strip_dir_ptr = opendir(strips_dir);
    DIR *nav_dir_ptr = opendir(nav_dir);

    // Initialization
    const int screenWidth = 1024;
    const int screenHeight = 768;
    const int screenMinWidth = 1024;
    const int screenMinHeight = 768;

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Hackles");
    SetWindowMinSize(screenMinWidth, screenMinHeight);
    SetTargetFPS(60);

    // Load the window icon
    Image icon = LoadImage(window_icon);
    SetWindowIcon(icon);

    StripCollection strips[MAX_LANGUAGES];
    
    // Recursively load the strips and texts into memory
    int language_index = 0;
    if (strip_dir_ptr != NULL) {
        while ((de = readdir(strip_dir_ptr)) != NULL) {
            strips[language_index].stripAmount = 0;

            // Storing the language
            strcpy(strips[language_index].language, de->d_name);
            
            // Checking if it's a directory
            if (strcmp(de->d_name, ".") && strcmp(de->d_name, "..")) {
                struct stat st;
                char full_language_path[strlen(strips_dir) + strlen(de->d_name) + 2];
                strcpy(full_language_path, strips_dir);
                strcat(full_language_path, "/");
                strcat(full_language_path, de->d_name);
                stat(full_language_path, &st);

                // If it's a directory
                if (S_ISDIR(st.st_mode)) {
                    // Full strips dir path
                    char full_image_dir_path[strlen(full_language_path) + strlen(strips_image_dir) + 2];
                    strcpy(full_image_dir_path, full_language_path);
                    strcat(full_image_dir_path, "/");
                    strcat(full_image_dir_path, strips_image_dir);

                    // Full text dir path
                    char text_full_dir_path[strlen(full_language_path) + strlen(strips_text_dir) + 2];
                    strcpy(text_full_dir_path, full_language_path);
                    strcat(text_full_dir_path, "/");
                    strcat(text_full_dir_path, strips_text_dir);

                    struct dirent *strip_de;
                    DIR *strip_image_dir_ptr = opendir(full_image_dir_path);

                    if (strip_image_dir_ptr != NULL) {
                        int strip_index = 0;
                        while ((strip_de = readdir(strip_image_dir_ptr)) != NULL) {
                            // Checking if it's a png file
                            if (strstr(strip_de->d_name, ".png") != NULL) {
                                // Get the full path of the image
                                char image_full_path[strlen(full_image_dir_path) + strlen(strip_de->d_name) + 2];
                                strcpy(image_full_path, full_image_dir_path);
                                strcat(image_full_path, "/");
                                strcat(image_full_path, strip_de->d_name);

                                // Load the strip name (without the extension)
                                strcpy(strips[language_index].strips[strip_index].name, strip_de->d_name);
                                strips[language_index].strips[strip_index].name[strlen(strip_de->d_name) - 4] = '\0';

                                // Load the strip image
                                strips[language_index].strips[strip_index].texture = LoadTexture(image_full_path);
                                SetTextureFilter(strips[language_index].strips[strip_index].texture, TEXTURE_FILTER_TRILINEAR);

                                // Load the strip number from the filename with the pattern cartoon<number>.png
                                strips[language_index].strips[strip_index].number = atoi(strip_de->d_name + 7);

                                // Load the strip text with the format
                                // Date: date
                                // Text: comment about the strip. Can be multiple lines
                                // The text files contains the same name as the image file
                                const char *date_prefix = "Date: ";
                                const char *text_prefix = "Text: ";

                                char text_full_path[strlen(text_full_dir_path) + strlen(strip_de->d_name) + 2];
                                strcpy(text_full_path, text_full_dir_path);
                                strcat(text_full_path, "/");
                                strcat(text_full_path, strip_de->d_name);
                                text_full_path[strlen(text_full_path) - 4] = '\0';
                                strcat(text_full_path, ".txt");

                                FILE *text_file = fopen(text_full_path, "r");
                                if (text_file != NULL) {
                                    char line[8192];
                                    int line_count = 0;
                                    while (fgets(line, sizeof(line), text_file)) {
                                        if (strncmp(line, date_prefix, strlen(date_prefix)) == 0) {
                                            strcpy(strips[language_index].strips[strip_index].stripText.date, line + strlen(date_prefix));
                                        } else if (strncmp(line, text_prefix, strlen(text_prefix)) == 0) {
                                            struct Text *text = malloc(sizeof(struct Text));
                                            strcpy(text->text, line + strlen(text_prefix));
                                            strips[language_index].strips[strip_index].stripText.texts[line_count] = text;
                                            line_count++;
                                            strips[language_index].strips[strip_index].stripText.textCount = line_count;
                                        }
                                    }
                                }
                                strip_index++;
                                strips[language_index].stripAmount = strip_index;
                            }
                        }
                    }
                }
                language_index++;
            }
        }
    }

    // Sort the strips by number
    for (int i = 0; i < language_index; i++) {
        SortStrips(strips[i].strips, strips[i].stripAmount);
    }

    Navigation navigation;
    if ((strip_dir_ptr != NULL) && (nav_dir_ptr != NULL)) {
        // Navigation file names
        char nav_first_png[] = NAV_FIRST;
        char nav_previous_png[] = NAV_PREVIOUS;
        char nav_random_png[] = NAV_RANDOM;
        char nav_next_png[] = NAV_NEXT;
        char nav_last_png[] = NAV_LAST;

        // Load the navigation images into memory
        while ((de = readdir(nav_dir_ptr)) != NULL) {
            // Checking if it's a png file
            if (strstr(de->d_name, ".png") != NULL) {

                // Get the full path of the image
                char image_full_path[strlen(nav_dir) + strlen(de->d_name) + 2];
                strcpy(image_full_path, nav_dir);
                strcat(image_full_path, "/");
                strcat(image_full_path, de->d_name);

                // Load the image
                if (strcmp(de->d_name, nav_first_png) == 0) {
                    navigation.first.texture = LoadTexture(image_full_path);
                    SetTextureFilter(navigation.first.texture, TEXTURE_FILTER_TRILINEAR);
                    strcpy(navigation.first.text, "First");
                } else if (strcmp(de->d_name, nav_last_png) == 0) {
                    navigation.last.texture = LoadTexture(image_full_path);
                    SetTextureFilter(navigation.last.texture, TEXTURE_FILTER_TRILINEAR);
                    strcpy(navigation.last.text, "Last");
                } else if (strcmp(de->d_name, nav_next_png) == 0) {
                    navigation.next.texture = LoadTexture(image_full_path);
                    SetTextureFilter(navigation.next.texture, TEXTURE_FILTER_TRILINEAR);
                    strcpy(navigation.next.text, "Next");
                } else if (strcmp(de->d_name, nav_previous_png) == 0) {
                    navigation.previous.texture = LoadTexture(image_full_path);
                    SetTextureFilter(navigation.previous.texture, TEXTURE_FILTER_TRILINEAR);
                    strcpy(navigation.previous.text, "Previous");
                } else if (strcmp(de->d_name, nav_random_png) == 0) {
                    navigation.random.texture = LoadTexture(image_full_path);
                    SetTextureFilter(navigation.random.texture, TEXTURE_FILTER_TRILINEAR);
                    strcpy(navigation.random.text, "Random");
                }
            }
        }
    }

    int count = 0;
    int base_font_size = 25;
    float var_scale = 0.5;
    NavigationButtonsGeometryArray navigation_buttons;
    int maxButtonWidth = 26;
    int maxButtonHeight = 22;
    float selectedButtonScale = 1.2;
    int current_language_index = 0;
    int navigation_keys[] = { 0, KEY_LEFT, 0, KEY_RIGHT, 0 };

    // Loading Carlito Font
    CustomFont carlito;
    for (int size = 8; size < MAX_FONT_AMOUNT; size++) {
        carlito.regular[size] = LoadFontEx("fonts/Carlito-Regular.ttf", size, 0, 0);
        carlito.bold[size] = LoadFontEx("fonts/Carlito-Bold.ttf", size, 0, 0);
        carlito.italic[size] = LoadFontEx("fonts/Carlito-Italic.ttf", size, 0, 0);
        carlito.boldItalic[size] = LoadFontEx("fonts/Carlito-BoldItalic.ttf", size, 0, 0);
    }

    // Loading Notosans Nerd Font Mono
    CustomFont noto_mono_nerd;
    for (int size = 8; size < MAX_FONT_AMOUNT; size++) {
        noto_mono_nerd.regular[size] = LoadFontEx("fonts/NotoMonoNerdFontMono-Regular.ttf", size, 0, 0);
    }

    CustomFont fonts[2] = { carlito, noto_mono_nerd };

    while (!WindowShouldClose()) {
        // Update
        ClearBackground(WHITE);
        BeginDrawing();

        
        int current_screen_width = GetScreenWidth();
        int current_screen_height = GetScreenHeight();
      
        // Draw an error message if the directory could not be opened
        if (strip_dir_ptr == NULL) {
            char err_message[] = "Could not open current directory \"";
            char full_err_message[strlen(err_message) + strlen(strips_dir) + 4];
            strcpy(full_err_message, err_message);
            strcat(full_err_message, strips_dir);
            strcat(full_err_message, "\"");
            Vector2 err_message_position = { 10, screenHeight / 2 };
            DrawTextEx(carlito.bold[30], full_err_message, err_message_position, 30, 1, RED);
        } else {
            // Dynamic parameters
            float vertical_scale = (float)current_screen_height / (float)screenHeight;
            float horizontal_scale = (float)current_screen_width / (float)screenWidth;
            float scale = (vertical_scale < horizontal_scale) ? vertical_scale : horizontal_scale;
            float half_current_screen_width = (float)current_screen_width / 2;
            Vector2 current_mouse_position = GetMousePosition();

            // Draw the date
            Vector2 date_position = { 10, (current_screen_height / 20) * 0.25 };
            DrawTextEx(carlito.bold[30], strips[current_language_index].strips[count].stripText.date, date_position, 30, 1, BLACK);

            // Draw the strip number
            char strip_number_text[4];
            sprintf(strip_number_text, "%d", strips[current_language_index].strips[count].number);
            Vector2 strip_number_dimensions = MeasureTextEx(carlito.bold[30], strip_number_text, 30, 1);
            Vector2 strip_number_position = { current_screen_width - strip_number_dimensions.x - 10, (current_screen_height / 20) * 0.25 };
            DrawTextEx(carlito.bold[30], strip_number_text, strip_number_position, 30, 1, BLACK);

            // Draw the navigation buttons
            navigation_buttons = CalculateNavigationButtonsGeometry(half_current_screen_width, current_screen_height - (navigation_buttons.scaledContainer.height * 0.7), maxButtonWidth, maxButtonHeight, (scale * 1.2 + var_scale), selectedButtonScale, navigation, carlito);
            count = DrawNavigation(navigation_buttons, current_mouse_position, strips[current_language_index].stripAmount, count, selectedButtonScale, navigation_keys);
            
            DrawStripAndText(half_current_screen_width, 5 * current_screen_height / 100, &strips[current_language_index].strips[count], count, fonts, base_font_size, scale, current_screen_width, 80 * current_screen_height / 100);
        }
        EndDrawing();
    }

    // De-Initialization
    for (int i = 0; i < language_index; i++) {
        for (int j = 0; j < strips[i].stripAmount; j++) {
            UnloadTexture(strips[i].strips[j].texture);
        }
    }

    CloseWindow();
}