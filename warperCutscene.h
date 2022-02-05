#ifndef WARPERCUTSCENE_H_INCLUDED
#define WARPERCUTSCENE_H_INCLUDED

#include "warper.h"
#include "warperInterface.h"

typedef struct _warperActor {
    cDoubleRect position; /**< actual actor position */
    warperAnimatedSprite* animatedSpr; /**< sprite->drawRect is to be ignored; this is only for render info purposes */
    bool pauseAnimationWhenWaiting;  /**< true, if this animated sprite should not continue to play while waiting for a textbox to be closed. This flag isn't used internally, and should be read by whatever function is playing the cutscene to block animations when waiting */
} warperActor;

typedef struct _warperAnimation
{
    warperActor* actors;  /**< Array of `warperActor`s */
    int numActors;
    int currentFrame;
    int frameCount;  /**< Number of transition frames between last animation and this one */
} warperAnimation;

typedef struct _warperCutsceneBox
{
    warperTextBox** boxes; /**< Array of `warperTextBox*`s */
    cResource** boxResources;  /**< Array of `cResource*`s */
    int* framesAppear;  /**< array of integers that specify what frame of the corresponding animation this box should appear (pauses current animation until it closes and lasts until the next animation starts playing) */
    int numBoxes;
    int currentBox;
} warperCutsceneBox;

typedef struct _warperCutscene {
    warperAnimation* animations;
    warperCutsceneBox* boxes; /**< Array of `warperCutsceneBox`s, length equal to numAnimations. Each box in the array corresponds to an animation at the same index. boxes[i].numBoxes == 0 means to not display a box there */
    int numAnimations;
    int currentAnimation;
    int totalFrames;  /**< Total number of animations' (transitions') frames */
    bool waitingForBox;  /**< true if animation should not progress past the current animation */
    char* tilemapFilepath;  /**< holds the filepath to the source of the tilemap we base this cutscene on */
    int tilemapLine;  /**< holds the line number corresponding to the tilemap we base this cutscene on */
} warperCutscene;

void initWarperActor(warperActor* actor, cDoubleRect pos, warperAnimatedSprite* spr, bool pauseSpriteWhenWaiting);
char* exportWarperActor(warperActor actor, warperAnimatedSprite** sprites, int numSprites);

void initWarperAnimation(warperAnimation* animation, warperActor* actors, int actorsLength, int frames);
void destroyWarperAnimation(warperAnimation* animation);
char* exportWarperAnimation(warperAnimation animation, warperAnimatedSprite** sprites, int numSprites);

void initWarperCutsceneBox(warperCutsceneBox* box, warperTextBox** boxes, int* framesAppear, int boxesLength);
void incrementWarperCutsceneBox(warperCutscene* cutscene);
void destroyWarperCutsceneBox(warperCutsceneBox* box, bool destroyResources);
char* exportWarperCutsceneBox(warperCutsceneBox box);

void initWarperCutscene(warperCutscene* cutscene, warperAnimation* animations, warperCutsceneBox* boxes, int animationsLength, char* tilemapFilepath, int tilemapLine);
void iterateWarperCutscene(warperCutscene* cutscene);
void destroyWarperCutscene(warperCutscene* cutscene,  bool destroyAnimations, bool destroyTextBoxes, bool destroyBoxResources);

void importWarperCutscene(warperCutscene* cutscene, char* filepath);
void exportWarperCutscene(warperCutscene cutscene, char* filepath);

#endif // WARPERCUTSCENE_H_INCLUDED
