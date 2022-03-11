#include <windows.h>

#include "numwin.h"

#define ID_OK_BUT 1
#define ID_CANCEL_BUT 2
#define ID_WIDTH_NUM 3
#define ID_HEIGHT_NUM 4


typedef struct
{
  int width;
  int height;
  int retval;
} ANSWER;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd);

void RegisterSizeDialog(HINSTANCE hInstance)
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
  wndclass.lpszClassName = "SizeDialog";
  wndclass.hIconSm = 0;

  RegisterClassEx(&wndclass);
}

int OpenSizeDialog(HWND hparent, int *width, int *height)
{
  HWND hwnd;
  MSG msg;
  RECT rect;

  ANSWER ans;

  GetWindowRect(hparent, &rect);

  ans.width = *width;
  ans.height = *height;
  ans.retval = 0;

  hwnd = CreateWindowEx(
	  WS_EX_CLIENTEDGE,
	  "SizeDialog",
	  "Enter crossword size",
	  WS_POPUP | WS_OVERLAPPED | WS_CAPTION,
	  rect.left, //0,//CW_USEDEFAULT,
	  rect.top, //0,//CW_USEDEFAULT,
	  300, //rect.right - rect.left,
	  200, //rect.bottom - rect.top, //450,
	  hparent,
	  NULL,
	  (HINSTANCE) GetWindowLongPtr(hparent, GWLP_HINSTANCE),
	  &ans
	  );

  ShowWindow(hwnd, SW_SHOWNORMAL);
  UpdateWindow(hwnd);

  while(IsWindow(hwnd))
  {
    GetMessage(&msg, 0, 0, 0);
    TranslateMessage(&msg);
	DispatchMessage(&msg);
  }

  *width = ans.width;
  *height = ans.height;

  return ans.retval;

}


static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  ANSWER *ans;

  ans = (ANSWER *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
  switch(msg)
  {
    case WM_CREATE:
	  ans = (ANSWER *) ((CREATESTRUCT *) lParam)->lpCreateParams;
	  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) ans);
	  CreateControls(hwnd);
	  SetNumWin(GetDlgItem(hwnd, ID_WIDTH_NUM), ans->width, 1, 100, 1);
	  SetNumWin(GetDlgItem(hwnd, ID_HEIGHT_NUM), ans->height, 1, 100, 1);
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
		  ans->width = (int) GetNumWinVal(GetDlgItem(hwnd, ID_WIDTH_NUM));
		  ans->height = (int) GetNumWinVal(GetDlgItem(hwnd, ID_HEIGHT_NUM));
		  ans->retval = 1;
		  DestroyWindow(hwnd);
          return 0;
		case ID_CANCEL_BUT:
		  ans->retval = 9;
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
	  "Width",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  rect.bottom -150, 
	  60,
	  20,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
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
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
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
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
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
	
   
	

}