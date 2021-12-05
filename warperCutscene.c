#include "warperCutscene.h"

void initWarperActor(warperActor* actor, cDoubleRect pos, warperAnimatedSprite* spr, bool pauseSpriteWhenWaiting)
{
    actor->position = pos;
    actor->animatedSpr = spr;
    actor->pauseAnimationWhenWaiting = pauseSpriteWhenWaiting;
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
    cutscene->boxes[curAnimation].boxResources[cutscene->boxes[curAnimation].currentBox]->renderLayer = 0;  //hide the old box
    cutscene->boxes[curAnimation].currentBox++;  //increment to the next box

    if (cutscene->boxes[curAnimation].currentBox >= cutscene->boxes[curAnimation].numBoxes)
    {  //if we've gone beyond the number of boxes we have
        cutscene->waitingForBox = false;  //unblock the cutscene
    }
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

void importWarperCutscene(char* filepath)
{
    //TODO
}

void exportWarperCutscene(warperCutscene cutscene, char* filepath)
{
    int maxNumAnimatedSprs = 0;
    for(int i = 0; i < cutscene.numAnimations; i++)
        maxNumAnimatedSprs += cutscene.animations[i].numActors;  //total up the theoretical maximum number of different animated sprites

    warperAnimatedSprite** animatedSprites = calloc(maxNumAnimatedSprs, sizeof(warperAnimatedSprite*));
    int numAnimatedSprs = 0;
    //NOTE: This "find all unique animated sprite pointers" thing would be a great thing to use hash tables for!
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

    //export each animated sprite
    for(int i = 0; i < numAnimatedSprs; i++)
    {
        char* data = exportWarperAnimatedSprite(*animatedSprites[i]);
        //export to the file
        free(data);
    }

    //export each "key frame" to the file, referencing animated sprites as numbers corresponding to their index in the animated sprite array
    //>export text boxes and the frames they appear, ignoring empty text boxes
    //>export animation information
    //>>export data for each actor, referencing the animated sprites by their index in the previously exported animated sprite array (this will be much easier with hash tables as well)
}
