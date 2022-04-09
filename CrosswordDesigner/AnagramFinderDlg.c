#include <ctype.h>

#include <windows.h>

#include "wordmatcher.h"


#define ID_OK_BUT 1
#define ID_RESULTS_LST 2
#define ID_WORD_EDT 3

#define ID_LEVEL_GRP 4
#define ID_LEVEL0_RAD 5
#define ID_LEVEL1_RAD 6
#define ID_LEVEL2_RAD 7
#define ID_LEVEL3_RAD 8

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd);
static void MatchWord(HWND hwnd);
static char *GetTxt(HWND hwnd);
static void trim(char *str);

static HFONT g_hcaptionfont;
static HFONT g_hmessagefont;

void RegisterAnagramFinderDialog(HINSTANCE hInstance)
{
  WNDCLASSEX wndclass;
  NONCLIENTMETRICS metrics;

  wndclass.cbSize = sizeof(WNDCLASSEX);
  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = hInstance;
  wndclass.hIcon = 0;
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = CreateSolidBrush( GetSysColor(COLOR_BTNFACE) );
  wndclass.lpszMenuName = 0;
  wndclass.lpszClassName = "AnagramFinderDialog";
  wndclass.hIconSm = 0;

  RegisterClassEx(&wndclass);

  metrics.cbSize = sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
  g_hmessagefont = CreateFontIndirect(&metrics.lfMessageFont);
  g_hcaptionfont = CreateFontIndirect(&metrics.lfCaptionFont);
}

int OpenAnagramFinderDialog(HWND hparent)
{
  HWND hwnd;
  MSG msg;
  RECT rect;

  GetWindowRect(hparent, &rect);


  hwnd = CreateWindowEx(
	  WS_EX_CLIENTEDGE,
	  "AnagramFinderDialog",
	  "Anagram Finder",
	  WS_POPUP | WS_OVERLAPPED | WS_CAPTION,
	  rect.left, //0,//CW_USEDEFAULT,
	  rect.top, //0,//CW_USEDEFAULT,
	  310, //rect.right - rect.left,
	  400, //rect.bottom - rect.top, //450,
	  hparent,
	  NULL,
	  (HINSTANCE) GetWindowLongPtr(hparent, GWLP_HINSTANCE),
	  0
	  );

  ShowWindow(hwnd, SW_SHOWNORMAL);
  UpdateWindow(hwnd);

  while(IsWindow(hwnd))
  {
    GetMessage(&msg, 0, 0, 0);
    TranslateMessage(&msg);
	DispatchMessage(&msg);
  }

  return 0;

}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 // SETTINGS *set;

 // set = (SETTINGS *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
  switch(msg)
  {
    case WM_CREATE:
	  //set = (SETTINGS *) ((CREATESTRUCT *) lParam)->lpCreateParams;
	  //SetWindowLongPtr(hwnd, GWLP_USERDATA, (long) set);
	  CreateControls(hwnd);
	  CheckRadioButton(hwnd, ID_LEVEL0_RAD, ID_LEVEL3_RAD, ID_LEVEL0_RAD);
	  return 0;
	case WM_DESTROY:
	  return 0;
	case WM_PAINT:
	  //PaintMe(hwnd);
	  break;
	case WM_COMMAND:
	  switch(LOWORD(wParam))
	  {
	    case ID_OK_BUT: // OK Button
		  DestroyWindow(hwnd);
          return 0;
		case ID_WORD_EDT:
	      MatchWord(hwnd);
		  break;
		case ID_LEVEL0_RAD:
		case ID_LEVEL1_RAD:
		case ID_LEVEL2_RAD:
		case ID_LEVEL3_RAD:
		  MatchWord(hwnd);
		  break;
	  }
	  break;
  }
  return DefWindowProc(hwnd, msg,wParam, lParam);
}

static void CreateControls(HWND hwnd)
{
	HWND hctl;
	RECT rect;

    GetClientRect(hwnd, &rect);

	hctl = CreateWindowEx(
	  0,
	  "static",
	  "Find anagrams of",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  250, 
	  220,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );
	SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);

	hctl = CreateWindowEx(
	  0,
	  "edit",
	  "",
	  WS_CHILD | WS_VISIBLE | WS_BORDER,
	  40,
	  270, 
	  220,
	  20,
	  hwnd,
	  (HMENU) ID_WORD_EDT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "listbox",
	  "",
	  WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL,
	  10,
	  30, 
	  175,
	  200,
	  hwnd,
	  (HMENU) ID_RESULTS_LST,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

     hctl = CreateWindowEx(
	  0,
	  "button",
	  "Level",
	  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
	  rect.right - 100,
	  0, 
	  80,
	  125,
	  hwnd,
	  (HMENU) ID_LEVEL_GRP,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );
	 SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);


	  hctl = CreateWindowEx(
	  0,
	  "button",
	  "Easy",
	  WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON,
	  rect.right - 90,
	  25, 
	  60,
	  20,
	  hwnd,
	  (HMENU) ID_LEVEL0_RAD,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );
	  SendMessage(hctl, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), TRUE);

	   hctl = CreateWindowEx(
	  0,
	  "button",
	  "Medium",
	  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
	  rect.right - 90,
	  50, 
	  60,
	  20,
	  hwnd,
	  (HMENU) ID_LEVEL1_RAD,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	   SendMessage(hctl, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), TRUE);

	    hctl = CreateWindowEx(
	  0,
	  "button",
	  "Rare",
	  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
	  rect.right - 90,
	  75, 
	  60,
	  20,
	  hwnd,
	  (HMENU) ID_LEVEL2_RAD,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );
	  SendMessage(hctl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

		hctl = CreateWindowEx(
			0,
			"button",
			"Very rare",
			WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			rect.right - 90,
			100,
			60,
			20,
			hwnd,
			(HMENU)ID_LEVEL3_RAD,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			0
		);
		
    SendMessage(hctl, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), TRUE);
	

	hctl = CreateWindowEx(
	  0,
	  "button",
	  "OK",
	  WS_CHILD | WS_VISIBLE,
	  rect.right/2 - 25,
	  rect.bottom - 30, 
	  50,
	  20,
	  hwnd,
	  (HMENU) ID_OK_BUT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );
	SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);


	

}

static void MatchWord(HWND hwnd)
{
  char *word;
  char **matches;
  int Nmatches;
  int i;
  int level;

  if (IsDlgButtonChecked(hwnd, ID_LEVEL0_RAD))
	  level = 0;
  else if (IsDlgButtonChecked(hwnd, ID_LEVEL1_RAD))
	  level = 1;
  else  if (IsDlgButtonChecked(hwnd, ID_LEVEL2_RAD))
	  level = 2;
  else
	  level = 3;

  word = GetTxt(GetDlgItem(hwnd, ID_WORD_EDT));
  trim(word);
  for(i=0;word[i];i++)
    word[i] = toupper(word[i]);
  matches = findanagrams(word, level, &Nmatches);
 
  SendMessage(GetDlgItem(hwnd, ID_RESULTS_LST), LB_RESETCONTENT, 0, 0);
  for(i=0;i<Nmatches && i < 256;i++) 
	 SendMessage(GetDlgItem(hwnd, ID_RESULTS_LST), LB_ADDSTRING, 0, (LPARAM) matches[i]);
  
  for(i=0;i<Nmatches;i++)
    free(matches[i]);
  free(matches);
  free(word);
}

static char *GetTxt(HWND hwnd)
{
  int len;
  char *answer;

  len = GetWindowTextLength(hwnd);
  answer = malloc(len +1);
  if(!answer)
    return 0;
  GetWindowText(hwnd, answer, len+1);
  answer[len] = 0;

  return answer;

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