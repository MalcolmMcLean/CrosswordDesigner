#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "crossword.h"
#include "undo.h"
#include "GridWin.h"


typedef struct
{
  int width;
  int height;
  CROSSWORD *cw;
  int selx;
  int sely;
  HFONT hsmallfont;
  int xpos;
  int ypos;
  int hasfocus;
} GRIDWIN;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void DoMouseLDown(HWND hwnd, GRIDWIN *gw, int x, int y);
static void MoveCursor(HWND hwnd, GRIDWIN *gw, int vk);
static void PaintMe(HWND hwnd, GRIDWIN *gw);
static int CellToRect(GRIDWIN *gw, RECT *r, int cx, int cy);
static int PixelToCell(GRIDWIN *gw, int x, int y, int *cx, int *cy);
static int IsArrowKey(int vk);

static void ScrollVMessage(HWND hwnd, GRIDWIN *gw, int msg, int val);
static void ScrollHMessage(HWND hwnd, GRIDWIN *gw, int msg, int val);
static void DoScrollBars(HWND hwnd, GRIDWIN *gw);


void RegisterGridWin(HINSTANCE hInstance)
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
  wndclass.hbrBackground = CreateSolidBrush( RGB(0, 0, 0) );// (HBRUSH) (COLOR_MENU + 1);
  wndclass.lpszMenuName = 0;
  wndclass.lpszClassName = "gridwin";
  wndclass.hIconSm = LoadIcon(0, IDI_APPLICATION); //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HEART));

  RegisterClassEx(&wndclass);

}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  GRIDWIN *gw;
  RECT rect;

  GetClientRect(hwnd, &rect);

  gw = (GRIDWIN *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch(msg)
  {
    case WM_CREATE:
	  gw = malloc(sizeof(GRIDWIN));
	  gw->cw = (CROSSWORD *) ((CREATESTRUCT *) lParam)->lpCreateParams;
	  gw->selx = -1;
	  gw->sely = -1;
	  gw->xpos = 0;
	  gw->ypos = 0;
	  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) gw);
	  gw->hsmallfont = CreateFont(8, 5, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial");
	  gw->hasfocus = 0;
	  DoScrollBars(hwnd, gw);
	  return 0;
	case WM_DESTROY:
	  DeleteObject(gw->hsmallfont);
	  free(gw);
	  return 0;
	case WM_PAINT:
	  PaintMe(hwnd, gw);
	  return 0;
	case WM_LBUTTONDOWN:
	  DoMouseLDown(hwnd, gw, LOWORD(lParam), HIWORD(lParam));
	  SetFocus(hwnd);
	  return 0;
	case WM_CHAR:
		if(gw->selx != -1 && gw->sely != -1)
		{
		  int ch = wParam;
		  if(isalpha(ch) || ch == ' ')
		  {
			undo_push(gw->cw);
		    crossword_setcell(gw->cw, gw->selx, gw->sely, toupper(ch) );
		    InvalidateRect(hwnd, 0, TRUE);
		    UpdateWindow(hwnd);
		  }
		  else
		  {
			undo_push(gw->cw);
			crossword_setcell(gw->cw, gw->selx, gw->sely, 0 );
		    InvalidateRect(hwnd, 0, TRUE);
		    UpdateWindow(hwnd);
		  }
		  SendMessage(GetParent(hwnd), WM_COMMAND, (WPARAM) GetWindowLong(hwnd, GWL_ID), 0); 
		}
		return 0;
    case WM_VSCROLL:
      ScrollVMessage(hwnd, gw, LOWORD(wParam), HIWORD(wParam));
	  break;
	case WM_HSCROLL:
	  ScrollHMessage(hwnd, gw, LOWORD(wParam), HIWORD(wParam));
	  break;
	case WM_KEYDOWN:
	  if(IsArrowKey(wParam))
	  {
	    MoveCursor(hwnd, gw, wParam);
		return 0;
	  }
	  break;
    case GW_SETCROSSWORD:
		if (gw->cw != (CROSSWORD*)lParam &&
			crossword_gridsidentical(gw->cw, (CROSSWORD*)lParam))
		{
			gw->cw = (CROSSWORD*)lParam;
	    }
		else
		{
			gw->cw = (CROSSWORD*)lParam;

			DoScrollBars(hwnd, gw);
			InvalidateRect(hwnd, 0, TRUE);
			UpdateWindow(hwnd);
		}
	  break;
	case WM_SETFOCUS:
		gw->hasfocus = 1;
		InvalidateRect(hwnd, 0, TRUE);
		UpdateWindow(hwnd);
		break;
	case WM_KILLFOCUS:
		gw->hasfocus = 0;
		InvalidateRect(hwnd, 0, TRUE);
		UpdateWindow(hwnd);
		break;

  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}



static void DoMouseLDown(HWND hwnd, GRIDWIN *gw, int x, int y)
{
	 int cx, cy;
     int err;

	 err = PixelToCell(gw, x, y, &cx, &cy);
	 if(err)
	   return;
	 gw->selx = cx;
	 gw->sely = cy;
	 InvalidateRect(hwnd, 0, TRUE);
	 UpdateWindow(hwnd);
}

static void MoveCursor(HWND hwnd, GRIDWIN *gw, int vk)
{
  RECT rect;

  CellToRect(gw, &rect, gw->selx, gw->sely);
  rect.left -= 2;
  rect.top -= 2;
  rect.right += 2;
  rect.bottom += 2;
  InvalidateRect(hwnd, &rect, TRUE);
  switch(vk)
  {
    case VK_UP:
	  if(gw->sely > 0)
		gw->sely--;
	  break;
    case VK_DOWN:
	  if(gw->sely < gw->cw->height-1)
		gw->sely++;
	  break;
	case VK_RIGHT:
		if(gw->selx < gw->cw->width -1)
		  gw->selx++;
	  break;
	case VK_LEFT:
	  if(gw->selx > 0)
		gw->selx--;
	  break;
  }
  CellToRect(gw, &rect, gw->selx, gw->sely);
  rect.left-=2;
  rect.top-=2;
  rect.right+=2;
  rect.bottom+=2;
  InvalidateRect(hwnd, &rect, TRUE);
  UpdateWindow(hwnd);
}

static void PaintMe(HWND hwnd, GRIDWIN *gw)
{
  RECT rect;
  int i;
  int ii;
  COLORREF col;
  HDC hdc;
  PAINTSTRUCT ps;
  char str[32];

  HBRUSH blackbrush;
  HBRUSH whitebrush;
  HPEN greenpen;

  hdc = BeginPaint(hwnd, &ps);

  GetClientRect(hwnd, &rect);

  blackbrush = CreateSolidBrush( RGB(0, 0, 0) );
  whitebrush = CreateSolidBrush( RGB(255, 255, 255));
  greenpen = CreatePen(PS_SOLID, 3, RGB(0, 128, 0));

  for(i=0;i<gw->cw->height;i++)
    for(ii=0;ii<gw->cw->width;ii++)
	{
		col = RGB(0, 255, 0);
        CellToRect(gw, &rect, ii, i);
		if(gw->cw->grid[i*gw->cw->width+ii] == 0)
		  FillRect(hdc, &rect, blackbrush);
	    else
		{
			FillRect(hdc, &rect, whitebrush);
			str[0] = gw->cw->solution[i*gw->cw->width+ii];
			str[1] = 0;
			TextOut(hdc, rect.left +5, rect.top+5, str, 1);
		}

	}
	
  SelectObject(hdc, gw->hsmallfont);
  SetBkMode(hdc, TRANSPARENT);
  for(i=0;i<gw->cw->height;i++)
    for(ii=0;ii<gw->cw->width;ii++)
	{
		CellToRect(gw, &rect, ii, i);
		if(gw->cw->numbers[i*gw->cw->width+ii] != 0)
		{
		  sprintf(str, "%d", gw->cw->numbers[i*gw->cw->width+ii]);
		  TextOut(hdc, rect.left+1, rect.top+2, str, strlen(str));
		}
	}

  GetClientRect(hwnd, &rect);
  SelectObject(hdc, GetStockObject(BLACK_PEN) );

  for(i=0;i<gw->cw->height;i++)
  {
	CellToRect(gw, &rect, 0, i);
	MoveToEx(hdc, 0, rect.top, 0);
	LineTo(hdc, gw->cw->width*24, rect.top); 
  }
  for(i=0;i<gw->cw->width;i++)
  {
	CellToRect(gw, &rect, i, 0);
	MoveToEx(hdc, rect.left, 0, 0);
	LineTo(hdc, rect.left, gw->cw->height * 24);
  }
  MoveToEx(hdc, 0, 0, 0);
  LineTo(hdc, rect.right-1, 0);
  LineTo(hdc, rect.right-1, rect.bottom-1);
  LineTo(hdc, 0, rect.bottom-1);
  LineTo(hdc, 0, 0);
 


  if(gw->hasfocus && gw->selx >= 0 && gw->sely >= 0)
  {
     CellToRect(gw, &rect, gw->selx, gw->sely);
	 SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	 SelectObject(hdc, greenpen);
	 Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
  }
  
  DeleteObject(blackbrush);
  DeleteObject(whitebrush);
  DeleteObject(greenpen);

  EndPaint(hwnd, &ps);
}

static int CellToRect(GRIDWIN *gw, RECT *r, int cx, int cy)
{
  r->left = cx * 24 - gw->xpos;
  r->right = r->left + 24;
  r->top = cy * 24 - gw->ypos;
  r->bottom = r->top + 24;

  return 0;
}

static int PixelToCell(GRIDWIN *gw, int x, int y, int *cx, int *cy)
{
/*
   if(x < 0 || x >= gw->width || y < 0 || y >= gw->height)
     return -1;
   *cx = (x * cw->width) / gw->width;
   *cy = (y * cw->height) / gw->height;
*/
	*cx = (x + gw->xpos) / 24;
	*cy = (y + gw->ypos)/ 24;
   return 0;
}

static int IsArrowKey(int vk)
{
  if(vk == VK_UP || vk == VK_DOWN || vk == VK_RIGHT || vk == VK_LEFT)
	return 1;
  return 0;
}

static void ScrollVMessage(HWND hwnd, GRIDWIN *gw, int msg, int val)
{
  RECT rect;

  GetClientRect(hwnd, &rect);

  switch(msg) 
  { 
    // User clicked the shaft above the scroll box. 
 
    case SB_PAGEUP: 
      gw->ypos -= (rect.bottom - rect.top); 
      break; 
 
    // User clicked the shaft below the scroll box. 
 
    case SB_PAGEDOWN: 
      gw->ypos += (rect.bottom - rect.top); 
      break; 
 
    // User clicked the top arrow. 
 
    case SB_LINEUP: 
      gw->ypos--; 
      break; 
 
   // User clicked the bottom arrow. 
 
    case SB_LINEDOWN: 
      gw->ypos++; 
      break; 
 
   // User dragged the scroll box. 
 
   case SB_THUMBTRACK: 
     gw->ypos = val; 
     break; 
  }
  
  if(gw->ypos > gw->cw->height * 24 - (rect.bottom - rect.top))
	gw->ypos = gw->cw->height *24 - (rect.bottom -rect.top);
  if(gw->ypos < 0)
	gw->ypos = 0;

  DoScrollBars(hwnd, gw);
  InvalidateRect(hwnd, 0, 0);
  UpdateWindow(hwnd);
}

static void ScrollHMessage(HWND hwnd, GRIDWIN *gw, int msg, int val)
{
  RECT rect;

  GetClientRect(hwnd, &rect);

  switch(msg) 
  { 
    // User clicked the shaft above the scroll box. 
 
    case SB_PAGELEFT: 
      gw->xpos -= (rect.right - rect.left); 
      break; 
 
    // User clicked the shaft below the scroll box. 
 
    case SB_PAGERIGHT: 
      gw->xpos += (rect.right - rect.left); 
      break; 
 
    // User clicked the top arrow. 
 
    case SB_LINELEFT: 
      gw->xpos--; 
      break; 
 
   // User clicked the bottom arrow. 
 
    case SB_LINERIGHT: 
      gw->xpos++; 
      break; 
 
   // User dragged the scroll box. 
 
   case SB_THUMBTRACK: 
     gw->xpos = val; 
     break; 
  }
  
  if(gw->xpos > gw->cw->width * 24 - (rect.right - rect.left))
	gw->xpos = gw->cw->width * 24 - (rect.right - rect.left);
  if(gw->xpos < 0)
	gw->xpos = 0;

  DoScrollBars(hwnd, gw);
  InvalidateRect(hwnd, 0, 0);
  UpdateWindow(hwnd);
}

static void DoScrollBars(HWND hwnd, GRIDWIN *gw)
{
	SCROLLINFO si; 
	RECT winrect;
	RECT clientrect;
  
    GetWindowRect(hwnd, &winrect);
	
	if (gw->cw->height * 24 <= winrect.bottom - winrect.top)
		ShowScrollBar(hwnd, SB_VERT, 0);
	else
	{
		ShowScrollBar(hwnd, SB_VERT, 1);
		GetClientRect(hwnd, &clientrect);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		si.nMin = 0;
		si.nMax = gw->cw->height * 24 - 1;
		si.nPage = (clientrect.bottom - clientrect.top);
		si.nPos = gw->ypos;

		SetScrollInfo(hwnd, SB_VERT, &si, 1);
	}

	if (gw->cw->height * 24 <= winrect.right - winrect.left)
		ShowScrollBar(hwnd, SB_HORZ, 0);
	else
	{
		ShowScrollBar(hwnd, SB_HORZ, 1);
		GetClientRect(hwnd, &clientrect);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		si.nMin = 0;
		si.nMax = gw->cw->width * 24 - 1;
		si.nPage = (clientrect.right - clientrect.left);
		si.nPos = gw->xpos;

		SetScrollInfo(hwnd, SB_HORZ, &si, 1);
	}
}