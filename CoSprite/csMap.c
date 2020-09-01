#include "csMap.h"

/** \brief Initialize a csMap struct
 * \param map - map struct to initialize
 * \param numEntries - the number of entries in the map
 * \param keys - the list of strings containing the keys (matched up to values)
 * \param values - the list of strings containing the corresponding values
 * \param subMap - a sub-map
 */
void initCSMap(csMap* map, int numEntries, char* keys[], char* values[], csMap* subMaps[], int* entryType[])
{
    map->entries = numEntries;
    map->keys = calloc(numEntries, sizeof(char*));
    map->values = calloc(numEntries, sizeof(char*));
    map->subMaps = calloc(numEntries, sizeof(csMap*));
    map->entryTypes = calloc(numEntries, sizeof(int));
    for(int i = 0; i < numEntries; i++)
    {
        int keysSize = strlen(keys[i]) + 1;
        map->keys[i] = calloc(keysSize, sizeof(char));
        strncpy(map->keys[i], keys[i], keysSize);

        int valuesSize = strlen(values[i]) + 1;
        map->values[i] = calloc(valuesSize, sizeof(char));
        strncpy(map->values[i], values[i], valuesSize);

        if (entryType != NULL)
            map->entryTypes[i] = (*entryType)[i];
        else
            map->entryTypes[i] = 0;

        if (subMaps != NULL)
            map->subMaps[i] = subMaps[i];
        else
            map->subMaps[i] = NULL;
    }
}

/** \brief Gets how many elements are in a JSON string
 * \param json - the JSON string
 * \return int - how many elements (not including sub-elements) are in the JSON string
*/
int getJsonElements(char* json)
{
    int length = strlen(json);

    int objLevel = 0;
    int elements = 0;
    bool inDouble = false;
    bool inSingle = false;
    bool isArray = false;

    if (json[0] == '[')
    {
        isArray = true;
        elements++; //elements = 1 + amount of commas
    }

    for(int i = 0; i < length; i++)
    {  //iterate, looking for colons only inside the first set of {}s, and count how many items we have
        if ((json[i] == '{' || json[i] == '[') && !(inDouble || inSingle))  //if there is a { or [ not inside quotes
            objLevel++;
        if ((json[i] == '}' || json[i] == ']') && !(inDouble || inSingle))  //if there is a } or ] not inside quotes
            objLevel--;
        if (json[i] == '\"' && !inSingle)  //if we are starting/ending a double-quoted block not inside single quotes
            inDouble = !inDouble;
        if (json[i] == '\'' && !inDouble)  //if we are starting/ending a single-quoted block not inside double quotes
            inSingle = !inSingle;

        if (objLevel == 1 && !(inDouble || inSingle) && (json[i] == ':' || (isArray && json[i] == ',')))  //if there is a colon (or comma if it is an array) and it isn't in quotes and this is the "root" JSON object level
            elements++;
    }
    return elements;
}

/** \brief Takes in syntactically correct JSON in string format, outputs data to a csMap
 * \param map - map struct to initialize and fill in
 * \param json - the JSON data in string format. If there are any syntax errors, this function has undefined output
*/
void jsonToCSMap(csMap* map, char* json)
{  //handle chunks without using strtok, bc of single-quotes in double quote blocks or vice versa
    const bool debug = false;
    int length = strlen(json);
    int elements = getJsonElements(json);
    if (debug)
        printf("num elements = %d\n", elements);

    //allocate memory for each element
    map->entries = elements;
    map->keys = calloc(elements, sizeof(char*));
    map->values = calloc(elements, sizeof(char*));
    map->subMaps = calloc(elements, sizeof(csMap*));
    map->entryTypes = calloc(elements, sizeof(int));

    //char* jsonCopy = calloc(length + 1, sizeof(char));
    //strncpy(jsonCopy, json, length + 1);

    int objLevel = 0, startOfValue = 0, endOfValue = 0, pos = 0;
    int subObjCount = 0;
    bool isKey = true, writeReady = false, outsideQuoteValue = false;
    bool inDouble = false, inSingle = false;
    bool isArray = false;
    if (json[0] == '[')
    {
        isKey = false;
        isArray = true;
        if (debug)
            printf("found array\n");
    }

    for(int i = 0; i < length; i++)
    {
        //printf("%c\n", json[i]);
        if (json[i] == '\"' && !inSingle)  //if we are starting/ending a double-quoted block not inside single quotes
        {
            if (!(inDouble && json[i - 1] == '\\'))
            {  //ignore an escaped quote
                    inDouble = !inDouble;
                if (inDouble)  //if we now decide we are in quotes
                    startOfValue = i + 1;
                else
                {
                    endOfValue = i;
                    writeReady = true;
                }
            }
        }
        if (json[i] == '\'' && !inDouble)  //if we are starting/ending a single-quoted block not inside double quotes
        {
            if (!(inSingle && json[i - 1] == '\\'))
            {  //ignore an escaped quote
                    inSingle = !inSingle;
                if (inSingle)  //if we now decide we are in quotes
                    startOfValue = i + 1;
                else
                {
                    endOfValue = i;
                    writeReady = true;
                }
            }
        }
        if (!(inDouble || inSingle))
        {  //we are out of quotes
            if (json[i] == '{' || json[i] == '[')  //if there is a { or [ not inside quotes
            {   //we have found a sub-map
                //TODO: This can be optimized to only go from current position to end of string
                objLevel++;
                if (objLevel == 2)
                {  //only handle the next level; any subsequent levels will be handles by the recursive call
                    bool subObjIsArray = false;
                    if (json[i] == '[')
                        subObjIsArray = true;

                    subObjCount++;
                    bool subInDouble = false;
                    bool subInSingle = false;
                    //printf("%c%c\n", openSubMap, closeSubMap);
                    int subObj = 0;
                    int startPos = 0, endPos = length, openBrackets = 0;
                    for(int x = 0; x < length; x++)
                    {  //find where this sub-map is and get the substring without messing up strtok_r()
                        if (json[x] == '\"' && !subInSingle)  //if we are starting/ending a double-quoted block not inside single quotes
                            subInDouble = !subInDouble;
                        if (json[x] == '\'' && !subInDouble)  //if we are starting/ending a single-quoted block not inside double quotes
                            subInSingle = !subInSingle;

                        if ((json[x] == '{' || json[x] == '[') && !(subInDouble || subInSingle))
                        {
                            if (++openBrackets == objLevel)
                                startPos = x;
                        }
                        if ((json[x] == '}' || json[x] == ']') && !(subInDouble || subInSingle))
                        {
                            if (openBrackets-- == objLevel)
                            {
                                subObj++;
                                if (subObj == subObjCount)
                                {
                                    endPos = x + 1;
                                    break;
                                }
                            }
                        }
                    }
                    char* subJson = calloc(endPos - startPos + 1, sizeof(char));
                    strncpy(subJson, json + startPos, endPos - startPos);
                    if (debug)
                        printf("-> %s goes into subMaps[%d]\n", subJson, pos);
                    csMap* tempMap = malloc(sizeof(csMap));
                    jsonToCSMap(tempMap, subJson);  //init new map and fill in values
                    map->entryTypes[pos] = (subObjIsArray) ? 2 : 1;
                    map->subMaps[pos] = tempMap;
                    if (debug)
                        printf("<- submap type %d into pos %d: Comes out into objLevel %d\n", map->entryTypes[pos], pos, objLevel);
                    pos++;  //signifies we have completed the entry
                    free(subJson);
                }
            }
            else
            {
                if (json[i] == ':')
                {  //we are about to read a value/sub-map
                    isKey = false;  //the next element found should be a value/sub-map
                }
                else
                {
                    if (json[i] == ',' && !outsideQuoteValue)
                    {  //we are about to read a key
                        isKey = true;  //the next element found should be a key
                    }
                    else
                    {
                        if (endOfValue != 0 && objLevel == 1)
                        {  //if we are in the root level, assign keys/values. Subsequent levels are handled by the recursive call
                            if (writeReady && pos < elements)
                            {
                                if (isKey)
                                {
                                    int keyLength = endOfValue - startOfValue;
                                    map->keys[pos] = calloc(keyLength + 1, sizeof(char));
                                    strncpy(map->keys[pos], json + startOfValue, keyLength);

                                    startOfValue = 0;
                                    endOfValue = 0; //signifies we have completed the substring

                                    if (debug)
                                        printf("%s goes into key %d\n", map->keys[pos], pos);
                                }
                                else
                                {
                                    if (isArray)
                                    {
                                        int numLen = 1 + (int) log10(pos);
                                        map->keys[pos] = calloc(numLen + 1, sizeof(char));
                                        snprintf(map->keys[pos], numLen + 1, "%d", pos);
                                        if (debug)
                                            printf("key = %s\n", map->keys[pos]);
                                    }
                                    int valueLength = endOfValue - startOfValue + 2;
                                    map->values[pos] = calloc(valueLength + 1, sizeof(char));
                                    strncpy(map->values[pos], json + startOfValue - 1, valueLength);

                                    if (debug)
                                        printf("%s goes into values %d\n", map->values[pos], pos);
                                    pos++;  //signifies we have completed the entry

                                    startOfValue = 0;
                                    endOfValue = 0; //signifies we have completed the substring
                                }
                                writeReady = false;
                            }
                        }
                        else
                        {
                            //we are not in quotes, not ready to write, not in a sub-map, not currently deciding if we are in a key or value
                            if (json[i] != '\"' && json[i] != '\'' && objLevel == 1)
                            {  //if we are not reading in a literal quote
                                //then we must be reading in a value not in quotes (number, boolean, or null)
                                if (json[i] != ' ' && json[i] != '\t' && json[i] != '\n')
                                {  //if no whitespace
                                        if (debug)
                                            printf(">>>%c\n", json[i]);
                                        if (startOfValue == 0)
                                        {
                                            startOfValue = i;
                                            outsideQuoteValue = true;
                                            if (debug)
                                                printf("start %d. isArray = %d\n", startOfValue, isArray);
                                        }

                                }
                                if ((json[i] == ',' || json[i] == '}' || json[i] == ']') && startOfValue != 0 && pos < elements)
                                {
                                    if (isArray)
                                    {
                                        int numLen = 1;
                                        if (pos != 0)
                                            numLen += (int) log10(pos);

                                        if (debug)
                                            printf("pos = %d, numLen = %d\n", pos, numLen);

                                        map->keys[pos] = calloc(numLen + 1, sizeof(char));
                                        snprintf(map->keys[pos], numLen + 1, "%d", pos);
                                        if (debug)
                                            printf("key = %s\n", map->keys[pos]);
                                    }
                                    int valueLength = i - startOfValue;
                                    map->values[pos] = calloc(valueLength + 1, sizeof(char));
                                    strncpy(map->values[pos], json + startOfValue, valueLength);

                                    if (debug)
                                        printf(">>>%s goes into values %d\n", map->values[pos], pos);
                                    pos++;  //signifies we have completed the entry

                                    startOfValue = 0;
                                    outsideQuoteValue = false;  //signifies we have completed the substring
                                    if (!isArray)  //arrays have pre-defined keys
                                        isKey = true;  //set the isKey flag because we skip setting it earlier
                                }
                                else
                                {
                                    if (debug)
                                        printf(">>>char %c, start %d, pos %d, elements %d\n", json[i], startOfValue, pos, elements);
                                }
                            }
                        }
                    }
                }
                if (json[i] == '}' || json[i] == ']')  //if there is a } or ] not inside quotes, we are stepping down a level
                    objLevel--;
            }
        }
    }


    /*  //does not work- the nature of strtok makes recursive calls difficult
    char* saveJsonCopyPtr;
    char delimiters[3] = "\"\'";  //:,
    char* jsonPieces = strtok_r(jsonCopy, delimiters, &saveJsonCopyPtr);
    //get first chunk

    int pos = 0;
    bool isKey = true;
    while(jsonPieces != NULL && pos <= elements - 1)
    {  //go through each item and set each key and value/sub-map
        printf("-- %s\n", jsonPieces);
        if (strchr(jsonPieces, '{') != NULL)
        {
            if (strchr(jsonPieces, '}') != NULL)
            {
                map->isSubMap[pos] = true;
                map->subMaps[pos] = NULL;  //empty object
                pos++;
            }
            else
            {
                objLevel++;
                if (objLevel > 1)
                {
                    bool subInDouble = false;
                    bool subInSingle = false;
                    int startPos = 0, endPos = length, openBrackets = 0;
                    for(int i = 0; i < length; i++)
                    {  //find where this sub-map is and get the substring without messing up strtok_r()
                        if (json[i] == '\"' && !subInSingle)  //if we are starting/ending a double-quoted block not inside single quotes
                            subInDouble = !subInDouble;
                        if (json[i] == '\'' && !subInDouble)  //if we are starting/ending a single-quoted block not inside double quotes
                            subInSingle = !subInSingle;

                        if (json[i] == '{' && !(subInDouble || subInSingle))
                        {
                            if (++openBrackets == objLevel)
                                startPos = i;
                        }
                        if (json[i] == '}' && !(subInDouble || subInSingle))
                        {
                            if (openBrackets-- == objLevel)
                                endPos = i + 1;
                        }
                    }
                    char* subJson = calloc(endPos - startPos + 1, sizeof(char));
                    strncpy(subJson, json + startPos, endPos - startPos);
                    printf("-> %s\n", subJson);
                    csMap* tempMap = malloc(sizeof(csMap));
                    jsonToCSMap(tempMap, subJson);  //init new map and fill in values
                    map->isSubMap[pos] = true;
                    map->subMaps[pos] = tempMap;
                    pos++;
                    free(subJson);
                    printf("out, new pos %d\n", pos);
                }
            }
        }
        else
        {
            if (strchr(jsonPieces, '}') != NULL)
            {
                objLevel--;
                if (objLevel <= 0)
                    break;
            }
            else
            {
                if (strchr(jsonPieces, ':') != NULL)
                    isKey = false;  //the next element found should be a value/sub-map
                else
                {
                    if (strchr(jsonPieces, ',') != NULL)
                        isKey = true;  //the next element found should be a key
                    else
                    {
                        if (isKey)
                        {
                            int keyLength = strlen(jsonPieces) + 1;
                            map->keys[pos] = calloc(keyLength, sizeof(char));
                            strncpy(map->keys[pos], jsonPieces, keyLength);

                            printf("%s goes into key %d\n", map->keys[pos], pos);
                        }
                        else
                        {
                            int valueLength = strlen(jsonPieces) + 1;
                            map->values[pos] = calloc(valueLength, sizeof(char));
                            strncpy(map->values[pos], jsonPieces, valueLength);

                            printf("%s goes into values %d\n", map->values[pos], pos);
                            pos++;
                        }
                    }
                }
            }
        }

        jsonPieces = strtok_r(NULL, delimiters, &saveJsonCopyPtr);
        //get next chunk
    }
    //*/
    //printf("FULLY COMPLETE one loop\n");
}

/** \brief Outputs JSON in string format from a csMap struct
 * \param map - the map you want to convert
 * \return char* - the JSON data in string format
*/
char* csMapToJson(csMap map)
{
    if (map.entries == 0 || map.keys == NULL)
        return "{}";
    size_t size = 8192;  //TODO: Maybe calculate size of JSON string if it's not too hard
    char* jsonString = calloc(size, sizeof(char));
    strncpy(jsonString, "{", size);
    for(int i = 0; i < map.entries; i++)
    {
        strncat(jsonString, "\'", size);
        strncat(jsonString, map.keys[i], size);
        strncat(jsonString, "\':", size);
        if (map.entryTypes[i] != 0)
        {
            if (map.subMaps[i] != NULL)
            {
                char* tempData;

                if (map.entryTypes[i] == 1)  //if entry type is a JSON obj
                    tempData = csMapToJson(*(map.subMaps[i]));
                else  //entry type is an array
                    tempData = csMapToArray(*(map.subMaps[i]));

                strncat(jsonString, tempData, size);
                free(tempData);
            }
            else
                strncat(jsonString, (map.entryTypes[i] == 1) ? "{}" : "[]", size);
        }
        else
        {
            int len = strlen(map.values[i]);
            if (map.values[i][0] == '\'' || map.values[i][0] == '\"')
            {
                strncat(jsonString, "\'", size);

                char* temp = calloc(len, sizeof(char));
                strncpy(temp, map.values[i] + 1, len - 2);
                strncat(jsonString, temp, size);
                free(temp);

                strncat(jsonString, "\'", size);
            }
            else
                strncat(jsonString, map.values[i], size);
        }

        if (i < map.entries - 1)
            strncat(jsonString, ",", size);
    }
    strncat(jsonString, "}", size);
    return jsonString;
}

/** \brief Converts a csMap into a JSON-style array (in string form)
 *
 * \param map csMap the csMap you wish to convert
 * \return char* the string holding the JSON-style array data
 */
char* csMapToArray(csMap map)
{
    if (map.entries == 0 || map.keys == NULL)
        return "[]";
    size_t size = 8192;  //TODO: Maybe calculate size of array string if it's not too hard
    char* arrString = calloc(size, sizeof(char));
    strncpy(arrString, "[", size);
    for(int i = 0; i < map.entries; i++)
    {
        if (map.entryTypes[i] != 0)
        {
            //print a sub-obj or array
            if (map.subMaps[i] != NULL)
            {
                char* tempData;

                if (map.entryTypes[i] == 1)  //if entry type is a JSON obj
                    tempData = csMapToJson(*(map.subMaps[i]));
                else  //entry type is an array
                    tempData = csMapToArray(*(map.subMaps[i]));

                strncat(arrString, tempData, size);
                free(tempData);
            }
            else
                strncat(arrString, (map.entryTypes[i] == 1) ? "{}" : "[]", size);
        }
        else
            strncat(arrString, map.values[i], size);

        if (i < map.entries - 1)
            strncat(arrString, ",", size);
    }
    strncat(arrString, "]", size);
    return arrString;
}

/** \brief Creates a csMap using a single string in the format ("key"->str)
 * \param map - the map you want to initialize and fill in
 * \param str - the string data you want inserted
*/
void stringToCSMap(csMap* map, char* str)
{
    map->entries = 1;
    map->subMaps = NULL;
    int* entryIsValue = malloc(sizeof(int));
    *entryIsValue = 0;
    map->entryTypes = entryIsValue;
    map->keys = calloc(1, sizeof(char*));
    map->values = calloc(1, sizeof(char*));
    map->keys[0] = "key";
    map->values[0] = str;
}

/** \brief Find a map's entry given that value's key
 * \param map - the map you wish to search
 * \param key - the key you wish to find the value for
 * \return char* - the found entry or a JSON string containing the found sub-map
*/
char* traverseCSMapByKey(csMap map, char* key)
{
    for(int i = 0; i < map.entries; i++)
    {
        if (strcmp(map.keys[i], key) == 0)
        {
            if (map.entryTypes[i] == 1)
                return csMapToJson(*(map.subMaps[i]));

            if (map.entryTypes[i] == 2)
                return csMapToArray(*(map.subMaps[i]));

            return map.values[i];
        }

    }
    return NULL;
}

/** \brief Find a map's entry given that value's key, and return the value as a csMap
 * \param map - the map you wish to search
 * \param key - the key you wish to find the value for
 * \return csMap - a csMap containing what was found, or a key/value pair of ("key"->"<found data>").
*/
csMap* traverseCSMapByKeyGetMap(csMap map, char* key)
{
    for(int i = 0; i < map.entries; i++)
    {
        if (strcmp(map.keys[i], key) == 0)
        {
            if (map.entryTypes[i] == 0)
                stringToCSMap(map.subMaps[i], map.values[i]);

            return map.subMaps[i];
        }

    }
    return NULL;
}

/** \brief Internal use only; resizes the memory blocks used by a csMap
 *
 * \param map - the csMap* pointing to the map to be modified
 * \param entries - the new number of entries to have (entries < current: will lose data!)
 * \return bool - true if succeeded, false otherwise
 */
bool resizeCSMap(csMap* map, int entries)
{
    void* newKeys = realloc(map->keys, entries * sizeof(char*));
    if (newKeys == NULL)
        return false;

    void* newValues = realloc(map->values, entries * sizeof(char*));
    if (newValues == NULL)
        return false;

    void* newSubMaps = realloc(map->subMaps, entries * sizeof(csMap*));
    if (newSubMaps == NULL)
        return false;

    map->keys = (char**) newKeys;
    map->values = (char**) newValues;
    map->subMaps = (csMap**) newSubMaps;

    map->entries = entries;
    return true;
}

/** \brief Adds a key/value pair to a csMap.
 *
 * \param map - the csMap you wish to modify
 * \param key - the key/name of the data
 * \param value - the value of the data
 * \return bool - true if succeeded, false if not
 */
bool addDataEntryToCSMap(csMap* map, char* key, char* value)
{
    bool success = resizeCSMap(map, map->entries + 1);
    if (!success)
        return false;

    int newKeyLen = strlen(key);
    map->keys[map->entries - 1] = calloc(newKeyLen + 1, sizeof(char));
    if (map->keys[map->entries - 1] == NULL)
        return false;

    int newValueLen = strlen(value);
    map->values[map->entries - 1] = calloc(newValueLen + 1, sizeof(char));
    if (map->values[map->entries - 1] == NULL)
        return false;

    strncpy(map->keys[map->entries - 1], key, newKeyLen);
    strncpy(map->values[map->entries - 1], value, newValueLen);

    return true;
}

/** \brief Adds an array entry to a csMap.
 *
 * \param map - the csMap you wish to modify
 * \param name - the name the array should be given
 * \param arr - the array data in csMap format
 * \return bool - true if succeeded, false if not
 */
bool addArrayEntryToCSMap(csMap* map, char* name, csMap arr)
{
    bool success = resizeCSMap(map, map->entries + 1);
    if (!success)
        return false;

    int newKeyLen = strlen(name);
    map->keys[map->entries - 1] = calloc(newKeyLen + 1, sizeof(char));
    if (map->keys[map->entries - 1] == NULL)
        return false;

    map->subMaps[map->entries - 1] = malloc(sizeof(csMap));
    if (map->subMaps[map->entries - 1] == NULL)
        return false;

    strncpy(map->keys[map->entries - 1], name, newKeyLen + 1);
    memcpy(map->subMaps[map->entries - 1], &arr, sizeof(csMap));
    map->entryTypes[map->entries - 1] = 2; //array type
    return true;
}

/** \brief Adds an object/csMap entry to a csMap.
 *
 * \param map - the csMap you wish to add to
 * \param name - the name the object should be given
 * \param obj - the csMap object
 * \return bool - true if succeeded, false if not
 */
bool addObjEntryToCSMap(csMap* map, char* name, csMap obj)
{
    bool success = resizeCSMap(map, map->entries + 1);
    if (!success)
        return false;

    int newKeyLen = strlen(name);
    map->keys[map->entries - 1] = calloc(newKeyLen + 1, sizeof(char));
    if (map->keys[map->entries - 1] == NULL)
        return false;

    map->subMaps[map->entries - 1] = malloc(sizeof(csMap));
    if (map->subMaps[map->entries - 1] == NULL)
        return false;

    strncpy(map->keys[map->entries - 1], name, newKeyLen + 1);
    memcpy(map->subMaps[map->entries - 1], &obj, sizeof(csMap));
    map->entryTypes[map->entries - 1] = 1; //object
    return true;
}

/** \brief
 *
 * \param map - the csMap you wish to remove from
 * \param key - the name or key of the entry to be removed
 * \return bool - true if succeeded, false if not
 */
bool removeEntryFromCSMap(csMap* map, char* key)
{
    //find the entry index, then once found move it to the end
    int entryIndex = map->entries; //impossible index; acts as success flag
    for(int i = 0; i < map->entries; i++)
    {
        if (strcmp(map->keys[i], key) == 0)
            entryIndex = i;

        if (i > entryIndex)
        {
            //swap the elements at i and i - 1

            { //swap the keys
                char* temp = map->keys[i];
                map->keys[i] = map->keys[i - 1];
                map->keys[i - 1] = temp;
            }
            { //swap the values
                char* temp = map->values[i];
                map->values[i] = map->values[i - 1];
                map->values[i - 1] = temp;
            }
            { //swap the sub-maps
                csMap* temp = map->subMaps[i];
                map->subMaps[i] = map->subMaps[i - 1];
                map->subMaps[i - 1] = temp;
            }
            { //swap the sub-map flags
                int temp = map->entryTypes[i];
                map->entryTypes[i] = map->entryTypes[i - 1];
                map->entryTypes[i - 1] = temp;
            }
        }
    }

    if (entryIndex == map->entries)
        return false;


    //free the data (now at the end), then reallocate to a size one smaller
    free(map->keys[map->entries - 1]);
    map->keys[map->entries - 1] = NULL;

    free(map->values[map->entries - 1]);
    map->values[map->entries - 1] = NULL;

    destroyCSMap(map->subMaps[map->entries - 1]);
    map->entryTypes[map->entries - 1] = 0;

    return resizeCSMap(map, map->entries - 1);
}

/** \brief Destroys a csMap and frees all its memory (including sub-maps)
 * \param map - the map you wish to destroy
*/
void destroyCSMap(csMap* map)
{
    if (map != NULL)
    {
        for(int i = 0; i < map->entries; i++)
        {
            free(map->keys[i]);
            map->keys[i] = NULL;

            free(map->values[i]);
            map->values[i] = NULL;

            destroyCSMap(map->subMaps[i]);
            map->entryTypes[i] = 0;
        }
        free(map->keys);
        map->keys = NULL;

        free(map->values);
        map->values = NULL;

        free(map->subMaps);
        map->subMaps = NULL;

        free(map->entryTypes);
        map->entryTypes = NULL;

        map->entries = 0;
    }
}
