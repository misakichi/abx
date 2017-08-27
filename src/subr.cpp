/**
 *  @file   subr.cpp
 *  @brief  �T�u���[�`��
 *  @author Masashi KITAMURA (tenka@6809.net)
 *  @note
 *      Boost Software License Version 1.0
 */
#include "subr.hpp"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <io.h>



/*---------------------------------------------------------------------------*/

char const* StrSkipSpc(char const* s)
{
    while (*s && *(unsigned char *)s <= ' ')
        s++;
    return s;
}

char const* StrSkipNotSpc(char const* s)
{
    while (*(unsigned char *)s > ' ')
        s++;
    return s;
}


char *StrLwrN(char* str, size_t size)
{
    unsigned char* s = (unsigned char*)str;
    unsigned char* e = s + size;
    while (s < e) {
        unsigned c = *s;
        if ('A' <= c && c <= 'Z') {
            c += 'a' - 'A';
            *s = c;
        }
        ++s;
    }
    return str;
}


char *FIL_DelLastDirSep(char *dir)
{
    if (dir) {
        char *p = FIL_BaseName(dir);
        if (strlen(p) > 1) {
            p = STREND(dir);
            if (p[-1] == '\\' || p[-1] == '/')
                p[-1] = 0;
        }
    }
    return dir;
}



/*---------------------------------------------------------------------------*/

/* �Ƃ肠�����A�A�Z���u���\�[�X�Ƃ̌��ˍ����ŁA�_�~�[�֐���p�� */
static int  FIL_zenFlg = 1;         /* 1:MS�S�p�ɑΉ� 0:���Ή� */


void    FIL_SetZenMode(int ff)
{
    FIL_zenFlg = ff;
}

int FIL_GetZenMode(void)
{
    return FIL_zenFlg;
}


char *FIL_BaseName(char const* adr)
{
    char const* p;

    p = adr;
    while (*p != '\0') {
        if (*p == ':' || *p == '/' || *p == '\\')
            adr = p + 1;
        if (FIL_zenFlg && ISKANJI((*(unsigned char *)p)) && *(p+1) )
            p++;
        p++;
    }
    return (char*)adr;
}


char *FIL_ChgExt(char filename[], char const* ext)
{
    char *p;

    p = FIL_BaseName(filename);
    p = strrchr( p, '.');
    if (p == NULL) {
        if (ext) {
            strcat(filename,".");
            strcat( filename, ext);
        }
    } else {
        if (ext == NULL)
            *p = 0;
        else
            strcpy(p+1, ext);
    }
    return filename;
}


char *FIL_AddExt(char filename[], char const* ext)
{
    if (ext) {
        if (strrchr(FIL_BaseName(filename), '.') == NULL) {
            strcat(filename,".");
            strcat(filename, ext);
        }
    }
    return filename;
}


char *FIL_NameUpr(char *s0)
{
    /* �S�p�Q�o�C�g�ڂ��l������ strupr */
    char *s = s0;
    while (*s) {
        if (FIL_zenFlg && ISKANJI(*s) && s[1]) {
            s += 2;
        } else if (ISLOWER(*s)) {
            *s = TOUPPER(*s);
            s++;
        } else {
            s++;
        }
    }
    return s0;
}


int FIL_FdateCmp(const char *tgt, const char *src)
{
    // ��̃t�@�C���̓��t�̑召���r����.
    // tgt ���V������� 1(��), �����Ȃ�� 0, tgt ���Â���� -1(��)
    struct _finddatai64_t srcData;
    struct _finddatai64_t tgtData;
    intptr_t srcFindHdl, tgtFindHdl;
    time_t srcTm, tgtTm;

    srcFindHdl = _findfirsti64((char *)src, &srcData);
    srcTm = (srcFindHdl == -1) ? 0 : srcData.time_write;

    tgtFindHdl = _findfirsti64((char *)tgt, &tgtData);
    if (tgtFindHdl == -1)
        return -1;
    tgtTm = (tgtFindHdl == -1) ? 0 : tgtData.time_write;

    if (tgtTm < srcTm)
        return -1;
    else if (tgtTm > srcTm)
        return 1;
    return 0;
}


int FIL_GetTmpDir(char *t)
{
    char *p;
    char nm[FIL_NMSZ+2];

    if (*t) {
        p = STPCPY(nm, t);
    } else {
        p = getenv("TMP");
        if (p == NULL) {
            p = getenv("TEMP");
            if (p == NULL) {
              #if 1
                p = (char*)".\\";
              #else
                printfE("���ϐ�TMP��TEMP, �܂���-w<DIR>�Ńe���|�����E�f�B���N�g�����w�肵�Ă�������\n");
                /*printfE("���ϐ�TMP��TEMP�������إ�ިڸ�؂��w�肵�Ă�������\n");*/
              #endif
            }
        }
        p = STPCPY(nm, p);
    }
    if (p[-1] != '\\' && p[-1] != ':' && p[-1] != '/')
        strcat(nm,"\\");
    strcat(nm,"*.*");
    _fullpath(t, nm, FIL_NMSZ);
    p = FIL_BaseName(t);
    *p = 0;
    if (p[-1] == '\\')
        p[-1] = 0;
    return 0;
}
