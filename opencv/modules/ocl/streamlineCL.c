#ifdef __linux
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

#define LEN 1024

char src[LEN];
char dst[LEN];

int callBack(const char *file, const struct stat *sb, int flag)
{
    int len = strlen(file);
    FILE *fp;
    if(len > 3 && (*(file + len - 3) == '.')
            && ( (*(file + len - 2) == 'c') || (*(file + len - 2) == 'C') )
            && ( (*(file + len - 1) == 'l') || (*(file + len - 1) == 'L') ))
    {
        int i, j;
        char *s;
        strcpy(src, "const char *");
        for(i = len - 1; i > 0; i--)
            if(*(file + i) == '/')
                break;
        if(i <= 0)
            s = (char *)file;
        else
            s = (char *)&file[i+1];
        j = 0;
        while(i < len - 4)
            dst[j++] = file[(i++)+1];
        fp = fopen(file, "r");
        if(!fp)
            return -1;
        fprintf(stdout, "%s\n", strcat(strcat(src, dst), " ="));
        while(!feof(fp))
        {
            int i, len;
            memset(src, 0, sizeof(src));
            memset(dst, 0, sizeof(dst));
            fgets(src, LEN, fp);
            len = strlen(src);
            if(len >= LEN - 3)
                return -1;
            else if(len > 1)
            {
                dst[0] = '\"';
                memcpy(&dst[1], src, len);
                dst[len] = '\\';
                dst[len+1] = 'n';
                dst[len+2] = '\"';
                dst[len+3] = 0;
                fprintf(stdout, "%s\n", dst);
            }
        }
        fprintf(stdout, ";\n");
        fclose(fp);
    }
    return 0;
}

int main()
{
    fprintf(stdout, "namespace cv { namespace ocl {\n");
    ftw("./src/kernel/", callBack, 16);
    fprintf(stdout, "}}\n");
}
#else
// streamlineCL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>

#define LEN 1024

_TCHAR src[LEN];
_TCHAR dst[LEN];

int CL2char(_TCHAR *file, FILE *out)
{
    int len = _tcslen(file);
    FILE *fp;
    if(len > 3 && (*(file + len - 3) == _TEXT('.'))
            && ( (*(file + len - 2) == _TEXT('c')) || (*(file + len - 2) == _TEXT('C')) )
            && ( (*(file + len - 1) == _TEXT('l')) || (*(file + len - 1) == _TEXT('L')) ))
    {
        int i, j;
        _TCHAR *s;
        _tcscpy_s(src, _TEXT("const char *"));
        for(i = len - 1; i > 0; i--)
            if(*(file + i) == _TEXT('/') || *(file + i) == _TEXT('\\'))
                break;
        if(i <= 0)
            s = (_TCHAR *)file;
        else
            s = (_TCHAR *)&file[i+1];
        j = 0;
        while(i < len - 4)
            dst[j++] = file[(i++)+1];
        _tfopen_s(&fp, file, _TEXT("r"));
        if(!fp)
            return -1;
        _ftprintf(out, _TEXT("%s\n"), _tcscat(_tcscat(src, dst), _TEXT(" =")));
        while(!feof(fp))
        {
            int len;
            memset(src, 0, sizeof(src));
            memset(dst, 0, sizeof(dst));
            _fgetts(src, LEN, fp);
            len = _tcslen(src);
            if(len >= LEN - 3)
                return -1;
            else if(len > 1)
            {
                dst[0] = _TEXT('\"');
                _tcscpy(&dst[1], src);
                dst[len] = _TEXT('\\');
                dst[len+1] = _TEXT('n');
                dst[len+2] = _TEXT('\"');
                dst[len+3] = 0;
                _ftprintf(out, _TEXT("%s\n"), dst);
            }
        }
        _ftprintf(out, _TEXT(";\n"));
        fclose(fp);
    }
    return 0;
}

//use ansi
int _tmain(int argc, _TCHAR *argv[])
{
    _TCHAR szPath[MAX_PATH];
    _TCHAR dst[MAX_PATH];
    if(argc == 2)
    {
        szPath = argv[1];
    }
    else
    {
        printf("Directory Path missing!");
        return -1;
    };


    //if( !GetCurrentDirectory( MAX_PATH, szPath  ) )
    //{
    //	printf("GetModuleFileName failed (%d)\n", GetLastError());
    //return -1;
    //}

    int l = _tcslen(szPath);
    _tcscpy_s(dst, szPath);
    _tcscpy(dst + l, _TEXT("\\src\\kernels.cpp"));
    _tcscpy(szPath + l, _TEXT("\\src\\kernel\\*.*"));
    l = _tcslen(szPath);
    WIN32_FIND_DATA wfd;
    HANDLE filehandle = FindFirstFile(szPath, &wfd);
    if(filehandle == INVALID_HANDLE_VALUE)
    {
        _tprintf(_TEXT("FindFirstFile failed\n"));
        return -1;
    }
    FILE *out;
    _tfopen_s(&out, dst, _TEXT("w"));
    fprintf(out, "namespace cv { namespace ocl {\n");
    do
    {
        if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            _tcscpy(szPath + l - 3, wfd.cFileName);
            CL2char(szPath, out);
        }
    }
    while(FindNextFile(filehandle, &wfd));
    FindClose(filehandle);
    fprintf(out, "}}\n");
    fclose(out);
    return 0;
}
#endif
