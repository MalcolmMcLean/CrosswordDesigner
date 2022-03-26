//
//  JSONParser.h
//  StanProto4
//
//  Created by Malcolm McLean on 29/05/2015.
//
//

#ifndef jsonparser_h
#define jsonparser_h

#include "jsmn.h"
#include <string.h>
#include <assert.h>

typedef struct
{
   char *mJson;
   int mNtokens;
   jsmntok_t *mTokens;
   int mFirstKey;
   int mLastToken;
   int mIsRoot;
   int mIsArray;
} JSONParser;

typedef JSONParser JSONArray;

    JSONParser *JSONParser_clone(const JSONParser *jp);
    JSONParser *JSONParser_create(const char *json);
    void killJSONParser(JSONParser *jp);
    void killJSONArray(JSONArray *ja);

    JSONParser *JSONParser_getObject(JSONParser *jp, const char *id, int *err);
    JSONParser *JSONParser_getObjectA(JSONParser *jp, const char *id, int index, int *err);
    JSONArray *JSONParser_getArray(JSONParser *jp, const char *id, int *err);
    JSONParser *JSONArray_getObject(JSONArray *ja, int index, int *err);
    JSONArray *JSONArray_getArray(JSONArray *ja, int index, int *err);  
    int JSONParser_getArrayLength(JSONParser *jp, const char *id, int *err);
    int JSONArray_getLength(JSONArray *ja);
    int JSONParser_getBoolean(JSONParser *jp, const char *id, int *err);
    double JSONParser_getReal(JSONParser *jp, const char *id, int *err);
    int JSONParser_getInteger(JSONParser *jp, const char *id, int *err);
    char * JSONParser_getString(JSONParser *jp, const char *id, int *err);
    char * JSONParser_getStringA(JSONParser *jp, const char *id, int index, int *err);
    int JSONParser_getBooleanA(JSONParser *jp, const char *id, int index, int *err);
    double JSONParser_getRealA(JSONParser *jp, const char *id, int index, int *err);
    int JSONParser_getIntegerA(JSONParser *jp, const char *id, int index, int *err);
    int JSONArray_getBoolean(JSONArray *ja, int index, int *err);
    double JSONArray_getReal(JSONArray *ja, int index, int *err);
    int JSONArray_getInteger(JSONArray *ja, int index, int *err);
    char *JSONArray_getString(JSONArray *ja, int index, int *err);
   /*static 
    int getKey(JSONParser *jp, const char *id);
    int getSkip(JSONParser *jp, int index);
    int getArrayIndex(JSONParser *jp, int array, int index);
    */

char *escapeJSON(const char *input);
        
#endif 
