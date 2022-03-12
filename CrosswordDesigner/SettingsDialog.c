#include <ctype.h>
#include <windows.h>

#include "numwin.h"
#include "SettingsDialog.h"


#define ID_OK_BUT 1
#define ID_CANCEL_BUT 2
#define ID_COPYRIGHT_EDT 3
#define ID_TITLE_EDT 4
#define ID_AUTHOR_EDT 5
#define ID_EDITOR_EDT 6
#define ID_PUBLISHER_EDT 7
#define ID_DATE_EDT 8

typedef struct
{
  int width;
  int height;
  int retval;
} ANSWER;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd);
static void InitialiseControls(HWND hwnd, SETTINGS* set);

static char *GetTxt(HWND hwnd);
static void SetTxt(HWND hwnd, const char* text);
static void trim(char* str);
static char *mystrdup(const char *str);

void RegisterSettingsDialog(HINSTANCE hInstance)
{
  WNDCLASSEX wndclass;

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
  wndclass.lpszClassName = "SettingsDialog";
  wndclass.hIconSm = 0;

  RegisterClassEx(&wndclass);
}

int OpenSettingsDialog(HWND hparent, SETTINGS *set)
{
  HWND hwnd;
  MSG msg;
  RECT rect;

  GetWindowRect(hparent, &rect);


  hwnd = CreateWindowEx(
	  WS_EX_CLIENTEDGE,
	  "SettingsDialog",
	  "Settings",
	  WS_POPUP | WS_OVERLAPPED | WS_CAPTION,
	  rect.left, //0,//CW_USEDEFAULT,
	  rect.top, //0,//CW_USEDEFAULT,
	  300, //rect.right - rect.left,
	  300, //rect.bottom - rect.top, //450,
	  hparent,
	  NULL,
	  (HINSTANCE) GetWindowLongPtr(hparent, GWLP_HINSTANCE),
	  set
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
  SETTINGS *set;

  set = (SETTINGS *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
  switch(msg)
  {
    case WM_CREATE:
	  set = (SETTINGS *) ((CREATESTRUCT *) lParam)->lpCreateParams;
	  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) set);
	  CreateControls(hwnd);
	  InitialiseControls(hwnd, set);
	  //SetNumWin(GetDlgItem(hwnd, ID_WIDTH_NUM), ans->width, 1, 100, 1);
	  //SetNumWin(GetDlgItem(hwnd, ID_HEIGHT_NUM), ans->height, 1, 100, 1);
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
			free(set->title);
			free(set->author);
			free(set->editor);
			free(set->publisher);
			free(set->date);
			free(set->copyright);

			set->title = GetTxt(GetDlgItem(hwnd, ID_TITLE_EDT));
			set->author = GetTxt(GetDlgItem(hwnd, ID_AUTHOR_EDT));
			set->editor = GetTxt(GetDlgItem(hwnd, ID_EDITOR_EDT));
			set->publisher = GetTxt(GetDlgItem(hwnd, ID_PUBLISHER_EDT));
			set->date = GetTxt(GetDlgItem(hwnd, ID_DATE_EDT));
			set->copyright = GetTxt(GetDlgItem(hwnd, ID_COPYRIGHT_EDT));

		  DestroyWindow(hwnd);
          return 0;
		case ID_CANCEL_BUT:
		  DestroyWindow(hwnd);
          return 0;
	
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
	  "Title",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  20, 
	  60,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "edit",
	  "",
	  WS_CHILD | WS_VISIBLE | WS_BORDER,
	  100,
	  20, 
	  rect.right - 100 -10,
	  20,
	  hwnd,
	  (HMENU) ID_TITLE_EDT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "static",
	  "Author",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  50, 
	  60,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "edit",
	  "",
	  WS_CHILD | WS_VISIBLE | WS_BORDER,
	  100,
	  50, 
	  rect.right - 100 -10,
	  20,
	  hwnd,
	  (HMENU) ID_AUTHOR_EDT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "static",
	  "Editor",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  80, 
	  60,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "edit",
	  "",
	  WS_CHILD | WS_VISIBLE | WS_BORDER,
	  100,
	  80, 
	  rect.right - 100 -10,
	  20,
	  hwnd,
	  (HMENU) ID_EDITOR_EDT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "static",
	  "Publisher",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  110, 
	  75,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "edit",
	  "",
	  WS_CHILD | WS_VISIBLE | WS_BORDER,
	  100,
	  110, 
	  rect.right - 100 - 10,
	  20,
	  hwnd,
	  (HMENU) ID_PUBLISHER_EDT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "static",
	  "Date",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  140, 
	  60,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "edit",
	  "",
	  WS_CHILD | WS_VISIBLE | WS_BORDER,
	  100,
	  140, 
	  rect.right - 100 - 10,
	  20,
	  hwnd,
	  (HMENU) ID_DATE_EDT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "static",
	  "Copyright",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  170, 
	  65,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "edit",
	  "",
	  WS_CHILD | WS_VISIBLE | WS_BORDER,
	  100,
	  170, 
	  rect.right - 100 -10,
	  20,
	  hwnd,
	  (HMENU) ID_COPYRIGHT_EDT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );


	hctl = CreateWindowEx(
	  0,
	  "button",
	  "OK",
	  WS_CHILD | WS_VISIBLE,
	  rect.right/2 - 100,
	  rect.bottom -50, 
	  50,
	  20,
	  hwnd,
	  (HMENU) ID_OK_BUT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "button",
	  "Cancel",
	  WS_CHILD | WS_VISIBLE,
	  rect.right/2,
	  rect.bottom -50, 
	  100,
	  20,
	  hwnd,
	  (HMENU) ID_CANCEL_BUT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );
	
	/*
    hctl = CreateWindowEx(
	  0,
	  "static",
	  "Width",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  rect.bottom -150, 
	  60,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "numwin",
	  "",
	  WS_CHILD | WS_VISIBLE,
	  rect.right/2 - 25,
	  rect.bottom -150, 
	  60,
	  20,
	  hwnd,
	  (HMENU) ID_WIDTH_NUM,
	  (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
	  0
	  );
	

	hctl = CreateWindowEx(
	  0,
	  "static",
	  "Height",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  rect.bottom -100, 
	  60,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
	  0
	  );

	hctl = CreateWindowEx(
	  0,
	  "numwin",
	  "",
	  WS_CHILD | WS_VISIBLE,
	  rect.right/2 - 25,
	  rect.bottom -100, 
	  60,
	  20,
	  hwnd,
	  (HMENU) ID_HEIGHT_NUM,
	  (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
	  0
	  );
	  */
	

}

static void InitialiseControls(HWND hwnd, SETTINGS *set)
{
	SetTxt(GetDlgItem(hwnd, ID_TITLE_EDT), set->title);
	SetTxt(GetDlgItem(hwnd, ID_AUTHOR_EDT), set->author);
	SetTxt(GetDlgItem(hwnd, ID_EDITOR_EDT), set->editor);
	SetTxt(GetDlgItem(hwnd, ID_PUBLISHER_EDT), set->publisher);
	SetTxt(GetDlgItem(hwnd, ID_DATE_EDT), set->date);
	SetTxt(GetDlgItem(hwnd, ID_COPYRIGHT_EDT), set->copyright);
	
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

  trim(answer);
  if (strlen(answer) == 0)
  {
	  free(answer);
	  answer = 0;
  }

  return answer;

}

static void SetTxt(HWND hwnd, const char* text)
{
	if (text)
		SetWindowText(hwnd, text);
	else
		SetWindowText(hwnd, "");
}

static void trim(char* str)
{
	int i;
	int len;

	for (i = 0; isspace((unsigned char)str[i]); i++)
		;
	if (i > 0)
		memmove(str, str + i, strlen(str) + 1 - i);
	len = strlen(str);
	while (len--)
	{
		if (isspace((unsigned char)str[len]))
			str[len] = 0;
		else
			break;
	}
}


/*
  strdup drop-in replacement
*/
static char *mystrdup(const char *str)
{
  char *answer;

  answer = malloc(strlen(str) + 1);
  if(!answer)
    return 0;
  strcpy(answer, str);
  return answer;
}