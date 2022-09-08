#include "warperCutscene.h"

void initWarperActor(warperActor* actor, cDoubleRect pos, warperAnimatedSprite* spr, bool pauseSpriteWhenWaiting)
{
    actor->position = pos;
    actor->animatedSpr = spr;
    actor->pauseAnimationWhenWaiting = pauseSpriteWhenWaiting;
}

void importWarperActor(warperActor* actor, char* data, warperAnimatedSprite* animSprites, int numAnimSprites, cSprite* sprites, int numSprites)
{
    char* savePtr = data;
    //printf("\nactor data %s\n", data);
    actor->position.x = strtof(strtok_r(data, "(,", &savePtr), NULL);
    actor->position.y = strtof(strtok_r(savePtr, ",", &savePtr), NULL);
    actor->position.w = strtof(strtok_r(savePtr, ",", &savePtr), NULL);
    actor->position.h = strtof(strtok_r(savePtr, ",;", &savePtr), NULL);
    actor->pauseAnimationWhenWaiting = strtof(strtok_r(savePtr, ";", &savePtr), NULL);


    if (savePtr[0] == '\"')
    {  //if the remaining data (sprite data) was actual data, and not a referenced index
        //printf("> %s\n", tempData);
        int spriteIndex = -1;
        importWarperAnimatedSprite(actor->animatedSpr, savePtr, &spriteIndex);
        if (spriteIndex != -1 && spriteIndex < numSprites)
            actor->animatedSpr->sprite = &(sprites[spriteIndex]);
    }
    else
    {  //if it was an index
        int index = strtol(strtok_r(savePtr, ";\0", &savePtr), NULL, 10);
        if (index != -1 && index < numAnimSprites)
            actor->animatedSpr = &(animSprites[index]);
    }

    printf("actor data %f, %f, %f, %f, %d, %s\n", actor->position.x, actor->position.y, actor->position.w, actor->position.h, actor->pauseAnimationWhenWaiting, actor->animatedSpr->sprite->textureFilepath);
}

char* exportWarperActor(warperActor actor, warperAnimatedSprite** sprites, int numSprites)
{
    int actorDataSize = 2 + 2 + 4 + 2 + 4 * 11 + 1;  //2 wrapping $'s plus 2 wrapping ()'s plus 4 commas plus 2 ;'s plus 4 * 11-character [max] floats plus 1 for good measure
    char* actorData = NULL;
    char* spriteData = NULL;
    int foundIndex = -1;

    for(int j = 0; j < numSprites; j++)
    {
        if (actor.animatedSpr == sprites[j])
        {
            foundIndex = j;
            break;
        }
    }
    if (foundIndex > -1)
    {
        int chars = charsInNum(foundIndex);
        spriteData = calloc(chars + 1, sizeof(char));
        actorDataSize += chars;  //plus the length of the actor sprite index
        snprintf(spriteData, chars + 1, "%d", foundIndex);
    }
    else
    {
        spriteData = exportWarperAnimatedSprite(*actor.animatedSpr, -1);
        actorDataSize += strlen(spriteData);  //plus the length of the actor sprite data
    }

    actorData = calloc(actorDataSize, sizeof(char));

    if (!actorData || !spriteData)
    {
        cLogEvent(warperLogger, "ERROR", "WARPER: CutsceneBox", "Cannot initialize actor data");
        return NULL;
    }

    snprintf(actorData, actorDataSize, "$(%f,%f,%f,%f);%d;%s$", actor.position.x, actor.position.y, actor.position.w, actor.position.h, actor.pauseAnimationWhenWaiting, spriteData);

    free(spriteData);

    return actorData;
}

void initWarperAnimation(warperAnimation* animation, warperActor* actors, int actorsLength, int frames)
{
    animation->actors = calloc(actorsLength, sizeof(warperActor));
    animation->currentFrame = 0;

    if (!animation->actors)
    {
        cLogEvent(warperLogger, "ERROR", "WARPER: Animation", "Cannot initialize actors/positions array");
        animation->numActors = 0;
        animation->frameCount = 0;
        return;
    }

    for(int i = 0; i < actorsLength; i++)
        animation->actors[i] = actors[i];

    animation->numActors = actorsLength;
    animation->frameCount = frames;

}

/** \brief Destroys animation data, but does not destroy sprite data referenced by animation's actors (this must be cleaned up separately)
 *
 * \param animation warperAnimation* - the animation pointer to what needs to be destroyed
 */
void destroyWarperAnimation(warperAnimation* animation)
{
    free(animation->actors);
    animation->actors = NULL;
    animation->frameCount = 0;
    animation->numActors = 0;
    animation->currentFrame = 0;
}

void importWarperAnimation(warperAnimation* animation, char* data, warperAnimatedSprite* animSprites, int numAnimSprites, cSprite* sprites, int numSprites)
{
    char* savePtr = data;
    //printf("animation data %s\n", data);
    animation->numActors = strtol(strtok_r(data, "&;", &savePtr), NULL, 10);
    animation->actors = calloc(animation->numActors, sizeof(warperActor));

    animation->frameCount = strtol(strtok_r(savePtr, "&;", &savePtr), NULL, 10);
    animation->currentFrame = 0;

    for(int i = 0; i < animation->numActors; i++)
    {
        char* actorData = strtok_r(savePtr, "$", &savePtr);
        importWarperActor(&(animation->actors[i]), actorData, animSprites, numAnimSprites, sprites, numSprites);
    }
}

/** \brief
 *
 * \param animation warperAnimation - animation to export
 * \param sprites warperAnimatedSprite** - NULL to export raw animated sprite data instead of reference. Otherwise contains array of unique animated sprites to match reference to
 * \param numSprites int - the length of `sprites`
 * \return char* - exported animation data
 *
 */
char* exportWarperAnimation(warperAnimation animation, warperAnimatedSprite** sprites, int numSprites)
{
    int animDataSize = charsInNum(animation.numActors) + charsInNum(animation.frameCount) + 2 + 2 + 1;  //length of numbers plus 2 wrapping characters plus 2 separators plus 1 for good measure
    char* animData = NULL;
    char** actorsData = calloc(animation.numActors, sizeof(char*));

    //add array of actors
    for(int i = 0; i < animation.numActors; i++)
    {
        actorsData[i] = exportWarperActor(animation.actors[i], sprites, numSprites);
        animDataSize += strlen(actorsData[i]) + 1;  //plus separator character
    }

    animData = calloc(animDataSize + 1, sizeof(char));
    snprintf(animData, animDataSize, "&%d;%d;", animation.numActors, animation.frameCount);

    for(int i = 0; i < animation.numActors; i++)
    {
        strncat(animData, actorsData[i], animDataSize);
        if (i < animation.numActors - 1)
            strncat(animData, "+", animDataSize);
        free(actorsData[i]);
    }
    free(actorsData);

    strncat(animData, "&", animDataSize);

    return animData;
}

void initWarperCutsceneBox(warperCutsceneBox* box, warperTextBox** boxes, int* framesAppear, int boxesLength)
{
    box->currentBox = 0;

    if (boxesLength > 0)
    {
        box->boxes = calloc(boxesLength, sizeof(warperTextBox*));
        box->boxResources = calloc(boxesLength, sizeof(cResource*));
        box->framesAppear = calloc(boxesLength, sizeof(int));
        box->currentBox = 0;

        if (!box->boxes || !box->framesAppear || !box->boxResources)
        {
            cLogEvent(warperLogger, "ERROR", "WARPER: CutsceneBox", "Cannot initialize boxes array");
            box->numBoxes = 0;
            return;
        }

        for (int i = 0; i < boxesLength; i++)
        {
            box->boxes[i] = boxes[i];
            box->boxResources[i] = malloc(sizeof(cResource));
            if (!box->boxResources[i])
            {
                cLogEvent(warperLogger, "ERROR", "WARPER: CutsceneBox", "Cannot initialize box resources");
                box->numBoxes = 0;
                return;
            }
            initCResource(box->boxResources[i], box->boxes[i], drawWarperTextBox, destroyWarperTextBox, 0);
            box->framesAppear[i] = framesAppear[i];
        }
    }
    else
    {
        box->boxes = NULL;
        box->boxResources = NULL;
        box->framesAppear = NULL;
    }

    box->numBoxes = boxesLength;
}

void incrementWarperCutsceneBox(warperCutscene* cutscene)
{
    int curAnimation = cutscene->currentAnimation;
    if (cutscene->boxes[curAnimation].boxResources != NULL)
    {
        cutscene->boxes[curAnimation].boxResources[cutscene->boxes[curAnimation].currentBox]->renderLayer = 0;  //hide the old box
        cutscene->boxes[curAnimation].currentBox++;  //increment to the next box
    }
    cutscene->waitingForBox = false;  //unblock the cutscene (letting another box or the overall cutscene continue)
}

/** \brief Destroys cutscene textbox data, but does not destroy warperTextBox data referenced by the cutscene data (this must be cleaned up separately)
 *
 * \param box warperCutsceneBox* - pointer to the cutscene box to be destroyed
 * \param destroyResources bool - if true, will destroy the resources generated upon init. False is the recommended option as the resources will be cleaned up through the cScene handling the textboxes in most cases
 */
void destroyWarperCutsceneBox(warperCutsceneBox* box, bool destroyResources)
{
    free(box->boxes);
    free(box->framesAppear);
    box->boxes = NULL;
    box->framesAppear = NULL;
    box->currentBox = 0;

    if (destroyResources)
    {  //should be passed as false when box resources are put into the scene drawing the animation
        for(int i = 0; i < box->numBoxes; i++)
        {
            if (box->boxResources[i] != NULL)
                destroyCResource(box->boxResources[i]);

            free(box->boxResources[i]);
            box->boxResources[i] = NULL;
        }
    }

    box->numBoxes = 0;
}

void importWarperCutsceneBox(warperCutsceneBox* box, char* data)
{
    //TODO complete
    box->currentBox = 0;

    char* savePtr = data;
    box->numBoxes = strtol(strtok_r(savePtr, "#%", &savePtr), NULL, 10);
    if (box->numBoxes > 0)
    {
        box->boxes = calloc(box->numBoxes, sizeof(warperTextBox*));  //allocate array for boxes
        box->framesAppear = calloc(box->numBoxes, sizeof(int));  //allocate array for framesAppear
        box->boxResources = calloc(box->numBoxes, sizeof(cResource*));  //allocate array for resources

        for(int i = 0; i < box->numBoxes; i++)
        {
            box->boxes[i] = malloc(sizeof(warperTextBox));  //allocate current box
            //parse box data
            char* boxData = strtok_r(savePtr, "{}$", &savePtr);
            //printf("boxData = %s\n", boxData);
            importWarperTextBox(box->boxes[i], boxData);
            //printf("import cutscene box %s\n", savePtr);
        }

        for(int i = 0; i < box->numBoxes; i++)
        {
            box->framesAppear[i] = strtol(strtok_r(savePtr, "#$", &savePtr), NULL, 10);
            //printf("import cutscene frames %s\n", savePtr);

            //init box resource
            box->boxResources[i] = malloc(sizeof(cResource));
            //printf("box resource %x\n", box->boxResources[i]);
            initCResource(box->boxResources[i], box->boxes[i], drawWarperTextBox, destroyWarperTextBox, 0);
        }
    }
    else
    {
        box->boxes = NULL;
        box->framesAppear = NULL;
        box->boxResources = NULL;
    }
    //
}

char* exportWarperCutsceneBox(warperCutsceneBox box)
{
    int boxesLen = 2 + charsInNum(box.numBoxes) + 2 + 1;  //2 containing '$' characters plus the chars to represent the number of boxes, plus 2 for dividing % characters, plus 1 for good measure
    char** animBoxData = calloc(box.numBoxes, sizeof(char*));

    for(int i = 0; i < box.numBoxes; i++)
    {
        int dataLen = 0;
        animBoxData[i] = exportWarperTextBox(*(box.boxes[i]), &dataLen);  //convert every box's data
        boxesLen += dataLen + 1 + charsInNum(box.framesAppear[i]) + 1;  //calculate the length of the string we need to allocate: size of box data, plus 1 for separator, plus size of framesAppear int, plus 1 for separator
    }

    char* boxesData = calloc(boxesLen + 1, sizeof(char));  //allocate the string we will return
    snprintf(boxesData, boxesLen, "#%d%%", box.numBoxes);  //put the preliminary information in there ('%%' delimits the percent character in formatted print functions)

    for(int i = 0; i < box.numBoxes; i++)
    {
        //iterate through the exported data array, place it into the final string, then deallocate it
        strncat(boxesData, animBoxData[i], boxesLen);
        free(animBoxData[i]);

        if (i < box.numBoxes - 1)
            strncat(boxesData, "$", boxesLen);  //add the separator
    }
    free(animBoxData);
    strncat(boxesData, "%", boxesLen);

    //now convert framesAppear data
    for(int i = 0; i < box.numBoxes; i++)
    {
        int framesSize = charsInNum(box.framesAppear[i]) + 1;
        char* framesData = calloc(framesSize, sizeof(char));
        snprintf(framesData, framesSize, "%d", box.framesAppear[i]);
        strncat(boxesData, framesData, boxesLen);
        free(framesData);

        if (i < box.numBoxes - 1)
            strncat(boxesData, "$", boxesLen);  //add the separator
    }

    strncat(boxesData, "%#", boxesLen);  //close it off

    //printf("boxesLen = %d for str \"%s\"\n", boxesLen, boxesData);

    return boxesData;
}

void initWarperCutscene(warperCutscene* cutscene, warperAnimation* animations, warperCutsceneBox* boxes, int animationsLength, char* tilemapFilepath, int tilemapLine)
{
    cutscene->animations = calloc(animationsLength, sizeof(warperAnimation));
    cutscene->boxes = calloc(animationsLength, sizeof(warperCutsceneBox));
    cutscene->totalFrames = 0;
    cutscene->currentAnimation = 0;
    cutscene->waitingForBox = false;

    if (!cutscene->animations || !cutscene->boxes)
    {
        cLogEvent(warperLogger, "ERROR", "WARPER: Animation", "Cannot initialize animations array");
        return;
    }

    for(int i = 0; i < animationsLength; i++)
    {
        cutscene->animations[i] = animations[i];
        cutscene->totalFrames += animations[i].frameCount;
        cutscene->boxes[i] = boxes[i];
    }

    cutscene->numAnimations = animationsLength;

    cutscene->tilemapFilepath = calloc(strlen(tilemapFilepath) + 1, sizeof(char));
    if (!cutscene->tilemapFilepath)
    {
        cLogEvent(warperLogger, "ERROR", "WARPER: Animation", "Cannot initialize tilemap filepath string");
        return;
    }
    else
        strncpy(cutscene->tilemapFilepath, tilemapFilepath, strlen(tilemapFilepath));

    cutscene->tilemapLine = tilemapLine;
}

void iterateWarperCutscene(warperCutscene* cutscene)
{
    int curAnimation = cutscene->currentAnimation;

    if (curAnimation < cutscene->numAnimations)
    {
        if (!cutscene->waitingForBox)
            cutscene->animations[curAnimation].currentFrame++;  //next frame

        if (cutscene->boxes[curAnimation].numBoxes > 0)  //if there is at least one box for this current animation, handle boxes
        {
            int currentBox = cutscene->boxes[curAnimation].currentBox;
            if (currentBox < cutscene->boxes[curAnimation].numBoxes && cutscene->boxes[curAnimation].framesAppear[currentBox] <= cutscene->animations[curAnimation].currentFrame)
            {  //if the current box at this current animation should appear on this frame
                cutscene->boxes[curAnimation].boxResources[currentBox]->renderLayer = 2;
                cutscene->waitingForBox = true;  //display the current box and pause at the end of the current animation until done
            }
        }

        if (cutscene->animations[curAnimation].currentFrame >= cutscene->animations[curAnimation].frameCount)  //if we are going to the next animation
        {
            if (!cutscene->waitingForBox)  //if we aren't waiting on a box
                cutscene->currentAnimation++;  //go to the next animation
        }
        else
        {  //we are still in the current animation
            for(int i = 0; i < cutscene->animations[curAnimation].numActors; i++)
            {  //move to actors' next positions
                if (curAnimation > 0 && !cutscene->waitingForBox)  //if a next position exists
                {
                    cutscene->animations[curAnimation].actors[i].animatedSpr->sprite->drawRect.x += (cutscene->animations[curAnimation].actors[i].position.x - cutscene->animations[curAnimation - 1].actors[i].position.x) / cutscene->animations[curAnimation].frameCount;
                    //sprite's x position += the difference between last position and the position we need to head to, divided by the number of frames it will take to get there
                    cutscene->animations[curAnimation].actors[i].animatedSpr->sprite->drawRect.y += (cutscene->animations[curAnimation].actors[i].position.y - cutscene->animations[curAnimation - 1].actors[i].position.y) / cutscene->animations[curAnimation].frameCount;
                    //sprite's y position += the difference between last position and the position we need to head to, divided by the number of frames it will take to get there
                    cutscene->animations[curAnimation].actors[i].animatedSpr->sprite->drawRect.w = cutscene->animations[curAnimation].actors[i].position.w;
                    cutscene->animations[curAnimation].actors[i].animatedSpr->sprite->drawRect.h = cutscene->animations[curAnimation].actors[i].position.h;
                    //sprite's dimensions immediately become the new dimensions
                }
            }
        }
    }
}

void destroyWarperCutscene(warperCutscene* cutscene, bool destroyAnimations, bool destroyTextBoxes, bool destroyBoxResources)
{
    if (destroyAnimations)
    {
        for(int i = 0; i < cutscene->numAnimations; i++)
        {
            destroyWarperAnimation(&(cutscene->animations[i]));
        }
    }

    if (destroyTextBoxes)
    {
        for(int i = 0; i < cutscene->numAnimations; i++)
        {
            destroyWarperCutsceneBox(&(cutscene->boxes[i]), destroyBoxResources);
        }
    }

    free(cutscene->animations);
    free(cutscene->boxes);
    cutscene->animations = NULL;
    cutscene->boxes = NULL;
    cutscene->totalFrames = 0;
    cutscene->currentAnimation = 0;
    cutscene->numAnimations = 0;
}

/** \brief
 *
 * \param cutscene warperCutscene* - cutscene with the data to be filled in
 * \param filepath char* - the filepath where the cutscene data can be found
 * \param cutsceneSprites cSprite*** - the reference to the cSprite* array that will be fed into the cScene [one additional slot will be left open for the cursor]
 * \param numSprites int - reference to an int that will, on completion, hold the number of sprites [additional cursor slot not included]
 * \param cutsceneResources cResources*** - the reference to the cResource* array that will be fed into the cScene
 * \param numSprites int - reference to an int that will, on completion, hold the number of resources
 */
void importWarperCutscene(warperCutscene* cutscene, char* filepath, cSprite*** cutsceneSprites, int* numSprites, cResource*** cutsceneResources, int* numResources)
{
    /* file format:
    1: Lengths of each array
    1: cSprite array
    2: animatedSprite array
    3: textbox array
    4: animation data (actors etc)
    */
    cutscene->currentAnimation = 0;
    cutscene->waitingForBox = false;

    //start interpreting the lengths of each array
    int dataSize = 2 + 3 * 3 + 1;  //2 digits + 3-char numbers (max expected) * 3 numbers + 1 for good measure
    char* data = calloc(dataSize, sizeof(char));
    readLine(filepath, 0, dataSize, &data);  //read line 1
    char* savePtr = data;
    int spritesLen = strtol(strtok_r(savePtr, ",", &savePtr), NULL, 10);
    int animSpritesLen = strtol(strtok_r(savePtr, ",", &savePtr), NULL, 10);
    cutscene->numAnimations = strtol(savePtr, NULL, 10);
    printf("%d,%d,%d\n", spritesLen, animSpritesLen, cutscene->numAnimations);
    free(data);

    //start interpreting the cSprite data
    cSprite* sprites = calloc(spritesLen, sizeof(cSprite));
    dataSize = 200 * spritesLen + 1; //approx. 200 characters per sprite
    data = calloc(dataSize, sizeof(char));
    readLine(filepath, 1, dataSize, &data);  //read line 2
    savePtr = data;
    //get each cSprite
    for(int i = 0; i < spritesLen; i++)
    {
        char* spriteData = strtok_r(savePtr, "|@", &savePtr);
        //printf("%s\n", spriteData);
        importCSprite(&(sprites[i]), spriteData);
    }
    free(data);

    //start interpreting the animatedSprite data
    warperAnimatedSprite* animSprites = calloc(animSpritesLen, sizeof(warperAnimatedSprite));
    dataSize = 5000 * animSpritesLen;  //approx 5,000 characters per animated sprite TODO REDUCE THIS!!!
    //int animSpritesDataSize = dataSize;
    data = calloc(dataSize, sizeof(char));
    readLine(filepath, 2, dataSize, &data);
    if (data == NULL)
        printf("ERROR: NO SPACE\n");
    savePtr = data;
    printf("TESTING\n%s\n", data);
    //get each animated sprite
    for(int i = 0; i < animSpritesLen; i++)
    {
        int spriteIndex = -1;
        char* spriteData = strtok_r(savePtr, "|@", &savePtr);
        //printf("%s\n", spriteData);
        importWarperAnimatedSprite(&(animSprites[i]), spriteData, &spriteIndex);
        if (spriteIndex > -1)
        {
            animSprites[i].sprite = &(sprites[spriteIndex]);
        }
        else
        {
            //ERROR
            printf("ERROR: imported sprite index not found\n");
        }
    }
    free(data);
    //*/

    //import textboxes
    warperCutsceneBox* boxes = calloc(cutscene->numAnimations, sizeof(warperCutsceneBox));
    dataSize = 500 * cutscene->numAnimations;  //approx 500 characters per box (but usually way less for no box)
    /*NOTE: This must be a max() expression because of the way readLine has been implemented (aka poorly)
            If a previous line is too long, then the counter for lines will increment at the max length, resulting in the string reutrned being from somewhere BEFORE the target line
            This fix is also applied to the below animation import
    //*/
    //TODO: this can be fixed by simply reading the file stream and counting newline characters
    data = calloc(dataSize, sizeof(char));
    readLine(filepath, 3, dataSize, &data);
    savePtr = data;
    printf("TESTING\n%s\n", data);
    //get each box
    for(int i = 0; i < cutscene->numAnimations; i++)
    {
        char* boxData = strtok_r(savePtr, "|@", &savePtr);
        printf("%s\n", boxData);
        importWarperCutsceneBox(&(boxes[i]), boxData);
    }
    free(data);
    cutscene->boxes = boxes;

    //import the rest of the animation data
    warperAnimation* animations = calloc(cutscene->numAnimations, sizeof(warperAnimation));
    dataSize = 100 * cutscene->numAnimations;  //approx 100 characters per animation
    data = calloc(dataSize, sizeof(char));
    readLine(filepath, 4, dataSize, &data);
    savePtr = data;
    //printf("TESTING\n%s\n", data);
    //get each animation
    for(int i = 0; i < cutscene->numAnimations; i++)
    {
        char* animationData = strtok_r(savePtr, "|@", &savePtr);
        printf("%s\n", animationData);
        importWarperAnimation(&(animations[i]), animationData, animSprites, animSpritesLen, sprites, spritesLen);
    }
    free(data);
    cutscene->animations = animations;
    //done filling in cutscene

    //process cutscene for conversion to the structures needed by the cScene
    //sprites
    *numSprites = spritesLen;
    *cutsceneSprites = calloc(spritesLen + 1, sizeof(cSprite*));
    for(int i = 0; i < spritesLen; i++)
        *cutsceneSprites[i] = &sprites[i];

    //resources
    *numResources = 0;
    for(int i = 0; i < cutscene->numAnimations; i++)
        *numResources += cutscene->boxes[i].numBoxes;  //get number of resources


    *cutsceneResources = calloc(*numResources, sizeof(cResource*));
    int crIndex = 0;
    for(int i = 0; i < cutscene->numAnimations; i++)
    {
        //generate array of cResource*'s
        for(int j = 0; j < cutscene->boxes[i].numBoxes; j++)
        {
            (*cutsceneResources)[crIndex++] = cutscene->boxes[i].boxResources[j];
        }
    }
}

void exportWarperCutscene(warperCutscene cutscene, char* filepath)
{
    createFile(filepath);

    int maxNumAnimatedSprs = 0;
    for(int i = 0; i < cutscene.numAnimations; i++)
        maxNumAnimatedSprs += cutscene.animations[i].numActors;  //total up the theoretical maximum number of different animated sprites

    warperAnimatedSprite** animatedSprites = calloc(maxNumAnimatedSprs, sizeof(warperAnimatedSprite*));
    int numAnimatedSprs = 0;
    //NOTE: This "find all unique animated sprite pointers" thing would be a great thing to use hash tables for?
    for(int i = 0; i < cutscene.numAnimations; i++)
    {  //for each animation
        for(int j = 0; j < cutscene.animations[i].numActors; j++)
        {  //for each actor in each animation
            bool found = false;
            for(int k = 0; k < numAnimatedSprs; k++)
            {  //for each sprite pointer already indexed
                if (animatedSprites[k] == cutscene.animations[i].actors[j].animatedSpr)
                {  //if it's one we already have indexed
                    found = true;
                    break;  //we don't need to add it again
                }
            }
            if (!found)
            {  //if we haven't indexed it yet, add it
                animatedSprites[numAnimatedSprs] = cutscene.animations[i].actors[j].animatedSpr;
                numAnimatedSprs++;
            }
        }
    }

    //find each unique cSprite
    //max number of cSprites == numAnimatedSprs
    cSprite** sprites = calloc(numAnimatedSprs, sizeof(numAnimatedSprs));
    int numSprs = 0;
    for(int i = 0; i < numAnimatedSprs; i++)
    {
        bool found = false;
        for(int j = 0; j < numSprs; j++)
        {
            if (animatedSprites[i]->sprite == sprites[j])
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            sprites[numSprs] = animatedSprites[i]->sprite;
            numSprs++;
        }
    }

    //export the size of each array
    int lengthsSize = 2 + charsInNum(numSprs) + charsInNum(numAnimatedSprs) + charsInNum(cutscene.numAnimations) + 1;  //2 separator characters, + size of all 3 numbers + 1 for good measure
    char* lengthsData = calloc(lengthsSize, sizeof(char));
    snprintf(lengthsData, lengthsSize, "%d,%d,%d", numSprs, numAnimatedSprs, cutscene.numAnimations);
    appendLine(filepath, lengthsData, true);
    free(lengthsData);

    appendLine(filepath, "|", false);
    //export each cSprite
    for(int i = 0; i < numSprs; i++)
    {
        char* data = exportCSprite(*sprites[i]);
        appendLine(filepath, data, false);
        free(data);
        if (i < numSprs - 1)
            appendLine(filepath, "@", false);
    }
    appendLine(filepath, "|", true);

    appendLine(filepath, "|", false);
    //export each animated sprite, referencing the unique cSprite by its index in the exported cSprite table
    for(int i = 0; i < numAnimatedSprs; i++)
    {
        int sprIndex = -1;  //just in case, this will print the string value. However, this should never occur in practice

        //find index of referenced sprite
        for(int j = 0; j < numSprs; j++)
        {
            if (animatedSprites[i]->sprite == sprites[j])
            {
                sprIndex = j;
                break;
            }
        }

        char* data = exportWarperAnimatedSprite(*animatedSprites[i], sprIndex);
        appendLine(filepath, data, false);
        appendLine(filepath, "@", false);
        free(data);
    }
    appendLine(filepath, "|", true);

    //export each "key frame" to the file, referencing animated sprites as numbers corresponding to their index in the animated sprite array
    //>export text boxes and the frames they appear, ignoring empty text boxes
    appendLine(filepath, "|", false);
    for(int i = 0; i < cutscene.numAnimations; i++)
    {
        char* boxesData = exportWarperCutsceneBox(cutscene.boxes[i]);
        appendLine(filepath, boxesData, false);
        free(boxesData);  //TODO: this causes a weird breakpoint to appear for whatever reason but boxesData will always be calloc'd properly so I'm not quite sure what's going on here
        if (i < cutscene.numAnimations - 1)
            appendLine(filepath, "@", false);

    }
    appendLine(filepath, "|", true);

    //>export animation information
    appendLine(filepath, "|", false);
    for(int i = 0; i < cutscene.numAnimations; i++)
    {
        //>>export data for each animation, referencing the actor's animated sprites by their index in the previously exported animated sprite array (this will be much easier (?) with hash tables as well)
        char* animationData = exportWarperAnimation(cutscene.animations[i], animatedSprites, numAnimatedSprs);
        appendLine(filepath, animationData, false);
        if (i < cutscene.numAnimations - 1)
            appendLine(filepath, "@", false);
        free(animationData);
    }
    appendLine(filepath, "|", true);
}
