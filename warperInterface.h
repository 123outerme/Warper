#ifndef WARPERINTERFACE_H_INCLUDED
#define WARPERINTERFACE_H_INCLUDED

#include "warper.h"
#include "battleSystem.h"

#define CURSOR_NORMAL 0
#define CURSOR_ALLY 1
#define CURSOR_NEUTRAL 2
#define CURSOR_ENEMY 3
#define CURSOR_HOVER 4
#define CURSOR_SELECT 5
#define CURSOR_A 6
#define CURSOR_B 7
#define CURSOR_NORMAL_BLOCKED 8
#define CURSOR_ALLY_BLOCKED 9
#define CURSOR_GRAY_BLOCKED 10
#define CURSOR_ENEMY_OK 11

typedef struct _warperTextBox
{
    cDoubleRect rect;
    SDL_Color outlineColor;
    SDL_Color bgColor;
    SDL_Color highlightColor;
    cText* texts;
    bool* isOption;
    int textsSize;
    bool isMenu;
    int selection;
    int storedSelection;
} warperTextBox;

typedef struct _warperPath
{
    node* path;
    int pathLength;
    SDL_Color pathColor;
    int pathfinderWidth;
    int pathfinderHeight;
    double distance;
} warperPath;

typedef struct _warperCircle
{
    int deltaDegrees;
    double radius;
    cDoublePt center;
    SDL_Color circleColor;
    bool filled;
} warperCircle;

//functions:
//CoSprite helper functs:
void initWarperTextBox(warperTextBox* textBox, cDoubleRect rect, SDL_Color outlineColor, SDL_Color bgColor, SDL_Color highlightColor, cText* texts, bool* isOption, int textsSize, bool isMenu);
void drawWarperTextBox(void* textBoxSubclass, cCamera camera);
void destroyWarperTextBox(void* textBoxSubclass);
void drawWarperPath(void* path, cCamera camera);
void destroyWarperPath(void* path);
void drawWarperCircle(void* circle, cCamera camera);
void destroyWarperCircle(void* circle);
void drawCursor(void* cursor, cCamera camera);
void destroyCursor(void* cursor);

//text box helper functs:
void createBattleTextBox(warperTextBox* textBox, cDoubleRect dimensions, cDoublePt margins, double verticalSpacing, bool justify, char** strings, bool* isOptions, int stringsLength, int tileSize);
void createMenuTextBox(warperTextBox* textBox, cDoubleRect dimensions, cDoublePt margins, double verticalSpacing, bool justify, Uint8 bgOpacity, char** strings, bool* isOptions, int stringsLength, cFont* font);
void checkWarperTextBoxHover(warperTextBox* textBox, SDL_MouseMotionEvent motion);
void checkWarperTextBoxSelection(warperTextBox* textBox, int xClick, int yClick);
void importWarperTextBox(warperTextBox* textBox, char* data);
char* exportWarperTextBox(warperTextBox textBox, int* exportedLen);

//cursor
void updateCursorPos(SDL_MouseMotionEvent motion, bool debugPrint);
void updateCursorIcon(int id);

//Other UI/UX
void loadGridModel(warperTilemap tilemap, c2DModel* gridModel, Uint8 opacity);

extern cSprite cursorSprite;  //global cursor sprite

#endif // WARPERINTERFACE_H_INCLUDED
