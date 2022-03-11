#include <windows.h>

#include "GenWordListDialog.h"

#define ID_OK_BUT 1
#define ID_CANCEL_BUT 2
#define ID_JIGSAW_RAD 3
#define ID_ANNEALING_RAD 4


typedef struct
{
  int algorithm;
} ANSWER;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd);

void RegisterGenWordListDialog(HINSTANCE hInstance)
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
  wndclass.lpszClassName = "GenWordListDialog";
  wndclass.hIconSm = 0;

  RegisterClassEx(&wndclass);
}

int OpenGenWordListDialog(HWND hparent)
{
  HWND hwnd;
  MSG msg;
  RECT rect;

  ANSWER ans;

  ans.algorithm = JIGSAW_ALGORITHM;

  GetWindowRect(hparent, &rect);


  hwnd = CreateWindowEx(
	  WS_EX_CLIENTEDGE,
	  "GenWordListDialog",
	  "Generate from word list",
	  WS_POPUP | WS_OVERLAPPED | WS_CAPTION,
	  rect.left, //0,//CW_USEDEFAULT,
	  rect.top, //0,//CW_USEDEFAULT,
	  300, //rect.right - rect.left,
	  500, //rect.bottom - rect.top, //450,
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

  return ans.algorithm;

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
		case ID_CANCEL_BUT:
		  ans->algorithm = CANCEL_ALGORITHM;
		  DestroyWindow(hwnd);
		  return 0;
		case ID_JIGSAW_RAD:
		   if(HIWORD(wParam) == BN_CLICKED)
			   ans->algorithm = JIGSAW_ALGORITHM;
		   break;
		case ID_ANNEALING_RAD:
		   if(HIWORD(wParam) == BN_CLICKED)
			   ans->algorithm = ANNEALING_ALGORITHM;
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
	char *text = "Warning this will delete your current crossword.\n\n"
		"The Jigsaw algorithm is deterministic (but only works on 256 words)\n"
		"The annealing algorithm is a bit slower but more random\n\n"
		"Word lists are just ascii lists of words, one per line\n";


    GetClientRect(hwnd, &rect);

	 hctl = CreateWindowEx(
	  0,
	  "static",
	  "",
	  WS_CHILD | WS_VISIBLE,
	  10,
	  20, 
	  rect.right - 20,
	  rect.bottom - 200,
	  hwnd,
	  (HMENU) 0,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

   SetWindowText(hctl, text);


	hctl = CreateWindowEx(
	  0,
	  "button",
	  "Jigsaw algorithm",
	  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
	  30,
	  rect.bottom -200, 
	  150,
	  20,
	  hwnd,
	  (HMENU) ID_JIGSAW_RAD,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

   hctl = CreateWindowEx(
	  0,
	  "button",
	  "Annealing algorithm",
	  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
	  30,
	  rect.bottom -180, 
	  150,
	  20,
	  hwnd,
	  (HMENU) ID_ANNEALING_RAD,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );

   hctl = CreateWindowEx(
	  0,
	  "button",
	  "OK",
	  WS_CHILD | WS_VISIBLE,
	  rect.right/2 - 25 - rect.right/4,
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
	  rect.right/2 - 25 + rect.right/4,
	  rect.bottom -50, 
	  50,
	  20,
	  hwnd,
	  (HMENU) ID_OK_BUT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0
	  );
	
   
	

}