#include "warperInterface.h"

cSprite cursorSprite;

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
void initWarperTextBox(warperTextBox* textBox, cDoubleRect rect, SDL_Color outlineColor, SDL_Color bgColor, SDL_Color highlightColor, cText* texts, bool* isOption, int textsSize, bool isMenu)
{
    textBox->rect = rect;
    textBox->outlineColor = outlineColor;
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

    //draw outline
    SDL_SetRenderDrawColor(global.mainRenderer, textBox->outlineColor.r, textBox->outlineColor.g, textBox->outlineColor.b, textBox->outlineColor.a);
    SDL_RenderDrawRect(global.mainRenderer, &boxRect);

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
        SDL_Rect selectionRect = (SDL_Rect) {textBox->texts[textBox->selection].rect.x, textBox->texts[textBox->selection].rect.y, textBox->texts[textBox->selection].rect.w, textBox->texts[textBox->selection].rect.h};
        SDL_RenderFillRect(global.mainRenderer, &selectionRect);
    }

    SDL_SetRenderDrawColor(global.mainRenderer, prevR, prevG, prevB, prevA);
}

/** \brief CoSprite helper function; if using, cast textBox to a void*
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

/** \brief CoSprite helper function
 *
 * \param path void*
 * \param camera cCamera
 */
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

/** \brief CoSprite helper function; if using, cast path to a void*
 *
 * \param path void*
 */
void destroyWarperPath(void* path)
{
    warperPath* nodePath = (warperPath*) path;
    if (nodePath->path)
        free(nodePath->path);

    nodePath->path = NULL;
    nodePath->pathLength = 0;
}

/** \brief CoSprite helper function
 *
 * \param circle void*
 * \param camera cCamera
 */
void drawWarperCircle(void* circle, cCamera camera)
{
    warperCircle* wCircle = (warperCircle*) circle;

    if (!wCircle->filled)
    {
        Uint8 prevR = 0, prevG = 0, prevB = 0, prevA = 0;
        SDL_GetRenderDrawColor(global.mainRenderer, &prevR, &prevG, &prevB, &prevA);

        SDL_SetRenderDrawColor(global.mainRenderer, wCircle->circleColor.r, wCircle->circleColor.g, wCircle->circleColor.b, wCircle->circleColor.a);

        for(int d = 0; d <= 360; d += wCircle->deltaDegrees)
            SDL_RenderDrawLine(global.mainRenderer, (wCircle->center.x - camera.rect.x + (wCircle->radius * cos(degToRad(d - wCircle->deltaDegrees)))) * global.windowW / camera.rect.w, (wCircle->center.y - camera.rect.y + (wCircle->radius * sin(degToRad(d - wCircle->deltaDegrees)))) * global.windowH / camera.rect.h,
                                (wCircle->center.x - camera.rect.x + (wCircle->radius * cos(degToRad(d)))) * global.windowW / camera.rect.w, (wCircle->center.y - camera.rect.y + (wCircle->radius * sin(degToRad(d)))) * global.windowH / camera.rect.h);

        SDL_SetRenderDrawColor(global.mainRenderer, prevR, prevG, prevB, prevA);
    }
    else
    {
        /* from https://stackoverflow.com/questions/1201200/fast-algorithm-for-drawing-filled-circles
        int r2 = wCircle->radius * wCircle->radius;
        int area = r2 << 2;
        int rr = ((int) wCircle->radius) << 1;

        for (int i = 0; i < area; i++)
        {
            int tx = (i % rr) - wCircle->radius;
            int ty = (i / rr) - wCircle->radius;

            if (tx * tx + ty * ty <= r2)
                SDL_RenderDrawPoint(global.mainRenderer, wCircle->center.x + tx - camera.rect.x, wCircle->center.y + ty - camera.rect.y);
        }
        //*/

        /* from https://stackoverflow.com/questions/1201200/fast-algorithm-for-drawing-filled-circles
        for(int y = -1 * wCircle->radius; y <= wCircle->radius; y++)
            for(int x = -1 * wCircle->radius; x <= wCircle->radius; x++)
                if(x * x + y * y <= wCircle->radius * wCircle->radius)
                    SDL_RenderDrawPoint(global.mainRenderer, wCircle->center.x + x - camera.rect.x, wCircle->center.y + y - camera.rect.y);
        //*/

        //both of the above ran really slow. Until I can figure out why, I'm just cheating with this poorly drawn circle of arbitrary size
        //*
        cSprite cheatCircle;  //init the circle
        initCSprite(&cheatCircle, NULL, "./assets/filledCircleCheat.png", 0,
                    (cDoubleRect) {wCircle->center.x - wCircle->radius, wCircle->center.y - wCircle->radius, 2 * wCircle->radius, 2 * wCircle->radius},
                    (cDoubleRect) {0, 0, 507, 507}, NULL, 1.0, SDL_FLIP_NONE, 0, false, false, NULL, 5);

        //do some color and alpha modding
        SDL_SetTextureColorMod(cheatCircle.texture, wCircle->circleColor.r, wCircle->circleColor.g, wCircle->circleColor.b);
        SDL_SetTextureAlphaMod(cheatCircle.texture, wCircle->circleColor.a);

        drawCSprite(cheatCircle, camera, false, false);  //draw it then free the memory
        destroyCSprite(&cheatCircle);
        //*/
    }
}

void drawCursor(void* cursor, cCamera camera)
{
    drawCSprite(cursorSprite, camera, false, false);
    if (cursorSprite.id == CURSOR_SELECT)
        updateCursorIcon(CURSOR_NORMAL);  //update the select sprite back to normal
}

void destroyCursor(void* cursor)
{
    //nothing needed so far
}

/** \brief CoSprite helper function; if using, cast circle to a void*
 *
 * \param circle void*
 */
void destroyWarperCircle(void* circle)
{
    warperCircle* wCircle = (warperCircle*) circle;
    wCircle->center = (cDoublePt) {0, 0};
    wCircle->deltaDegrees = 0;
    wCircle->radius = 0;
    wCircle->circleColor = (SDL_Color) {0,0,0,0};
}

/** \brief Shorthand funct to create a battle textbox (menu)
 *
 * \param textBox warperTextBox* - text box pointer to fill in
 * \param dimensions cDoubleRect - dimensions of the textbox
 * \param strings char** - array of char*s with your lines of text
 * \param isOptions bool* - array of bools with true = clickable/actable, false = not actable
 * \param stringsLength int - length of strings = length of isOptions
 * \param int tileSize - tilemap's tile size
 */
void createBattleTextBox(warperTextBox* textBox, cDoubleRect dimensions, cDoublePt margins, double verticalSpacing, bool justify, char** strings, bool* isOptions, int stringsLength, int tileSize)
{
    int textCount = stringsLength + 2;  //3 options + the +/- buttons
    cText* texts = calloc(textCount, sizeof(cText));
    for(int i = 0; i < textCount - 2; i++)
    {
        int xPos = dimensions.x + margins.x;
        if (justify)
            xPos = dimensions.w / 2 - (strlen(strings[i]) * global.mainFont.fontSize / 2) + dimensions.x;

        initCText(&(texts[i]), strings[i], (cDoubleRect) {xPos, dimensions.y + i * tileSize + margins.y + verticalSpacing * i, dimensions.w - 2 * tileSize, dimensions.h - i * tileSize}, dimensions.w - 2 * tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
    }

    initCText(&(texts[textCount - 2]), "-", (cDoubleRect) {dimensions.x + dimensions.w - 2 * tileSize, dimensions.y, tileSize, tileSize}, tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
    initCText(&(texts[textCount - 1]), "+", (cDoubleRect) {dimensions.x + dimensions.w - 2 * tileSize, 19 * tileSize, tileSize, tileSize}, tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 0);

    bool* fixedIsOptions = calloc(stringsLength + 2, sizeof(bool));  //adding isOptions for + and -
    for(int i = 0; i < stringsLength; i++)
    {
        fixedIsOptions[i] = isOptions[i];
    }
    fixedIsOptions[stringsLength] = true; //setting true for the + and - options
    fixedIsOptions[stringsLength + 1] = true;

    initWarperTextBox(textBox, dimensions, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xC0}, (SDL_Color) {0xFF, 0x00, 0x00, 0x20}, texts, fixedIsOptions, textCount, true);

    for(int i = 0; i < textCount; i++)
        destroyCText(&(texts[i]));

    free(texts);
    free(fixedIsOptions);
}

/** \brief Shorthand funct to create a menu textbox
 *
 * \param textBox warperTextBox* - text box pointer to fill in
 * \param dimensions cDoubleRect - dimensions of the textbox
 * \param margins cDoublePt - margins between textbox and text, {.x = left, .y = top}
 * \param verticalPadding double - padding between individual text items (lines)
 * \param justify bool - set to justify menu text vertically (overrides margins.x)
 * \param bgOpacity Uint8 - percent opacity off of default, where 255 is 100%
 * \param strings char** - array of char*s with your lines of text
 * \param isOptions bool* - array of bools with true = clickable/actable, false = not actable
 * \param stringsLength int - length of strings = length of isOptions
 * \param font cFont* - font you want to use
 */
void createMenuTextBox(warperTextBox* textBox, cDoubleRect dimensions, cDoublePt margins, double verticalSpacing, bool justify, Uint8 bgOpacity, char** strings, bool* isOptions, int stringsLength, cFont* font)
{
    cText* texts = calloc(stringsLength, sizeof(cText));
    for(int i = 0; i < stringsLength; i++)
    {
        int xPos = dimensions.x + margins.x;
        if (justify)
            xPos = dimensions.w / 2 - (strlen(strings[i]) * font->fontSize / 2) + dimensions.x;

        initCText(&(texts[i]), strings[i], (cDoubleRect) {xPos, dimensions.y + i * font->fontSize + margins.y + verticalSpacing * i, 30 * font->fontSize, font->fontSize}, 30 * font->fontSize,
                  (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, font, 1.0, SDL_FLIP_NONE, 0, true, 5);
    }

    initWarperTextBox(textBox, dimensions, (SDL_Color) {0x00, 0x00, 0x00, bgOpacity}, (SDL_Color) {0xC0, 0xC0, 0xC0, bgOpacity}, (SDL_Color) {0xFF, 0x00, 0x00, 0x20 * bgOpacity / 0xFF}, texts, isOptions, stringsLength, true);

    for(int i = 0; i < stringsLength; i++)
        destroyCText(&(texts[i]));

    free(texts);
}

void checkWarperTextBoxHover(warperTextBox* textBox, SDL_MouseMotionEvent motion)
{
    if (motion.x >= 0 && motion.y >= 0 && motion.x <= SCREEN_PX_WIDTH && motion.y <= SCREEN_PX_HEIGHT)
    {
        if (motion.x > textBox->rect.x && motion.x < textBox->rect.x + textBox->rect.w && motion.y > textBox->rect.y && motion.y < textBox->rect.y + textBox->rect.h)
        {
            for(int i = 0; i < textBox->textsSize; i++)
            {
                if (textBox->isOption[i] && (motion.x > textBox->texts[i].rect.x && motion.x < textBox->texts[i].rect.x + textBox->texts[i].rect.w &&
                    motion.y > textBox->texts[i].rect.y && motion.y < textBox->texts[i].rect.y + textBox->texts[i].rect.h))
                {
                    updateCursorIcon(CURSOR_HOVER);
                }
            }
        }
    }
}

/** \brief Automatically updates storedSelection and selection of a textbox based on what you click
 *
 * \param textBox warperTextBox* - text box you want updated
 * \param xClick int - x coordinate of click (based on text box's text rects)
 * \param yClick int - y coordinate of click (based on text box's text rects)
 */
void checkWarperTextBoxSelection(warperTextBox* textBox, int xClick, int yClick)
{
    if (xClick >= textBox->rect.x && xClick <= textBox->rect.x + textBox->rect.w && yClick >= textBox->rect.y && yClick <= textBox->rect.y + textBox->rect.h)
    {
        textBox->storedSelection = textBox->selection;

        for(int i = 0; i < textBox->textsSize; i++)
        {
            if (textBox->isOption[i] && (xClick > textBox->texts[i].rect.x && xClick < textBox->texts[i].rect.x + textBox->texts[i].rect.w &&
                yClick > textBox->texts[i].rect.y && yClick < textBox->texts[i].rect.y + textBox->texts[i].rect.h))
            {
                //we clicked on an element
                textBox->selection = i;
                updateCursorIcon(CURSOR_SELECT);
            }
        }
    }
}

void importWarperTextBox(warperTextBox* textBox, char* data)
{
    //TODO
}

char* exportWarperTextBox(warperTextBox textBox, int* exportedLen)
{
    const int colorsSize = 3 * ((3 + 1) * 4 + 3) + 3; //3 characters plus 1 separator per color channel, times 4 channels, plus 2 parentheses and 1 separator per SDL_Color, times 3 colors, plus containing brackets and 1 for good measure
    const int miscSize = 1 + 4 * min(charsInNum(textBox.textsSize), 2) + (4 * 12) + 5 + 1;  //1 boolean plus 2 selection variables (max characters == textsSize, min. 2 [for case "-1"]) plus 1 size variable + 2 parentheses plus 4 * (11 digits plus 1 separator) for dimensions + 4 separators + 1 for good measure
    const int boolsSize = textBox.textsSize * 2 + 2 + 1;  //two characters per bool: value and separator, plus 2 containing brackets, plus 1 for good measure

    int* textsSizes = calloc(textBox.textsSize, sizeof(int));

    int dataSize = colorsSize + 1 + miscSize + boolsSize + 1 + 1;  //colors size plus containing '|', plus misc size (and eventually plus strings size) plus bools size plus ending '|', plus 1 for good measure

    for(int i = 0; i < textBox.textsSize; i++)
    {
        textsSizes[i] = strlen(textBox.texts[i].str) + 4;  //sizes of each individual text item, plus 2 for quotes and 1 for separator, and 1 for good measure
        dataSize += textsSizes[i];  //add it to dataSize variable
    }

    char* boxData = calloc(dataSize, sizeof(char));
    char* tempData = calloc(miscSize, sizeof(char));

    //colorsSize + 1 for the containing '|' and separator ';'
    snprintf(boxData, colorsSize + 2, "{[(%d,%d,%d,%d),(%d,%d,%d,%d),(%d,%d,%d,%d)];", textBox.bgColor.r, textBox.bgColor.g, textBox.bgColor.b, textBox.bgColor.a,
                                                                                        textBox.highlightColor.r, textBox.highlightColor.g, textBox.highlightColor.b, textBox.highlightColor.a,
                                                                                        textBox.outlineColor.r, textBox.outlineColor.g, textBox.outlineColor.b, textBox.outlineColor.a);

    snprintf(tempData, miscSize, "%d;%d;%d;%d;(%f,%f,%f,%f);", textBox.isMenu, textBox.selection, textBox.storedSelection, textBox.textsSize, textBox.rect.x, textBox.rect.y, textBox.rect.w, textBox.rect.h);
    strncat(boxData, tempData, dataSize);

    free(tempData);

    strncat(boxData, "[", dataSize);
    for(int i = 0; i < textBox.textsSize; i++)
    {
        tempData = calloc(textsSizes[i] + 3, sizeof(char));
        snprintf(tempData, textsSizes[i] + 3, "\"%s\"", textBox.texts[i].str);
        strncat(boxData, tempData, dataSize);

        if (i < textBox.textsSize - 1)
            strncat(boxData, ",", dataSize);

        free(tempData);
    }
    free(textsSizes);

    strncat(boxData, "];[", dataSize);

    for(int i = 0; i < textBox.textsSize; i++)
    {
        tempData = calloc(2, sizeof(char));
        snprintf(tempData, 2, "%d", textBox.isOption[i]);
        strncat(boxData, tempData, dataSize);

        if (i < textBox.textsSize - 1)
            strncat(boxData, ",", dataSize);

        free(tempData);
    }
    strncat(boxData, "]}", dataSize);

    *exportedLen = dataSize;

    return boxData;
}

void updateCursorPos(SDL_MouseMotionEvent motion, bool debugPrint)
{
    if (motion.x > -1 && motion.y > -1 && motion.x <= SCREEN_PX_WIDTH && motion.y <= SCREEN_PX_HEIGHT)
    {
        if (debugPrint)
            printf("%d, %d\n", motion.x, motion.y);
        cursorSprite.drawRect.x = motion.x;
        cursorSprite.drawRect.y = motion.y;
    }
}


void updateCursorIcon(int id)
{
    cursorSprite.id = id;
    cursorSprite.srcClipRect.x = 16 * (id % 2);  //0 if even, 16 if odd
    cursorSprite.srcClipRect.y = 64 + 16 * (id / 2); //64 if 0-1, 80 if 2-3, etc.
}

/** \brief
 *
 * \param tilemap warperTilemap - the tilemap to use
 * \param gridModel c2DModel* - the model that will hold the grid
 * \return void
 *
 */
void loadGridModel(warperTilemap tilemap, c2DModel* gridModel, Uint8 opacity)
{
    SDL_Texture* uiTilesetTexture;
    loadIMG("./assets/uiTilesheet.png", &uiTilesetTexture);
    SDL_SetTextureAlphaMod(uiTilesetTexture, opacity);

    cSprite* gridSprites = calloc(tilemap.width * tilemap.height, sizeof(cSprite));
    for(int x = 0; x < tilemap.width; x++)
    {
        for(int y = 0; y < tilemap.height; y++)
        {
            initCSprite(&gridSprites[x * tilemap.height + y], uiTilesetTexture, "./assets/uiTilesheet.png", 1,
                        (cDoubleRect) {tilemap.tileSize * x, tilemap.tileSize * y, tilemap.tileSize, tilemap.tileSize},
                        (cDoubleRect) {0, 32, 32, 32},
                        NULL, 1.0, SDL_FLIP_NONE, 0.0, false, false, NULL, 5);
        }
    }
    initC2DModel(gridModel, gridSprites, tilemap.width * tilemap.height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);

    free(gridSprites);
}
