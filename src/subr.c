/**
 *  @file   subr.c
 *  @brief  �T�u���[�`��
 *  @author Masashi KITAMURA (tenka@6809.net)
 *  @note
 *      Boost Software License Version 1.0
 */
#include "subr.h"
#include <stdarg.h>
#include <io.h>



/* ------------------------------------------------------------------------ */
int     debugflag;

char *strncpyZ(char *dst, char *src, size_t size)
{
    strncpy(dst, src, size);
    dst[size-1] = 0;
    return dst;
}

char *StrSkipSpc(char *s)
{
    while (*s && *(unsigned char *)s <= ' ')
        s++;
    return s;
}

char *StrSkipNotSpc(char *s)
{
    while (*(unsigned char *)s > ' ')
        s++;
    return s;
}

char *FIL_DelLastDirSep(char *dir)
{
    char *p;

    if (dir) {
        p = FIL_BaseName(dir);
        if (strlen(p) > 1) {
            p = STREND(dir);
            if (p[-1] == '\\' || p[-1] == '/')
                p[-1] = 0;
        }
    }
    return dir;
}



/*--------------------------------------*/

SLIST_T *SLIST_Add(SLIST_T **p0, char *s)
{
    SLIST_T *p;

    p = *p0;
    if (p == NULL) {
        p = callocE(1, sizeof(SLIST_T));
        p->s = strdupE(s);
        *p0 = p;
    } else {
        while (p->link != NULL) {
            p = p->link;
        }
        p->link = callocE(1, sizeof(SLIST_T));
        p = p->link;
        p->s = strdupE(s);
    }
    return p;
}



/*--------------------- �G���[�����t���̕W���֐� ---------------------------*/
volatile void printfE(char *fmt, ...)
{
    va_list app;

    va_start(app, fmt);
/*  fprintf(stdout, "%s %5d : ", src_name, src_line);*/
    vfprintf(stdout, fmt, app);
    va_end(app);
    exit(1);
}

void *mallocE(size_t a)
    /* �G���[������Α�exit�� malloc() */
{
    void *p;
 #if 1
    if (a == 0)
        a = 1;
 #endif
    p = calloc(1,a);
//printf("malloc(0x%x)\n",a);
    if (p == NULL) {
        printfE("������������܂���( %d byte(s))\n",a);
    }
    return p;
}

void *callocE(size_t a, size_t b)
    /* �G���[������Α�exit�� calloc() */
{
    void *p;

 #if 1
    if (b == 0)
        b = 1;
 #endif
    p = calloc(a,b);
//printf("calloc(0x%x,0x%x)\n",a,b);
    if (p == NULL) {
        printfE("������������܂���(%d*%d byte(s))\n",a,b);
    }
    return p;
}

void *reallocE(void *m, size_t a)
    /* �G���[������Α�exit�� calloc() */
{
    void *p;
    p = realloc(m, a);
//printf("realloc(0x%x,0x%x)\n",m,a);
    if (p == NULL) {
        printfE("������������Ȃ��ł�(%d byte(s))\n",a);
    }
    return p;
}

char *strdupE(char *s)
    /* �G���[������Α�exit�� strdup() */
{
    char *p;
//printf("strdup('%s')\n",s);
    if (s == NULL)
        return callocE(1,1);
    p = strdup(s);
    if (p == NULL) {
        printfE("������������܂���(����%d+1)\n",strlen(s));
    }
    return p;
}

void freeE(void *p)
{
    if (p)
        free(p);
    /*return 0;*/
}

/* ------------------------------------------------------------------------ */
FILE *fopenE(char *name, char *mod)
    /* �G���[������Α�exit�� fopen() */
{
    FILE *fp;
    fp = fopen(name,mod);
    if (fp == NULL) {
        printfE("�t�@�C�� %s ���I�[�v���ł��܂���\n",name);
    }
    setvbuf(fp, NULL, _IOFBF, 1024*1024);
    return fp;
}

size_t  fwriteE(void *buf, size_t sz, size_t num, FILE *fp)
    /* �G���[������Α�exit�� fwrite() */
{
    size_t l;

    l = fwrite(buf, sz, num, fp);
    if (ferror(fp)) {
        printfE("�t�@�C�������݂ŃG���[����\n");
    }
    return l;
}

size_t  freadE(void *buf, size_t sz, size_t num, FILE *fp)
    /* �G���[������Α�exit�� fread() */
{
    size_t l;

    l = fread(buf, sz, num, fp);
    if (ferror(fp)) {
        printfE("�t�@�C���Ǎ��݂ŃG���[����\n");
    }
    return l;
}


/* ------------------------------------------------------------------------ */
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
              #if 10
                p = ".\\";
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


/*---------------------------------------------------------------------------*/
/* 32�r�b�g�ł̂Ƃ� ... 16�r�b�g�ł̓A�Z���u���\�[�X�̂ق� */
#ifndef C16

/* �Ƃ肠�����A�A�Z���u���\�[�X�Ƃ̌��ˍ����ŁA�_�~�[�֐���p�� */
static int  FIL_zenFlg = 1;         /* 1:MS�S�p�ɑΉ� 0:���Ή� */
static int  FIL_wcFlg  = 0x08;


void    FIL_SetZenMode(int ff)
{
    FIL_zenFlg = ff;
}

int FIL_GetZenMode(void)
{
    return FIL_zenFlg;
}


void    FIL_SetWcMode(int ff)
{
    FIL_wcFlg = ff;
}

int FIL_GetWcMode(void)
{
    return FIL_wcFlg;
}


/*--------------------------------------------*/

char *FIL_BaseName(char *adr)
{
    char *p;

    p = adr;
    while (*p != '\0') {
        if (*p == ':' || *p == '/' || *p == '\\')
            adr = p + 1;
        if (FIL_zenFlg && ISKANJI((*(unsigned char *)p)) && *(p+1) )
            p++;
        p++;
    }
    return adr;
}

char *FIL_ChgExt(char filename[], char *ext)
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


char *FIL_AddExt(char filename[], char *ext)
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

#endif






#if 1   //

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


#endif

/*---------------------------------------------------------------------------*/
