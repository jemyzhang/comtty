#include <string.h>
#include "commonFunc.h"

char dlgofn[1024];
char dlgsfn[1024];

#if defined(linux)
#include <stdio.h>
#include <stdlib.h>

char *fopenDlg(char *title,char *filter, char *initaldir)
{
    char cmdstr[256];
    _init_array(dlgofn, sizeof(dlgofn));
    sprintf(cmdstr,
            "/usr/bin/zenity --title '%s' \\" \
            "--file-selection \\" \
            "--filename='%s'/ --file-filter='%s'",
            title, initaldir, filter);
    FILE *fp;
    fp = popen(cmdstr,"r");
    if(fp == NULL){
        return NULL;
    }
    while(fgets(dlgofn, sizeof(dlgofn)-1,fp) != NULL){

    }
    pclose(fp);

    return strlen(dlgofn) > 0 ? dlgofn : NULL;
}

char *fsaveDlg(char *title,char *filter, char *initaldir)
{
    char cmdstr[256];
    _init_array(dlgsfn, sizeof(dlgsfn));
    sprintf(cmdstr,
            "/usr/bin/zenity --title '%s' \\" \
            "--file-selection --save --confirm-overwrite \\" \
            "--filename='%s'/ --file-filter='%s'",
            title, initaldir, filter);
    FILE *fp;
    fp = popen(cmdstr,"r");
    if(fp == NULL){
        return NULL;
    }
    while(fgets(dlgsfn, sizeof(dlgsfn)-1,fp) != NULL){

    }
    pclose(fp);

    return strlen(dlgsfn) > 0 ? dlgsfn : NULL;
}
#endif

#if defined(__WINDOWS__)
#include <w32api/windows.h>
#include <w32api/commdlg.h>

char *fopenDlg(char *title,char *filter, char *initaldir)
{
    OPENFILENAME OpenFile;
    
    _init_array(dlgofn, sizeof(dlgofn));
    memset(&OpenFile,0,sizeof(OPENFILENAME));
    OpenFile.lStructSize = sizeof(OPENFILENAME);
    OpenFile.lpstrFile = dlgofn;//(char *)malloc(1024);
    OpenFile.nMaxFile = sizeof(dlgofn);
    OpenFile.lpstrFileTitle = OpenFile.lpstrFile ;
    OpenFile.nMaxFileTitle = OpenFile.nMaxFile;
    if(filter != NULL) OpenFile.lpstrFilter = filter; 
    if(initaldir != NULL) OpenFile.lpstrInitialDir = initaldir ;
    if(title != NULL) OpenFile.lpstrTitle = title ;

    if(GetOpenFileName(&OpenFile) == 1)
    {
        return(OpenFile.lpstrFile);
    }
    else
    {
        return NULL;
    }
}

char *fsaveDlg(char *title,char *filter, char *initaldir)
{
    OPENFILENAME SaveFile;

    _init_array(dlgsfn, sizeof(dlgsfn));
    memset(&SaveFile,0,sizeof(OPENFILENAME));
    SaveFile.lStructSize = sizeof(OPENFILENAME);
    SaveFile.lpstrFile = dlgsfn;//(char *)malloc(1024);
    SaveFile.nMaxFile = sizeof(dlgsfn);//1024-1;
    SaveFile.lpstrFileTitle = SaveFile.lpstrFile ;
    SaveFile.nMaxFileTitle = SaveFile.nMaxFile;
    if(filter != NULL) SaveFile.lpstrFilter = filter; 
    if(initaldir != NULL) SaveFile.lpstrInitialDir = initaldir ;
    if(title != NULL) SaveFile.lpstrTitle = title ;

    if(GetSaveFileName(&SaveFile) == 0)
    {
        return NULL;
    }
    else
    {
        return(SaveFile.lpstrFile);
    }
}
#endif
