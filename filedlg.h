#ifndef _FILEDLG_H
#define _FILEDLG_H
#include <w32api/windows.h>
#include <w32api/commdlg.h>
#include "commonFunc.h"

char *fopenDlg(char *title,char *filter, char *initaldir);
char *fsaveDlg(char *title,char *filter, char *initaldir);

#endif /*_FILEDLG_H*/
