#include <stdlib.h>
#include <windows.h>

#define ID_ANIMATION_BAR 2
#define ID_STOP_BUT 1

typedef struct
{
	HWND hwnd;
	int stopped;
	DWORD tick;
} WAITDIALOG;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd);
static void PaintMe(HWND hwnd, WAITDIALOG *wd);

void RegisterWaitDialog(HINSTANCE hInstance)
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
	wndclass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = "WaitDialog";
	wndclass.hIconSm = 0;

	RegisterClassEx(&wndclass);
}

WAITDIALOG *WaitDialog(HWND hparent, char *message)
{
	WAITDIALOG *wd;
	RECT rect;
	int x, y;

	GetClientRect(hparent, &rect);
	x = rect.left + (rect.right - rect.left - 300) / 2;
	y = rect.top + (rect.bottom - rect.top - 300) / 2;

	wd = malloc(sizeof(WAITDIALOG));
	wd->stopped = 0;
	wd->tick = GetTickCount();
	wd->hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"WaitDialog",
		message,
		WS_POPUP | WS_OVERLAPPED | WS_CAPTION,
		x, 
		y,
		300, 
		300, 
		hparent,
		NULL,
		(HINSTANCE)GetWindowLongPtr(hparent, GWLP_HINSTANCE),
		wd
		);

	ShowWindow(wd->hwnd, SW_SHOWNORMAL);
	UpdateWindow(wd->hwnd);

	return wd;
}

void KillWaitDialog(WAITDIALOG *wd)
{
	DestroyWindow(wd->hwnd);
	free(wd);
}

void WaitDialog_Tick(WAITDIALOG *wd)
{
	MSG msg;
	DWORD tock;
	RECT rect;

	if (IsWindow(wd->hwnd))
	{
		while (PeekMessage(&msg, 0, 0, 0, 1))
		{
			if (msg.hwnd == wd->hwnd || IsChild(wd->hwnd, msg.hwnd))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		tock = GetTickCount();
		if (abs(tock - wd->tick) > 20)
		{
			rect.top = 50;
			rect.bottom = 70;
			rect.left = 0;
			rect.right = 300;
			wd->tick = tock;
			InvalidateRect(wd->hwnd, &rect, TRUE);
			UpdateWindow(wd->hwnd);
		}
	}

}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WAITDIALOG *wd;

	wd = (WAITDIALOG *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg)
	{
	case WM_CREATE:
		wd = (WAITDIALOG *)((CREATESTRUCT *)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)wd);
		CreateControls(hwnd);
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_PAINT:
		PaintMe(hwnd, wd);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_STOP_BUT: 
			wd->stopped = 1;
			return 0;
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void CreateControls(HWND hwnd)
{
	RECT rect;
	HWND hctl;

	GetClientRect(hwnd, &rect);
	hctl = CreateWindowEx(
		0,
		"button",
		"STOP",
		WS_CHILD | WS_VISIBLE,
		rect.right / 2 - 30,
		rect.bottom - 50,
		60,
		25,
		hwnd,
		(HMENU)ID_STOP_BUT,
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		0
		);

}

static void PaintMe(HWND hwnd, WAITDIALOG *wd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HBRUSH hbrush;
	int x;

	hdc = BeginPaint(hwnd, &ps);
	hbrush = CreateSolidBrush(RGB(200, 30, 30));
	SelectObject(hdc, hbrush);
	x = (wd->tick / 30) % 280;
	Ellipse(hdc, x, 50, x + 20, 70);
	DeleteObject(hbrush);
	EndPaint(hwnd, &ps);
}