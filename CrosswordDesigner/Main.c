#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "resource1.h"

#include "crossword.h"
#include "fillgrid.h"
#include "wordmatcher.h"
#include "blankgrids.h"
#include "crosswordhtml.h"
#include "crosswordxpf.h"
#include "crosswordpuz.h"
#include "crosswordipuz.h"
#include "crosswordfromlist.h"
#include "genjigsaw.h"
#include "undo.h"

#include "loadwordlist.h"
#include "console.h"
#include "numwin.h"
#include "OpenFileDlg.h"
#include "MyMessageBox.h"
#include "SettingsDialog.h"
#include "SizeDialog.h"
#include "GenWordListDialog.h"
#include "WordmatcherDlg.h"
#include "AnagramFinderDlg.h"
#include "HelpDialog.h" 
#include "AboutDialog.h"
#include "WaitDialog.h"
#include "GridWin.h"
#include "ClueWin.h"
#include "PrintCrossword.h"

#define ID_GRIDWIN 10
#define ID_NACROSS_TXT 11
#define ID_NDOWN_TXT 12
#define ID_CLUEWIN 13
#define ID_LEVEL_GRP 15
#define ID_LEVEL0_RAD 16
#define ID_LEVEL1_RAD 17
#define ID_LEVEL2_RAD 18
#define ID_LEVEL3_RAD 19

#define ID_LOAD_MNU 101
#define ID_SAVE_MNU 102
#define ID_PRINT_MNU 103
#define ID_HTML_MNU 104
#define ID_HTML_INTERACTIVE_MNU 105
#define ID_EXIT_MNU 106
#define ID_RESIZE_MNU 107
#define ID_COPY_PUZZLE_MNU 108
#define ID_COPY_SOLUTION_MNU 109
#define ID_COPY_SOLUTIONTXT_MNU 110
#define ID_COPY_CLUES_MNU 111
#define ID_SETTINGS_MNU 112
#define ID_WORDMATCHER_MNU 113
#define ID_ANAGRAMFINDER_MNU 114
#define ID_FILLGRID_MNU 115
#define ID_GENFROMLIST_MNU 116
#define ID_STARTGRID_MNU 117
#define ID_RNDAMERICANGRID_MNU 118
#define ID_HELP_MNU 119
#define ID_ABOUT_MNU 120
#define ID_UNDO_MNU 121
#define ID_REDO_MNU 122

HINSTANCE hInst;
HWND g_hMainHwnd;
HFONT g_hcaptionfont;
HFONT g_hinputfont;
CROSSWORD *cw;
SETTINGS settings = { 0, };
int g_difficulty = 0;
int g_dirty = 0;

static ATOM MyRegisterClass(HINSTANCE hInstance);
static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd, CROSSWORD *cw);
static void FillMenus(HWND hwnd);
static int GetDifficulty(HWND hwnd);
static void Load(HWND hwnd);
static void Undo(HWND hwnd);
static void Redo(HWND hwnd);
static void Save(HWND hwnd);
static void ExportAsHTML(HWND hwnd);
static void ExportAsInteractiveHTML(HWND hwnd);
static void EditSettings(HWND hwnd);
static void SetSize(HWND hwnd);
static void SetPuzzleSize(HWND hwnd, int width, int height);
static void CopyClues(HWND hwnd);
static void CopySolutionAsText(HWND hwnd);
static void CopyPuzzleGrid(HWND hwnd);
static void CopySolutionGrid(HWND hwnd);
static void GenFromWordList(HWND hwnd);
static void EnableUndo(void *ptr, int enableundo, int enableredo);
static int FillGridCallback(void *ptr);
static char* crossword_getsolution_toprint(CROSSWORD* cwd);
static int setcrosswordtorandomamerican(CROSSWORD* cwd);
static int settings_changed(CROSSWORD* cwd, SETTINGS* set);
static int mystrcmpx(const char *stra, const char* strb);
static int extensionequals(char *fname, char *ext);
static char *mystrdupx(const char* str);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	//UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);
	NONCLIENTMETRICS metrics;
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	static ACCEL accelerators[2] = {
		{FCONTROL | FNOINVERT | FVIRTKEY, 0x5A , ID_UNDO_MNU},
		{FCONTROL | FNOINVERT | FVIRTKEY, 0x59, ID_REDO_MNU}
	};

	srand((unsigned)time(0));
	// Initialize global strings
//	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//	LoadString(hInstance, IDC_WINHELLO4, szWindowClass, MAX_LOADSTRING);
	RegisterConsole(hInstance);
	RegisterMyMessageBox(hInstance);
	//MakeConsole(hInstance);
	MyRegisterClass(hInstance);
	RegisterNumWin(hInstance);
    RegisterGridWin(hInstance);
	RegisterClueWin(hInstance);
	RegisterSizeDialog(hInstance);
	RegisterSettingsDialog(hInstance);
	RegisterWordMatcherDialog(hInstance);
	RegisterAnagramFinderDialog(hInstance);
	RegisterHelpDialog(hInstance);
	RegisterAboutDialog(hInstance);
	RegisterGenWordListDialog(hInstance);
	RegisterWaitDialog(hInstance);

	// Comment in to get debug console
	//AllocConsole();
	//freopen("CONOUT$", "w", stdout);
	//printf("Hello console\n");

	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	g_hinputfont = CreateFontIndirect(&metrics.lfMessageFont);
	g_hcaptionfont = CreateFontIndirect(&metrics.lfCaptionFont);
    

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

//	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINHELLO4));
	hAccelTable = CreateAcceleratorTable(&accelerators, 2);
	assert(hAccelTable);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(g_hMainHwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
static ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	wcex.lpszMenuName	= 0; //MAKEINTRESOURCE(IDC_WINHELLO4);
	wcex.lpszClassName	= "CrosswordDesigner"; //szWindowClass;
	wcex.hIconSm		= 0; //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow("CrosswordDesigner", "Crossword Designer", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 1024, 800, NULL, NULL, hInstance, NULL);
   g_hMainHwnd = hWnd;

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HWND *hwndptr;
	WAITDIALOG *wd;
	HBRUSH hbrush;
	LOGBRUSH logbrush;


	switch (message)
	{
	case WM_CREATE:
	   if (wordmatcherinit() != 0)
	   {
		   MyMessageBox(hWnd, "Not enough memory", "Fatal error", MB_ICONERROR | MB_OK);
		   exit(-1);
	   }
	   cw = createcrossword(9, 9);
	   CreateControls(hWnd, cw);
	   FillMenus(hWnd);
	   CheckRadioButton(hWnd, ID_LEVEL0_RAD, ID_LEVEL3_RAD, ID_LEVEL0_RAD + g_difficulty);
	   SetPuzzleSize(hWnd, 15, 15);
	   hwndptr = malloc(sizeof(HWND));
	   hwndptr[0] = hWnd;
	   undo_init(EnableUndo, hwndptr);
	   break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_LOAD_MNU:
		   undo_push(cw);
		   Load(hWnd);
		   break;
		case ID_SAVE_MNU:
			Save(hWnd);
			break;
		case ID_PRINT_MNU:
			PrintCrossword(hWnd, cw);
			break;
		case ID_HTML_MNU:
			ExportAsHTML(hWnd);
			break;
		case ID_HTML_INTERACTIVE_MNU:
			ExportAsInteractiveHTML(hWnd);
			break;
		case ID_EXIT_MNU:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case ID_SETTINGS_MNU:
			EditSettings(hWnd);
			break;
		case ID_RESIZE_MNU:
			undo_push(cw);
			SetSize(hWnd);
			break;
		case ID_COPY_PUZZLE_MNU:
			CopyPuzzleGrid(hWnd);
			break;
		case ID_COPY_SOLUTION_MNU:
			CopySolutionGrid(hWnd);
			break;
		case ID_COPY_SOLUTIONTXT_MNU:
			CopySolutionAsText(hWnd);
			break;
		case ID_COPY_CLUES_MNU:
			CopyClues(hWnd);
			break;
		case ID_WORDMATCHER_MNU:
			OpenWordMatcherDialog(hWnd);
			break;
		case ID_ANAGRAMFINDER_MNU:
			OpenAnagramFinderDialog(hWnd);
			break;
		case ID_FILLGRID_MNU:
			wd = WaitDialog(hWnd, "Filling grid");
			undo_push(cw);
			fillgrid(cw, GetDifficulty(hWnd), FillGridCallback, wd);
			KillWaitDialog(wd);
			SendMessage(GetDlgItem(hWnd, ID_GRIDWIN), GW_SETCROSSWORD, 0, (LPARAM) cw);
		    SendMessage(GetDlgItem(hWnd, ID_CLUEWIN), CW_SETCROSSWORD, 0, (LPARAM) cw);
			g_dirty = 1;
			break;
		case ID_GENFROMLIST_MNU:
			undo_push(cw);
			GenFromWordList(hWnd);
		   break;
		case ID_STARTGRID_MNU:
			undo_push(cw);
            crossword_randgrid(cw);
			SendMessage(GetDlgItem(hWnd, ID_GRIDWIN), GW_SETCROSSWORD, 0, (LPARAM) cw);
		    SendMessage(GetDlgItem(hWnd, ID_CLUEWIN), CW_SETCROSSWORD, 0, (LPARAM) cw);
			g_dirty = 1;
			break;
		case ID_RNDAMERICANGRID_MNU:
			undo_push(cw);
			if (cw->width != 15 || cw->height != 15)
				SetPuzzleSize(hWnd, 15, 15);
			setcrosswordtorandomamerican(cw);
			SendMessage(GetDlgItem(hWnd, ID_GRIDWIN), GW_SETCROSSWORD, 0, (LPARAM)cw);
			SendMessage(GetDlgItem(hWnd, ID_CLUEWIN), CW_SETCROSSWORD, 0, (LPARAM)cw);
			g_dirty = 1;
			break;
		case ID_HELP_MNU:
			OpenHelpDialog(hWnd);
			break;
		case ID_ABOUT_MNU:
			OpenAboutDialog(hWnd);
			break;
		case ID_GRIDWIN:
			SendMessage(GetDlgItem(hWnd, ID_CLUEWIN), CW_REFRESH, 0, 0);
			g_dirty = 1;
		    break;
		case ID_UNDO_MNU:
			Undo(hWnd);
			if (!undo_hasundos())
				g_dirty = 0;
			break;
		case ID_REDO_MNU:
			Redo(hWnd);
			g_dirty = 1;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		//TextOut(hdc, 10, 10, "Hello World", 11);
		EndPaint(hWnd, &ps);
		break;
	//case WM_CTLCOLORSTATIC:
		//hbrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
		//GetObject(hbrush, sizeof(logbrush), &logbrush);
		//SetBkColor((HDC)wParam, logbrush.lbColor);
		//return (BOOL) hbrush;
		//break;
	case WM_CLOSE:
		if (g_dirty)
		{
			if (MyMessageBox(hWnd, "You have unsaved data, really quit ?", "Closing", MB_YESNOCANCEL) == IDYES)
			{
				DestroyWindow(hWnd);
			}
			return 0;
		}
		else
			DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static void CreateControls(HWND hwnd, CROSSWORD *cw)
{
	HWND hctl;
	RECT rect;
	GetClientRect(hwnd, &rect);

      hctl = CreateWindowEx(
	  0,
	  "gridwin",
	  "",
	  WS_CHILD,
	  100,
	  100,
	  240 - 24,
	  240 - 24,
	  hwnd,
	  (HMENU) ID_GRIDWIN,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  cw);
  assert(hctl);
  ShowWindow(hctl, SW_SHOWNORMAL);

   hctl = CreateWindowEx(
	  WS_EX_STATICEDGE,
	  "cluewin",
	  "",
	  WS_CHILD,
	  550,
	  50,
	  450,
	  680,
	  hwnd,
	  (HMENU) ID_CLUEWIN,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  cw);
  assert(hctl);
  ShowWindow(hctl, SW_SHOWNORMAL);

  
  hctl = CreateWindowEx(
	  0,
	  "button",
	  "Word complexity",
	  WS_CHILD | BS_GROUPBOX,
	  50,
	  430,
	  130,
	  125,
	  hwnd,
	  (HMENU)ID_LEVEL_GRP,
	  (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
  );
  assert(hctl);
  SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);
  ShowWindow(hctl, SW_SHOWNORMAL);

  hctl = CreateWindowEx(
	  0,
	  "button",
	  "Easy (+ phrases)",
	  WS_CHILD | WS_GROUP | BS_AUTORADIOBUTTON,
	  55,
	  430+25,
	  95,
	  20,
	  hwnd,
	  (HMENU)ID_LEVEL0_RAD,
	  (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
  );
  SendMessage(hctl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
  assert(hctl);
  ShowWindow(hctl, SW_SHOWNORMAL);

  hctl = CreateWindowEx(
	  0,
	  "button",
	  "Medium",
	  WS_CHILD  | BS_AUTORADIOBUTTON,
	  55,
	  430+50,
	  60,
	  20,
	  hwnd,
	  (HMENU)ID_LEVEL1_RAD,
	  (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
  );
  SendMessage(hctl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
  assert(hctl);
  ShowWindow(hctl, SW_SHOWNORMAL);

  hctl = CreateWindowEx(
	  0,
	  "button",
	  "Rare",
	  WS_CHILD | BS_AUTORADIOBUTTON,
	  55,
	  430+75,
	  60,
	  20,
	  hwnd,
	  (HMENU)ID_LEVEL2_RAD,
	  (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
  );
  SendMessage(hctl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
  assert(hctl);
  ShowWindow(hctl, SW_SHOWNORMAL);

  hctl = CreateWindowEx(
	  0,
	  "button",
	  "Very rare",
	  WS_CHILD | BS_AUTORADIOBUTTON,
	  55,
	  430 + 100,
	  90,
	  20,
	  hwnd,
	  (HMENU)ID_LEVEL3_RAD,
	  (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
  );
  SendMessage(hctl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
  assert(hctl);
  ShowWindow(hctl, SW_SHOWNORMAL);


 
 
  
}

static void FillMenus(HWND hwnd)
{
  HMENU mainmenu;
  HMENU filemenu;
  HMENU editmenu;
  HMENU aboutmenu;
  HMENU randomgridmenu;
  HMENU clipboardmenu;

  filemenu = CreateMenu();
  AppendMenu(filemenu, MF_STRING, ID_LOAD_MNU, "Open...");
  AppendMenu(filemenu, MF_STRING, ID_SAVE_MNU, "Save...");
 // AppendMenu(filemenu, MF_STRING, ID_PRINT_MNU, "Print");
  AppendMenu(filemenu, MF_STRING, ID_HTML_MNU, "Export as HTML...");
  AppendMenu(filemenu, MF_STRING, ID_HTML_INTERACTIVE_MNU, "Export as interactive HTML...");
  AppendMenu(filemenu, MF_SEPARATOR, 0, "");
  AppendMenu(filemenu, MF_STRING, ID_EXIT_MNU, "Exit");


  randomgridmenu = CreateMenu();
  AppendMenu(randomgridmenu, MF_STRING, ID_STARTGRID_MNU, "English-style");
  AppendMenu(randomgridmenu, MF_STRING, ID_RNDAMERICANGRID_MNU, "American-style");

  clipboardmenu = CreateMenu();
  AppendMenu(clipboardmenu, MF_STRING, ID_COPY_PUZZLE_MNU, "Copy puzzle grid");
  AppendMenu(clipboardmenu, MF_STRING, ID_COPY_SOLUTION_MNU, "Copy solution grid");
  AppendMenu(clipboardmenu, MF_STRING, ID_COPY_SOLUTIONTXT_MNU, "Copy solution (as text)");
  AppendMenu(clipboardmenu, MF_STRING, ID_COPY_CLUES_MNU, "Copy clues");


  editmenu = CreateMenu();
  AppendMenu(editmenu, MF_STRING, ID_UNDO_MNU, "Undo\t\t\tCtrl-Z");
  AppendMenu(editmenu, MF_STRING, ID_REDO_MNU, "Redo\t\t\tCtrl-Y");
  AppendMenu(editmenu, MF_STRING, ID_RESIZE_MNU, "Set size...");
  AppendMenu(editmenu, MF_STRING | MF_POPUP, (UINT_PTR)randomgridmenu, "Starter grid");
  AppendMenu(editmenu, MF_STRING, ID_SETTINGS_MNU, "Settings...");
  AppendMenu(editmenu, MF_STRING | MF_POPUP, (UINT_PTR)clipboardmenu, "Copy to clipboard");
  AppendMenu(editmenu, MF_STRING, ID_WORDMATCHER_MNU, "Word matcher...");
  AppendMenu(editmenu, MF_STRING, ID_ANAGRAMFINDER_MNU, "Anagram finder...");
  AppendMenu(editmenu, MF_STRING, ID_FILLGRID_MNU, "Fill grid");
  AppendMenu(editmenu, MF_STRING, ID_GENFROMLIST_MNU, "Generate from word list...");

  aboutmenu = CreateMenu();
  AppendMenu(aboutmenu, MF_STRING, ID_HELP_MNU, "Help...");
  AppendMenu(aboutmenu, MF_STRING, ID_ABOUT_MNU, "About...");

  mainmenu = CreateMenu();
  AppendMenu(mainmenu, MF_POPUP, (UINT_PTR) filemenu, "File");
  AppendMenu(mainmenu, MF_POPUP, (UINT_PTR) editmenu, "Edit");
  AppendMenu(mainmenu, MF_POPUP, (UINT_PTR) aboutmenu, "Help");

  SetMenu(hwnd, mainmenu);
}

static void Load(HWND hwnd)
{
  CROSSWORD **cwlist = 0;
  int N;
  char *fname;
  int err;

  fname = GetCrosswordFile(hwnd);
  if(fname)
  {
	  if (extensionequals(fname, ".puz"))
	  {
		  CROSSWORD *newcw = loadfrompuz(fname, &err);
		  if (newcw)
		  {
			  cwlist = malloc(4 * sizeof(CROSSWORD *));
			  cwlist[0] = newcw;
		  }
	  }
	  else if (extensionequals(fname, ".ipuz"))
	  {
		  CROSSWORD* newcw = loadfromipuz(fname, &err);
		  if (newcw)
		  {
			  cwlist = malloc(4 * sizeof(CROSSWORD*));
			  cwlist[0] = newcw;
		  }
	  }
	  else
        cwlist = loadfromxpf(fname, &N, &err);
	 if(cwlist == 0)
	 {
	   MyMessageBox(hwnd, "Load failed", "Wug", MB_OK);
	 }
	 else
	 {
		 undo_push(cw);
    	cw = cwlist[0];
		SetPuzzleSize(hwnd, cw->width, cw->height);
		SendMessage(GetDlgItem(hwnd, ID_GRIDWIN), GW_SETCROSSWORD, 0, (LPARAM) cw);
		SendMessage(GetDlgItem(hwnd, ID_CLUEWIN), CW_SETCROSSWORD, 0, (LPARAM) cw);
		g_dirty = 0;
	 }
  }
  free(fname);
}

static int GetDifficulty(HWND hwnd)
{
	int level = 0;
	if (IsDlgButtonChecked(hwnd, ID_LEVEL0_RAD))
		level = 0;
	else if (IsDlgButtonChecked(hwnd, ID_LEVEL1_RAD))
		level = 1;
	else if (IsDlgButtonChecked(hwnd, ID_LEVEL2_RAD))
		level = 2;
	else if (IsDlgButtonChecked(hwnd, ID_LEVEL3_RAD))
		level = 3;

	return level;
}

static void Undo(HWND hwnd)
{
	CROSSWORD *tempcw;

	tempcw = undo_pop(cw);
	if (!tempcw)
		return;
	
	SetPuzzleSize(hwnd, tempcw->width, tempcw->height);
	SendMessage(GetDlgItem(hwnd, ID_GRIDWIN), GW_SETCROSSWORD, 0, (LPARAM)tempcw);
	SendMessage(GetDlgItem(hwnd, ID_CLUEWIN), CW_SETCROSSWORD, 0, (LPARAM)tempcw);
	killcrossword(cw);
	cw = tempcw; 
	
}

static void Redo(HWND hwnd)
{
	CROSSWORD* tempcw;

	tempcw = undo_redo();
	if (!tempcw)
		return;
	SetPuzzleSize(hwnd, tempcw->width, tempcw->height);
	SendMessage(GetDlgItem(hwnd, ID_GRIDWIN), GW_SETCROSSWORD, 0, (LPARAM)tempcw);
	SendMessage(GetDlgItem(hwnd, ID_CLUEWIN), CW_SETCROSSWORD, 0, (LPARAM)tempcw);
	killcrossword(cw);
	cw = tempcw;
}

static void Save(HWND hwnd)
{
  char *fname;

  fname = GetCrosswordSaveFile(hwnd);
  if (fname)
  {
	  if (extensionequals(fname, ".puz"))
		  saveaspuz(cw, fname);
	  else if (extensionequals(fname, ".ipuz"))
		  saveasipuz(cw, fname);
	  else
		  saveasxpf(cw, fname);
	  g_dirty = 0;
  }
  free(fname);

}

static void ExportAsHTML(HWND hwnd)
{
   char *fname;
   int err;

  fname = GetHTMLSaveFile(hwnd);
  if(fname)
    crosswordhtml(fname, cw, &err);
  free(fname);
}

static void ExportAsInteractiveHTML(HWND hwnd)
{
   char *fname;
   int err;

  fname = GetHTMLSaveFile(hwnd);
  if(fname)
    crosswordinteractivehtml(fname, cw, &err);
  free(fname);
}

static void EditSettings(HWND hwnd)
{
	free(settings.title);
	free(settings.author);
	free(settings.editor);
	free(settings.publisher);
	free(settings.date);
	free(settings.copyright);
	settings.title = mystrdupx(cw->title);
	settings.author = mystrdupx(cw->author);
	settings.editor = mystrdupx(cw->editor);
	settings.publisher = mystrdupx(cw->publisher);
	settings.date = mystrdupx(cw->date);
	settings.copyright = mystrdupx(cw->copyright);
	OpenSettingsDialog(hwnd, &settings);
	if (settings_changed(cw, &settings))
	{
		undo_push(cw);
		free(cw->title);
		free(cw->author);
		free(cw->editor);
		free(cw->publisher);
		free(cw->date);
		free(cw->copyright);
		cw->title = mystrdupx(settings.title);
		cw->author = mystrdupx(settings.author);
		cw->editor = mystrdupx(settings.editor);
		cw->publisher = mystrdupx(settings.publisher);
		cw->date = mystrdupx(settings.date);
		cw->copyright = mystrdupx(settings.copyright);

		g_dirty = 1;
	}
	
}

static void SetSize(HWND hwnd)
{
  int width, height;

  width = cw->width;
  height = cw->height;
  if(OpenSizeDialog(hwnd, &width, &height))
  {
    SetPuzzleSize(hwnd, width, height);
	g_dirty = 1;
  }
}

static void SetPuzzleSize(HWND hwnd, int width, int height)
{
	int x, y;
	int winwidth, winheight;
	int changed = 0;

	x = 50;
	y = 50;
	if (cw->width != width || cw->height != height)
	{
		crossword_resize(cw, width, height);
		changed = 1;
	}
	if(width <= 15 && height <= 15)
	{
	  x = 50 - ( (width - 15) * 24 ) / 2;
	  y = 50 - ( (height - 15) * 24 ) /2;
	  MoveWindow( GetDlgItem(hwnd, ID_GRIDWIN), x, y, width * 24 +2, height * 24 +2, TRUE);
	}
	else
	{
	  if(width <= 15)
	     x = 50 - ( (width - 15) * 24) /2;
	  if(height <= 15)
		 y = 50 - ( (height - 15) * 24) /2;
	  winwidth = width;
	  winheight = height;
	  if(width > 15)
	    winwidth = 15;
	  if(height > 15)
	    winheight = 15;
	   MoveWindow( GetDlgItem(hwnd, ID_GRIDWIN), x, y, winwidth * 24, winheight * 24, TRUE);
	}
	if (changed)
		SendMessage(GetDlgItem(hwnd, ID_GRIDWIN), GW_SETCROSSWORD, 0, (LPARAM) cw);
}

static void CopyClues(HWND hwnd)
{
	HGLOBAL hGlobal = 0;
	char* clues = 0;
	char* lock = 0;

	clues = crossword_getcluestext(cw);
	if (!clues)
		goto error_exit;
	hGlobal = GlobalAlloc(GMEM_MOVEABLE, strlen(clues) + 1);
	if (!hGlobal)
		goto error_exit;
	lock = GlobalLock(hGlobal);
	if (!lock)
		goto error_exit;
	
	strcpy(lock, clues);
	GlobalUnlock(lock);

	if (OpenClipboard(hwnd))
	{
		EmptyClipboard();
		SetClipboardData(CF_TEXT, lock);
		CloseClipboard();
	}
	else
		goto error_exit;
	free(clues);
	return;
error_exit:
	if (hGlobal)
		GlobalFree(hGlobal);
	free(clues);
	MyMessageBox(hwnd, "There was an error copying your clues to the clipboard", "Windows error", MB_OK);
}

static void CopySolutionAsText(HWND hwnd)
{
	HGLOBAL hGlobal = 0;
	char* solutiontxt = 0;
	char* lock = 0;

	solutiontxt = crossword_getsolution_toprint(cw);
	printf("%s", solutiontxt);
	if (!solutiontxt)
		goto error_exit;
	hGlobal = GlobalAlloc(GMEM_MOVEABLE, strlen(solutiontxt) + 1);
	if (!hGlobal)
		goto error_exit;
	lock = GlobalLock(hGlobal);
	if (!lock)
		goto error_exit;

	strcpy(lock, solutiontxt);
	GlobalUnlock(lock);

	if (OpenClipboard(hwnd))
	{
		EmptyClipboard();
		SetClipboardData(CF_TEXT, lock);
		CloseClipboard();
	}
	else
		goto error_exit;
	free(solutiontxt);
	return;
error_exit:
	if (hGlobal)
		GlobalFree(hGlobal);
	free(solutiontxt);
	MyMessageBox(hwnd, "There was an error copying your solution to the clipboard", "Windows error", MB_OK);
}
static void CopyPuzzleGrid(HWND hwnd)
{
  int bmwidth = cw->width * 24;
  int bmheight = cw->height * 24;
  HDC hDCWindow = GetDC(hwnd);
  HDC hDCMem = CreateCompatibleDC(hDCWindow); //MemoryDC
  HBITMAP hMemBmp = CreateCompatibleBitmap(hDCWindow, bmwidth,bmheight);

  SelectObject(hDCMem, hMemBmp);
  SelectObject(hDCMem, GetStockObject(WHITE_BRUSH));
  Rectangle(hDCMem, 0, 0, bmwidth, bmheight);
  DrawGrid(hDCMem, cw, 0, 0, bmwidth, bmheight);
  if (OpenClipboard(hwnd))
  {
     EmptyClipboard();
     SetClipboardData (CF_BITMAP, hMemBmp );
     CloseClipboard();
  }
  else
    MyMessageBox(hwnd, "There was an error copying your data to the clipboard", "Windows error", MB_OK);

  DeleteObject(hMemBmp);
  DeleteDC(hDCMem);

}

static void CopySolutionGrid(HWND hwnd)
{
  int bmwidth = cw->width * 24;
  int bmheight = cw->height * 24;
  HDC hDCWindow = GetDC(hwnd);
  HDC hDCMem = CreateCompatibleDC(hDCWindow); //MemoryDC
  HBITMAP hMemBmp = CreateCompatibleBitmap(hDCWindow, bmwidth,bmheight);

  SelectObject(hDCMem, hMemBmp);
  SelectObject(hDCMem, GetStockObject(WHITE_BRUSH));
  Rectangle(hDCMem, 0, 0, bmwidth, bmheight);
  DrawSolution(hDCMem, cw, 0, 0, bmwidth, bmheight);
  if (OpenClipboard(hwnd))
  {
     EmptyClipboard();
     SetClipboardData (CF_BITMAP, hMemBmp );
     CloseClipboard();
  }
  else
    MyMessageBox(hwnd, "There was an error copying your data to the clipboard", "Windows error", MB_OK);

  DeleteObject(hMemBmp);
  DeleteDC(hDCMem);

}

static void GenFromWordList(HWND hwnd)
{
  char *fname;
  char **list;
  int N;
  int err;
  char *grid;
  int x, y;
  int algorithm;

  fname = GetWordListFile(hwnd);
  if(!fname)
    return;
  list = loadwordlist(fname, &N, &err);
  if(!list)
    return;
  algorithm = OpenGenWordListDialog(hwnd);
  shufflewordlist(list, N);
  grid = malloc(cw->width * cw->height);
  if(algorithm == ANNEALING_ALGORITHM)
    crosswordfromlist(grid, cw->width, cw->height, list, N, 1000, 1);
  else if(algorithm == JIGSAW_ALGORITHM)
    jigsawcrossword(grid, cw->width, cw->height, list, N);
  if(algorithm != CANCEL_ALGORITHM)
  {
    for(y=0;y<cw->height;y++)
      for(x=0;x<cw->width;x++)
	    crossword_setcell(cw, x, y, grid[y*cw->width+x]);

    SendMessage(GetDlgItem(hwnd, ID_GRIDWIN), GW_SETCROSSWORD, 0, (LPARAM) cw);
    SendMessage(GetDlgItem(hwnd, ID_CLUEWIN), CW_SETCROSSWORD, 0, (LPARAM) cw);
	g_dirty = 1;
  }

  killwordlist(list, N);
  free(grid);
  free(fname);
}

static void EnableUndo(void *ptr, int enableundo, int enableredo)
{
	HWND hwnd;
	HMENU hmenu;

	hwnd = *(HWND*)ptr;
	hmenu = GetMenu(hwnd);
	if (enableundo)
		EnableMenuItem(hmenu, ID_UNDO_MNU, MFS_ENABLED);
	else
		EnableMenuItem(hmenu, ID_UNDO_MNU, MFS_GRAYED);

	if (enableredo)
		EnableMenuItem(hmenu, ID_REDO_MNU, MFS_ENABLED);
	else
		EnableMenuItem(hmenu, ID_REDO_MNU, MFS_GRAYED);

}


static int FillGridCallback(void *ptr)
{
	WAITDIALOG *wd = ptr;
	WaitDialog_Tick(wd);
	return wd->stopped;
}

static char* crossword_getsolution_toprint(CROSSWORD* cwd)
{
	int x, y;
	int index = 0;
	char* answer = malloc(cwd->width * cwd->height + cwd->height + 1);
	if (!answer)
		goto error_exit;
	for (y = 0; y < cwd->height; y++)
	{
		for (x = 0; x < cwd->width; x++)
			answer[index++] = cwd->grid[y * cwd->width + x] ? cwd->solution[y * cwd->width + x] : '#';
		answer[index++] = '\n';

	}
	answer[index++] = 0;

	return answer;
	
error_exit:
	free(answer);
	return 0;
}

static int setcrosswordtorandomamerican(CROSSWORD* cwd)
{
	int width, height;
	char* grid;
	int x, y;

	grid = randomamericanblankgrid(&width, &height);
	if (!grid)
		return - 1;
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			crossword_setcell(cwd, x, y, grid[y * width + x] == '#' ? 0 : ' ');
	free(grid);

	return 0;
}

static int settings_changed(CROSSWORD *cwd, SETTINGS *set)
{
	if (mystrcmpx(cwd->title, set->title))
		return 1;
	if (mystrcmpx(cwd->title, set->author))
		return 1;
	if (mystrcmpx(cwd->editor, set->editor))
		return 1;
	if (mystrcmpx(cwd->publisher, set->publisher))
		return 1;
	if (mystrcmpx(cwd->date, set->date))
		return 1;
	if (mystrcmpx(cwd->copyright, set->copyright))
		return 1;

	return 0;
}

static int mystrcmpx(const char* stra, const char* strb)
{
	if (stra == NULL && strb == NULL)
		return 0;
	if (stra == NULL)
		return -1;
	if (strb == NULL)
		return 1;
	return strcmp(stra, strb);
}

static int extensionequals(char *fname, char *ext)
{
	char *dot;
	dot = strrchr(fname, '.');
	if (!dot)
		return 0;
	if (!_stricmp(ext, dot))
		return 1;
	return 0;
}

static char *mystrdupx(const char *str)
{
	char *answer = 0;

	if (str && strlen(str) > 0)
	{
		answer = malloc(strlen(str) + 1);
		if (answer)
			strcpy(answer, str);
	}   

	return answer;
}

/*

 // Send the DIB to the clipboard
     if (OpenClipboard())
     {
          BeginWaitCursor();
          EmptyClipboard();
          SetClipboardData (CF_DIB, hDIB );
          CloseClipboard();
          EndWaitCursor();
     }

	 HDC hDCWindow=::GetDC(hWindow);
     HDC hDCMem=CreateCompatibleDC(hDCWindow); //MemoryDC
     
     //create MemoryBitmap for MemoryDC
     HBITMAP hMemBmp=CreateCompatibleBitmap(hDCWindow,BitmapWidth,BitmapHeight);
     
     SelectObject(hDCMem,hMemBmp);
*/