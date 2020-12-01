#ifndef WARPERINTERFACE_H_INCLUDED
#define WARPERINTERFACE_H_INCLUDED

#include "warper.h"
#include "battleSystem.h"

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
void checkWarperTextBoxClick(warperTextBox* textBox, int xClick, int yClick);
void destroyWarperTextBox(void* textBoxSubclass);
void drawWarperPath(void* path, cCamera camera);
void destroyWarperPath(void* path);
void drawWarperCircle(void* circle, cCamera camera);
void destroyWarperCircle(void* circle);

//text box helper functs:
void createBattleTextBox(warperTextBox* textBox, cDoubleRect dimensions, char** strings, bool* isOptions, int stringsLength, int tileSize);
void createMenuTextBox(warperTextBox* textBox, cDoubleRect dimensions, char** strings, bool* isOptions, int stringsLength, cFont* font);

#endif // WARPERINTERFACE_H_INCLUDED
