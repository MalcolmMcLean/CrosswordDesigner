#include <Windows.h>
#include <Winspool.h>
#include <CommDlg.h>

#include <stdio.h>
#include <ctype.h>

#include "crossword.h"
#include "PrintCrossword.h"


HDC GetPrinterDC (HWND Hwnd);


static int GetClueHeight(HDC hdc, char *clue, int no, int width, HFONT nofont, HFONT cluefont);
static void PrintClue(HDC hdc, char *clue, int no, int x, int y, int width, HFONT nofont, HFONT cluefont);

static char *getfirstspace(char *str);

int PrintCrossword(HWND hwnd, CROSSWORD *cw)
{
	HDC prn;
    int cxpage, cypage;
	HFONT DisplayFont;
	HFONT NumberFont;
	int i;
	int x, y;

	static DOCINFO di = { sizeof (DOCINFO), TEXT ("CROSSWORD : Printing...")};

  	prn = GetPrinterDC(hwnd); 

 
    cxpage = GetDeviceCaps (prn, HORZRES);
    cypage = GetDeviceCaps (prn, VERTRES);
 
    StartDoc (prn, &di);
 
    StartPage (prn) ;
 
    SetMapMode (prn, MM_ISOTROPIC);
    SetWindowExtEx(prn, 1500,1500, NULL);
    SetViewportExtEx(prn, cxpage/2, cypage/2, NULL);
    SetViewportOrgEx(prn, 0, 0, NULL);
 

    Rectangle (prn,15,15,cxpage,cypage);
    Rectangle(prn,50,50,cxpage/2-50,cypage/2-50);
    MoveToEx (prn,50,500,NULL);
    LineTo(prn, cxpage-50,500);
            
	DisplayFont = CreateFont (166, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, "Arial Rounded MT Bold");
    SelectObject (prn, DisplayFont);

	if(cw->title)
      TextOut (prn, 300,200,cw->title, strlen(cw->title));

	DrawGrid(prn, cw, 100, 550, cw->width * 50, cw->height * 50);

	DisplayFont = CreateFont (40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, "Arial Rounded MT Bold");

	NumberFont = CreateFont (40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, "Arial Rounded MT Bold");
    
	
	
	SelectObject (prn, NumberFont);
	y = 600 + cw->height * 50 + 130;
	x = 100;
    TextOut(prn, x, y, "Across", 6);

	y = 600 + cw->height * 50 + 200;
	x = 100;
	for(i=0;i<cw->Nacross;i++)
	{
		int ht;
		char *clue;

		clue = cw->cluesacross[i];

		if(!clue)
		  clue = "A clue";

		ht = GetClueHeight(prn, clue, cw->numbersacross[i], cxpage/4 - 100, NumberFont, DisplayFont);
		if(y + ht > cypage/2 - 100)
		{
		  y = 600 + cw->height * 50 + 200;
	      x = cxpage/4 + 50;
		}
		PrintClue(prn, clue, cw->numbersacross[i], x, y, cxpage/4 - 100, NumberFont, DisplayFont);
		y += ht;
	}

	if(x == 100)
	{
	   y = 600 + cw->height * 50 + 130;
	   x = cxpage/4 + 50;
	}

	SelectObject (prn, NumberFont);
    TextOut(prn, x, y, "Down", 6);
	y += 70;

	for(i=0;i<cw->Ndown;i++)
	{
		int ht;
		char *clue;

		clue = cw->cluesdown[i];

		if(!clue)
		  clue = "A clue";

		ht = GetClueHeight(prn, clue, cw->numbersdown[i], cxpage/4 - 100, NumberFont, DisplayFont);
		if(y + ht > cypage/2 - 100)
		{
		}
		PrintClue(prn, clue, cw->numbersdown[i], x, y, cxpage/4 - 100, NumberFont, DisplayFont);
		y += ht;
	}

 
    EndPage (prn);
    EndDoc(prn);
    DeleteDC(prn);

	return 0;

}

static int GetClueHeight(HDC hdc, char *clue, int no, int width, HFONT nofont, HFONT cluefont)
{
  char numstr[32];
  SIZE size;
  int firstx;
  char *spaceptr;
  char *lastspace;
  char *base;
  int answer = 0;

  SelectObject(hdc, nofont);
  sprintf(numstr, "%d. ", no);
  GetTextExtentPoint32(hdc, numstr, strlen(numstr), &size);
  firstx = size.cx;

  SelectObject(hdc, cluefont);
  GetTextExtentPoint(hdc, clue, strlen(clue), &size);
  if(size.cx + firstx < width)
    return size.cy;

  spaceptr = clue;
  do
  {
	lastspace = spaceptr;
    spaceptr = getfirstspace(lastspace+1);
	GetTextExtentPoint32(hdc, clue, spaceptr-clue, &size);
  } while(*spaceptr && size.cx + firstx < width);

  if(lastspace == clue)
  {
  }
  answer += size.cy;

  do
  {
    base = lastspace + 1;
    while(isspace((unsigned) *base))
      base++;
	spaceptr = base;
	do
	{
	  lastspace = spaceptr;
	  spaceptr = getfirstspace(lastspace+1);
	  GetTextExtentPoint32(hdc, clue, spaceptr-base, &size);
	} while(*spaceptr && size.cx < width);

	if(lastspace == base)
	{
	}
	answer += size.cy;
  } while(*spaceptr);

  return answer;
}

static void PrintClue(HDC hdc, char *clue, int no, int x, int y, int width, HFONT nofont, HFONT cluefont)
{
	char numstr[32];
  SIZE size;
  int firstx;
  char *spaceptr;
  char *lastspace;
  char *base;
  int answer = 0;

  SelectObject(hdc, nofont);
  sprintf(numstr, "%d. ", no);
  GetTextExtentPoint32(hdc, numstr, strlen(numstr), &size);
  firstx = size.cx;
  TextOut(hdc, x, y, numstr, strlen(numstr));

  SelectObject(hdc, cluefont);
  GetTextExtentPoint32(hdc, clue, strlen(clue), &size);
  if(size.cx + firstx < width)
  {
    TextOut(hdc, x + firstx, y, clue, strlen(clue));
    return;
  }

  spaceptr = clue;
  do
  {
	lastspace = spaceptr;
    spaceptr = getfirstspace(lastspace+1);
	GetTextExtentPoint32(hdc, clue, spaceptr-clue, &size);
  } while(*spaceptr && size.cx + firstx < width);

  if(lastspace == clue)
  {
  }
  TextOut(hdc, x+firstx, y, clue, lastspace - clue); 
  y += size.cy;

  do
  {
    base = lastspace + 1;
    while(isspace((unsigned) *base))
      base++;
	spaceptr = base;
	do
	{
	  lastspace = spaceptr;
	  spaceptr = getfirstspace(lastspace+1);
	  GetTextExtentPoint32(hdc, clue, spaceptr-base, &size);
	} while(*spaceptr && size.cx < width);

	if(*spaceptr == 0)
	  lastspace = spaceptr;

	if(lastspace == base)
	{
	}
	TextOut(hdc, x, y, base, lastspace - base); 
	y += size.cy;
  } while(*spaceptr);

}

void DrawGrid(HDC hdc, CROSSWORD *cw, int x, int y, int width, int height)
{
  int i, ii;
  int cx, cy, cx2, cy2;
  int fonth, fontw;
  char str[32];
  HFONT hfont;

  SelectObject(hdc, GetStockObject(BLACK_PEN));
  SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
  Rectangle(hdc, x, y, x+width, y+height);
  SelectObject(hdc, GetStockObject(BLACK_BRUSH));

  fonth = height/(cw->height * 2);
  if(fonth < 5)
    fonth = 5;
  fontw = (fonth * 2)/3;
  if(fontw < 5)
    fontw = 5;

  hfont = CreateFont(fonth, fontw, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, "Arial");

  SelectObject(hdc, hfont);
  SetBkMode(hdc, TRANSPARENT);

  for(i=0;i<cw->height;i++)
	  for(ii=0;ii<cw->width;ii++)
	  {
	    cx = (ii * width)/cw->width + x;
		cx2 =((ii+1) * width)/cw->width + x;
		cy = (i * height)/cw->height + y;
		cy2 =((i+1) * height)/cw->height + y;
		if(cw->grid[i*cw->width+ii] == 0)
			  Rectangle(hdc, cx, cy, cx2+1, cy2+1);
		else
		{
			if(cw->numbers[i*cw->width+ii])
			{
			  sprintf(str, "%d", cw->numbers[i*cw->width+ii]);
			  TextOut(hdc, cx, cy, str, strlen(str));
			}
		}
	  }
  for(i=0;i<cw->height;i++)
  {
    cy = (i * height)/cw->height + y;
	MoveToEx(hdc, x, cy, 0);
	LineTo(hdc, x+width, cy);
  }
  for(i=0;i<cw->width;i++)
  {
	  cx = (i * width)/cw->width + x;
	  MoveToEx(hdc, cx, y, 0);
	  LineTo(hdc, cx, y + height);
  }
}

void DrawSolution(HDC hdc, CROSSWORD *cw, int x, int y, int width, int height)
{
  int i, ii;
  int cx, cy, cx2, cy2;
  int fonth, fontw;
  int xoffset, yoffset;
  char str[32];
  HFONT hfont;

  SelectObject(hdc, GetStockObject(BLACK_PEN));
  SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
  Rectangle(hdc, x, y, x+width, y+height);
  SelectObject(hdc, GetStockObject(BLACK_BRUSH));

  fonth = height / cw->height - height/(cw->height * 5);
  if(fonth < 5)
    fonth = 5;
  fontw = (fonth * 2)/4;
  if(fontw < 5)
    fontw = 5;

  xoffset = 2;
  yoffset = (height/cw->height - fonth)/2; 

  hfont = CreateFont(fonth, fontw, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, "Arial");

  SelectObject(hdc, hfont);
  SetBkMode(hdc, TRANSPARENT);

  for(i=0;i<cw->height;i++)
	  for(ii=0;ii<cw->width;ii++)
	  {
	    cx = (ii * width)/cw->width + x;
		cx2 =((ii+1) * width)/cw->width + x;
		cy = (i * height)/cw->height + y;
		cy2 =((i+1) * height)/cw->height + y;
		if(cw->grid[i*cw->width+ii] == 0)
			  Rectangle(hdc, cx, cy, cx2+1, cy2+1);
		else
		{
			if(cw->grid[i*cw->width+ii])
			{
				SIZE sz;
				char charstr[2] = { 0 };
				charstr[0] = cw->solution[i * cw->width + ii];
				GetTextExtentPoint(hdc, charstr, 1, &sz);
				xoffset = (width / cw->width - sz.cx) / 2;
				sprintf(str, "%c", cw->solution[i*cw->width+ii]);
				TextOut(hdc, cx + xoffset, cy + yoffset, str, strlen(str));
			}
		}
	  }
  for(i=0;i<cw->height;i++)
  {
    cy = (i * height)/cw->height + y;
	MoveToEx(hdc, x, cy, 0);
	LineTo(hdc, x+width, cy);
  }
  for(i=0;i<cw->width;i++)
  {
	  cx = (i * width)/cw->width + x;
	  MoveToEx(hdc, cx, y, 0);
	  LineTo(hdc, cx, y + height);
  }
}


HDC GetPrinterDC (HWND Hwnd)
{
	HDC hdc;
// Initialize a PRINTDLG structure's size and set the PD_RETURNDC flag set the Owner flag to hwnd.
// The PD_RETURNDC flag tells the dialog to return a printer device context.
    PRINTDLG pd = {0};
    pd.lStructSize = sizeof( pd );
    pd.hwndOwner = Hwnd;
    pd.Flags = PD_RETURNDC;

 
// Retrieves the printer DC
    PrintDlg(&pd);
    hdc = pd.hDC;
    return hdc;
}

static char *getfirstspace(char *str)
{
  char *answer = str;
  while(*answer)
    if(isspace( (unsigned) *answer))
	  return answer;
	else
	  answer++;
  return answer;
}
