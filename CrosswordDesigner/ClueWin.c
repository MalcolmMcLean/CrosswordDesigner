#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "crossword.h"
#include "console.h"
#include "ClueWin.h"

#define ID_ACROSS_EDT 10
#define ID_DOWN_EDT 11
#define ID_ACROSSCLUES_TXT 12
#define ID_DOWNCLUES_TXT 13

typedef struct
{
  int number;
  char *clue;
} CLUES;

typedef struct
{
  CROSSWORD *cw;
  HWND across_edt;
  HWND down_edt;
  char *downtext;
  char *uptext;
} CLUEWIN;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd, CLUEWIN *cluewin);
static void ParseAcrossClues(HWND hwnd, CLUEWIN *cluewin);
static void ParseDownClues(HWND hwnd, CLUEWIN *cluewin);
static int CheckClues(HWND hwnd, CROSSWORD *cw, CLUES *clues, int Nclues, char *dir);

static CLUES *parseclues(char *str, int *N, char *message, int *err);
static char *getacrosstext(CROSSWORD *cw);
static char *getdowntext(CROSSWORD *cw);
static void trimbrackets(char *clue);
static void trim(char *str);

void RegisterClueWin(HINSTANCE hInstance)
{
  WNDCLASSEX wndclass;

  wndclass.cbSize = sizeof(WNDCLASSEX);
  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = hInstance;
  wndclass.hIcon = LoadIcon(0, IDI_APPLICATION); //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HEART));
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = CreateSolidBrush( RGB(0, 255, 0) );// (HBRUSH) (COLOR_MENU + 1);
  wndclass.lpszMenuName = 0;
  wndclass.lpszClassName = "cluewin";
  wndclass.hIconSm = LoadIcon(0, IDI_APPLICATION); //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HEART));

  RegisterClassEx(&wndclass);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  CLUEWIN *cluewin;
  int id;
  int event;
  char *str;

  cluewin = (CLUEWIN *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch(msg)
  {
    case WM_CREATE:
	  cluewin = malloc(sizeof(CLUEWIN));
	  cluewin->cw = (CROSSWORD *) ((CREATESTRUCT *) lParam)->lpCreateParams;
	  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) cluewin);
	  CreateControls(hwnd, cluewin);
	  return 0;
	case WM_COMMAND:
	   id = LOWORD(wParam);
	   event = HIWORD(wParam);
       switch(id)
	   {
	   case ID_ACROSS_EDT:
		   if(event == EN_KILLFOCUS)
		     ParseAcrossClues(hwnd, cluewin);
		   break;
	   case ID_DOWN_EDT:
		   if(event == EN_KILLFOCUS)
		     ParseDownClues(hwnd, cluewin);
	        break;
	   }
	   break;
	case CW_SETCROSSWORD:
		cluewin->cw = (CROSSWORD *) lParam;
		SendMessage(hwnd, CW_REFRESH, 0, 0);
		break;
	case CW_REFRESH:
       str = getacrosstext(cluewin->cw);
	   SetWindowText(GetDlgItem(hwnd, ID_ACROSS_EDT), str);
	   free(str);
	   str = getdowntext(cluewin->cw);
	   SetWindowText(GetDlgItem(hwnd, ID_DOWN_EDT), str);
	   free(str);
	   break;
	case WM_DESTROY:
	  free(cluewin);
	  return 0;
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void CreateControls(HWND hwnd, CLUEWIN *cluewin)
{
  RECT rect;
  HWND hctl;

  GetClientRect(hwnd, &rect);
  hctl = CreateWindowEx( 
	  0,
	  "static",
	  "",
	  WS_CHILD | WS_VISIBLE,
	  0,
	  0,
	  rect.right/2,
	  20,
	  hwnd,
	  (HMENU) ID_ACROSSCLUES_TXT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0);
  SetWindowText(hctl, "Clues across");

  hctl = CreateWindowEx( 
	  0,
	  "static",
	  "",
	  WS_CHILD | WS_VISIBLE,
	  rect.right/2,
	  0,
	  rect.right/2,
	  20,
	  hwnd,
	  (HMENU) ID_DOWNCLUES_TXT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0);
  SetWindowText(hctl, "Clues down");

  cluewin->across_edt = CreateWindowEx( 
	  0,
	  "edit",
	  "",
	  WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
	  0,
	  20,
	  rect.right/2,
	  rect.bottom - 20,
	  hwnd,
	  (HMENU) ID_ACROSS_EDT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0);
   cluewin->down_edt = CreateWindowEx(
	  0,
	  "edit",
	  "",
	  WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
	  rect.right/2,
	  20,
	  rect.right/2,
	  rect.bottom -20,
	  hwnd,
	  (HMENU) ID_DOWN_EDT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0);
}

static void ParseAcrossClues(HWND hwnd, CLUEWIN *cluewin)
{
  int len;
  char *buff;
  CLUES *clues;
  int err;
  char message[1024];
  int Nclues;
  int i;

  len = GetWindowTextLengthA(cluewin->across_edt);
  buff = malloc(len + 10);
  GetWindowTextA(cluewin->across_edt, buff, len+1);
  buff[len] = 0;
  clues = parseclues(buff, &Nclues, message, &err);
  CheckClues(hwnd, cluewin->cw, clues, Nclues, "across");
  for(i=0;i<Nclues;i++)
	  crossword_setacrossclue(cluewin->cw, clues[i].number, clues[i].clue);
  free(buff);
  free(clues);
}

static void ParseDownClues(HWND hwnd, CLUEWIN *cluewin)
{
  int len;
  char *buff;
  CLUES *clues;
  int err;
  char message[1024];
  int Nclues;
  int i;

  len = GetWindowTextLengthA(cluewin->down_edt);
  buff = malloc(len + 10);
  GetWindowTextA(cluewin->down_edt, buff, len+1);
  buff[len] = 0;
  clues = parseclues(buff, &Nclues, message, &err);
  CheckClues(hwnd, cluewin->cw, clues, Nclues, "down");
  for(i=0;i<Nclues;i++)
	  crossword_setdownclue(cluewin->cw, clues[i].number, clues[i].clue);
 
  free(buff);
  free(clues);
}

static int CheckClues(HWND hwnd, CROSSWORD *cw, CLUES *clues, int Nclues, char *dir)
{
  int i, ii;
  char msg[1024];

  for(i=0;i<Nclues;i++)
    for(ii=i+1;ii<Nclues;ii++)
	  if(clues[i].number == clues[ii].number)
	  {
		  sprintf(msg, "Duplicate %s clue %d", dir, clues[i].number);
		  MessageBox(hwnd, msg, "Problem with clues", MB_OK);
		  return -1;
	  }
  if(!strcmp(dir, "across"))
  {
    for(i=0;i<Nclues;i++)
    {
	  for(ii=0;ii<cw->Nacross;ii++)
	    if(cw->numbersacross[ii] == clues[i].number)
	      break;
	  if(ii == cw->Nacross)
	  {
	     sprintf(msg, "Across clue %d not found in puzzle", clues[i].number);
		 MessageBox(hwnd, msg, "Problem with clues", MB_OK);
		 return -1;
	  }
    }
  }
  else
  {
	for(i=0;i<Nclues;i++)
    {
	  for(ii=0;ii<cw->Ndown;ii++)
	    if(cw->numbersdown[ii] == clues[i].number)
	      break;
	  if(ii == cw->Ndown)
	  {
	     sprintf(msg, "Down clue %d not found in puzzle", clues[i].number);
		 MessageBox(hwnd, msg, "Problem with clues", MB_OK);
		 return -1;
	  }
    }
  }

  return 0;
}

static CLUES *parseclues(char *str, int *N, char *message, int *err)
{
	int Nfound = 0;
	CLUES *answer = 0;
	int num;
	char *end;
	char *clue;
	int i, j;

	Con_Printf("String ///\n%s\\\n", str);

	while(*str)
	{
	  while(isspace( (unsigned char) *str))
		  str++;
	  if(isdigit( (unsigned char) *str))
	  {
	    num = strtol(str, &end, 10);
		str = end;
		while(*str == ' ' || *str == '\t')
		  str++;
		if(*str == '.')
		  str++;
		i = 0;
		while(str[i] && str[i] != '\n')
		  i++;
		clue = malloc(i+1);
		for(j=0;j<i;j++)
		  clue[j] = str[j];
		clue[j] = 0;

		trim(clue);
		trimbrackets(clue);

		answer = realloc(answer, (Nfound + 1) * sizeof(CLUES));
		answer[Nfound].number = num;
		answer[Nfound].clue = clue;
		Nfound++;
		str += j;

		Con_Printf("Clue %d ***%s***\n", answer[Nfound-1].number, answer[Nfound-1].clue);

	  }
	  else
		if(*str)
	      str++;
	}
	*N = Nfound;
	*err = 0;
	return answer;
}

static char *getacrosstext(CROSSWORD *cw)
{
  char *answer = 0;
  int i;
  int N = 10;
  char *ptr;
  char numbuff[32];

  for(i=0;i<cw->Nacross;i++)
  {
	  if(cw->cluesacross[i] == 0)
	    N += 32;
	  else
	    N += strlen(cw->cluesacross[i]) + 10;
  }
  answer = malloc(N);
  ptr = answer;
  for(i=0;i<cw->Nacross;i++)
  {
	  sprintf(numbuff, "%d. ", cw->numbersacross[i]);
	  strcpy(ptr, numbuff);
      ptr += strlen(numbuff);
	  if(cw->cluesacross[i])
	  {
	    strcpy(ptr, cw->cluesacross[i]);
	    ptr += strlen(cw->cluesacross[i]);
		
	  }
	  else
	  {
	    strcpy(ptr, "Enter clue here");
		ptr += strlen("Enter clue here");
	  }
      if(ptr > answer && ptr[-1] != ')')
	  {
	    sprintf(numbuff, "(%d)", (int) strlen(cw->wordsacross[i]));
	    strcpy(ptr, numbuff);
	    ptr += strlen(numbuff);
	  }
	  
	  strcpy(ptr, "\r\n");
	  ptr+=2;
  }
  ptr[0] = 0;
  return answer;
}

static char *getdowntext(CROSSWORD *cw)
{
  char *answer = 0;
  int i;
  int N = 10;
  char *ptr;
  char numbuff[32];

  for(i=0;i<cw->Ndown;i++)
  {
	  if(cw->cluesdown[i] == 0)
	    N += 32;
	  else
	    N += strlen(cw->cluesdown[i]) + 10;
  }
  answer = malloc(N);
  ptr = answer;
  for(i=0;i<cw->Ndown;i++)
  {
	  sprintf(numbuff, "%d. ", cw->numbersdown[i]);
	  strcpy(ptr, numbuff);
      ptr += strlen(numbuff);
	  if(cw->cluesdown[i])
	  {
	    strcpy(ptr, cw->cluesdown[i]);
	    ptr += strlen(cw->cluesdown[i]);
		
	  }
	  else
	  {
	    strcpy(ptr, "Enter clue here");
		ptr += strlen("Enter clue here");
	  }
	  if(ptr > answer && ptr[-1] != ')')
	  {
	    sprintf(numbuff, "(%d)", (int) strlen(cw->wordsdown[i]));
	    strcpy(ptr, numbuff);
	    ptr += strlen(numbuff);
	  }
	  
	  strcpy(ptr, "\r\n");
	  ptr+=2;
  }
  ptr[0] = 0;
  return answer;
}

static void trimbrackets(char *clue)
{
  int len;
  int first;
  int i;

  len = strlen(clue);
  if(len > 0 && clue[len-1] == ')')
  {
    first = len-1;
	while(first >= 0 && clue[first] != '(')
	  first--;
	if(first = -1)
	  return;
    for(i=first+1;i<len-1;i++)
	  if(!isdigit((unsigned char) clue[i]) && 
		  !isspace((unsigned char) clue[i]) &&
		  clue[i] != ',')
      return;
	clue[first] = 0;
  }
  trim(clue);
}

static void trim(char *str)
{
  int i;
  int len;

  for(i=0;isspace((unsigned char) str[i]);i++)
	  ;
  if(i > 0)
    memmove(str, str + i, strlen(str) + 1 - i);
  len = strlen(str);
  while(len--)
  {
    if(isspace((unsigned char) str[len]))
	  str[len] = 0;
	else
	  break;
  }
}
