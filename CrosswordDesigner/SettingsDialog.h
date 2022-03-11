#ifndef settingsdialog_h
#define settingsdialog_h

typedef struct
{
  char *copyright;
  char *publisher;
  char *author;
  char *editor;
  char *date;
  char *title;
} SETTINGS;

void RegisterSettingsDialog(HINSTANCE hInstance);
int OpenSettingsDialog(HWND hparent, SETTINGS *set);

#endif