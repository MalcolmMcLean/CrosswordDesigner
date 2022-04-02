#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "crossword.h"
#include "undo.h"
#include "console.h"
#include "ClueWin.h"


#define ID_CLUEENTRYNUM_TXT 101
#define ID_CLUEENTRYWORD_TXT 102
#define ID_CLUEENTRYCLUE_EDT 103

#define ID_CLUEENTRYBASE 1000

#define NOTIFY_RESIZE 100

#define ENTRYACROSS -1
#define ENTRYDOWN -2

typedef struct
{
  HWND hwnd;
  RECT rect;
  int index;
  int number;
  CROSSWORD* cw;
} CLUEENTRY;

typedef struct
{
  CROSSWORD *cw;
  CLUEENTRY* entries;
  int Nentries;
  int ypos;
  HWND across_edt;
  HWND down_edt;
  char *downtext;
  char *uptext;
} CLUEWIN;

static HFONT g_hfont = 0;
static HFONT g_inputfont = 0;
static HFONT g_hcaptionfont;

static void RegisterClueEntry(HINSTANCE hInstance);

static CLUEENTRY* CreateClueEntries(HWND hwnd, CROSSWORD* cw, int *Nentries);
static void KillClueEntries(CLUEENTRY* entries, int N);
static int WordStructureChanged(CROSSWORD* cw, CLUEENTRY* entries, int Nentries);
static void RefreshEntryWords(CLUEENTRY* entries, int Nentries);

static void LayoutClueEntries(HWND hwnd, CLUEWIN* cw);
static void GetScrollIndices(HWND hwnd, CLUEWIN* cw, int* firstindex, int* lastindex, int* yfract);
static void ScrollVMessage(HWND hwnd, CLUEWIN* cw, int msg, int val);
static void DoScrollBars(HWND hwnd, CLUEWIN* cw);

static LRESULT CALLBACK ClueEntryWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateClueEntryControls(HWND hwnd, CLUEENTRY* ce);

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd, CLUEWIN *cluewin);

static char *getacrosstext(CROSSWORD *cw);
static char *getdowntext(CROSSWORD *cw);
static void trimbrackets(char *clue);
static void trim(char *str);



void RegisterClueWin(HINSTANCE hInstance)
{
  WNDCLASSEX wndclass;
  NONCLIENTMETRICS metrics;

  wndclass.cbSize = sizeof(WNDCLASSEX);
  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = hInstance;
  wndclass.hIcon = LoadIcon(0, IDI_APPLICATION); //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HEART));
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)); //CreateSolidBrush( RGB(0, 255, 0) );// (HBRUSH) (COLOR_MENU + 1);
  wndclass.lpszMenuName = 0;
  wndclass.lpszClassName = "cluewin";
  wndclass.hIconSm = LoadIcon(0, IDI_APPLICATION); //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HEART));

  RegisterClassEx(&wndclass);

  RegisterClueEntry(hInstance);

  metrics.cbSize = sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
  g_inputfont = CreateFontIndirect(&metrics.lfMessageFont);
  g_hcaptionfont = CreateFontIndirect(&metrics.lfCaptionFont);
}


static void RegisterClueEntry(HINSTANCE hInstance)
{
	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = ClueEntryWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(0, IDI_APPLICATION); //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HEART));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)); //CreateSolidBrush(RGB(0, 255, 0));// (HBRUSH) (COLOR_MENU + 1);
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = "clueentry";
	wndclass.hIconSm = LoadIcon(0, IDI_APPLICATION); //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HEART));

	RegisterClassEx(&wndclass);
}

char* crossword_getcluestext(CROSSWORD* cw)
{
	char* answer = 0;
	char* cluesacross = 0;
	char* cluesdown = 0;
	int buffsize = 0;

	cluesacross = getacrosstext(cw);
	if (!cluesacross)
		goto error_exit;
	cluesdown = getdowntext(cw);
	if (!cluesdown)
		goto error_exit;
	buffsize = strlen(cluesacross) + strlen(cluesdown) + 256 + 1;
	answer = malloc(buffsize);
	if (!answer)
		goto error_exit;
	strcpy(answer, "Across:\n\n");
	strcat(answer, cluesacross);
	strcat(answer, "\nDown:\n\n");
	strcat(answer, cluesdown);

	free(cluesacross);
	free(cluesdown);
	return answer;
error_exit:
	free(cluesacross);
	free(cluesdown);
	free(answer);
	return 0;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  CLUEWIN *cluewin;
  int id;
  int event;

  cluewin = (CLUEWIN *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch(msg)
  {
    case WM_CREATE:
	  cluewin = malloc(sizeof(CLUEWIN));
	  cluewin->cw = (CROSSWORD *) ((CREATESTRUCT *) lParam)->lpCreateParams;
	  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) cluewin);
	  ShowScrollBar(hwnd, SB_VERT, TRUE);
	  g_hfont = CreateFont(8, 5, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		  DEFAULT_PITCH | FF_DONTCARE, "Tahoma"/*"Arial"*/);
	  cluewin->entries = CreateClueEntries(hwnd, cluewin->cw, &cluewin->Nentries);
	  cluewin->ypos = 0;
	  DoScrollBars(hwnd, cluewin);
	  LayoutClueEntries(hwnd, cluewin);
	  return 0;
	case WM_COMMAND:
	   id = LOWORD(wParam);
	   event = HIWORD(wParam);
	   if (id >= ID_CLUEENTRYBASE)
	   {
		   if (event == NOTIFY_RESIZE)
		   {
			   LayoutClueEntries(hwnd, cluewin);
			   DoScrollBars(hwnd, cluewin);
			   InvalidateRect(hwnd, 0, FALSE);
		   }
	   }
	   break;
	case WM_VSCROLL:
		ScrollVMessage(hwnd, cluewin, LOWORD(wParam), HIWORD(wParam));
		break;
	case WM_ERASEBKGND:
		return 0;
	case CW_SETCROSSWORD:
		cluewin->cw = (CROSSWORD *) lParam;
		KillClueEntries(cluewin->entries, cluewin->Nentries);
		cluewin->entries = CreateClueEntries(hwnd, cluewin->cw, &cluewin->Nentries);
		cluewin->ypos = 0;
		DoScrollBars(hwnd, cluewin);
		LayoutClueEntries(hwnd, cluewin);
		SendMessage(hwnd, CW_REFRESH, 0, 0);
		break;
	case CW_REFRESH:
		if (WordStructureChanged(cluewin->cw, cluewin->entries, cluewin->Nentries))
		{
			KillClueEntries(cluewin->entries, cluewin->Nentries);
			cluewin->entries = CreateClueEntries(hwnd, cluewin->cw, &cluewin->Nentries);
			cluewin->ypos = 0;
			DoScrollBars(hwnd, cluewin);
			LayoutClueEntries(hwnd, cluewin);
		}
		else
		{
			RefreshEntryWords(cluewin->entries, cluewin->Nentries);
		}
	   break;
	case WM_DESTROY:
	  free(cluewin);
	  return 0;
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}


static void ScrollVMessage(HWND hwnd, CLUEWIN* cw, int msg, int val)
{
	RECT rect;
	int clientheight = 0;
	int i;

	GetClientRect(hwnd, &rect);
	for (i = 0; i < cw->Nentries; i++)
		clientheight += cw->entries[i].rect.bottom;

	switch (msg)
	{
		// User clicked the shaft above the scroll box. 

	case SB_PAGEUP:
		cw->ypos -= (rect.bottom - rect.top);
		break;

		// User clicked the shaft below the scroll box. 

	case SB_PAGEDOWN:
		cw->ypos += (rect.bottom - rect.top);
		break;

		// User clicked the top arrow. 

	case SB_LINEUP:
		cw->ypos--;
		break;

		// User clicked the bottom arrow. 

	case SB_LINEDOWN:
		cw->ypos++;
		break;

		// User dragged the scroll box. 

	case SB_THUMBTRACK:
		cw->ypos = val;
		break;
	}

	if (cw->ypos > clientheight - (rect.bottom - rect.top))
		cw->ypos = clientheight - (rect.bottom - rect.top);
	if (cw->ypos < 0)
		cw->ypos = 0;

	DoScrollBars(hwnd, cw);
	LayoutClueEntries(hwnd, cw);
	InvalidateRect(hwnd, 0, 0);
	UpdateWindow(hwnd);
}

static void DoScrollBars(HWND hwnd, CLUEWIN* cw)
{
	SCROLLINFO si;
	RECT winrect;
	RECT clientrect;
	int clientheight = 0;
	int i;

	for (i = 0; i < cw->Nentries; i++)
		clientheight += cw->entries[i].rect.bottom;

	GetWindowRect(hwnd, &winrect);

	if (0 && cw->cw->height * 25 <= winrect.bottom - winrect.top)
		ShowScrollBar(hwnd, SB_VERT, 0);
	else
	{
		ShowScrollBar(hwnd, SB_VERT, 1);
		GetClientRect(hwnd, &clientrect);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		si.nMin = 0;
		si.nMax = clientheight - 1;
		si.nPage = (clientrect.bottom - clientrect.top);
		si.nPos = cw->ypos;

		SetScrollInfo(hwnd, SB_VERT, &si, 1);
	}

}


static CLUEENTRY* CreateClueEntries(HWND hwnd, CROSSWORD* cw, int *Nentries)
{
	int N = cw->Nacross + cw->Ndown;
	CLUEENTRY* answer = 0;
	int i;
	RECT rect;

	GetClientRect(hwnd, &rect);

	N = 2 + cw->Nacross + cw->Ndown;
	answer = malloc(N * sizeof(CLUEENTRY));
	if (!answer)
		return 0;
	for (i = 0; i < N; i++)
	{
		answer[i].cw = cw;

		if (i == 0)
		{
			answer[i].index = ENTRYACROSS;
			answer[i].number = 0;
		}
		else if (i == cw->Nacross + 1)
		{
			answer[i].index = ENTRYDOWN;
			answer[i].number = 0;
		}
		else if (i < cw->Nacross + 1)
		{
			answer[i].index = i - 1;
			answer[i].number = cw->numbersacross[answer[i].index];
		}
		else
		{
			answer[i].index = i - 2;
			answer[i].number = cw->numbersdown[answer[i].index - cw->Nacross];
		}

		answer[i].hwnd = CreateWindowEx
		(
			0,
			"clueentry",
			"",
			WS_CHILD,
			0,
			0,
			rect.right,
			answer[i].index == ENTRYDOWN ? 42 : 32,
			hwnd,
			(HMENU)(ID_CLUEENTRYBASE + i),
			(HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			&answer[i]
		);
		GetClientRect(answer[i].hwnd, &answer[i].rect);
	}

	*Nentries = N;
	return answer;
}

static void KillClueEntries(CLUEENTRY* entries, int N)
{
	int i;
	if (entries)
	{
		for (i = 0; i < N; i++)
		{
			DestroyWindow(entries[i].hwnd);
		}
		free(entries);
	}
}

static int WordStructureChanged(CROSSWORD* cw, CLUEENTRY* entries, int Nentries)
{
	int i;
	if (cw->Nacross + cw->Ndown != Nentries - 2)
		return 1;
	for (i = 0; i < Nentries; i++)
		if (entries[i].index == ENTRYDOWN)
			break;
	if (cw->Nacross != i -1)
		return 1;

	for (i = 0; i < Nentries; i++)
	{
		if (entries[i].index == ENTRYACROSS || entries[i].index == ENTRYDOWN)
			continue;
		if (entries[i].index < cw->Nacross)
		{
			if (cw->numbersacross[entries[i].index] != entries[i].number)
				return 1;
		}
		else
		{
			if (cw->numbersdown[entries[i].index - cw->Nacross] != entries[i].number)
				return 1;
		}

	}

	return 0;

}

static void RefreshEntryWords(CLUEENTRY* entries, int Nentries)
{
	int i;

	for (i = 0; i < Nentries; i++)
	{
		if (entries[i].index == ENTRYACROSS || entries[i].index == ENTRYDOWN)
			continue;
		if (entries[i].index < entries[i].cw->Nacross)
			SetWindowText(GetDlgItem(entries[i].hwnd, ID_CLUEENTRYWORD_TXT), entries[i].cw->wordsacross[entries[i].index]);
		else
			SetWindowText(GetDlgItem(entries[i].hwnd, ID_CLUEENTRYWORD_TXT), entries[i].cw->wordsdown[entries[i].index - entries[i].cw->Nacross]);
	}

}

static void LayoutClueEntries(HWND hwnd, CLUEWIN* cw)
{
	RECT rect;
	int firstindex;
	int lastindex;
	int yfract;
	int i;
	int y;

	GetClientRect(hwnd, &rect);
	GetScrollIndices(hwnd, cw, &firstindex, &lastindex, &yfract);
	for (i = 0; i < firstindex; i++)
		ShowWindow(cw->entries[i].hwnd, SW_HIDE);
	y = -yfract;
	for (i = firstindex; i < lastindex; i++)
	{
		BOOL redraw = (i == firstindex || i == lastindex - 1) ? TRUE : FALSE;
		MoveWindow(cw->entries[i].hwnd, 0, y, rect.right, cw->entries[i].rect.bottom, redraw);
		ShowWindow(cw->entries[i].hwnd, SW_SHOWNORMAL);
		y += cw->entries[i].rect.bottom;
	}
	for (i = lastindex; i < cw->Nentries; i++)
		ShowWindow(cw->entries[i].hwnd, SW_HIDE);

}

static void GetScrollIndices(HWND hwnd, CLUEWIN* cw, int* firstindex, int* lastindex, int* yfract)
{
	RECT rect;
	int i;
	int y = 0;
	int lasty = 0;
	int height = 0;
	GetClientRect(hwnd, &rect);
	for (i = 0; i < cw->Nentries; i++)
	{
		lasty = y;
		y += cw->entries[i].rect.bottom;
		if (y >= cw->ypos)
			break;
	}
	if (i == cw->Nentries)
	{
		*firstindex = 0;
		*lastindex = cw->Nentries;
		*yfract = 0;
		return;
	}
	*firstindex = i;
	*yfract = cw->ypos - lasty;
	height = cw->entries[i].rect.bottom - *yfract;
	while (i++ < cw->Nentries)
	{
		if (height > rect.bottom)
			break;
		height += cw->entries[i].rect.bottom;
	}
	*lastindex = i;
}

static LRESULT CALLBACK ClueEntryWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CLUEENTRY* clueentry;
	int id;
	int event;
	char clue[256];

	clueentry = (CLUEENTRY*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg)
	{
	case WM_CREATE:
		//clueentry = malloc(sizeof(CLUEENTRY));
		clueentry = (CLUEENTRY*)((CREATESTRUCT*)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)clueentry);
		CreateClueEntryControls(hwnd, clueentry);
		return 0;
	case WM_COMMAND:
		id = LOWORD(wParam);
		event = HIWORD(wParam);
		
		switch (id)
		{
		case ID_CLUEENTRYCLUE_EDT:
			if (event == EN_CHANGE)
			{
				HDC hdc = GetDC(hwnd);
				SIZE sz;
				RECT rectedt;
				int linesneeded;
				GetClientRect(GetDlgItem(hwnd, ID_CLUEENTRYCLUE_EDT), &rectedt);
				SelectObject(hdc, g_inputfont);
				GetWindowText(GetDlgItem(hwnd, ID_CLUEENTRYCLUE_EDT), clue, 255);
				GetTextExtentPoint32(hdc, clue, strlen(clue), &sz);
				linesneeded = sz.cx / (rectedt.right - 16) + 1;
				if (linesneeded > 0 && linesneeded != rectedt.bottom / 17)
				{
					WPARAM wp;
					MoveWindow(GetDlgItem(hwnd, ID_CLUEENTRYCLUE_EDT),
						50, 10, rectedt.right, 17 * linesneeded, TRUE);
					clueentry->rect.bottom = 32 + (linesneeded - 1) * 17;
					wp = MAKEWPARAM(GetWindowLong(hwnd, GWL_ID), NOTIFY_RESIZE);
					SendMessage(GetParent(hwnd), WM_COMMAND, wp, 0);
					InvalidateRect(hwnd, 0, FALSE);
					
				}
			}
			if (event == EN_KILLFOCUS)
			{
				int dirty = 0;
				int index;
				GetWindowText(GetDlgItem(hwnd, ID_CLUEENTRYCLUE_EDT), clue, 255);
				if (clueentry->index < clueentry->cw->Nacross)
				{
					if (clueentry->cw->cluesacross[clueentry->index] == 0 ||
						strcmp(clue, clueentry->cw->cluesacross[clueentry->index]))
							dirty = 1;
				}
				else
				{
					if (clueentry->cw->cluesdown[clueentry->index - clueentry->cw->Nacross] == 0 ||
						strcmp(clue, clueentry->cw->cluesdown[clueentry->index - clueentry->cw->Nacross]))
							dirty = 1;
					
				}
				if (event == EN_KILLFOCUS && dirty)
				{
					undo_push(clueentry->cw);
					if (clueentry->index < clueentry->cw->Nacross)
						crossword_setacrossclue(clueentry->cw, clueentry->number, clue);
					else
						crossword_setdownclue(clueentry->cw, clueentry->number, clue);

				}

			}
			break;
		}
		break;
	case WM_DESTROY:
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);

}

static void CreateClueEntryControls(HWND hwnd, CLUEENTRY *ce)
{
	RECT rect;
	RECT winrect;
	HWND hctl;
	char* clue;
	char buff[256];
	
	GetWindowRect(hwnd, &winrect);
	GetClientRect(hwnd, &rect);

	if (ce->index == ENTRYACROSS)
	{
		hctl = CreateWindowEx(
			0,
			"static",
			"",
			WS_CHILD | WS_VISIBLE | SS_CENTER,
			0,
			3,
			rect.right,
			rect.bottom -3,
			hwnd,
			(HMENU)ID_CLUEENTRYNUM_TXT,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			0);
		SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);
		SetWindowText(hctl, "Clues Across");
		return;
	}
	else if (ce->index == ENTRYDOWN)
	{
		hctl = CreateWindowEx(
			0,
			"static",
			"",
			WS_CHILD | WS_VISIBLE | SS_CENTER,
			0,
			13,
			rect.right,
			29,
			hwnd,
			(HMENU)ID_CLUEENTRYNUM_TXT,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			0);
		SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);
		SetWindowText(hctl, "Clues Down");
		return;
	}

	
	hctl = CreateWindowEx(
		0,
		"static",
		"",
		WS_CHILD | WS_VISIBLE,
		2,
		0,
		20,
		rect.bottom -1,
		hwnd,
		(HMENU)ID_CLUEENTRYNUM_TXT,
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		0);
	SendMessage(hctl, WM_SETFONT, (WPARAM)g_hfont, TRUE);
	if (ce->index < ce->cw->Nacross)
		sprintf(buff, "%d.", ce->cw->numbersacross[ce->index]);
	else
		sprintf(buff, "%d.", ce->cw->numbersdown[ce->index - ce->cw->Nacross]);
	SetWindowText(hctl, buff);

	hctl = CreateWindowEx(
		0,
		"static",
		"",
		WS_CHILD | WS_VISIBLE,
		22,
		0,
		150,
		10,
		hwnd,
		(HMENU)ID_CLUEENTRYWORD_TXT,
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		0);
	SendMessage(hctl, WM_SETFONT, (WPARAM)g_hfont, TRUE);
	if (ce->index < ce->cw->Nacross)
		snprintf(buff, 256, "%s", ce->cw->wordsacross[ce->index]);
	else
		snprintf(buff, 256, "%s", ce->cw->wordsdown[ce->index - ce->cw->Nacross]);
	SetWindowText(hctl, buff); 

	hctl = CreateWindowEx(
		0,
		"edit",
		"",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE,
		50,
		10,
		winrect.right - winrect.left - 50 - 20,
		17,
		hwnd,
		(HMENU)ID_CLUEENTRYCLUE_EDT,
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		0);
	SendMessage(hctl, WM_SETFONT, (WPARAM)g_inputfont, TRUE);
	if (ce->index < ce->cw->Nacross)
		clue = ce->cw->cluesacross[ce->index];
	else
		clue = ce->cw->cluesdown[ce->index - ce->cw->Nacross];
	if (clue == 0 || strlen(clue) == 0)
	{
		sprintf(buff, "clue %d", ce->number);
		clue = buff;
	}
	
	SetWindowText(hctl, clue);

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
  if (!answer)
	  goto error_exit;
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
error_exit:
  free(answer);
  return 0;
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
  if (!answer)
	  goto error_exit;
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
error_exit:
  free(answer);
  return 0;
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
