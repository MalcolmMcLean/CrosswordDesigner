#include <windows.h>

#include "numwin.h"

#define ID_OK_BUT 1


typedef struct
{
  int width;
  int height;
} ANSWER;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd);

void RegisterAboutDialog(HINSTANCE hInstance)
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
  wndclass.lpszClassName = "AboutDialog";
  wndclass.hIconSm = 0;

  RegisterClassEx(&wndclass);
}

int OpenAboutDialog(HWND hparent)
{
  HWND hwnd;
  MSG msg;
  RECT rect;

  ANSWER ans;

  GetWindowRect(hparent, &rect);


  hwnd = CreateWindowEx(
	  WS_EX_CLIENTEDGE,
	  "AboutDialog",
	  "About crossword designer",
	  WS_POPUP | WS_OVERLAPPED | WS_CAPTION,
	  rect.left, //0,//CW_USEDEFAULT,
	  rect.top, //0,//CW_USEDEFAULT,
	  300, //rect.right - rect.left,
	  300, //rect.bottom - rect.top, //450,
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

  return 0;

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
	  "",
	  WS_CHILD | WS_VISIBLE | SS_CENTER,
	  10,
	  20, 
	  rect.right - 20,
	  rect.bottom - 100,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

   SetWindowText(hctl, "Crossword Designer\n\nBy Malcolm McLean\n\n version 1.4 (prototype)\n\nThis program is freeware");


	hctl = CreateWindowEx(
	  0,
	  "button",
	  "OK",
	  WS_CHILD | WS_VISIBLE,
	  rect.right/2 - 25,
	  rect.bottom -50, 
	  50,
	  20,
	  hwnd,
	  (HMENU) ID_OK_BUT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );
	
   
	

}