/********************************************************************
*                             numwin.c                              *
*               Code for an intelligent number control.             *
********************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <assert.h>

#include "numwin.h"


#define ID_EDIT 1
#define ID_UPDOWN 2

typedef struct
{
  double x;         // the value
  double min;       // minimum
  double max;       // maximum
  double delta;     // accuracy allowed (0 = any value)
  int sigfigs;      // number of significant figures (decimal)
} NUM;


#define clamp(x, min, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))


static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd);
static void ResizeControls(HWND hwnd, int width, int height);
static void UpdownChange(HWND hwnd, NUM *num, NMUPDOWN *d);
static void EditChange(HWND hwnd, NUM *num);
static void EditEnd(HWND hwnd, NUM *num);
static void SetVal(HWND hwnd, NUM *num, double x);

static NUM *createnumber(void);
static void killnumber(NUM *num);
static int stringvalid(NUM *num, const char *str);
static double roundoff(double x, double delta);

/*
  Register the number window
  Params: hInstance - instance hook

*/

void RegisterNumWin(HINSTANCE hInstance)
{
  WNDCLASSEX wndclass;
  INITCOMMONCONTROLSEX iccex;

  wndclass.cbSize = sizeof(WNDCLASSEX);
  wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  wndclass.lpfnWndProc = WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = hInstance;
  wndclass.hIcon = NULL;
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = CreateSolidBrush( GetSysColor(COLOR_WINDOW) );// (HBRUSH) (COLOR_MENU + 1);
  wndclass.lpszMenuName = 0;
  wndclass.lpszClassName = "numwin";
  wndclass.hIconSm = NULL;

  RegisterClassEx(&wndclass);

  iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  iccex.dwICC = ICC_UPDOWN_CLASS;
  InitCommonControlsEx(&iccex);
}

/*
  SetNumWin - set up the parameters for the number window
  Params: hwnd - control handle
          x - value to set to
		  min - minimum value
		  max - maximum value
		  delta - amount of increment
*/
void SetNumWin(HWND hwnd, double x, double min, double max, double delta)
{
  assert( GetWindowLongPtr(hwnd, GWLP_WNDPROC) == (LPARAM) WndProc );

  SendMessage(hwnd, NW_SETMIN, 0, (LPARAM) &min);
  SendMessage(hwnd, NW_SETMAX, 0, (LPARAM) &max);
  SendMessage(hwnd, NW_SETDELTA, 0, (LPARAM) &delta);
  SendMessage(hwnd, NW_SETX, 0, (LPARAM) &x);
}

/*
  SetNumWinVal - set the value of a number window
  Params: hwnd - control handle
          x - new value to set to
  Notes: x will be clamped if not in range.
*/
void SetNumWinVal(HWND hwnd, double x)
{
  assert( GetWindowLongPtr(hwnd, GWLP_WNDPROC) == (LPARAM) WndProc );

  SendMessage(hwnd, NW_SETX, 0, (LPARAM) &x);
}

/*
  GetNumWinVal - retrieves the value from a numebr window
  Params: hwnd - control handle
  Returns: the value held in the control
*/
double GetNumWinVal(HWND hwnd)
{
  NUM *num;

  assert( GetWindowLongPtr(hwnd, GWLP_WNDPROC) == (LPARAM) WndProc );
  num = (NUM *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
  return num->x;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  NUM *num;
 
  num = (NUM *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch(msg)
  {
    case WM_CREATE:
	  num = createnumber();
	  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) num);
	  CreateControls(hwnd);
	  return 0;
	case WM_DESTROY:
	  killnumber(num);
	  return 0;
	case WM_SIZE:
	  ResizeControls(hwnd, LOWORD(lParam), HIWORD(lParam));
	  return 0;
	case WM_ENABLE:
	  EnableWindow(GetDlgItem(hwnd, ID_EDIT), (BOOL) wParam);
	  EnableWindow(GetDlgItem(hwnd, ID_UPDOWN), (BOOL) wParam);
	  break;
	case WM_SETFONT:
	  return SendMessage( GetDlgItem(hwnd, ID_EDIT), msg, wParam, lParam);
	case WM_NOTIFY:
	  if(LOWORD(wParam) == ID_UPDOWN)
	  {
        NMHDR *hdr = (NMHDR *) lParam;
		if(hdr->code == UDN_DELTAPOS)
		{
          UpdownChange(hwnd, num, (NMUPDOWN *) lParam);
		  return -1; // don't allow change
		}
	  }
	  break;
	case WM_COMMAND:
	  if(LOWORD(wParam) == ID_EDIT)
	  {
		if(HIWORD(wParam) == EN_CHANGE)
		{
          EditChange(hwnd, num);
		}
		if(HIWORD(wParam) == EN_KILLFOCUS)
		{
	      EditEnd(hwnd, num);
		}
	  }
	  break;
	case NW_SETMIN:
		num->min = *(double *) lParam;
		SetVal(hwnd, num, num->x);
		break;
	case NW_SETMAX:
		num->max = *(double *) lParam;
		SetVal(hwnd, num, num->x);
		break;
	case NW_SETDELTA:
		num->delta = *(double *) lParam;
		SetVal(hwnd, num, num->x);
		break;
	case NW_SETX:
		SetVal(hwnd, num, *(double*)lParam);
		break;
  }

  return DefWindowProc(hwnd, msg,wParam, lParam);
}

static void CreateControls(HWND hwnd)
{
  RECT rect;
  HWND hctl;

  GetClientRect(hwnd, &rect);

  hctl = CreateWindowEx(
	  WS_EX_CLIENTEDGE,
	  "edit",
	  "0",
	  WS_VISIBLE | WS_CHILD | ES_RIGHT | SS_SUNKEN,
	  rect.left,
	  rect.top,
	  rect.right - rect.left - 15,
	  rect.bottom - rect.top + 1,
	  hwnd,
	  (HMENU) ID_EDIT,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0);

  SendMessage(hctl, EM_SETLIMITTEXT, 63, 0);
  SendMessage(hctl, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

  hctl = CreateUpDownControl(
	  WS_CHILD | WS_BORDER | WS_VISIBLE,
	  rect.right - 14,
      rect.top,
	  15,
	  rect.bottom - rect.top + 1,
	  hwnd,
	  ID_UPDOWN,
	  (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	  0, // hwnd buddy
	  1000,
      -1000,
	  0
	  );
}

static void ResizeControls(HWND hwnd, int width, int height)
{
  MoveWindow( GetDlgItem(hwnd, ID_EDIT), 
	  0,
	  0,
	  width - 15,
	  height,
	  TRUE
	  );

  MoveWindow( GetDlgItem(hwnd, ID_UPDOWN),
	  width - 15,
	  0,
	  15,
	  height,
	  TRUE
	  );

}

static void UpdownChange(HWND hwnd, NUM *num, NMUPDOWN *d)
{
  char buff[64];
  double x;

  GetWindowText( GetDlgItem(hwnd, ID_EDIT), buff, 63);
  SetVal(hwnd, num, atof(buff));

  x = num->x + d->iDelta * num->delta;
  x = clamp(x, num->min, num->max);
  sprintf(buff, "%g",x);
  SetWindowText( GetDlgItem(hwnd, ID_EDIT), buff);
  if(num->x != x)
  {
    num->x = x;
	SendMessage( (HWND) GetWindowLongPtr(hwnd, GWLP_HWNDPARENT), 
	           WM_COMMAND,
			   MAKEWPARAM( GetWindowLong(hwnd, GWL_ID), NWN_CHANGE ),
			   0
			  );

  }
}

static void EditChange(HWND hwnd, NUM *num)
{
  HWND hedit;
  char inbuff[64];
  char outbuff[64];

  hedit = GetDlgItem(hwnd, ID_EDIT);
 
  GetWindowText(hedit, inbuff, 63);
  if(!stringvalid(num, inbuff))
  {
    sprintf(outbuff, "%g", num->x);
	if(strcmp(inbuff, outbuff))
	  SetWindowText(hedit, outbuff);
  }
}

static void EditEnd(HWND hwnd, NUM *num)
{
  HWND hedit;
  double x;
  char buff[64];

  hedit = GetDlgItem(hwnd, ID_EDIT);
 
  GetWindowText(hedit, buff, 63);
  x = atof(buff);
  
  if( num->delta && fmod(x, num->delta) )
	x = roundoff(x, num->delta);
  x = clamp(x, num->min, num->max);

  sprintf(buff, "%g", x);
  SetWindowText(hedit, buff);
  
  if(num->x == x)
	return;

  num->x = x;

  SendMessage( (HWND) GetWindowLongPtr(hwnd, GWLP_HWNDPARENT), 
	           WM_COMMAND,
			   MAKEWPARAM( GetWindowLong(hwnd, GWL_ID), NWN_CHANGE ),
			   0
			  );

}

static void SetVal(HWND hwnd, NUM *num, double x)
{
  char buff[64];

  if( num->delta && fmod(x, num->delta) )
	x = roundoff(x, num->delta);
  x = clamp(x, num->min, num->max);

  if(x == num->x)
	  return;

  num->x = x;
  sprintf(buff, "%g", x);
  SetWindowText(GetDlgItem(hwnd, ID_EDIT), buff);

  SendMessage( (HWND) GetWindowLongPtr(hwnd, GWLP_HWNDPARENT), 
	           WM_COMMAND,
			   MAKEWPARAM( GetWindowLong(hwnd, GWL_ID), NWN_CHANGE ),
			   0
			  );
}

static NUM *createnumber(void)
{
  NUM *num;

  num = malloc(sizeof(NUM));
  if(!num)
	  return 0;

  num->x = 0.0;
  num->max = DBL_MAX;
  num->min = -DBL_MAX;
  num->delta = 1.0;
  num->sigfigs = 0;

  return num;
}

static void killnumber(NUM *num)
{
  free(num);
}

static int stringvalid(NUM *num, const char *str)
{
  const char *ptr;
  int point = 0;

  if(*str == 0)
	return 1;
  
  ptr = str;

  if(*ptr == '-')
  {
    if(num->min >= 0.0)
	  return 0;
	else ptr++;
  }

  while(*ptr)
  {
	if(*ptr == '.')
    {
      if(num->delta == floor(num->delta))
		return 0;
      if(point++)
		return 0;
    }
    else if(!isdigit(*ptr))
	  return 0;
    ptr++;
  }

  return 1;
}

static double roundoff(double x, double delta)
{
  double err;
  
  err = fmod(x, delta);
  if(err > delta/2)
	return x + delta - err;
  return x - err;
}