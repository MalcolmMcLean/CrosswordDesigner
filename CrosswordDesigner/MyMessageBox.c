#include <Windows.h>

#define ID_OK_BUT 1
#define ID_CANCEL_BUT 2
#define ID_YES_BUT 3
#define ID_NO_BUT 4

typedef struct
{
	const char* message;
	const char* caption;
	int flags;
	int returnvalue;
} MESSAGEBOX;

static HFONT g_hcaptionfont;
static HFONT g_hmessagefont;

static void killmessagebox(MESSAGEBOX* mb);
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd, MESSAGEBOX* mb);
static void GetMessageBoxSize(HWND hparent, const char *caption, const char* message, int type, int* width, int* height);
static void GetMultiLineTextSize(HDC hdc, const char* text, SIZE* sz);

void RegisterMyMessageBox(HINSTANCE hInstance)
{
	NONCLIENTMETRICS metrics;
	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = 0;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = "MyMessageBox";
	wndclass.hIconSm = 0;

	RegisterClassEx(&wndclass);

	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	g_hmessagefont = CreateFontIndirect(&metrics.lfMessageFont);
	g_hcaptionfont = CreateFontIndirect(&metrics.lfCaptionFont);
}

int MyMessageBox(HWND hparent, const char* message, const char* caption, int flags)
{
	RECT rect;
	int x, y;
	MESSAGEBOX* mb = 0;
	MSG msg;
	int width, height;
	HWND hwnd = 0;
	int answer = IDOK;

	GetMessageBoxSize(hparent, caption, message, flags,  &width, &height);
	GetWindowRect(hparent, &rect);
	x = rect.left + (rect.right - rect.left - width) / 2;
	y = rect.top + (rect.bottom - rect.top - height) / 2;

	mb = malloc(sizeof(MESSAGEBOX));
	if (!mb)
		goto error_exit;
	mb->caption = caption;
	mb->message = message;
	mb->flags = flags;
	mb->returnvalue = IDOK;

	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"MyMessageBox",
		caption,
		WS_POPUP | WS_OVERLAPPED | WS_CAPTION,
		x,
		y,
		width,
		height,
		hparent,
		NULL,
		(HINSTANCE)GetWindowLongPtr(hparent, GWLP_HINSTANCE),
		mb
	);

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	while (IsWindow(hwnd))
	{
		GetMessage(&msg, 0, 0, 0);
		if (hwnd == msg.hwnd || IsChild(hwnd, msg.hwnd))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	answer = mb->returnvalue;
	killmessagebox(mb);
	return answer;

error_exit:
	killmessagebox(mb);
	return -1;
}

static void killmessagebox(MESSAGEBOX* mb)
{
	if (mb)
	{
		free(mb);
	}
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MESSAGEBOX *mb;

	mb = (MESSAGEBOX *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg)
	{
	case WM_CREATE:
		mb = (MESSAGEBOX *)((CREATESTRUCT*)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)mb);
		CreateControls(hwnd, mb);
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_PAINT:
		//PaintMe(hwnd);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_OK_BUT: // OK Button
			mb->returnvalue = IDOK;
			DestroyWindow(hwnd);
			return 0;
		case  ID_YES_BUT:
			mb->returnvalue = IDYES;
			DestroyWindow(hwnd);
			return 0;
		case ID_NO_BUT:
			mb->returnvalue = IDNO;
			DestroyWindow(hwnd);
			return 0;
		case ID_CANCEL_BUT:
			mb->returnvalue = IDCANCEL;
			DestroyWindow(hwnd);
			return 0;
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void CreateControls(HWND hwnd, MESSAGEBOX *mb)
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
		(HMENU)0,
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		0
	);
	SendMessage(hctl, WM_SETFONT, (WPARAM)g_hmessagefont, TRUE);

	SetWindowText(hctl, mb->message);


	if (mb->flags == MB_OK)
	{
		hctl = CreateWindowEx(
			0,
			"button",
			"OK",
			WS_CHILD | WS_VISIBLE,
			rect.right / 2 - 25,
			rect.bottom - 50,
			50,
			20,
			hwnd,
			(HMENU)ID_OK_BUT,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			0
		);
		SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);
	}
	else if (mb->flags == MB_YESNOCANCEL)
	{
		hctl = CreateWindowEx(
			0,
			"button",
			"Yes",
			WS_CHILD | WS_VISIBLE,
			10,
			rect.bottom - 50,
			50,
			20,
			hwnd,
			(HMENU)ID_YES_BUT,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			0
		);
		SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);

		hctl = CreateWindowEx(
			0,
			"button",
			"No",
			WS_CHILD | WS_VISIBLE,
			70,
			rect.bottom - 50,
			50,
			20,
			hwnd,
			(HMENU)ID_NO_BUT,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			0
		);
		SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);


		hctl = CreateWindowEx(
			0,
			"button",
			"Cancel",
			WS_CHILD | WS_VISIBLE,
			130,
			rect.bottom - 50,
			80,
			20,
			hwnd,
			(HMENU)ID_CANCEL_BUT,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			0
		);
		SendMessage(hctl, WM_SETFONT, (WPARAM)g_hcaptionfont, TRUE);
	}

}

static void GetMessageBoxSize(HWND hparent, const char *caption, const char* message, int type, int* width, int* height)
{
	HDC hdc = GetDC(hparent);
	SIZE sz;
	int w, h;
	int minimumwidth;

	if (type == MB_OK)
		minimumwidth = 250;
	else if (type == MB_YESNOCANCEL)
		minimumwidth = 300;

	SelectObject(hdc, g_hmessagefont);
	GetTextExtentPoint32(hdc, message, strlen(caption), &sz);
	w = sz.cx + 50;
	GetMultiLineTextSize(hdc, message, &sz);

	h = sz.cy + 150;
	if (w < sz.cx + 50)
		w = sz.cx + 50;
	if (w < minimumwidth)
		w = minimumwidth;
	if (width)
		*width = w;
	if (height)
		*height = h;
	ReleaseDC(hparent, hdc);
}

static void GetMultiLineTextSize(HDC hdc, const char* text, SIZE* sz)
{
	SIZE size;
	const char* ptr = text;
	char* newline = strchr(text, '\n');
	int nlines = 0;

	sz->cx = 0;
	sz->cy = 0;
	while (newline)
	{
		GetTextExtentPoint32(hdc, ptr, newline - ptr, &size);
		if (sz->cx < size.cx)
			sz->cx = size.cx;
		nlines++;
		ptr = newline + 1;
		newline = strchr(ptr, '\n');
	 }
	nlines++;
	GetTextExtentPoint32(hdc, ptr, strlen(ptr), &size);
	if (sz->cx < size.cx)
		sz->cx = size.cx;
	GetTextExtentPoint32(hdc, "Ly", 2, &size);
	sz->cy = nlines * size.cy;
}