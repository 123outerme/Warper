#include "crankFile.h"

/** \brief Creates a file, or clears contents if it exists.
 *
 * \param filePath - valid string filepath (relative or absolute)
 * \return Error code: Code 0: No error. Code -1: Error opening
 */
int createFile(char* filePath)
{
	FILE* filePtr;
	filePtr = fopen(filePath,"w");
	if (!filePtr)
	{
		printf("Error opening/creating file!\n");
		return -1;
	}
	else
    {
        fclose(filePtr);
		return 0;
    }
}

/** \brief Checks if a file exists and if it has certain number of lines.
 *
 * \param filePath - valid string filepath (relative or absolute)
 * \param desiredLines - Compares this number to actual lines. If desiredLines < 0, gets number of lines instead.
 * \return 1 if desiredLines >= 0 and desiredLines >= lines. 0 otherwise. If desiredLines < 0, returns number of lines instead.
 */
int checkFile(char* filePath, int desiredLines)
{
    FILE* filePtr = fopen(filePath, "r");
	if (!filePtr)
		return false;
    char ch;
    int lines = 0;
    while(!feof(filePtr))
    {
      ch = fgetc(filePtr);
      if(ch == '\n')
      {
        lines++;
      }
    }
    fclose(filePtr);
    return desiredLines >= 0 ? lines >= desiredLines : lines;
}

/** \brief Adds a line of text to the end of a file
 *
 * \param filePath - valid string filepath (relative or absolute)
 * \param stuff - string containing desired text.
 * \return Error code: Code 0: No error. Code -1: Error opening file
 */
int appendLine(char* filePath, char* stuff, bool addNewline)
{
	FILE* filePtr;
	filePtr = fopen(filePath,"a");
	if (!filePtr)
	{
		printf("Error opening file!\n");
		return -1;
	}
	else
	{
		fprintf(filePtr, (addNewline ? "%s\n" : "%s"), stuff);
		fclose(filePtr);
		return 0;
	}
}

/** \brief inserts a line at a certain position, if the file isn't too big
 *
 * \param
 * \param
 * \param
 * \param
 * \return -1 if failed to open or supply a valid line num, 0 if succeeded
 */
int replaceLine(char* filePath, int lineNum, char* stuff, int maxLength, bool addNewline)
{
    int maxLines = checkFile(filePath, -1) + 1;
    //printf("%d\n", maxLines);
    if (lineNum < 0 || lineNum > maxLines)
        return -1;
    char** allLines = calloc(maxLines, sizeof(char*));
    if (!allLines)
        return -1;
    for(int i = 0; i < maxLines; i++)
    {
        allLines[i] = calloc(maxLength, sizeof(char));
        if (!readLine(filePath, i, maxLength, &(allLines[i])))
            return -1;
        //printf("%s\n", allLines[i]);
    }

    strncpy(allLines[lineNum], stuff, maxLength);
    if (addNewline);
        strncat(allLines[lineNum], "\n", maxLength);
    //printf("%s at %d\n", allLines[lineNum], lineNum);

    createFile(filePath);
    for(int i = 0; i < maxLines; i++)
    {
        if (appendLine(filePath, allLines[i], false) == -1)
            return -1;
        //printf("%s\n", allLines[i]);
    }

    return 0;
}

/** \brief Reads a line of a file.
 *
 * \param filePath - valid string filepath (relative or absolute)
 * \param lineNum - the line number (starting from 0)
 * \param maxLength - how long the string should be, max.
 * \param output - valid pointer to your char* (should not be read-only)
 * \return NULL if it fails, otherwise your string
 */
char* readLine(char* filePath, int lineNum, int maxLength, char** output)
{
	FILE* filePtr = fopen(filePath,"r");
	if (!filePtr || !*output)
		return NULL;
	else
	{
        char* thisLine = calloc(maxLength, sizeof(char));
        fseek(filePtr, 0, SEEK_SET);
        for(int p = 0; p <= lineNum; p++)
            fgets(thisLine, maxLength, filePtr);
        //printf("%s @ %d\n", thisLine, thisLine);
        strncpy(*output, thisLine, maxLength);
        //printf("%s @ %d\n", output, output);
        fclose(filePtr);
        free(thisLine);
        return *output;
	}
}
