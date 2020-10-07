#include "warperInterface.h"

/** \brief
 *
 * \param textBox warperTextBox*
 * \param rect cDoubleRect
 * \param bgColor SDL_Color
 * \param highlightColor SDL_Color
 * \param texts cText*
 * \param isOption bool*
 * \param textsSize int
 * \param isMenu bool
 */
void initWarperTextBox(warperTextBox* textBox, cDoubleRect rect, SDL_Color bgColor, SDL_Color highlightColor, cText* texts, bool* isOption, int textsSize, bool isMenu)
{
    textBox->rect = rect;
    textBox->bgColor = bgColor;
    textBox->highlightColor = highlightColor;
    textBox->textsSize = textsSize;
    textBox->texts = calloc(textsSize, sizeof(cText));
    if (textBox->texts != NULL)
    {
        for(int i = 0; i < textsSize; i++)
        {
            initCText(&(textBox->texts[i]), texts[i].str, texts[i].rect, texts[i].maxW, texts[i].textColor, texts[i].bgColor, texts[i].font, texts[i].scale, texts[i].flip, texts[i].degrees, true, texts[i].renderLayer);
        }
    }
    else
    {
        printf("ERROR: not enough mem to alloc text box\n");
    }
    textBox->isOption = calloc(textsSize, sizeof(bool));
    if (textBox->isOption != NULL)
    {
        for(int i = 0; i < textsSize; i++)
            textBox->isOption[i] = isOption[i];
    }
    else
    {
        printf("ERROR: not enough mem to alloc text box\n");
    }
    textBox->isMenu = isMenu;
    textBox->selection = -1;
    textBox->storedSelection = -1;
}

/** \brief CoSprite helper function
 *
 * \param textBoxSubclass void*
 * \param camera cCamera
 */
void drawWarperTextBox(void* textBoxSubclass, cCamera camera)
{
    warperTextBox* textBox = (warperTextBox*) textBoxSubclass;

    SDL_Rect boxRect = (SDL_Rect) {textBox->rect.x, textBox->rect.y, textBox->rect.w, textBox->rect.h};

    Uint8 prevR = 0, prevG = 0, prevB = 0, prevA = 0;
    SDL_GetRenderDrawColor(global.mainRenderer, &prevR, &prevG, &prevB, &prevA);

    //draw text box
    SDL_SetRenderDrawColor(global.mainRenderer, textBox->bgColor.r, textBox->bgColor.g, textBox->bgColor.b, textBox->bgColor.a);
    SDL_RenderFillRect(global.mainRenderer, &boxRect);

    //draw cTexts
    for(int i = 0; i < textBox->textsSize; i++)
    {
        if (textBox->texts[i].renderLayer > 0)
            drawCText(textBox->texts[i], camera, false);
    }

    //draw selection highlight
    if (textBox->isMenu && textBox->selection != -1 && textBox->texts[textBox->selection].renderLayer > 0)
    {
        SDL_SetRenderDrawColor(global.mainRenderer, textBox->highlightColor.r, textBox->highlightColor.g, textBox->highlightColor.b, textBox->highlightColor.a);
        SDL_Rect selectionRect = (SDL_Rect) {textBox->rect.x, boxRect.y + textBox->selection * textBox->texts[textBox->selection].font->fontSize, textBox->texts[textBox->selection].rect.w, textBox->texts[textBox->selection].rect.h};
        SDL_RenderDrawRect(global.mainRenderer, &selectionRect);
    }

    SDL_SetRenderDrawColor(global.mainRenderer, prevR, prevG, prevB, prevA);
}

/** \brief CoSprite helper function; if using, cast textBox to a warperTextBox*
 *
 * \param textBoxSubclass void*
 */
void destroyWarperTextBox(void* textBoxSubclass)
{
    warperTextBox* textBox = (warperTextBox*) textBoxSubclass;

    textBox->rect = (cDoubleRect) {0,0,0,0};
    textBox->bgColor = (SDL_Color) {0,0,0,0};
    textBox->highlightColor = (SDL_Color) {0,0,0,0};

    for(int i = 0; i < textBox->textsSize; i++)
    {
        destroyCText(&(textBox->texts[i]));
    }
    textBox->textsSize = 0;
    textBox->isMenu = false;
    textBox->selection = 0;
}

void drawWarperPath(void* path, cCamera camera)
{
    warperPath* nodePath = (warperPath*) path;

    Uint8 prevR = 0, prevG = 0, prevB = 0, prevA = 0;
    SDL_GetRenderDrawColor(global.mainRenderer, &prevR, &prevG, &prevB, &prevA);

    SDL_SetRenderDrawColor(global.mainRenderer, nodePath->pathColor.r, nodePath->pathColor.g, nodePath->pathColor.b, nodePath->pathColor.a);

    for(int i = 1; i < nodePath->pathLength; i++)
    {
        SDL_RenderDrawLine(global.mainRenderer, nodePath->path[i - 1].x - camera.rect.x + nodePath->pathfinderWidth / 2, nodePath->path[i - 1].y - camera.rect.y + nodePath->pathfinderHeight / 2,
                           nodePath->path[i].x - camera.rect.x + nodePath->pathfinderWidth / 2, nodePath->path[i].y - camera.rect.y + nodePath->pathfinderHeight / 2);
    }

    SDL_SetRenderDrawColor(global.mainRenderer, prevR, prevG, prevB, prevA);
}

void destroyWarperPath(void* path)
{
    warperPath* nodePath = (warperPath*) path;
    if (nodePath->path)
        free(nodePath->path);

    nodePath->path = NULL;
    nodePath->pathLength = 0;
}

/** \brief Shorthand funct to create a battle textbox (menu)
 *
 * \param textBox warperTextBox*
 * \param dimensions cDoubleRect
 * \param strings char**
 * \param isOptions bool*
 * \param stringsLength int
 * \param tilemap warperTilemap
 */
void createBattleTextBox(warperTextBox* textBox, cDoubleRect dimensions, char** strings, bool* isOptions, int stringsLength, warperTilemap tilemap)
{
    {
        int textCount = stringsLength + 2;  //3 options + the +/- buttons
        cText* texts = calloc(textCount, sizeof(cText));
        for(int i = 0; i < textCount - 2; i++)
        {
            initCText(&(texts[i]), strings[i], (cDoubleRect) {5 * tilemap.tileSize, (14 + i) * tilemap.tileSize, 30 * tilemap.tileSize, (14 - i) * tilemap.tileSize}, 30 * tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
        }

        initCText(&(texts[textCount - 2]), "-", (cDoubleRect) {34 * tilemap.tileSize, 14 * tilemap.tileSize, tilemap.tileSize, tilemap.tileSize}, tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
        initCText(&(texts[textCount - 1]), "+", (cDoubleRect) {34 * tilemap.tileSize, 19 * tilemap.tileSize, tilemap.tileSize, tilemap.tileSize}, tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 0);

        bool* fixedIsOptions = calloc(stringsLength + 2, sizeof(bool));
        for(int i = 0; i < stringsLength; i++)
        {
            fixedIsOptions[i] = isOptions[i];
        }
        fixedIsOptions[stringsLength] = true; //setting true for the + and - options
        fixedIsOptions[stringsLength + 1] = true;

        initWarperTextBox(textBox, dimensions, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xC0}, (SDL_Color) {0xFF, 0x00, 0x00, 0xC0}, texts, fixedIsOptions, textCount, true);

        for(int i = 0; i < textCount; i++)
            destroyCText(&(texts[i]));

        free(texts);
        free(fixedIsOptions);
    }
}
