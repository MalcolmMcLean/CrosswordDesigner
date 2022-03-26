#include <windows.h>

#include <stdlib.h>
#include <string.h>

#include "OpenFileDlg.h"

static char *mystrdup(char *str);


char *GetCrosswordFile(HWND hwnd)
{
   OPENFILENAME ofn;
   char path[MAX_PATH];
   
   strcpy(path, " ");

   ofn.lStructSize = sizeof(OPENFILENAME); 
   ofn.hwndOwner = hwnd; 
   ofn.hInstance = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE); 
   ofn.lpstrFilter = "Crossword files\0*.xpf;*.xml;\0Puzzle files\0*.puz\0IPuzzle files\0*.ipuz\0All files\0*.*\0\0"; 
   ofn.lpstrCustomFilter = 0; 
   ofn.nMaxCustFilter = 0; 
   ofn.nFilterIndex = 0; 
   ofn.lpstrFile = path; 
   ofn.nMaxFile = MAX_PATH; 
   ofn.lpstrFileTitle = 0; 
   ofn.nMaxFileTitle = 0; 
   ofn.lpstrInitialDir = 0; 
   ofn.lpstrTitle = "Load Crossword File"; 
   ofn.Flags = OFN_FILEMUSTEXIST; 
   ofn.nFileOffset = 0; 
   ofn.nFileExtension = 0; 
   ofn.lpstrDefExt= ""; 
   ofn.lCustData = 0; 
   ofn.lpfnHook = 0; 
   ofn.lpTemplateName = 0;
   
   if( GetOpenFileName(&ofn) )
   {
     return mystrdup(path);
   }
   else
	 return 0;
} 

char *GetCrosswordSaveFile(HWND hwnd)
{
   OPENFILENAME ofn;
   char path[MAX_PATH];

   strcpy(path, "");
   
   ofn.lStructSize = sizeof(OPENFILENAME); 
   ofn.hwndOwner = hwnd; 
   ofn.hInstance = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE); 
   ofn.lpstrFilter = "Crossword files\0*.xpf;*.xml\0XPF files\0*.xpf\0Puzzle files\0*.puz\0IPuzzle files\0*.ipuz\0All files\0*.*\0\0"; 
   ofn.lpstrCustomFilter = 0; 
   ofn.nMaxCustFilter = 0; 
   ofn.nFilterIndex = 0; 
   ofn.lpstrFile = path; 
   ofn.nMaxFile = MAX_PATH; 
   ofn.lpstrFileTitle = 0; 
   ofn.nMaxFileTitle = 0; 
   ofn.lpstrInitialDir = 0; 
   ofn.lpstrTitle = "Save Crossword file"; 
   ofn.Flags = OFN_NOREADONLYRETURN; 
   ofn.nFileOffset = 0; 
   ofn.nFileExtension = 0; 
   ofn.lpstrDefExt= ""; 
   ofn.lCustData = 0; 
   ofn.lpfnHook = 0; 
   ofn.lpTemplateName = 0;
   
   if( GetSaveFileName(&ofn) )
     return mystrdup(path);
   else
	 return 0;
} 

char *GetHTMLSaveFile(HWND hwnd)
{
   OPENFILENAME ofn;
   char path[MAX_PATH];

   strcpy(path, "");
   
   ofn.lStructSize = sizeof(OPENFILENAME); 
   ofn.hwndOwner = hwnd; 
   ofn.hInstance = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE); 
   ofn.lpstrFilter = "HTML files\0*.html;All files\0*.*\0\0"; 
   ofn.lpstrCustomFilter = 0; 
   ofn.nMaxCustFilter = 0; 
   ofn.nFilterIndex = 0; 
   ofn.lpstrFile = path; 
   ofn.nMaxFile = MAX_PATH; 
   ofn.lpstrFileTitle = 0; 
   ofn.nMaxFileTitle = 0; 
   ofn.lpstrInitialDir = 0; 
   ofn.lpstrTitle = "Save Crossword as HTML"; 
   ofn.Flags = OFN_NOREADONLYRETURN; 
   ofn.nFileOffset = 0; 
   ofn.nFileExtension = 0; 
   ofn.lpstrDefExt= ""; 
   ofn.lCustData = 0; 
   ofn.lpfnHook = 0; 
   ofn.lpTemplateName = 0;
   
   if( GetSaveFileName(&ofn) )
     return mystrdup(path);
   else
	 return 0;
} 
char *GetWordListFile(HWND hwnd)
{
   OPENFILENAME ofn;
   char path[MAX_PATH];
   
   strcpy(path, " ");

   ofn.lStructSize = sizeof(OPENFILENAME); 
   ofn.hwndOwner = hwnd; 
   ofn.hInstance = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE); 
   ofn.lpstrFilter = "Word list files\0*.txt;*.xml;\0All files\0*.*\0\0"; 
   ofn.lpstrCustomFilter = 0; 
   ofn.nMaxCustFilter = 0; 
   ofn.nFilterIndex = 0; 
   ofn.lpstrFile = path; 
   ofn.nMaxFile = MAX_PATH; 
   ofn.lpstrFileTitle = 0; 
   ofn.nMaxFileTitle = 0; 
   ofn.lpstrInitialDir = 0; 
   ofn.lpstrTitle = "Load Word List File"; 
   ofn.Flags = OFN_FILEMUSTEXIST; 
   ofn.nFileOffset = 0; 
   ofn.nFileExtension = 0; 
   ofn.lpstrDefExt= ""; 
   ofn.lCustData = 0; 
   ofn.lpfnHook = 0; 
   ofn.lpTemplateName = 0;
   
   if( GetOpenFileName(&ofn) )
   {
     return mystrdup(path);
   }
   else
	 return 0;
} 

static char *mystrdup(char *str)
{
  char *answer;

  answer = malloc(strlen(str) + 1);
  if(answer)
    strcpy(answer, str);
  return answer;
}