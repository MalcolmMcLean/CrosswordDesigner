#ifndef waitdialog_h
#define waitdialog_h

typedef struct
{
	HWND hwnd;
	int stopped;
	DWORD tick;
} WAITDIALOG;

void RegisterWaitDialog(HINSTANCE hInstance);
WAITDIALOG *WaitDialog(HWND hparent, char *message);
void KillWaitDialog(WAITDIALOG *wd);
void WaitDialog_Tick(WAITDIALOG *wd);

#endif
