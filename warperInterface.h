#ifndef WARPERINTERFACE_H_INCLUDED
#define WARPERINTERFACE_H_INCLUDED

#include "CoSprite/csGraphics.h"
#include "warper.h"

typedef struct _warperTextBox
{
    cDoubleRect rect;
    SDL_Color bgColor;
    SDL_Color highlightColor;
    cText* texts;
    bool* isOption;
    int textsSize;
    bool isMenu;
    int selection;
    int storedSelection;
} warperTextBox;


//functions:
//CoSprite helper functs:
void initWarperTextBox(warperTextBox* textBox, cDoubleRect rect, SDL_Color bgColor, SDL_Color highlightColor, cText* texts, bool* isOption, int textsSize, bool isMenu);
void drawWarperTextBox(void* textBoxSubclass, cCamera camera);
void destroyWarperTextBox(void* textBoxSubclass);

//battle helper functs:
void createBattleTextBox(warperTextBox* textBox, cDoubleRect dimensions, char** strings, bool* isOptions, int stringsLength, warperTilemap tilemap);

#endif // WARPERINTERFACE_H_INCLUDED
