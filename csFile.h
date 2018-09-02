#ifndef CSFILE_H_INCLUDED
#define CSFILE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//defines:
#ifndef bool
    #define bool char
    #define false 0
    #define true 1
    #define boolToString(bool) (bool ? "true" : "false")
#endif // bool
#ifndef NULL
    #define NULL ((void*) 0)
#endif //NULL

//function prototypes:
int createFile(char* filePath);
int checkFile(char* filePath, int desiredLines);
int appendLine(char* filePath, char* stuff, bool addNewline);
int replaceLine(char* filePath, int lineNum, char* stuff, int maxLength, bool addNewline);
char* readLine(char* filePath, int lineNum, int maxLength, char** output);
#endif // CRANKFILE_H_INCLUDED
