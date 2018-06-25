#include "crankMain.h"
#include "crankGraphics.h"
#include "crankFile.h"
#include "crankUtility.h"

int main(int argc, char* argv[])
{
    int error = initCrank("", "Warper", 960, 480, "", 48);
    waitForKey(true);
    return error;
}
