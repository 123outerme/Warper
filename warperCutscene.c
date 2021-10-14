#include "warperCutscene.h"

void initWarperActor(warperActor* actor, cDoubleRect pos, warperAnimatedSprite* spr)
{
    actor->position = pos;
    actor->animatedSpr = spr;
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

void destroyWarperCutsceneBox(warperCutsceneBox* box)
{
    free(box->boxes);
    free(box->framesAppear);
    box->boxes = NULL;
    box->framesAppear = NULL;
    box->currentBox = 0;

    for(int i = 0; i < box->numBoxes; i++)
    {
        destroyCResource(box->boxResources[i]);
        free(box->boxResources[i]);
        box->boxResources[i] = NULL;
    }

    box->numBoxes = 0;

}

void initWarperCutscene(warperCutscene* cutscene, warperAnimation* animations, warperCutsceneBox* boxes, int animationsLength)
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
}

void iterateWarperCutscene(warperCutscene* cutscene)
{  //TODO: Test
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

void destroyWarperCutscene(warperCutscene* cutscene)
{
    free(cutscene->animations);
    free(cutscene->boxes);
    cutscene->animations = NULL;
    cutscene->boxes = NULL;
    cutscene->totalFrames = 0;
    cutscene->currentAnimation = 0;
    cutscene->numAnimations = 0;
}
