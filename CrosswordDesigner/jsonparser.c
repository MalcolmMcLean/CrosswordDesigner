/*
//  jsonparser.c
//
//  Created by Malcolm McLean on 29/05/2015.
//
*/
#include <stdlib.h>
#include <string.h>
#include "jsonparser.h"

static int getKey(JSONParser *jp, const char *id);
static int getSkip(JSONParser *jp, int index);
static int getArrayIndex(JSONParser *jp, int array, int index);

static int jsoneq(const char *json, jsmntok_t *tok, const char *s);
static int jsonboolval(const char *json, jsmntok_t *tok, int *err);
static double jsonrealval(const char *json, jsmntok_t *tok, int *err);
static int jsonintval(const char *json, jsmntok_t *tok, int *err);
static char *jsonstringval(const char *json, jsmntok_t *tok, int *err);
char *escapeJSON(const char *input);
static char *unescapeJSON(const char *input);
static char *mystrdup(const char *str);

/*
   It's a simple lightweight parser, built on top of an MIT-Licenced public domain
  parser by Serge (zserge on the web) called jsmn.
 
    It won't stand up to heavy use as access is O(N) in the number of elements,
    including arrays. But it should be fine for getting user parameters out
   of GUI dialogs.
 
Usage:
     json =  {
                "aflag" : true,
                "areal" : 3.14,
                "aninteger" : 42,
                "anarray" : [ 1 ,2, 3, 4 ]
                "subobject" : {
                                "value" : 11
                             }
             }
 
 
     int err = 0;  // err is sticky
     JSONParser jparser( json );
     bool flag = jparser.getBoolean("aflag", &err);
     int ix = jparser.getInteger("aninteger", &err);
     int N = jparser.getArrayLength("anarray", &err);
     if(N == -1)
        // not an array 
     for(i=0;i<N;i++)
       array[i] = jparser.getInteger("anarray", i, &err);
     JSONParser subparser = jparser.getObject("subobject", &err));
     int subint = subparser.getInteger("value");
 
     if(err)
        // something went wrong, we must have mistype a field name or something
 
 Notes: you need to know the format you are expect the data to arrive in, the parser has no
   way of querying for fields existence, other than reporting error for missing fields.
 
 We're passing all JSON about as UTF8
 */

/*
   default parser, can't parse anything
 */
JSONParser *JSONParser_null(void)
{
    JSONParser *jp;
    
    jp = malloc(sizeof(JSONParser));
    if (!jp)
       return 0;
    jp->mTokens = 0;
    jp->mNtokens = 0;
    jp->mIsRoot = 0;
    jp->mJson = 0;
    jp->mFirstKey = 0;
    jp->mLastToken = 0;
    jp->mIsArray = 0;

    return jp;
}

/*
   Copy constructor
   Note we only take a deep copy if the paser is a root parser
 */
JSONParser *JSONParser_clone(const JSONParser *jp)
{
    JSONParser *answer = JSONParser_null();
    if (!answer)
       return 0;
    answer->mFirstKey = jp->mFirstKey;
    answer->mLastToken = jp->mLastToken;
    answer->mNtokens = jp->mNtokens;
    if(jp->mIsRoot)
    {
        answer->mJson = mystrdup(jp->mJson);
        if (!answer->mJson)
           goto error_exit;
        answer->mTokens = (jsmntok_t *) malloc(answer->mNtokens * sizeof(jsmntok_t));
        if (answer->mNtokens)
            memcpy(answer->mTokens, jp->mTokens, answer->mNtokens * sizeof(jsmntok_t));
        else
            goto error_exit;
        answer->mIsRoot = 1;
    }
    else
    {
        answer->mJson = jp->mJson;
        answer->mTokens = jp->mTokens;
        answer->mIsRoot = 0;
    }

    return answer;
error_exit:
   killJSONParser(answer);
   return 0;
}

/*
   Main constructor
   We take private copy of the string and translate to C.
 */
JSONParser *JSONParser_create(const char *json)
{
    JSONParser *jp = JSONParser_null();
    jsmn_parser jparser;
    int N;

    if (!jp)
      return 0;
    jp->mJson = mystrdup(json);
    if (!jp->mJson)
      goto error_exit;

    jsmn_init(&jparser);
    N = jsmn_parse(&jparser, jp->mJson, strlen(jp->mJson), 0, 0);
    jp->mTokens = (jsmntok_t *) malloc(N * sizeof(jsmntok_t));
    if (!jp->mTokens)
       goto error_exit;
    jsmn_init(&jparser);
    jp->mNtokens = jsmn_parse(&jparser, jp->mJson, strlen(jp->mJson), jp->mTokens, N);
    if(jp->mNtokens > 0 && jp->mTokens[0].type == JSMN_OBJECT)
    {
        jp->mFirstKey = 1;
        jp->mLastToken = jp->mNtokens;
    }
    else
    {
        jp->mFirstKey = 0;
        jp->mLastToken = jp->mNtokens;
    }
    jp->mIsRoot = 1;

    return jp;
error_exit:
    killJSONParser(jp);
    return 0;
}

void killJSONParser(JSONParser *jp)
{
   if (jp)
   {
     if(jp->mIsRoot)
     {
       free(jp->mJson);
       free(jp->mTokens);
     }
     free(jp);
   }

}

void killJSONArray(JSONArray *ja)
{
   killJSONParser(ja);
}
 

/*
   These spawn daughter parsers for sub-objects.
    However really a daughter parser just has a different mFirstKey and
        mLastToken member.
 */
JSONParser *JSONParser_getObject(JSONParser *jp, const char *id, int *err)
{
    int key;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken)
    {
        if(jp->mTokens[key+1].type == JSMN_OBJECT)
        {
            JSONParser *answer = JSONParser_null();
            answer->mJson = jp->mJson;
            answer->mTokens = jp->mTokens;
            answer->mNtokens = jp->mNtokens;
            answer->mFirstKey = key+2;
            answer->mLastToken = key + getSkip(jp, key+1) +1;
            answer->mIsRoot = 0;
            return answer;
        }
    }
    *err = -1;
    return JSONParser_null();
}

JSONParser *JSONParser_getObjectA(JSONParser *jp, const char *id, int index, int *err)
{
    int key;
    int token_index;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken && jp->mTokens[key+1].type == JSMN_ARRAY)
    {
        token_index = getArrayIndex(jp, key +1, index);
        
        if(token_index >= 0)
        {
            if(jp->mTokens[token_index].type == JSMN_OBJECT)
            {
                JSONParser *answer = JSONParser_null();
                answer->mJson = jp->mJson;
                answer->mTokens = jp->mTokens;
                answer->mNtokens = jp->mNtokens;
                answer->mFirstKey = token_index+1;
                answer->mLastToken = token_index + getSkip(jp, token_index);
                answer->mIsRoot = 0;
                return answer;
            }
        }
    }
    *err = -1;
    return JSONParser_null();
}

JSONArray *JSONParser_getArray(JSONParser *jp, const char *id, int *err)
{
   int key;
   
   key = getKey(jp, id);
   if (key >= 0 && key + 1 < jp->mLastToken && jp->mTokens[key+1].type == JSMN_ARRAY)
   {
      JSONParser *answer = JSONParser_null();
      answer->mJson = jp->mJson;
      answer->mTokens = jp->mTokens;
      answer->mNtokens = jp->mNtokens;
      answer->mFirstKey = key+2;
      answer->mLastToken = key + getSkip(jp, key+1) +1;
      answer->mIsRoot = 0;
      answer->mIsArray = 1;
      return answer;

   }
   *err = -1;
   return JSONParser_null(); 
}

int JSONArray_getLength(JSONArray *ja)
{
   if (ja->mFirstKey > 0)
      return ja->mTokens[ja->mFirstKey-1].size;
   else
     return -1;
}

JSONParser *JSONArray_getObject(JSONArray *ja, int index, int *err)
{
   int token_index;

   token_index = getArrayIndex(ja, ja->mFirstKey-1, index);

   if(token_index >= 0)
   {
      if(ja->mTokens[token_index].type == JSMN_ARRAY)
      {
        JSONParser *answer = JSONParser_null();
        answer->mJson = ja->mJson;
        answer->mTokens = ja->mTokens;
        answer->mNtokens = ja->mNtokens;
        answer->mFirstKey = token_index+1;
        answer->mLastToken = token_index + getSkip(ja, token_index);
        answer->mIsRoot = 0;
        return answer;
     }
  }
  *err = -1;
  return JSONParser_null();

}

JSONArray *JSONArray_getArray(JSONArray *ja, int index, int *err)
{
   int token_index;

   token_index = getArrayIndex(ja, ja->mFirstKey-1, index);

   if(token_index >= 0)
   {
      if(ja->mTokens[token_index].type == JSMN_ARRAY)
      {
        JSONParser *answer = JSONParser_null();
        answer->mJson = ja->mJson;
        answer->mTokens = ja->mTokens;
        answer->mNtokens = ja->mNtokens;
        answer->mFirstKey = token_index+1;
        answer->mLastToken = token_index + getSkip(ja, token_index);
        answer->mIsRoot = 0;
        return answer;
     }
  }
  *err = -1;
  return JSONParser_null();
}

int JSONArray_getBoolean(JSONArray *ja, int index, int *err)
{
   int token_index;
   token_index = getArrayIndex(ja, ja->mFirstKey -1, index);
   if (token_index >= 0)
      return jsonboolval(ja->mJson, &ja->mTokens[token_index], err);
   *err = 1;

   return -1;
}


int JSONArray_getInteger(JSONArray *ja, int index, int *err)
{
   int token_index;
   token_index = getArrayIndex(ja, ja->mFirstKey -1, index);
   if (token_index >= 0)
      return jsonintval(ja->mJson, &ja->mTokens[token_index], err);
   *err = 1;

   return -1;
}

double JSONArray_getReal(JSONArray *ja, int index, int *err)
{
   int token_index;

   token_index = getArrayIndex(ja, ja->mFirstKey -1, index);
   if (token_index >= 0)
      return jsonrealval(ja->mJson, &ja->mTokens[token_index], err);
   *err = 1;
   return 0.0;
}

char *JSONArray_getString(JSONArray *ja, int index, int *err)
{
   int token_index;

   token_index = getArrayIndex(ja, ja->mFirstKey -1, index);
   if (token_index >= 0)
      return jsonstringval(ja->mJson, &ja->mTokens[token_index], err);
   *err = 1;
   return 0;
}

/*
   Length (number of elements) in an array
   0 = the empty array
   -1 = not an array
 */
int JSONParser_getArrayLength(JSONParser *jp, const char *id, int *err)
{
    int key;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken)
    {
        if(jp->mTokens[key+1].type == JSMN_ARRAY)
            return jp->mTokens[key+1].size;
    }
    *err = -1;
    return -1;
}

/*
   This is the heart of the system's inefficiency.
     jsmn tokenises the input, but there is no "sibling" pointer,
     so you have to pass over all the tokens to get the one you want,
     recursing to deal with nested objects.
 */
static int getSkip(JSONParser *jp, int index)
{
    int i, j;
    
    if (jp->mTokens[index].type == JSMN_PRIMITIVE)
    {
        return 1;
    }
    else if (jp->mTokens[index].type  == JSMN_STRING)
    {
        return 1;
    }
    else if (jp->mTokens[index].type == JSMN_OBJECT)
    {
        j = 0;
        for (i = 0; i < jp->mTokens[index].size; i++)
        {
            j += getSkip(jp, index + j + 1);
            j += getSkip(jp, index + j + 1);
        }
        return j+1;
    }
    else if (jp->mTokens[index].type == JSMN_ARRAY)
    {
        j = 0;
        for (i = 0; i < jp->mTokens[index].size; i++)
        {
            j += getSkip(jp, index+j+1);
            
        }
        return j+1;
    }
    
    return -1;
}

/*
   JSON works on key/value pairs. Keys are supposed to be always strings.
   For efficiency, we assume that the key won't contain any escapes.
 
   You pass over at each level, using getSkip() to get the sibling. 
   So each parser can only parse the top level of each object.
 
 */
static int getKey(JSONParser *jp, const char *id)
{
    int i = jp->mFirstKey;
    
    while(i < jp->mNtokens )
    {
        if (jp->mTokens[i].type == JSMN_PRIMITIVE)
        {
            return -1;
        }
        else if (jp->mTokens[i].type  == JSMN_STRING)
        {
            if(!jsoneq(jp->mJson, &jp->mTokens[i], id))
                return i;
            else
                i = i + getSkip(jp, i+1) + 1;
        }
        else if (jp->mTokens[i].type == JSMN_OBJECT)
        {
            return -1;
        }
        else if (jp->mTokens[i].type == JSMN_ARRAY)
        {
            return -1;
        }
    }
    
    return -1;
}

/*
   Convert an array index to a token.
   Not very efficent, we start at the beginning of the array and then through the
 siblings index times.
 */
static int getArrayIndex(JSONParser *jp, int array, int index)
{
    int i;
    int answer;
    
    if(index >= jp->mTokens[array].size)
        return -1;
    answer = array +1;
    for(i=0;i<index;i++)
    {
        answer += getSkip(jp, answer);
        if(answer >= jp->mLastToken)
            return -1;
    }
    return answer;
    
}

int JSONParser_getBoolean(JSONParser *jp, const char *id, int *err)
{
    int key;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken)
    {
        return jsonboolval(jp->mJson, &jp->mTokens[key+1], err);
    }
    *err = -1;
    return 0;
}

double JSONParser_getReal(JSONParser *jp, const char *id, int *err)
{
    int key;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken)
    {
        return jsonrealval(jp->mJson, &jp->mTokens[key+1], err);
    }
    *err = -1;
    return 0;
}

int JSONParser_getInteger(JSONParser *jp, const char *id, int *err)
{
    int key;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken)
    {
        return jsonintval(jp->mJson, &jp->mTokens[key+1], err);
    }
    *err = 1;
    return 0;

}

char *JSONParser_getString(JSONParser *jp, const char *id, int *err)
{
    int key;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken)
    {
        return jsonstringval(jp->mJson, &jp->mTokens[key+1], err);
    }
    *err = -1;
    return 0;
}

int JSONParser_getBooleanA(JSONParser *jp, const char *id, int index, int *err)
{
    int key;
    int token_index;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken && jp->mTokens[key+1].type == JSMN_ARRAY)
    {
        token_index = getArrayIndex(jp, key +1, index);
        
        if(token_index >= 0)
        {
            return jsonboolval(jp->mJson, &jp->mTokens[token_index], err);
        }
    }
    *err = -1;
    return 0;
 
}


double JSONParser_getRealA(JSONParser *jp, const char *id, int index, int *err)
{
    int key;
    int token_index;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken && jp->mTokens[key+1].type == JSMN_ARRAY)
    {
        token_index = getArrayIndex(jp, key +1, index);
        
        if(token_index >= 0)
        {
            return jsonrealval(jp->mJson, &jp->mTokens[token_index], err);
        }
    }
    *err = -1;
    return 0;

    
}

int JSONParser_getIntegerA(JSONParser *jp, const char *id, int index, int *err)
{
    int key;
    int token_index;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken && jp->mTokens[key+1].type == JSMN_ARRAY)
    {
        token_index = getArrayIndex(jp, key +1, index);
        
        if(token_index >= 0)
        {
            return jsonintval(jp->mJson, &jp->mTokens[token_index], err);
        }
    }
    *err = -1;
    return 0;

}

char *JSONParser_getStringA(JSONParser *jp, const char *id, int index, int *err)
{
    int key;
    int token_index;
    
    key = getKey(jp, id);
    if(key >= 0 && key +1 < jp->mLastToken && jp->mTokens[key+1].type == JSMN_ARRAY)
    {
        token_index = getArrayIndex(jp, key +1, index);
        
        if(token_index >= 0)
        {
            return jsonstringval(jp->mJson, &jp->mTokens[token_index], err);
        }
    }
    *err = -1;
    return 0;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
}

static int jsonboolval(const char *json, jsmntok_t *tok, int *err)
{
    if (tok->type == JSMN_PRIMITIVE &&
        strncmp(json + tok->start, "true", tok->end - tok->start) == 0)
    {
        return 1;
    }
    if (tok->type == JSMN_PRIMITIVE &&
        strncmp(json + tok->start, "false", tok->end - tok->start) == 0)
    {
        return 0;
    }
    *err = -1;
    return 0;
}

static double jsonrealval(const char *json, jsmntok_t *tok, int *err)
{
    char *end;
    double answer;
    
    if (tok->type == JSMN_PRIMITIVE)
    {
        answer = strtod(json + tok->start, &end);
        if(end != json + tok->end)
        {
            *err = -1;
        }
        return answer;
    }
    *err = -1;
    
    return -1;
}

static int jsonintval(const char *json, jsmntok_t *tok, int *err)
{
    char *end;
    long answer;
    
    if (tok->type == JSMN_PRIMITIVE)
    {
        answer = strtol(json + tok->start, &end, 10);
        if(end != json + tok->end)
        {
            *err = -1;
        }
        return (int) answer;
    }
    *err = -1;
    
    return -1;
}

static char *jsonstringval(const char *json, jsmntok_t *tok, int *err)
{
    char *answer;
    char *temp;

    if (tok->type == JSMN_STRING)
    {
        temp = malloc(tok->end - tok->start + 1);
        if (!temp)
        {
           *err = -1;
           return 0;
        }
        memcpy(temp, json + tok->start, tok->end - tok->start);
        temp[tok->end - tok->start] = 0;
        answer = unescapeJSON(temp);
        free(temp);
        if (!answer)
        {
           *err = -1;
           return 0;
        }
        return answer;
    }
    *err = -1;
    return 0;
}

static char *unescapeJSON(const char *input)
{
    enum
    {
        UNESCAPED  =0,
        ESCAPED = 1,
    };
    int s = UNESCAPED;
    int i;
    int j = 0;
    char *output = malloc(strlen(input) + 1);
    
    for (i = 0; input[i]; i++)
    {
        switch(s)
        {
            case ESCAPED:
            {
                switch(input[i])
                {
                    case '"':
                        output[j++] = '\"';
                        break;
                    case '/':
                        output[j++] = '/';
                        break;
                    case 'b':
                        output[j++] = '\b';
                        break;
                    case 'f':
                        output[j++] = '\f';
                        break;
                    case 'n':
                        output[j++] = '\n';
                        break;
                    case 'r':
                        output[j++] = '\r';
                        break;
                    case 't':
                        output[j++] = '\t';
                        break;
                    case '\\':
                        output[j++] = '\\';
                        break;
                    default:
                        output[j++] = input[i];
                        break;
                }
                
                s = UNESCAPED;
                break;
            }
            case UNESCAPED:
            {
                switch(input[i])
                {
                    case '\\':
                        s = ESCAPED;
                        break;
                    default:
                        output[j++] = input[i];
                        break;
                }
            }
        }
    }

    output[j] = 0;

    return output;
}

char *escapeJSON(const char *input)
{
    char *output;
    int i;
    int j = 0;

    output = malloc(strlen(input) * 2 + 1);
    if (!output)
       return 0;
    
    for (i = 0; input[i]; i++)
    {
        switch (input[i]) {
            case '"':
                output[j++] = '\\';
                output[j++] = '"';
                break;
/*
            case '/':
                output[j++] = '\\';
                output[j++] = '/';
                break;
*/
            case '\b':
                output[j++] = '\\';
                output[j++] = 'b';
                break;
            case '\f':
                output[j++] = '\\';
                output[j++] = 'f';
                break;
            case '\n':
                output[j++] = '\\';
                output[j++] = 'n';
                break;
            case '\r':
                output[j++] = '\\';
                output[j++] = 'r';
                break;
            case '\t':
                output[j++] = '\\';
                output[j++] = 't';
                break;
            case '\\':
                output[j++] = '\\';
                output[j++] = '\\';
                break;
            default:
                output[j++] = input[i];
                break;
        }
        
    }

    output[j] = 0;
    
    return output;
}

static char *mystrdup(const char *str)
{
   char *answer = malloc(strlen(str) +1);
   if (!answer)
     return 0;

   strcpy(answer, str);

   return answer;
}
