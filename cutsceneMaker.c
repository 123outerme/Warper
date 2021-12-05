#include "cutsceneMaker.h"

#define TEST_TILEMAP_FILEPATH "./maps/testMap.txt"
#define TEST_TILEMAP_LINENUM 0

#define TEST_CUTSCENE_EXPORT_FILEPATH "./assets/cutscenes/testCutscene.txt"

bool createNewCutscene()
{
    warperTilemap tilemap;

    //pick tilemap
    //...

    importWarperTilemap(&tilemap, TEST_TILEMAP_FILEPATH, TEST_TILEMAP_LINENUM);

    warperCutscene cutscene;
    int keyFrames = 0;  //number of "key frames" that have been created and readied for this cutscene
    //initialize cutscene
    {
        warperAnimation emptyAnimations[10];
        warperCutsceneBox emptyBoxes[10];

        for(int i = 0; i < 10; i++)
        {
            initWarperAnimation(&emptyAnimations[i], NULL, 0, 0);
            initWarperCutsceneBox(&emptyBoxes[i], NULL, NULL, 0);
        }

        initWarperCutscene(&cutscene, emptyAnimations, emptyBoxes, 10, TEST_TILEMAP_FILEPATH, TEST_TILEMAP_LINENUM);
    }

    //create cutscene
    bool quit = false, quitAll = false;
    cInputState input;

    while(!quit)
    {
        input = cGetInputState(true);

        if (input.quitInput || input.keyStates[SDL_SCANCODE_RETURN] || input.keyStates[SDL_SCANCODE_ESCAPE])
        {
            quit = true;
            quitAll = input.quitInput;
        }
        //
    }

    destroyWarperTilemap(&tilemap);

    exportWarperCutscene(cutscene, TEST_CUTSCENE_EXPORT_FILEPATH);

    return quitAll;
}
