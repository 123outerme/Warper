#include "csIO.h"

/** \brief gets a keypress
 *
 * \param useMouse - whether or not to count a click as a key
 * \return key you pressed as an SDL_Keycode, or -1 if a quit signal was sent
 */
SDL_Keycode getKey(bool useMouse)
{
    SDL_Event e;
    SDL_Keycode keycode = 0;
    while(SDL_PollEvent(&e) != 0)
    {
        if(e.type == SDL_QUIT)
            keycode = -1;
        else
        {
            if(e.type == SDL_KEYDOWN)
                keycode = e.key.keysym.sym;
            if (e.type == SDL_MOUSEBUTTONDOWN && useMouse)
                keycode = 1;
        }
    }
    return keycode;
}

/** \brief Just like getKey(), except it waits
 *
 * \param useMouse - whether or not to count a click as a key
 * \return key you pressed as an SDL_Keycode, or -1 if a quit signal was sent
 */
SDL_Keycode waitForKey(bool useMouse)
{
    SDL_Event e;
    bool quit = false;
    SDL_Keycode keycode = SDLK_ESCAPE;
    while(!quit)
    {
        while(SDL_PollEvent(&e) != 0)
        {
            if(e.type == SDL_QUIT)
                quit = true;
            else
            {
                if(e.type == SDL_KEYDOWN)
                {
                    keycode = e.key.keysym.sym;
                    quit = true;
                }
                if (e.type == SDL_MOUSEBUTTONDOWN && useMouse)
                {
                    keycode = 1;
                    quit = true;
                }
            }
        }
    }
    return keycode;
}

/** \brief tries to map a key
* \param key - the new key you want to set
* \param keyslot - the position in keymaps[] you want to set to
* \return 1 if successful and sets the keymap, otherwise 0 and preserves keymaps[].
*/
bool setKey(SDL_Scancode key, int keyslot)
{
    bool conflict = false;
    for(int i = 0; i < MAX_KEYMAPS; i++)
    {
        if (keymaps[i] == SDL_GetScancodeFromKey(key))
            conflict = true;
    }

    if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LCTRL || SDL_GetScancodeFromKey(key) == SDL_SCANCODE_RCTRL || SDL_GetScancodeFromKey(key) == SDL_SCANCODE_MINUS || SDL_GetScancodeFromKey(key) == SDL_SCANCODE_EQUALS)
        conflict = true;
    if (!conflict)
        keymaps[keyslot] = SDL_GetScancodeFromKey(key);

    return !conflict;
}
