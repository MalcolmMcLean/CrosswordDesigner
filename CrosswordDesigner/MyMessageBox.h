#ifndef mymessagebox_h
#define mymessagebox_h

void RegisterMyMessageBox(HINSTANCE hInstance);
int MyMessageBox(HWND hParent, const char* message, const char* caption, int flags);

#endif
