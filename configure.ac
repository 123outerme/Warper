AC_INIT([Warper], [0.1], [outerme@outlook.com])
AM_INIT_AUTOMAKE
AC_PROG_CC

# Checks for libraries.
AC_SEARCH_LIBS([SDL_Init], [SDL2], [], [
  AC_MSG_ERROR([unable to find SDL2])
])
AC_SEARCH_LIBS([IMG_Init], [SDL2_image], [], [
  AC_MSG_ERROR([unable to find SDL2_image])
])
AC_SEARCH_LIBS([TTF_Init], [SDL2_ttf], [], [
  AC_MSG_ERROR([unable to find SDL2_ttf])
])
AC_SEARCH_LIBS([Mix_OpenAudio], [SDL2_mixer], [], [
  AC_MSG_ERROR([unable to find SDL2_mixer])
])
#AC_SEARCH_LIBS([initCoSprite], [CoSprite], [], [
#  AC_MSG_ERROR([unable to find CoSprite])
#])

AC_CONFIG_FILES([Makefile])

# -lCoSprite
#assetdir = $(datadir)/assets
#asset_DATA = cb.bmp cb1.bmp Px437_ITT_BIOS_X.ttf tilesheet.png

AC_OUTPUT
