#include "csInput.h"



/** \brief gets a keystate
 *
 * \param useMouse - whether or not to count a click as a key
 * \return filled cInputState with the state of key and mouse inputs
 */
cInputState cGetInputState(bool useMouse)
{
    SDL_Event e;

    cInputState state = {SDL_GetKeyboardState(NULL), (SDL_MouseButtonEvent) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, false, false};

    while(SDL_PollEvent(&e) != 0)
    {
        //state.key = e.key; //saved out here so no matter what state.key will be populated
        if(e.type == SDL_QUIT)
            state.quitInput = true;
        else
        {
            if (e.type == SDL_MOUSEBUTTONDOWN && useMouse)
            {
                state.isClick = true;
                state.click = e.button;
            }
        }
    }
    return state;
}

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
            {
                keycode = -1;
                quit = true;
            }
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

void handleTextInput(char* text, SDL_Keycode key, int maxChar)
{
    static bool shift = false;
    static int namePos = 0;
    if (key >= SDLK_SPACE && key <= SDLK_z && namePos < maxChar)
    {
        if (shift && (key < SDLK_a || key > SDLK_z) && key != SDLK_SPACE)  //top row, hardcoded
        {
            SDL_Keycode temp = '_';
            if (key == SDLK_BACKQUOTE)
                temp = '~';
            if (key == SDLK_1)
                temp = '!';
            if (key == SDLK_2)
                temp = '@';
            if (key == SDLK_3)
                temp = '#';
            if (key == SDLK_4)
                temp = '$';
            if (key == SDLK_5)
                temp = '%';
            if (key == SDLK_6)
                temp = '^';
            if (key == SDLK_7)
                temp = '&';
            if (key == SDLK_8)
                temp = '*';
            if (key == SDLK_9)
                temp = '(';
            if (key == SDLK_0)
                temp = ')';
            if (key == SDLK_MINUS)
                temp = '_';
            if (key == SDLK_EQUALS)
                temp = '+';

            if (key == SDLK_BACKSLASH)
                temp = '|';
            if (key == SDLK_SEMICOLON)
                temp = ':';
            if (key == SDLK_QUOTE)
                temp = '\"';
            if (key == SDLK_SLASH)
                temp = '?';
            if (key == SDLK_COMMA)
                temp = '<';
            if (key == SDLK_PERIOD)
                temp = '>';
            text[namePos++] = temp;
        }
        else
        text[namePos++] = (shift ? toupper(key) : key);
    }
    if (key == SDLK_LSHIFT || key == SDLK_RSHIFT)
        shift = !shift;

    if (key == SDLK_BACKSPACE && namePos > 0)
        text[--namePos] = ' ';
}
