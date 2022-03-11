#ifndef genwordlistdialog_h
#define genwordlistdialog_h

#define CANCEL_ALGORITHM 0
#define JIGSAW_ALGORITHM 1
#define ANNEALING_ALGORITHM 2

void RegisterGenWordListDialog(HINSTANCE hInstance);
int OpenGenWordListDialog(HWND hparent);

#endif

