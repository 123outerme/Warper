#ifndef CSMAP_H_INCLUDED
#define CSMAP_H_INCLUDED

#include <stdio.h>  //for console printing (debugging)
#include <stdlib.h>  //for calloc/free
#include <string.h>  //for str___() functions
#include <math.h>  //for log10()

//#defines:
#ifndef bool
    #define bool char
    #define false 0
    #define true 1
    #define boolToString(bool) (bool ? "true" : "false")
#endif // bool

#ifndef NULL
    #define NULL ((void*) 0)
#endif //NULL

typedef struct _csMap
{
    char** keys;
    char** values;
    int entries;
    struct _csMap** subMaps;
    int* entryTypes;  /**< 0 - value, 1 - subMap, 2 - array */
} csMap;

void initCSMap(csMap* map, int numEntries, char* keys[], char* values[], csMap* subMaps[], int* entryType[]);
void jsonToCSMap(csMap* map, char* json);
char* csMapToJson(csMap map);
char* csMapToArray(csMap map);
void stringToCSMap(csMap* map, char* str);
char* traverseCSMapByKey(csMap map, char* key);
csMap* traverseCSMapByKeyGetMap(csMap map, char* key);
bool addDataEntryToCSMap(csMap* map, char* key, char* value);
bool addArrayEntryToCSMap(csMap* map, char* name, csMap arr);
bool addObjEntryToCSMap(csMap* map, char* name, csMap obj);
bool removeEntryFromCSMap(csMap* map, char* key);
void destroyCSMap(csMap* map);

#endif // CSMAP_H_INCLUDED
