#include "crosswordipuz.h"
#include "crossword.h"
#include "jsonparser.h"

#include <stdio.h>
#include <stdlib.h>

CROSSWORD *sparseipuz(char *json, int *error);
static char *fslurp(char *filename);

int saveasipuz(CROSSWORD* cw, char* fname)
{
    int answer = 0;
    FILE *fp = fopen(fname, "w");
    if (fp)
        fsaveasipuz(cw, fp);
    else
        answer = -1;
    fclose(fp);

    return answer;
}

int fsaveasipuz(CROSSWORD *cw, FILE *fp)
{
   int x, y;

   fprintf(fp, "{\n");
   fprintf(fp, "\"origin\": \"Crossword Designer by Malcolm McLean\",\n");
   fprintf(fp, "\"version\": \"http://ipuz.org/v1\",\n");
   fprintf(fp, "\"kind\": [\"http://ipuz.org/crossword#1\"],\n");
   fprintf(fp, "\"empty\": \"0\",\n");
  
   fprintf(fp, "\"dimensions\": { \"width\": %d, \"height\": %d },\n",
     cw->width, cw->height);
   fprintf(fp, "\n");
   fprintf(fp, "\"puzzle\": [\n");
   for (y =0; y < cw->height; y++)
   {
      fprintf(fp, "    [");
      for (x = 0; x < cw->width; x++)
      {
          if (cw->numbers[y * cw->width + x])
              fprintf(fp, "%d", cw->numbers[y * cw->width + x]);
          else if (cw->grid[y * cw->width + x])
              fprintf(fp, "0");
          else
              fprintf(fp, "\"#\"");
         if (x < cw->width -1)
           fprintf(fp, ",");
      }
      fprintf(fp, "]");
      if (y < cw->height -1)
        fprintf(fp, ",");
      fprintf(fp, "\n");
      
   }
   fprintf(fp, "],\n");
   fprintf(fp, "\n");
   fprintf(fp, "\"clues\":{\n");
   fprintf(fp, "    \"Across\": [\n");
   for (y = 0; y < cw->Nacross; y++)
   {
      if(cw->cluesacross[y])
      {
        char *escapedjson = escapeJSON(cw->cluesacross[y]);
        fprintf(fp, "        [%d,\"%s\"]", cw->numbersacross[y],
           escapedjson);
        free(escapedjson);
      }
      else
      {
        fprintf(fp, "        [%d,\"%s\"]", cw->numbersacross[y], "null");
      }
      if (y < cw->Nacross -1)
        fprintf(fp, ",");
      fprintf(fp, "\n");
   }
   fprintf(fp, "    ],\n");
   fprintf(fp, "\n");    

   fprintf(fp, "    \"Down\": [\n");
   for (y = 0; y < cw->Ndown; y++)
   {
      if(cw->cluesdown[y])
      {
        char *escapedjson = escapeJSON(cw->cluesdown[y]);
        fprintf(fp, "        [%d,\"%s\"]", cw->numbersdown[y],
           escapedjson);
        free(escapedjson);
      }
      else
      {
        fprintf(fp, "        [%d,\"%s\"]", cw->numbersdown[y], "null");
      } 
      if (y < cw->Ndown -1)
         fprintf(fp, ",");
      fprintf(fp, "\n");
   }
   fprintf(fp, "    ]\n");
   fprintf(fp, "},\n");
   fprintf(fp, "\n");
   fprintf(fp, "\"solution\":\n");
   fprintf(fp, "[\n");
   for(y =0; y < cw->height; y++)
   {
      fprintf(fp, "   [");
      for(x = 0; x < cw->width; x++)
      {
         int ch = cw->solution[y*cw->width+x];
         if (cw->grid[y*cw->width+x] == 0)
            ch = '#';
         fprintf(fp, "\"%c\"", ch);
         if (x < cw->width-1)
            fprintf(fp, ",");
      }
      fprintf(fp, "]");
      if (y < cw->height-1)
        fprintf(fp, ",");
      fprintf(fp, "\n");
 
   }
   fprintf(fp, "]\n");
   fprintf(fp, "}\n"); 

   return 0; 
}

CROSSWORD *loadfromipuz(char *filename, int *err)
{
   char *json = fslurp(filename);
   if (json)
       return sparseipuz(json, err);
   else
       if (err)
           *err = -1;
   return 0;
   
}

CROSSWORD *sparseipuz(char *json, int *error)
{
   CROSSWORD *cw = 0;
   int err = 0;
   int i;
   JSONParser *jparser = 0;
   JSONArray *solution = 0;
   JSONParser *dimensions = 0;
   JSONArray *row = 0;
   JSONParser *clues = 0;
   JSONArray *Across = 0;
   JSONArray *Down = 0;
   JSONArray *clue = 0;
   int width = 0;
   int height = 0;
   int x, y;

   json = strchr(json, '{');
   if (!json)
       goto parse_error;

   jparser = JSONParser_create(json);
   if (!jparser)
      goto parse_error;
   dimensions = JSONParser_getObject(jparser, "dimensions", &err);
   if (dimensions == 0 || err != 0)
       goto parse_error;
   width = JSONParser_getInteger(dimensions, "width", &err);
   height = JSONParser_getInteger(dimensions, "height", &err);
   if (err != 0)
      goto parse_error;
  
   if (width <= 0 || height <= 0)
      goto parse_error;
    
   cw = createcrossword(width, height);
   if (!cw)
      goto parse_error;
   
   solution = JSONParser_getArray(jparser, "solution", &err);
   if (!solution || err != 0)
      goto parse_error;

   for (y = 0; y < height; y++)
   {
      row = JSONArray_getArray(solution, y, &err);
      if (!row || err != 0)
         goto parse_error;
      if (JSONArray_getLength(row) != width)
         goto parse_error;
      for (x = 0; x < width; x++)
      {
         char *cell = JSONArray_getString(row, x, &err);
         if (cell && err == 0)
         {
            if (cell[0] && cell[0] != '#')
               crossword_setcell(cw, x, y, cell[0]);
         }
         free(cell);
         err = 0;
      }
      killJSONArray(row);
      row = 0;
   }

   clues = JSONParser_getObject(jparser, "clues", &err);
   if (clues && err == 0)
   {
      Across = JSONParser_getArray(clues, "Across", &err);
      if (Across && err == 0)
      {
         for (i =0; i < JSONArray_getLength(Across); i++)
         {
            clue = JSONArray_getArray(Across, i, &err);
            if (clue && err == 0 && JSONArray_getLength(clue) >= 2)
            {
               int id = JSONArray_getInteger(clue, 0, &err);
               char *cluestr = JSONArray_getString(clue, 1, &err);
               if (cluestr)
                   crossword_setacrossclue(cw, id, cluestr);
               free(cluestr);
            }
            killJSONArray(clue);
            clue = 0;
         }

      }
      killJSONArray(Across);
      Across = 0;

      Down = JSONParser_getArray(clues, "Down", &err);
      if (Down && err == 0)
      {
          for(i= 0; i<JSONArray_getLength(Down); i++)
          {
             clue = JSONArray_getArray(Down, i, &err);
             if (clue && err == 0 && JSONArray_getLength(clue) >= 2)
             {
                int id = JSONArray_getInteger(clue, 0, &err);
                char *cluestr = JSONArray_getString(clue, 1, &err);
                if (cluestr)
                    crossword_setdownclue(cw, id, cluestr);
                free(cluestr);
             }
             killJSONArray(clue);
             clue = 0;
          }
      }
      killJSONArray(Down);
      Down = 0;
   }

   killJSONArray(clue);
   killJSONArray(Across);
   killJSONArray(Down);
   killJSONArray(solution);
   killJSONArray(row);
   killJSONParser(clues);
   killJSONParser(dimensions);
   killJSONParser(jparser);

   return cw; 
parse_error:
   killJSONArray(clue);
   killJSONArray(Across);
   killJSONArray(Down);
   killJSONArray(solution);
   killJSONArray(row);
   killJSONParser(clues);
   killJSONParser(dimensions);
   killJSONParser(jparser);
    killcrossword(cw);
    if (error)
        *error = -2;
    return 0;
}

static char *fslurp(char *filename)
{
   int ch;
   int Nread = 0;
   int buffsize = 1024;
   FILE *fp = fopen(filename, "r");
   if (!fp)
     return 0;
   char *answer = malloc(buffsize);
   if (!answer)
      goto error_exit;

   while ( (ch = fgetc(fp)) != EOF)
   {
      answer[Nread++] = (char) ch;

      if (Nread > buffsize -1)
      {
         char *temp = realloc(answer, buffsize + 1024);
         if (!temp)
            goto error_exit;
         answer = temp;
         buffsize = buffsize + 1024;
      }

   }
   answer[Nread] = 0;
   fclose(fp);

   return answer;

error_exit:
   fclose(fp);
   free(answer);
   return 0;

      
}

