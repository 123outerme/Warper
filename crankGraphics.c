#include "crankGraphics.h"

/** \brief Initializes a cSprite object. You may want to create a wrapper method.
 *
 * \param sprite - a pointer to your sprite.
 * \param texture - a SDL_Texture* that holds your sprite's image
 * \param x - x position onscreen
 * \param y - y position onscreen
 * \param w - width of your sprite
 * \param h - height of your sprite
 * \param scale - size * this == drawn size
 * \param flip - SDL_RenderFlip value
 * \param degrees - rotation angle in degrees
 * \param subclass - void*. Do with it what you will, isn't used internally
 * \param drawPriority - 0 - not drawn. 1-5 - drawn. Lower number = drawn later
 */
void initCSprite(cSprite* sprite, SDL_Texture* texture, int id, SDL_Rect drawRect, SDL_Rect srcClipRect, double scale, SDL_RendererFlip flip, double degrees, void* subclass, int drawPriority)
{
    sprite->texture = texture;
    sprite->id = id;
    sprite->drawRect = drawRect;
    sprite->srcClipRect = srcClipRect;
    sprite->scale = scale;
    sprite->degrees = degrees;
    sprite->flip = flip;
    sprite->subclass = subclass;
    sprite->drawPriority = drawPriority;
}

/** \brief clears out a cSprite and its memory
 *
 * \param sprite - cSprite pointer
 */
void destroyCSprite(cSprite* sprite)
{
    SDL_DestroyTexture(sprite->texture);
    sprite->drawRect = (SDL_Rect) {0, 0, 0, 0};
    sprite->srcClipRect = (SDL_Rect) {0, 0, 0, 0};
    sprite->id = 0;
    sprite->scale = 0;
    sprite->degrees = 0;
    sprite->flip = SDL_FLIP_NONE;
    free(sprite->subclass);
    sprite->subclass = NULL;
    sprite->drawPriority = 0;

}

/** \brief draws a cSprite to the screen
 *
 * \param sprite - cSprite you want drawn
 * \param camera - cCamera to be used for drawing
 * \param update - if true, immediately presents renderer
 */
void drawCSprite(cSprite sprite, cCamera camera, bool update)
{
    SDL_RenderCopyEx(mainRenderer, sprite.texture, &(sprite.srcClipRect), &((SDL_Rect) {.x = sprite.drawRect.x - (camera.rect.x * windowW / camera.rect.w), .y = sprite.drawRect.y - (camera.rect.y * windowH / camera.rect.h), .w = sprite.drawRect.w * sprite.scale * camera.zoom, .h = sprite.drawRect.h * sprite.scale * camera.zoom}), sprite.degrees, NULL, sprite.flip);
    if (update)
        SDL_RenderPresent(mainRenderer);
}

/** \brief Initializes a cSprite object. You may want to create a wrapper method.
 *
 * \param model - a pointer to your model.
 * \param sprites - a pointer that holds your sprites
 * \param x - x position onscreen
 * \param y - y position onscreen
 * \param w - width of your sprite
 * \param h - height of your sprite
 * \param scale - size * this == drawn size
 * \param flip - SDL_RenderFlip value
 * \param degrees - rotation angle in degrees
 * \param subclass - void*. Do with it what you will, isn't used internally
 * \param drawPriority - 0 - not drawn. 1-5 - drawn. Lower number = drawn later
 */
void initC2DModel(c2DModel* model, cSprite* sprites, int numSprites, int x, int y, int w, int h, double scale, SDL_RendererFlip flip, double degrees, void* subclass, int drawPriority)
{
    model->sprites = (numSprites) ? sprites : NULL;
    model->numSprites = numSprites;
    model->rect = (SDL_Rect) {.x = x, .y = y, .w = w, .h = h};
    model->scale = scale;
    model->flip = flip;
    model->degrees = degrees;
    model->subclass = subclass;
    model->drawPriority = drawPriority;
}

/** \brief clears out a c2DModel and its memory
 *
 * \param model - c2DModel pointer
 */
void destroyC2DModel(c2DModel* model)
{
    for(int i = 0; i < model->numSprites; i++)
        destroyCSprite(&model->sprites[i]);
    model->rect = (SDL_Rect) {0, 0, 0, 0};
    model->scale = 0;
    model->degrees = 0;
    model->flip = SDL_FLIP_NONE;
    free(model->subclass);
    model->subclass = NULL;
    model->drawPriority = 0;
}

/** \brief draws a c2DModel to the screen
 *
 * \param model - c2DModel you want drawn
 * \param camera - cCamera to be used for drawing
 * \param update - if true, immediately presents renderer
 */
void drawC2DModel(c2DModel model, cCamera camera, bool update)
{
    for(int i = 0; i < model.numSprites; i++)
    {
        for(int priority = 5; priority >= 1; priority--)
        {
            if (model.sprites[i].drawPriority == priority)
            {
                SDL_RenderCopyEx(mainRenderer, model.sprites[i].texture, &(model.sprites[i].srcClipRect), &((SDL_Rect) {.x = model.rect.x + model.sprites[i].drawRect.x - (camera.rect.x * windowW / camera.rect.w), .y = model.rect.y + model.sprites[i].drawRect.y  - (camera.rect.y * windowH / camera.rect.h), .w = model.sprites[i].drawRect.w * (model.sprites[i].scale * model.scale * camera.zoom), .h = model.sprites[i].drawRect.h * (model.sprites[i].scale * model.scale * camera.zoom)}), model.sprites[i].degrees + model.degrees, NULL, (model.sprites[i].flip + model.flip) % 4);
            }
        }
    }
    if (update)
        SDL_RenderPresent(mainRenderer);
}

/** \brief initializes a cText object
 *
 * \param text - a pointer to your cText
 * \param string - the string you want
 * \param rect - SDL_Rect containing bounding box of text
 * \param textColor - color of text
 * \param bgColor - color of background box
 * \param drawPriority - 0 - not drawn. 1-5 - drawn. Lower number = drawn later
 */
void initCText(cText* text, char* string, SDL_Rect rect, SDL_Color textColor, SDL_Color bgColor, int drawPriority)
{
    text->string = calloc(strlen(string), sizeof(char));
    strcpy(text->string, string);
    text->rect = rect;
    text->textColor = textColor;
    text->bgColor = bgColor;
    text->drawPriority = drawPriority;
}

/** \brief clears out a cText and its memory
 *
 * \param text - cText pointer
 */
void destroyCText(cText* text)
{
    free(text->string);
    text->string = NULL;
    text->rect = (SDL_Rect) {0, 0, 0, 0};
    text->textColor = (SDL_Color) {0, 0, 0, 0};
    text->bgColor = (SDL_Color) {0, 0, 0, 0};
    text->drawPriority = 0;
}

/** \brief draws a cText to the screen
 *
 * \param text - cText you want drawn
 * \param camera - cCamera to be used for drawing
 * \param update - if true, immediately presents renderer
 */
void drawCText(cText text, cCamera camera, bool update)
{
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(mainRenderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(mainRenderer, text.bgColor.r, text.bgColor.g, text.bgColor.b, text.bgColor.a);
    SDL_RenderFillRect(mainRenderer, &(text.rect));
    drawText(text.string, text.rect.x - (camera.rect.x * windowW / camera.rect.w), text.rect.y - (camera.rect.y * windowH / camera.rect.h), text.rect.w, text.rect.h, text.textColor, update);
    SDL_SetRenderDrawColor(mainRenderer, r, g, b, a);
}

/** \brief Loads in an image resource
 *
 * \param res - cResource pointer
 * \param filepath - valid string filepath (relative or absolute)
 */
void initCResource(cResource* res, char* filepath)
{
    res->filepath = filepath;
    loadIMG(filepath, &(res->texture));
    SDL_QueryTexture(res->texture, NULL, NULL, &(res->w), &(res->h));
}

/** \brief clears out a cResource and its memory
 *
 * \param res - cResource pointer
 */
void destroyCResource(cResource* res)
{
    strcpy(res->filepath, "\0");
    SDL_DestroyTexture(res->texture);
    res->w = 0;
    res->h = 0;
}

/** \brief initializes a cCamera and its memory
 *
 * \param camera - a cCamera pointer
 * \param rect - the bounding rect of the camera
 */
void initCCamera(cCamera* camera, SDL_Rect rect, double zoom)
{
    camera->rect = rect;
    camera->zoom = zoom;
}

/** \brief clears out a cCamera and its memory
 *
 * \param camera - a cCamera pointer
 */
void destroyCCamera(cCamera* camera)
{
    camera->rect = (SDL_Rect) {0, 0, 0, 0};
    camera->zoom = 1.0;
}

/** \brief Initializes a cScene object.
 *
 * \param scenePtr - pointer to your cScene
 * \param sprites[] - array of cSprites to be drawn
 * \param spriteCount - how many elements in sprites[]
 * \param models[] - array of c2DModels to be drawn
 * \param modelCount - how many elements in model[]
 * \param resources[] - array of cResources
 * \param resCount - how many elements in resources[]
 * \param strings[] - array of cTexts
 * \param stringCount - how many elements in strings[]
 */
void initCScene(cScene* scenePtr, SDL_Color bgColor, cCamera* camera, cSprite sprites[], int spriteCount, c2DModel models[], int modelCount, cResource resources[], int resCount, cText strings[], int stringCount)
{
    scenePtr->camera = camera;
    scenePtr->bgColor = bgColor;
    scenePtr->sprites = (spriteCount > 0) ? &sprites : NULL;
    scenePtr->spriteCount = spriteCount;
    scenePtr->models = (modelCount > 0) ? &models : NULL;
    scenePtr->modelCount = modelCount;
    scenePtr->resources = (resCount > 0) ? &resources : NULL;
    scenePtr->resCount = resCount;
    scenePtr->strings = (stringCount > 0) ? &strings : NULL;
    scenePtr->stringCount = stringCount;
}

/** \brief clears out a cScene and all its memory, including sprites.
 *
 * \param scenePtr - pointer to your cScene
 */
void destroyCScene(cScene* scenePtr)
{
    destroyCCamera(scenePtr->camera);

    if (scenePtr->spriteCount > 0)
    {
        for(int i = 0; i < scenePtr->spriteCount; i++)
            destroyCSprite(scenePtr->sprites[i]);
        scenePtr->spriteCount = 0;
    }

    if (scenePtr->modelCount > 0)
    {
        for(int i = 0; i < scenePtr->modelCount; i++)
            destroyC2DModel(scenePtr->models[i]);
        scenePtr->modelCount = 0;
    }

    if (scenePtr->resCount > 0)
    {
        for(int i = 0; i < scenePtr->resCount; i++)
            destroyCResource(scenePtr->resources[i]);
        scenePtr->resCount = 0;
    }

    if (scenePtr->stringCount > 0)
    {
        for(int i = 0; i < scenePtr->stringCount; i++)
            destroyCText(scenePtr->strings[i]);
        scenePtr->stringCount = 0;
    }

    if (scenePtr->camera)
        destroyCCamera(scenePtr->camera);
}

/** \brief draws the CScene.
 *
 * \param scenePtr - pointer to your cScene
 * \param redraw - if nonzero, will update the screen
 */
void drawCScene(cScene* scenePtr, bool redraw)
{
    SDL_SetRenderDrawColor(mainRenderer, scenePtr->bgColor.r, scenePtr->bgColor.g, scenePtr->bgColor.b, scenePtr->bgColor.a);
    SDL_RenderClear(mainRenderer);
    for(int priority = 5; priority >= 1; priority--)
    {
        for(int i = 0; i < scenePtr->spriteCount; i++)
        {
            if (scenePtr->sprites[i]->drawPriority == priority)
                drawCSprite(*(scenePtr->sprites[i]), *(scenePtr->camera), false);
        }

        for(int i = 0; i < scenePtr->modelCount; i++)
        {
            if (scenePtr->models[i]->drawPriority == priority)
                drawC2DModel(*(scenePtr->models[i]), *(scenePtr->camera), false);
        }

        for(int i = 0; i < scenePtr->stringCount; i++)
        {
            if (scenePtr->strings[i]->drawPriority == priority)
                drawCText(*(scenePtr->strings[i]), *(scenePtr->camera), false);
        }
    }

    if (redraw)
        SDL_RenderPresent(mainRenderer);
}

/** \brief Draws text to the screen using mainFont, wrapped and bounded
 *
 * \param input - text to be drawn
 * \param x - x value of first letter
 * \param y - y value of first letter
 * \param maxW - how wide the text can get before wrapping
 * \param maxH - how tall the text can draw before being cut off
 * \param color - SDL_Color struct of color to be used
 * \param render - if true, immediately presents renderer
 */
void drawText(char* input, int x, int y, int maxW, int maxH, SDL_Color color, bool render)
{
    if (canDrawText)
    {
        SDL_Texture* txtTexture = NULL;
        int* wh;
        wh = loadTextTexture(input, &txtTexture, maxW, color, true);
        SDL_RenderCopy(mainRenderer, txtTexture, &((SDL_Rect){.w = *wh > maxW ? maxW : *wh, .h = *(wh + 1) > maxH ? maxH : *(wh + 1)}),
                                                 &((SDL_Rect){.x =  x, .y = y, .w = *wh > maxW ? maxW : *wh, .h = *(wh + 1) > maxH ? maxH : *(wh + 1)}));


        if (render)
            SDL_RenderPresent(mainRenderer);
        SDL_DestroyTexture(txtTexture);
    }
}
