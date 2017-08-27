/**
 *  @file   abx.cpp
 *  @brief  �t�@�C�����������A�Y���t�@�C�����𕶎���ɖ���(�o�b�`����)
 *  @author Masashi KITAMURA (tenka@6809.net)
 *  @date   1995-2017
 *  @note
 *      license
 *          �����BSD���C�Z���X
 *          see license.txt
 */

#include <list>
#include <set>
//#include <vector>
//#include <string>
#include <algorithm>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "subr.hpp"


/*---------------------------------------------------------------------------*/

#define APP_HELP_TITLE      "abx v3.9x(pre v4) ̧�ٖ�������,�Y��̧�ٖ��𕶎���ɖ���(�ޯ�����).  by tenk*\n"
#define APP_HELP_CMDLINE    "usage : %s [��߼��] ['�ϊ�������'] ̧�ٖ� [=�ϊ�������]\n"
#define APP_HELP_OPTS       "��߼��:                        ""�ϊ�����:            �ϊ���:\n"                       \
                            " -x[-]    �ޯ����s   -x-���Ȃ� "" $f ���߽(�g���q�t)   d:\\dir\\dir2\\filename.ext\n"  \
                            " -r[-]    �ިڸ�؍ċA          "" $g ���߽(�g���q��)   d:\\dir\\dir2\\filename\n"      \
                            " -n[-]    ̧�ٌ������Ȃ� -n-�L "" $v ��ײ��            d\n"                            \
                            " -a[nrhsd] �w��̧�ّ����Ō���  "" $p �ިڸ��(��ײ�ޕt) d:\\dir\\dir2\n"                \
                            "          n:��� s:���� h:�B�� "" $d �ިڸ��(��ײ�ޖ�) \\dir\\dir2\n"                  \
                            "          r:ذ�޵�ذ d:�ިڸ�� "" $c ̧��(�g���q�t)    filename.ext\n"                 \
                            " -z[N-M]  ����N�`M��FILE������ "" $x ̧��(�g���q��)    filename\n"                     \
                            " -d[A-B]  ���tA�`B��FILE������ "" $e �g���q            ext\n"                          \
                            " -s[neztam][r] ���(����)       "" $w �����إ�ިڸ��    (���ϐ�TMP�̓��e)\n"          \
                            "          n:�� e:�g���q z:���� "" $z ����(10�i10��)    1234567890 ��$Z�Ȃ�16�i8��\n"   \
                            "          t:���t a:���� r:�~�� "" $j ����              1993-02-14\n"                   \
                            "          m:��(��)             "" $i �A�Ԑ���          ��$I�Ȃ�16�i��\n"               \
                            " -u[-]    $c|$C��̧�ٖ��召����"" $$ $  $[ <  $` '  $n ���s  $t ���\n"                 \
                            " -l[-]    @���͂Ŗ��O�͍s�P��  "" $# #  $] >  $^ \"  $s ��  $l �����͂̂܂�\n"       \
                            " -ci[N:M] N:$i�̊J�n�ԍ�(M:�I) ""------------------------------------------------\n"   \
                            " -ct<FILE> FILE���V�����Ȃ�  ""-p<DIR>  $p�̋����ύX   ""-ck[-] ���{�ꖼ�̂݌���\n"  \
                            " +CFGFILE .CFĢ�َw��         ""-e<EXT>  ��̫�Ċg���q   ""-cy[-] \\���܂ޑS�p������\n"\
                            " @RESFILE ڽ��ݽ̧��           ""-o<FILE> �o��̧�َw��   ""-y     $cxfgdpw��\"�t��\n"  \
                            " :�ϊ���  CFG�Œ�`�����ϊ�    ""-i<DIR>  �����ިڸ��    ""-t[N]  �ŏ���N�̂ݏ���\n" \
                            " :        �ϊ����ꗗ��\��     ""-w<DIR>  TMP�ިڸ��     ""\n"


typedef std::list<std::string>  StrList;

enum { OBUFSIZ  = 0x80000 };    /* ��`�t�@�C�����̃T�C�Y               */
enum { FMTSIZ   = 0x80000 };    /* ��`�t�@�C�����̃T�C�Y               */


class ConvFmt;

/*---------------------------------------------------------------------------*/

class FSrh {
    bool            recFlg_;        // 1:�ċA���� 0:���Ȃ�
    bool            normalFlg_;     // ɰ�٥̧�ق�ϯ� 1:���� 0:���Ȃ�
    bool            topFlg_;
    bool            nonFileFind_;   // 1:�t�@�C���������Ȃ� 0:����
    unsigned        sortFlags_;
    bool            sortRevFlg_;
    bool            uplwFlg_;
    int             fattr_;         // ����̧�ّ���
    int             knjChk_;
    long            topN_;
    long            topCnt_;
    char            fpath_[FIL_NMSZ * 4];
    char            fname_[FIL_NMSZ+2];
    unsigned long   szMin_;
    unsigned long   szMax_;
    unsigned short  dateMin_;
    unsigned short  dateMax_;
    FILE*           outFp_;
    ConvFmt*        convFmt_;
    int (ConvFmt::*membFunc_)(FILE* fp, char const* path, FIL_FIND const* ff);

public:
    FSrh()
        : recFlg_(1)
        , normalFlg_(1)
        , topFlg_(0)
        , nonFileFind_(0)
        , sortFlags_(0)
        , sortRevFlg_(0)
        , uplwFlg_(0)
        , fattr_(0x3f)
        , knjChk_(0)
        , topN_(0)
        , topCnt_(0)
        //, fpath_()
        //, fname_()
        , szMin_(0)
        , szMax_(0)
        , dateMin_(0)
        , dateMax_(0)
        , outFp_(NULL)
        , convFmt_(NULL)
        , membFunc_(NULL)
    {
        memset(fpath_, 0, sizeof(fpath_));
        memset(fname_, 0, sizeof(fname_));
    }

private:
    static int ChkKnjs(const char *p)
    {
        unsigned char c;
        while((c = *(unsigned char *)p++) != 0) {
            if (c & 0x80)
                return 1;
        }
        return 0;
    }


    struct FileFindNameCmp {
        bool operator()(FIL_FIND const& l, FIL_FIND const& r) const {
         #ifdef _WIN32
            return _stricmp(l.name, r.name) < 0;
         #else
            return strcmp(l.name, r.name) < 0;
         #endif
        }

    private:
        FSrh const*     pFSrh_;
    };
    typedef std::set<FIL_FIND, FileFindNameCmp> FileFindDirTree;


    #ifdef _WIN32
    #define FNAME_GET_C(c, p) do {                              \
            (c) = *(unsigned char*)((p)++);                     \
            if (IsDBCSLeadByte(c) && *(p))                      \
                (c) = ((c) << 8) | *(unsigned char*)((p)++);    \
        } while (0)
    #else
    #define FNAME_GET_C(c, p)   ((c) = *((p)++))
    #endif

    /** �t�@�C�����̑召��r. ���l���������ꍇ�A�����Ⴂ�̐��l���m�̑召�𔽉f
    *   �召���ꎋ. �f�B���N�g���Z�p���[�^ \ / �����ꎋ.
    *   �ȊO�͒P���ɕ������r.
    */
    static int fnameNDigitCmp(const char* l, const char* r, size_t len)
    {
        const char* e = l + len;
        if (e < l)
            e = (const char*)-1;
        while (l < e) {
         #if defined(_MSC_VER) || defined(__BORLANDC__)
            typedef unsigned __int64 num_t;
            typedef __int64          dif_t;
         #else
            typedef unsigned long long  num_t;
            typedef long long           dif_t;
         #endif
            dif_t       n;
            unsigned    lc;
            unsigned    rc;

            FNAME_GET_C(lc, l);
            FNAME_GET_C(rc, r);

            if (lc <= 0x80 && isdigit(lc) && rc <= 0x80 && isdigit(rc)) {
             #if defined(_MSC_VER)
                num_t   lv = _strtoui64(l - 1, (char**)&l, 10);
                num_t   rv = _strtoui64(r - 1, (char**)&r, 10);
             #else
                num_t   lv = strtoull(l - 1, (char**)&l, 10);
                num_t   rv = strtoull(r - 1, (char**)&r, 10);
             #endif
                n = (dif_t)(lv - rv);
                if (n == 0)
                    continue;
                return (n < 0) ? -1 : 1;
            }

            if (lc < 0x80)
                lc = tolower(lc);
            if (rc < 0x80)
                rc = tolower(rc);

            n  = (dif_t)(lc - rc);
            if (n == 0) {
                if (lc == 0)
                    return 0;
                continue;
            }

            if ((lc == '/' && rc == '\\') || (lc == '\\' && rc == '/')) {
                continue;
            }

            return (n < 0) ? -1 : 1;
        }
        return 0;
    }
    #undef FNAME_GET_C

    int  Cmp(FIL_FIND const* f1, FIL_FIND const* f2) const
    {
        int n = 0;

        if (sortFlags_ & 0x20) {                      /* ���������͐��l�Ŕ�r���閼�O�\�[�g */
            n = fnameNDigitCmp(f1->name, f2->name, (size_t)-1);
            if (sortRevFlg_)
                return -n;
            return n;
        }
        if (sortFlags_ <= 1) {                        /* ���O�Ń\�[�g */
            n = strcmp(f1->name, f2->name);
            if (sortRevFlg_)
                return -n;
            return n;
        }

        if (sortFlags_ == 0x02) {                     /* �g���q */
            char const* p = strrchr(f1->name, '.');
            p = (p == NULL) ? "" : p;
            char const* q = strrchr(f2->name, '.');
            q = (q == NULL) ? "" : q;
            n = strcmp(p,q);

        } else if (sortFlags_ == 0x04) {              /* �T�C�Y */
            long t;
            t = f1->size - f2->size;
            n = (t > 0) ? 1 : (t < 0) ? -1 : 0;

        } else if (sortFlags_ == 0x08) {              /* ���� */
          #if defined __BORLANDC__
            long t;
            t = (long)f1->wr_date - (long)f2->wr_date;
            n = (t > 0) ? 1 : (t < 0) ? -1 : 0;
            if (n == 0) {
                t = (long)f1->wr_time - (long)f2->wr_time;
                n = (t > 0) ? 1 : (t < 0) ? -1 : 0;
            }
          #else
            __int64 t;
            t = f1->time_write - f2->time_write;
            n = (t > 0) ? 1 : (t < 0) ? -1 : 0;
          #endif
        } else if (sortFlags_ == 0x10) {              /* ���� */
            /* �A�[�J�C�u�����͎ז��Ȃ̂ŃI�t���� */
            n = ((int)f2->attrib & 0xDF) - ((int)f1->attrib & 0xDF);
        }

        if (n == 0) {
            n = strcmp(f1->name, f2->name);
            if (sortRevFlg_)
                n = -n;
        }
        if (sortRevFlg_)
            return -n;
        return n;
    }

    struct FileFindCmp {
        FileFindCmp(FSrh const* pFSrh=NULL) : pFSrh_(pFSrh) {}

        bool operator()(FIL_FIND const& l, FIL_FIND const& r) const {
            return pFSrh_->Cmp(&l, &r) < 0;
        }

    private:
        FSrh const*     pFSrh_;
    };
    typedef std::set<FIL_FIND, FileFindCmp>     FileFindTree;

    struct DoOne {
        DoOne(FSrh* pFSrh) : pFSrh_(pFSrh) {}

        void operator()(FIL_FIND const& ff) {
            if (pFSrh_->topFlg_) {
                if (pFSrh_->topCnt_ == 0)       /* �擪 N�݂̂̏����̂Ƃ� */
                    return;
                else
                    --pFSrh_->topCnt_;
            }
            char *t = STREND(pFSrh_->fpath_);
            strcpy(t, ff.name);
            (pFSrh_->convFmt_->*(pFSrh_->membFunc_))(pFSrh_->outFp_, pFSrh_->fpath_, &ff);
            *t = 0;
        }

    private:
        FSrh*       pFSrh_;
    };

    struct DoOneDir {
        DoOneDir(FSrh* pFSrh) : pFSrh_(pFSrh) {}

        void operator()(FIL_FIND const& ff) {
            char *t = STREND(pFSrh_->fpath_);
            strcpy(t, ff.name);
            strcat(t, "\\");
            pFSrh_->FindAndDo_SubSort();
            *t = 0;
        }

    private:
        FSrh*     pFSrh_;
    };


    int FindAndDo_SubSort()
    {
        FIL_FIND_HANDLE hdl = 0;
        FIL_FIND        ff = {0};
        char*           t;

        if (topFlg_) {
            topCnt_ = topN_;
        }

        {
            FileFindTree    tree(FileFindCmp(this));
            t    = STREND(fpath_);
            strcpy(t,fname_);
            hdl = FIL_FINDFIRST(fpath_, fattr_, &ff);
            if (FIL_FIND_HANDLE_OK(hdl)) {
                do {
                    *t = '\0';
                    if (normalFlg_ == 0 && (fattr_ & ff.attrib) == 0)
                        continue;
                    if ((fattr_ & 0x10) == 0 && (ff.attrib & 0x10))   /* �f�B���N�g�������łȂ��̂Ƀf�B���N�g�������������΂� */
                        continue;
                    if(  (ff.name[0] != '.')
                      && (  (szMin_ > szMax_) || ((int)szMin_ <= ff.size && ff.size <= (int)szMax_) )
                      && (  (dateMin_ > dateMax_) || (dateMin_ <= ff.wr_date && ff.wr_date <= dateMax_) )
                      && (  (knjChk_==0) || (knjChk_==1 && ChkKnjs(ff.name)) || (knjChk_==2 && strchr(ff.name,'\\'))
                                             || (knjChk_==-1&& !ChkKnjs(ff.name))|| (knjChk_==-2&& !strchr(ff.name,'\\'))  )
                      )
                    {
                        tree.insert(ff);
                    }
                } while (FIL_FINDNEXT(hdl, &ff) == 0);
                FIL_FINDCLOSE(hdl);
            }
            std::for_each(tree.begin(), tree.end(), DoOne(this));
        }

        if (recFlg_ /*&& nonFileFind_ == 0*/) {
            FileFindDirTree     dirTree;
            strcpy(t,"*.*");
            hdl = FIL_FINDFIRST(fpath_, 0x10, &ff);
            if (FIL_FIND_HANDLE_OK(hdl)) {
                do {
                    *t = '\0';
                    if ((ff.attrib & 0x10) && strcmp(ff.name, ".") && strcmp(ff.name, "..")) {
                        dirTree.insert(ff);
                    }
                } while (FIL_FINDNEXT(hdl, &ff) == 0);
                FIL_FINDCLOSE(hdl);
            }
            std::for_each(dirTree.begin(), dirTree.end(), DoOneDir(this));
        }
        return 0;
    }



    int FindAndDo_Sub()
    {
        FIL_FIND_HANDLE hdl;
        FIL_FIND        ff = {0};
        char *t;

        if (topFlg_) {
            topCnt_ = topN_;
        }
        t = STREND(fpath_);
        strcpy(t,fname_);
        hdl = FIL_FINDFIRST(fpath_, fattr_, &ff);
        if (FIL_FIND_HANDLE_OK(hdl)) {
            do {
                *t = '\0';
                if (normalFlg_ == 0 && (fattr_ & ff.attrib) == 0)
                    continue;
                if ((fattr_ & 0x10) == 0 && (ff.attrib & 0x10))   /* �f�B���N�g�������łȂ��̂Ƀf�B���N�g�������������΂� */
                    continue;
                if(  (ff.name[0] != '.')
                  && (  (szMin_ > szMax_) || ((int)szMin_ <= ff.size && ff.size <= (int)szMax_) )
                  && (  (dateMin_ > dateMax_) || (dateMin_ <= ff.wr_date && ff.wr_date <= dateMax_) )
                  && (  (knjChk_==0) || (knjChk_==1 && ChkKnjs(ff.name)) || (knjChk_==2 && strchr(ff.name,'\\'))
                                         || (knjChk_==-1&& !ChkKnjs(ff.name))|| (knjChk_==-2&& !strchr(ff.name,'\\'))  )
                  )
                {
                    strcpy(t, ff.name);
                    (convFmt_->*membFunc_)(outFp_, fpath_, &ff);
                    *t = 0;
                    if (topFlg_ && --topCnt_ == 0) {    /* �擪 N�݂̂̏����̂Ƃ� */
                        return 0;
                    }
                }
            } while (FIL_FINDNEXT(hdl, &ff) == 0);
            FIL_FINDCLOSE(hdl);
        }

        if (recFlg_) {
            strcpy(t,"*.*");
            hdl = FIL_FINDFIRST(fpath_, 0x10, &ff);
            if (FIL_FIND_HANDLE_OK(hdl)) {
                do {
                    *t = '\0';
                    if ((ff.attrib & 0x10) && ff.name[0] != '.') {
                        strcpy(t, ff.name);
                        strcat(t, "\\");
                        FindAndDo_Sub();
                    }
                } while (FIL_FINDNEXT(hdl, &ff) == 0);
                FIL_FINDCLOSE(hdl);
            }
        }
        return 0;
    }


public:
    int FindAndDo(char *path, int atr, bool recFlg, bool zenFlg,
                    long topn, unsigned sortFlags, int knjChk, bool nonFF,
                    unsigned long szmin, unsigned long szmax,
                    unsigned short dtmin, unsigned short dtmax,
                    FILE* outFp, ConvFmt* pConvFmt,
                    int (ConvFmt::*fun)(FILE* fp, char const* apath, FIL_FIND const* aff))
    {
        char *p;

        convFmt_    = pConvFmt;
        membFunc_   = fun;
        outFp_      = outFp;
        recFlg_     = recFlg;
        fattr_      = atr;
        topN_       = topn;
        topFlg_     = (topn != 0);
        knjChk_     = knjChk;
        szMin_      = szmin;
        szMax_      = szmax;
        dateMin_    = dtmin;
        dateMax_    = dtmax;
        nonFileFind_= nonFF;
        FIL_SetZenMode(zenFlg);
        /*printf("%lu(%lx)-%lu(%lx)\n",szmin,szmin,szmax,szmax);*/
        /*printf("date %04x-%04x\n",dtmin,dtmax);*/
        normalFlg_ = 0;
        if (atr & 0x100) {
            atr &= 0xff;
            fattr_ = atr;
            normalFlg_ = 1;
        }
        FIL_FullPath(path, fpath_);
        p = STREND(fpath_);
        if (p[-1] == ':' || p[-1] == '\\' || p[-1] == '/')
            strcat(fpath_, "*");
        p = FIL_BaseName(fpath_);
        strncpyZ(fname_, p, FIL_NMSZ);
        if (nonFileFind_) {   /* �t�@�C���������Ȃ��ꍇ */
            FIL_FIND ff;
            memset(&ff, 0, sizeof ff);
            if (   (knjChk_==0)
                || (knjChk_==1  && ChkKnjs(fname_))
                || (knjChk_==2  && strchr(fname_,'\\'))
                || (knjChk_==-1 && !ChkKnjs(fname_))
                || (knjChk_==-2 && !strchr(fname_,'\\')) )
            {
                (convFmt_->*membFunc_)(outFp, fpath_, &ff);
            }
            return 0;
        }
        /* �t�@�C����������ꍇ */
        *p = 0;
        if (sortFlags) {  /* �\�[�g���� */
            sortRevFlg_ = (sortFlags & 0x80) != 0;
            sortFlags_  =  sortFlags & 0x7f;
            uplwFlg_    = (sortFlags & 0x8000) != 0;
            return FindAndDo_SubSort();
        }
        return FindAndDo_Sub();
    }
};



/*---------------------------------------------------------------------------*/

class ConvFmt {
public:
	enum { VAR_NMSZ = FIL_NMSZ };
private:
    char        drv_[FIL_NMSZ];
    char        dir_[FIL_NMSZ];
    char        name_[FIL_NMSZ];
    char        ext_[FIL_NMSZ];
    bool        upLwrFlg_;
    int         num_;                         /* $i �Ő�������ԍ� */
    int         numEnd_;                      /* �A�Ԃ��t�@�C�����̕�����̑���ɂ���w��������ꍇ�̏I���A�h���X */
    char        tmpDir_[FIL_NMSZ];
    char        var_[10][VAR_NMSZ];
    char        pathDir_[FIL_NMSZ];
    char        chgPathDir_[FIL_NMSZ];
    char const* fmtBuf_;                /* �ϊ�����������߂� */
    char        obuf_[OBUFSIZ+FIL_NMSZ];   /* .cfg(.res) �ǂݍ��݂�A�o�͗p�̃o�b�t�@ */
    char        tgtnm_[FIL_NMSZ+FIL_NMSZ];
    char        tgtnmFmt_[FIL_NMSZ+FIL_NMSZ];
    char const* lineBuf_;
    bool        autoWqFlg_;                 /* $f���Ŏ����ŗ��[��"��t�����郂�[�h. */

public:
    ConvFmt()
        : upLwrFlg_(false)
        , num_(0)
        , numEnd_(0)
        , fmtBuf_(NULL)
        , lineBuf_(NULL)
        , autoWqFlg_(false)
    {
        memset(drv_, 0, sizeof(drv_));
        memset(dir_, 0, sizeof(dir_));
        memset(name_, 0, sizeof(name_));
        memset(ext_, 0, sizeof(ext_));
        memset(tmpDir_, 0, sizeof(tmpDir_));
        FIL_GetTmpDir(tmpDir_); /* �e���|�����f�B���N�g�����擾 */
        memset(var_, 0, sizeof(var_));
        memset(pathDir_, 0, sizeof(pathDir_));
        memset(chgPathDir_, 0, sizeof(chgPathDir_));
        memset(obuf_, 0, sizeof(obuf_));
        memset(tgtnm_, 0, sizeof(tgtnm_));
        memset(tgtnmFmt_, 0, sizeof(tgtnmFmt_));
    }

    void setChgPathDir(char const* dir) {
        //strncpyZ(chgPathDir_, dir, FIL_NMSZ);
        FIL_FullPath(dir, chgPathDir_);
        char* p = STREND(chgPathDir_);
        if (p[-1] == '\\' || p[-1] == '/') {
            p[-1] = '\0';
        }
    }

    char const* tmpDir() const { return tmpDir_; }

    void setTmpDir(char const* dir) {
        strncpyZ(tmpDir_, dir, FIL_NMSZ);
        FIL_GetTmpDir(tmpDir_); /* �e���|�����f�B���N�g�����擾 */
    }

    void setTgtnmFmt(char const* tgt) {
        strncpyZ(tgtnmFmt_, tgt, FIL_NMSZ);
    }

    void clearVar() {
        for (int l = 0; l < 10; l++)
            var_[l][0] = 0;
    }

    char const* getVar(unsigned n) const {
        return (n < 10) ? var_[n] : "";
    }

    bool setVar(unsigned m, char const* p, size_t l) {
        if (l < 1 || l >= (sizeof var_[0])-1)
            return false;
        memcpy(var_[m], p, l);
        var_[m][l+1] = '\0';
        return true;
    }

    void setNum(int num) { num_ = num; }

    void setUpLwrFlag(bool sw) { upLwrFlg_ = sw; }

    void setAutoWq(bool sw) { autoWqFlg_ = sw; }

    void setLineBuf(char const* lineBuf) { lineBuf_ = lineBuf; }

    void setFmtStr(char const* fmtBuf) {
        fmtBuf_ = fmtBuf;
    }

    char *StpCpy(char *d, char const* s, ptrdiff_t clm, int flg)
    {
        unsigned char c;
        size_t        n = 0;
        if (upLwrFlg_ == 0) {
            strcpy(d,s);
            n = strlen(s);
            d = d + n;
        } else if (flg == 0) {  /* �啶���� */
            while ((c = *(unsigned char *)s++) != '\0') {
                if (islower(c))
                    c = toupper(c);
                *d++ = (char)c;
                n++;
                if (ISKANJI(c) && *s && FIL_GetZenMode()) {
                    *d++ = *s++;
                    n++;
                }
            }
        } else {        /* �������� */
            while ((c = *(unsigned char *)s++) != '\0') {
                if (isupper(c))
                    c = tolower(c);
                *d++ = (char)c;
                n++;
                if (ISKANJI(c) && *s && FIL_GetZenMode()) {
                    *d++ = *s++;
                    n++;
                }
            }
        }
        clm -= (ptrdiff_t)n;
        while (clm > 0) {
            *d++ = ' ';
            --clm;
        }
        *d = '\0';
        return d;
    }

    void SplitPath(char const* fpath)
    {
        FIL_SplitPath(fpath, drv_, dir_, name_, ext_);

      #if 1
        /* �f�B���N�g�����̌���'\'���͂��� */
        size_t  l = strlen(dir_);
        if (l) {
            char*   p = dir_ + l - 1;
            if (*p == '\\' || *p == '/') {
                *p = 0;
            }
        }
      #else
        FIL_DelLastDirSep(dir_);  /* �f�B���N�g�����̌���'\'���͂��� */
      #endif
        strcpy(pathDir_,drv_);
        strcat(pathDir_,dir_);
        if (chgPathDir_[0]) {
            strcpy(pathDir_, chgPathDir_);
        }
        /* �g���q�� '.' ���͂��� */
        if (ext_[0] == '.') {
            memmove(ext_, ext_+1, strlen(ext_+1)+1);
        }
    }


    int Write(FILE* fp, char const* fpath, FIL_FIND const* ff)
    {
        SplitPath(fpath);

        StrFmt(tgtnm_, tgtnmFmt_, FIL_NMSZ, ff);                 // ����̃^�[�Q�b�g����ݒ�
        if (tgtnmFmt_[0] == 0 || FIL_FdateCmp(tgtnm_, fpath) < 0) { // ���t��r���Ȃ����A����ꍇ�̓^�[�Q�b�g���Â����
            StrFmt(obuf_, fmtBuf_, OBUFSIZ, ff);
            fprintf(fp, "%s", obuf_);
        }
        num_++;
        return 0;
    }


    void StrFmt(char *dst, char const* fmt, int sz, FIL_FIND const* ff)
    {
        char    buf[FIL_NMSZ*4] = {0};
        char    *b;
        int     f,n;
        char    drv[2];
        drv[0] = drv_[0];
        drv[1] = 0;

        char const* s = fmt;
        char*       p = dst;
        char*       pe = p + sz;
        char        c;
        while ((c = (*p++ = *s++)) != '\0' && p < pe) {
            if (c == '$') {
                --p;
                n = -1;
                c = *s++;
                if (c == '+') { /* +NN �͌����w�肾 */
                    n = strtoul(s,(char**)&s,10);
                    if (s == NULL || *s == 0)
                        break;
                    if (n >= FIL_NMSZ)
                        n = FIL_NMSZ;
                    c = *s++;
                }
                f = islower(c);
                switch (toupper(c)) {
                case 'S':   *p++ = ' ';     break;
                case 'T':   *p++ = '\t';    break;
                case 'N':   *p++ = '\n';    break;
                case '$':   *p++ = '$';     break;
                case '#':   *p++ = '#';     break;
                case '[':   *p++ = '<';     break;
                case ']':   *p++ = '>';     break;
                case '`':   *p++ = '\'';    break;
                case '^':   *p++ = '"';     break;

                case 'L':   p = StpCpy(p,lineBuf_,n,f);        break;
                case 'V':   p = StpCpy(p,drv,n,f);       break;
                case 'D':
                    if (autoWqFlg_) *p++ = '"';
                    p = StpCpy(p,dir_,n,f);
                    if (autoWqFlg_) *p++ = '"';
                    *p = 0;
                    break;
                case 'X':
                    if (autoWqFlg_) *p++ = '"';
                    p = StpCpy(p,name_,n,f);
                    if (autoWqFlg_) *p++ = '"';
                    *p = 0;
                    break;
                case 'E':
                    p = StpCpy(p,ext_,n,f);
                    break;
                case 'W':
                    if (autoWqFlg_) *p++ = '"';
                    p = StpCpy(p, tmpDir_, n, f);
                    if (autoWqFlg_) *p++ = '"';
                    *p = 0;
                    break;
                case 'P':
                    if (autoWqFlg_) *p++ = '"';
                    p = StpCpy(p,pathDir_,n,f);
                    if (autoWqFlg_) *p++ = '"';
                    *p = 0;
                    break;

                case 'C':
                    b = buf;
                    if (autoWqFlg_) *b++ = '"';
                    b = StpCpy(b,name_,0,f);
                    if (ext_[0]) {
                        b = STPCPY(b,".");
                        b = StpCpy(b,ext_,0,f);
                    }
                    if (autoWqFlg_) *b++ = '"';
                    *b = 0;
                    if (n < 0) n = 1;
                    p += sprintf(p, "%-*s", n, buf);
                    break;
                case 'F':
                    b = buf;
                    if (autoWqFlg_) *b++ = '"';
                    b = StpCpy(b,drv_,0,f);
                    b = StpCpy(b,dir_,0,f);
                    b = STPCPY(b,"\\");
                    b = StpCpy(b,name_,0,f);
                    if (ext_[0]) {
                        b = STPCPY(b,".");
                        b =StpCpy(b,ext_,0,f);
                    }
                    if (autoWqFlg_) *b++ = '"';
                    *b = 0;
                    if (n < 0) n = 1;
                    p += sprintf(p, "%-*s", n, buf);
                    break;
                case 'G':
                    b = buf;
                    if (autoWqFlg_) *b++ = '"';
                    b = StpCpy(b,drv_,0,f);
                    b = StpCpy(b,dir_,0,f);
                    b = STPCPY(b,"\\");
                    b = StpCpy(b,name_,0,f);
                    if (autoWqFlg_) *b++ = '"';
                    *b = '\0';
                    if (n < 0) n = 1;
                    p += sprintf(p, "%-*s", n, buf);
                    break;
                case 'O':
                    StpCpy(buf, tgtnm_, 0, f);
                    if (n < 0) n = 1;
                    p += sprintf(p, "%-*s", n, buf);
                    break;
                case 'Z':
                    if (f) {
                        if (n < 0)
                            n = 10;
                        p += sprintf(p, "%*d", n, ff->size);
                    } else {
                        if (n < 0)
                            n = 8;
                        p += sprintf(p, "%*X", n, ff->size);
                    }
                    break;

                case 'I':
                    if (f) {
                        if (n < 0)
                            n = 1;
                        p += sprintf(p, "%0*d", n, num_);
                    } else {
                        if (n < 0)
                            n = 1;
                        p += sprintf(p, "%0*X", n, num_);
                    }
                    break;

                case 'J':
                  #if defined __BORLANDC__
                    {   int y,m,d;
                        y = (1980+((unsigned short)ff->wr_date>>9));
                        m = (ff->wr_date>>5) & 0x0f;
                        d = (ff->wr_date   ) & 0x1f;
                        if (n < 0)
                            n = 10;
                        if (n >= 10) {
                            sprintf(buf, "%04d-%02d-%02d", y, m, d);
                        } else if (n >= 8) {
                            sprintf(buf, "%02d-%02d-%02d", y %100, m, d);
                        } else {
                            sprintf(buf, "%02d-%02d", m, d);
                        }
                        p += sprintf(p, "%-*s", n, buf);
                    }
                  #else
                    {   int y = 0, m = 0, d = 0;
                     #if defined _MSC_VER && _MSC_VER >= 1400
                        struct tm* ltm = _localtime64(&ff->time_write);
                     #else
                        struct tm* ltm = localtime(&ff->time_write);
                     #endif
                        if (ltm) {
                            y = ltm->tm_year + 1900;
                            m = ltm->tm_mon  + 1;
                            d = ltm->tm_mday;
                        }
                        if (n < 0)
                            n = 10;
                        if (n >= 10) {
                            sprintf(buf, "%04d-%02d-%02d", y, m, d);
                        } else if (n >= 8) {
                            sprintf(buf, "%02d-%02d-%02d", y %100, m, d);
                        } else {
                            sprintf(buf, "%02d-%02d", m, d);
                        }
                        p += sprintf(p, "%-*s", n, buf);
                    }
                  #endif
                    break;
                default:
                    if (c >= '1' && c <= '9') {
                        p = STPCPY(p, var_[c-'0']);
                    } else {
                        fprintf(stderr, "Incorrect '$' format : '$%c'\n",c);
                        /*fprintfE(stderr,".cfg �� $�w�肪��������(%c)\n",c);*/
                        // exit(1);
                    }
                }
            }
        }
    }


    int WriteLine0(FILE* fp, char const* s)
    {
        char* p = obuf_;
        char c;
        while ((c = (*p++ = *s++)) != '\0') {
            if (c == '$') {
                --p;
                c = *s++;
                if (c == '$') {
                    *p++ = '$';
                } else if (c >= '1' && c <= '9') {
                    p = STPCPY(p, var_[c-'0']);
                } else {
                    fprintf(stderr,"Incorrect '$' format : '$%c'\n",c);
                    /*fprintfE(stderr,"���X�|���X���� $�w�肪��������(%c)\n",c);*/
                    exit(1);
                }
            }
        }
        fprintf(fp, "%s\n", obuf_);
        return 0;
    }

};



/*--------------------------------------------------------------------------*/
class Opts {
public:
    ConvFmt&        rConvFmt_;
    bool            recFlg_;                /* �ċA�̗L�� */
    bool            zenFlg_;                /* MS�S�p�Ή� */
    bool            batFlg_;                /* �o�b�`���s */
    bool            batEx_;                 /* -b�̗L�� */
    bool            linInFlg_;              /* RES���͂��s�P�ʏ���*/
    bool            autoWqFlg_;
    bool            upLwrFlg_;
    int             knjChk_;                /* MS�S�p���݃`�F�b�N */
    int             fattr_;                 /* �t�@�C������ */
    int             sort_;                  /* �\�[�g */
    long            topN_;                  /* ������ */
    bool            noFindFile_;            /* �t�@�C���������Ȃ� */
    char            outname_[FIL_NMSZ];     /* �o�̓t�@�C���� */
    char            ipath_[FIL_NMSZ];       /* ���̓p�X�� */
    char *          iname_;                 /* ���̓t�@�C���� */
    char            dfltExt_[FIL_NMSZ];     /* �f�t�H���g�g���q */
    char *          dfltExtp_;              /* �f�t�H���g�g���q */
    char            chgPathDir_[FIL_NMSZ];
    char            exename_[FIL_NMSZ];
    unsigned long   szmin_;                 /* szmin > szmax�̂Ƃ���r���s��Ȃ�*/
    unsigned long   szmax_;
    unsigned short  dtmin_;                 /* dtmin > dtmax�̂Ƃ���r���s��Ȃ�*/
    unsigned short  dtmax_;
    int             renbanStart_;           /* �A�Ԃ̊J�n�ԍ�. ����0 */
    int             renbanEnd_;             /* �A�Ԃ̊J�n�ԍ�. ����0 */

public:
    Opts(ConvFmt& rConvFmt)
        : rConvFmt_(rConvFmt)
        , recFlg_(false)
        , zenFlg_(true)
        , batFlg_(false)
        , batEx_(false)
        , linInFlg_(false)
        , autoWqFlg_(false)
        , upLwrFlg_(false)
        , knjChk_(0)
        , fattr_(0)
        , sort_(0x00)
        , topN_(0)
        , noFindFile_(false)
        //, outname_()
        //, ipath_()
        , iname_(ipath_)
        //, dfltExt_()
        , dfltExtp_(NULL)
        //, chgPathDir_()
        //, exename_()
        , szmin_(0xFFFFFFFFUL)
        , szmax_(0UL)
        , dtmin_(0xFFFFU)
        , dtmax_(0)
        , renbanStart_(0)
        , renbanEnd_(0)
    {
        memset(outname_   , 0, sizeof(outname_));
        memset(ipath_     , 0, sizeof(ipath_));
        memset(dfltExt_   , 0, sizeof(dfltExt_));
        memset(chgPathDir_, 0, sizeof(chgPathDir_));
        memset(exename_   , 0, sizeof(exename_));
    }

    void setExename(char const* exename) {
        strncpyZ(exename_, exename, sizeof(exename_));
     #ifdef _WIN32
        StrLwrN(exename_, strlen(exename_));
     #endif
    }

    bool scan(char* s)
    {
        char* p = s + 1;
        int  c = *p++;
        c = toupper(c);
        switch (c) {
        case 'X':
            batFlg_ = (*p != '-');
            break;

        case 'R':
            recFlg_ = (*p != '-');
            break;

        case 'U':
            upLwrFlg_ = (*p != '-');
            break;

        case 'N':
            noFindFile_ = (*p != '-');
            if (*p == 'd' || *p == 'D')
                noFindFile_ = 2;
            break;

        case 'J':
            zenFlg_ = (*p != '-');
            break;

        case 'B':
            batEx_ = (*p != '-');
            break;

        case 'L':
            linInFlg_  = (*p != '-');
            break;

        case 'T':
            if (*p == 0) {
                topN_ = 1;
            } else {
                topN_ = strtol(p,NULL,0);
            }
            break;

        case 'C':
            c = toupper(*p);
            if (c == '-') {
                knjChk_ = 0;
            } else if (c == 'K') {
                knjChk_ = 1;
                if (p[1] == '-')
                    knjChk_ = -1;
            } else if (c == 'Y') {
                knjChk_ = 2;
                if (p[1] == '-')
                    knjChk_ = -2;
            } else if (c == 'T' /*|| c == 'F'*/) {  // 'F'�͋��݊�
                rConvFmt_.setTgtnmFmt(p + 1);
            } else if (c == 'I') {
                renbanStart_ = strtol(p+1, (char**)&p, 0);
                if (*p) {
                    renbanEnd_ = strtol(p+1, (char**)&p, 0);
                } else {
                    renbanEnd_ = 0;
                }
            } else {
                goto ERR_OPTS;
            }
            break;

        case 'Y':
            autoWqFlg_ = (*p != '-');
            break;

        case 'E':
            dfltExtp_ = strncpyZ(dfltExt_, p, FIL_NMSZ);
            if (*p == '$' && p[1] >= '1' && p[1] <= '9' && p[2] == 0) {
                strcpy(dfltExt_, rConvFmt_.getVar(p[1]-'0'));
            }
            /*dfltExt_[3] = 0;*/
            break;

        case 'O':
            if (*p == 0)
                goto ERR_OPTS;
            strcpy(outname_,p);
            break;

        case 'I':
            if (*p == 0)
                goto ERR_OPTS;
            FIL_FullPath(p, ipath_);
            p = STREND(ipath_);
            if (p[-1] != '\\' && p[-1] != '/') {
                *p++ = '\\';
                *p = '\0';
            }
            iname_ = p;
            break;

        case 'P':
            if (*p == 0)
                goto ERR_OPTS;
            rConvFmt_.setChgPathDir(p);
            break;

        case 'W':
            rConvFmt_.setTmpDir(p);
            break;

        case 'A':
            strupr(p);
            while (*p) {
                switch(*p) {
                case 'N': fattr_ |= 0x100; break;
                case 'R': fattr_ |= 0x001; break;
                case 'H': fattr_ |= 0x002; break;
                case 'S': fattr_ |= 0x004; break;
                case 'V': fattr_ |= 0x008; break;
                case 'D': fattr_ |= 0x010; break;
                case 'A': fattr_ |= 0x020; break;
                }
                ++p;
            }
            break;

        case 'S':
            c = 0;
            sort_ = 0x01;
            strupr(p);
            while (*p) {
                switch(*p) {
                case '-': sort_ = 0x00; break;
                case 'N': sort_ = 0x01; break;
                case 'E': sort_ = 0x02; break;
                case 'Z': sort_ = 0x04; break;
                case 'T': sort_ = 0x08; break;
                case 'A': sort_ = 0x10; break;
                case 'M': sort_ = 0x20; break;
                case 'R': c = 0x80;        break;
                }
                ++p;
            }
            sort_ |= c;
            break;

        case 'Z':
            szmin_ = (*p == '-') ? 0 : strtoul(p, &p, 0);
            if (*p == 'K' || *p == 'k')         p++, szmin_ *= 1024UL;
            else if (*p == 'M' || *p == 'm')    p++, szmin_ *= 1024UL*1024UL;
            if (*p) { /* *p == '-' */
                szmax_ = 0xffffffffUL;
                p++;
                if (*p) {
                    szmax_ = strtoul(p,&p,0);
                    if (*p == 'K' || *p == 'k')         p++, szmax_ *= 1024UL;
                    else if (*p == 'M' || *p == 'm')    p++, szmax_ *= 1024UL*1024UL;
                }
                if (szmax_ < szmin_)
                    goto ERR_OPTS;
            } else {
                szmax_ = szmin_;
            }
            break;

        case 'D':
            if (*p == 0) {
                dtmax_ = dtmin_ = 0;
            } else {
                unsigned long t;
                int y,m,d;
                if (*p == '-') {
                    dtmin_ = 0;
                } else {
                    t = strtoul(p,&p,10);
                    y = (int)((t/10000) % 100); y = (y >= 80) ? (y-80) : (y+100-80);
                    m = (int)((t / 100) % 100); if (m == 0 || 12 < m) goto ERR_OPTS;
                    d = (int)(t % 100);         if (d == 0 || 31 < d) goto ERR_OPTS;
                    dtmin_ = (y<<9)|(m<<5)|d;
                }
                if (*p) {
                    p++;
                    dtmax_ = 0xFFFFU;
                    if (*p) {
                        t = strtoul(p,&p,10);
                        y = (int)(t/10000)%100; y = (y>=80) ? (y-80) : (y+100-80);
                        m = (int)(t/100) % 100; if (m==0 || 12 < m) goto ERR_OPTS;
                        d = (int)(t % 100);     if (d==0 || 31 < d) goto ERR_OPTS;
                        dtmax_ = (y<<9)|(m<<5)|d;
                        if (dtmax_ < dtmin_)
                            goto ERR_OPTS;
                    }
                } else {
                    dtmax_ = dtmin_;
                }
            }
            break;

        case '?':
        case '\0':
            return Usage();

        default:
      ERR_OPTS:
          #if 1
            printf("�R�}���h���C���ł̃I�v�V�����w�肪�������� : %s\n", s);
          #else
            printf("Incorrect command line option : %s\n", s);
          #endif
            exit(1);
        }
        return true;
    }


    bool Usage() {
        printf(APP_HELP_TITLE APP_HELP_CMDLINE, exename_);
        printf("%s", APP_HELP_OPTS);
        return false;
    }
};


/*---------------------------------------------------------------------------*/

class ResCfgFile {
    Opts&       rOpts_;
    ConvFmt&    rConvFmt_;
    StrList&    rFileNameList_;
    StrList&    rBeforeStrList_;
    StrList&    rAfterStrList_;
    char*       fmtBuf_;
    char        Res_nm[FIL_NMSZ];
    char        Res_obuf[OBUFSIZ+FIL_NMSZ];
    char *      Res_p;
    int         varIdx_;
    int         varNo_[10];

public:
    ResCfgFile(Opts& rOpts, ConvFmt& rConvFmt, StrList& rFileNameList, StrList& rBeforeList, StrList& rAfterList, char* fmtBuf)
        : rOpts_(rOpts)
        , rConvFmt_(rConvFmt)
        , rFileNameList_(rFileNameList)
        , rBeforeStrList_(rBeforeList)
        , rAfterStrList_(rAfterList)
        , fmtBuf_(fmtBuf)
        , Res_p(Res_obuf)
        , varIdx_(1)
    {
        memset(Res_nm, 0x00, sizeof(Res_nm));
        memset(Res_obuf, 0x00, sizeof(Res_obuf));
        memset(varNo_, 0, sizeof(varNo_));
    }

private:
    /** Res_obuf�ɒ������e�L�X�g���P�s����
     * �s���̉��s�͍폜. Res_obuf�͔j��
     */
    char *Res_GetLine(void)
    {
        char *p;

        p = Res_p;
        if (*p == 0)
            return NULL;
        Res_p = strchr(p, '\n');
        if (Res_p != NULL) {
            if (Res_p[-1] == '\r')
                Res_p[-1] = 0;
            *Res_p++ = 0;
        } else {
            Res_p = STREND(p);
        }
        return p;
    }

    char *Res_SetDoll(char *p0)
    {
        char* p = p0;
        int m = *p++;
        if (m < '1' || m > '9')
            goto ERR;
        m -= '0';
        if (*p == '=') {
            ++p;
            size_t l = strcspn(p,"\t\n ");
          #if 1
            if (rConvFmt_.setVar(m, p, l) == false)
                goto ERR;
          #else
            if (l < 1 || l >= (sizeof rConvFmt_.var_[0])-1)
                goto ERR;
            memcpy(rConvFmt_.var_[m], p, l);
            rConvFmt_.var_[m][l+1] = 0;
          #endif
            p += l+1;
            goto RET;

      ERR:
            printf(".cfg �t�@�C���� $�m �w��ł����������̂����� : $%s\n",p0);
            exit(1);

        } else if (*p++ == ':') {
            int n = *p++;
            if (n < '1' || n > '9')
                goto ERR2;
            n -= '0';
            if (*p++ != '{')
                goto ERR2;
            int i = 0;
            do {
                size_t l = strcspn(p,"|}");
             #if 1
                if (i == varNo_[n]) {
                    if (rConvFmt_.setVar(m, p, l) == false)
                        goto ERR;
                    p = strchr(p,'}');
                    if (p == NULL)
                        goto ERR2;
                    p++;
                    goto RET;
                }
             #else
                if (l < 1 || l >= (sizeof rConvFmt_.var_[0])-1)
                    goto ERR;
                if (i == varNo_[n]) {
                    memcpy(rConvFmt_.var_[m], p, l);
                    rConvFmt_.var_[m][l+1] = 0;
                    p = strchr(p,'}'); if (p == NULL) goto ERR2;
                    p++;
                    goto RET;
                }
             #endif
                i++;
                p += l + 1;
            } while (p[-1] == '|');
      ERR2:
            printf(".cfg �t�@�C���� $�m=������w�� �܂��� $�m:�l{..}�w��ł����������̂����� : $%s\n",p0);
            exit(1);
        }
      RET:
        return p;
    }



    /** s ���󔒂ŋ�؂�ꂽ�P��(�t�@�C����)��name �ɃR�s�[����.
     * ������ "file name" �̂悤��"�������"���폜���ւ�ɊԂ̋󔒂��c��.
     */
    char const* Res_GetFileNameStr(char *d, char const* s)
    {
        int f = 0;

        s = StrSkipSpc(s);
        while (*s) {
            if (*s == '"')
                f ^= 1;
            else if (f == 0 && (*(unsigned char *)s <= ' '))
                break;
            else
                *d++ = *s;
            s++;
        }
        *d = 0;
        return s;
    }




    bool Res_GetFmts()
    {
        #define ISSPC(c)    ((unsigned char)c <= ' ')
        char    name[FIL_NMSZ];
        char*   p;
        char*   q;
        char*   d;
        int     mode;

        d = fmtBuf_;
        mode = 0;
        while ( (p = Res_GetLine()) != NULL ) {
            q = (char*)StrSkipSpc(p);
            if (strnicmp(q, "#begin", 6) == 0 && ISSPC(p[6])) {
                mode = 1;
                continue;
            } else if (strnicmp(q, "#body", 5) == 0 && ISSPC(p[5])) {
                mode = 0;
                continue;
            } else if (strnicmp(q, "#end", 4) == 0 && ISSPC(p[4])) {
                mode = 2;
                continue;
            }
            switch (mode) {
            case 0: /* #body */
                while (p && *p) {
                    p = (char*)StrSkipSpc(p);  /* �󔒃X�L�b�v */
                    switch (*p) {
                    case '\0':
                    case '#':
                        goto NEXT_LINE;
                    case '\'':
                        if (p[1] == 0) {
                            printf("���X�|���X�t�@�C��(��`�t�@�C����)��'�ϊ�������'�w�肪��������\n");
                            exit(1);
                        }
                        p++;
                        d = strchr(p, '\'');
                        if (d) {
                            *d = '\0';
                            d = STPCPY(fmtBuf_, p);
                        }
                        break;
                    case '=':
                        /*memcpy(rConvFmt_.fmtBuf_, Res_p, strlen(Res_p)+1);*/
                        d = fmtBuf_;
                      #if 0
                        if (p[1]) {
                            d = STPCPY(d, p+1);
                            *d++ = '\n';
                            *d   = '\0';
                        }
                      #endif
                        mode = 3;
                        goto NEXT_LINE;
                    case '-':               /* �I�v�V���������� */
                        q = (char*)StrSkipNotSpc(p);
                        if (*q) {
                            *q++ = 0;
                        } else {
                            q = NULL;
                        }
                        if (rOpts_.scan(p) == false) {
                            return false;
                        }
                        p = q;
                        break;
                    case '$':               /* $�ϐ��� */
                        p = Res_SetDoll(p+1);
                        break;
                    default:
                        if (rOpts_.linInFlg_) { /* �s�P�ʂŃt�@�C�������擾 */
                            rFileNameList_.push_back(p);
                            goto NEXT_LINE;
                        } else {            /* �󔒋�؂�Ńt�@�C�������擾 */
                            p = (char*)Res_GetFileNameStr(name, p);
                            rFileNameList_.push_back(p);
                        }
                    }
                }
              NEXT_LINE:;
                break;
            case 1: /* #begin */
                rBeforeStrList_.push_back(p);
                break;
            case 2: /* #end  */
                rAfterStrList_.push_back(p);
                break;
            case 3: /* = �o�b�t�@���ߖ{�� */
                d = STPCPY(d, p);
                *d++ = '\n';
                *d   = '\0';
                break;
            }
        }
        return true;
        #undef ISSPC
    }


public:
    bool GetResFile(char const* name)
        /* ���X�|���X�t�@�C������ */
    {
        size_t  l;

        if (name[0] == 0) {                     /* �t�@�C�������Ȃ���ΕW������ */
            l = fread(Res_obuf, 1, FMTSIZ, stdin);
        } else {
            strcpy(Res_nm,name);
            FIL_AddExt(Res_nm, "abx");
            FILE* fp = fopenE(Res_nm,"r");
            l = freadE(Res_obuf, 1, FMTSIZ, fp);
            fclose(fp);
        }
        Res_obuf[l] = 0;
        if (l == 0)
            return true;

        Res_p = Res_obuf;
        return Res_GetFmts();  /* ���ۂ̃t�@�C�����e�̏��� */
    }


private:
    bool Res_StrEqu(char *key, char *lin)
    {
        char    *k,*f;
        size_t  l;

        rConvFmt_.clearVar();

        k = key;
        f = lin;
        for (; ;) {
          NEXT:
            if (*k == *f) {
                if (*k == '\0')
                    return true;   /* �}�b�`������ */
                k++;
                f++;
                continue;
            }
            if (*f == '{') {
                f++;
                varNo_[varIdx_] = 0;
                do {
                    l = strcspn(f,"|}");
                    /*printf("l=%d\n",l);*/
					if (l < 1 || l >= ConvFmt::VAR_NMSZ-1) {
                        goto ERR1;
                    }
                    if (memcmp(k,f,l) == 0) {
                        if (varIdx_ >= 10) {
                            printf("%s �̂��錟���s��{..}��10�ȏ゠�� %s\n", Res_nm,lin);
                            exit(1);
                        }
                     #if 1
                        rConvFmt_.setVar(varIdx_, f, l);
                     #else
                        memcpy(rConvFmt_.var_[varIdx_], f, l);
                        rConvFmt_.var_[varIdx_][l+1] = 0;
                     #endif
                        ++varIdx_;
                        k += l;
                        f = strchr(f,'}');
                        if (f == NULL) {
                  ERR1:
                            printf("%s ��{..}�̎w�肪�������� %s\n",Res_nm, lin);
                            exit(1);
                        }
                        f++;
                        goto NEXT;
                    }
                    f += l + 1;
                    varNo_[varIdx_]++;
                } while (f[-1] == '|');
                varNo_[varIdx_] = 0;
            }
            break;
        }
        return false;          /* �}�b�`���Ȃ����� */

    }


public:
    /** ��`�t�@�C������
     */
    bool GetCfgFile(char *name, char *key)
    {
        FILE    *fp;
        size_t  l;
        char    *p;

        FIL_FullPath(name, Res_nm);
        fp = fopenE(Res_nm,"r");
        l = freadE(Res_obuf, 1, FMTSIZ, fp);
        fclose(fp);
        Res_obuf[l] = 0;
        if (l == 0)
            return false;

        if (key[1] == 0) /* ':'�����̎w��̂Ƃ� */
            printf("':�ϊ���'�ꗗ\n");
        /*l = 1;*/
        /*   */
        strupr(key);
        Res_p = Res_obuf;
        /* ���s+':'+�ϊ�����T�� */
        while ((Res_p = strstr(Res_p, "\n:")) != NULL) {
            Res_p ++;
            p = Res_GetLine();
            if (p)
                p = strtok(p, " \t\r");
            /*printf("cmp %s vs %s\n",key,p);*/
            if (p == NULL || *p == 0)
                continue;
            strupr(p);
            if (key[1]) {
                /* �ϊ�����������΃��X�|���X�Ɠ������������� */
                if (Res_StrEqu(key, p)) {
                    if ((p = strstr(Res_p, "\n:")) != NULL) {
                        *p = '\0';
                    }
                    return Res_GetFmts();
                }
            } else {    /* �����L�[���Ȃ���΁A�ꗗ�\�� */
                printf("\t%s\n",p);
            }
        }
        if (key[1])
            printf("%s �ɂ� %s �͒�`����Ă��Ȃ�\n", Res_nm, key);
        exit(1);
        return false;
    }
};



class App {
public:
    App()
        : outFp_(NULL)
        , opts_(convFmt_)
        , resCfgFile_(opts_, convFmt_, filenameList_, beforeStrList_, afterStrList_, fmtBuf_)
    {
        memset(abxName_, 0, sizeof(abxName_));
        memset(fmtBuf_, 0, sizeof(fmtBuf_));
    }


    int main(int argc, char *argv[]) {
        opts_.setExename(FIL_BaseName(argv[0]));    /*�A�v���P�[�V������*/
		if (argc < 2) {
            opts_.Usage();
			return 1;
		}

        FIL_ChgExt(strcpy(abxName_, argv[0]), "cfg");

        if (scanOpts(argc, argv) == false)
            return 1;
        if (outputText() == false)
            return 1;
        if (execBat() == false)
            return 1;
        return 0;
    }


private:

    bool scanOpts(int argc, char *argv[]) {
        /* �R�}���h���C���̃I�v�V����/�t�@�C����/�ϊ�������, �擾 */
        int f = 0;
        for (int i = 1; i < argc; ++i) {
            char* p = argv[i];
           LLL1:
            if (f) {
                if (fmtBuf_[0])
                    strcat(fmtBuf_, " ");
                strcat(fmtBuf_, p);
                if (f == 1) {
                    p = strrchr(fmtBuf_, '\'');
                    if (p) {
                        f = 0;
                        *p = 0;
                    }
                }
            } else if (*p == '\'') {
                f ^= 1;
                p++;
                if (*p)
                    goto LLL1;

            } else if (*p == '=') {
                f = 2;
                p++;
                if (*p)
                    goto LLL1;

            } else if (*p == '-') {
                if (opts_.scan(p) == false)
                    return false;

            } else if (*p == '@') {
                resCfgFile_.GetResFile(p+1);

            } else if (*p == '+') {
                ++p;
                if (*p == '\\' || *p == '/' || p[1] == ':') {
                    FIL_FullPath(p, abxName_);
                } else {
                    char fbuf[FIL_NMSZ];
                    strcpy(fbuf, argv[0]);
                    strcpy(FIL_BaseName(fbuf), p);
                    FIL_FullPath(fbuf, abxName_);
                }
                FIL_AddExt(abxName_, "cfg");

            } else if (*p == ':') {
                if (p[1] == '#') {
                    printf(":#�Ŏn�܂镶����͎w��ł��܂���i%s�j\n",p);
                    exit(1);
                }
                if (resCfgFile_.GetCfgFile(abxName_, p) == false)
                    return false;
            } else if (*p == '$') {
                if (p[1] >= '1' && p[1] <= '9' && p[2] == '=') {
                    unsigned    no  = p[1] - '0';
                    char const* s   = p + 3;
                 #if 1
                    convFmt_.setVar(no, s, strlen(s));
                 #else
                    strcpy(convFmt_.var_[p[1]-'0'], s);
                 #endif
                }
            } else {
                filenameList_.push_back(p);
            }
        }

        /* �o�b�`���s�̂Ƃ� */
        if (opts_.batFlg_) {
            strcpy(opts_.outname_, convFmt_.tmpDir());
            strcat(opts_.outname_,"\\_abx_tmp.bat");
        }

        if (opts_.fattr_ == 0) {     /* �f�t�H���g�̃t�@�C���������� */
            opts_.fattr_ = 0x127;
        }
        convFmt_.setUpLwrFlag(opts_.upLwrFlg_);

        /* �ϊ������񒲐� */
        if (fmtBuf_[0] == '\0') {
            if (opts_.recFlg_)
                strcpy(fmtBuf_, "$F\n");
            else
                strcpy(fmtBuf_, "$c\n");
        }
        if (strchr(fmtBuf_, '\n') == NULL)
            strcat(fmtBuf_, "\n");
        convFmt_.setFmtStr(fmtBuf_);

        convFmt_.setAutoWq(opts_.autoWqFlg_);       /* $f���Ŏ����ŗ��[��"��t�����郂�[�h. */

        return true;
    }


    bool outputText() {
        /* �o�̓t�@�C���ݒ� */
        if (opts_.outname_[0]) {
            outFp_ = fopenE(opts_.outname_, "wt");
        } else {
            outFp_ = stdout;
        }

        if (opts_.batEx_) {                 /* �o�b�`���s�p�ɐ擪�� echo off ��u�� */
            fprintf(outFp_, "@echo off\n");
        }
        /* ���O�o�̓e�L�X�g */
        for (StrList::iterator ite = beforeStrList_.begin(); ite != beforeStrList_.end(); ++ite) {
            convFmt_.WriteLine0(outFp_, ite->c_str());
        }

        /* -u && -s �Ȃ�΁A�w��t�@�C������啶���� */
        if (opts_.upLwrFlg_ && opts_.sort_) {
            opts_.sort_ |= 0x8000; /* �t�@�C���������Č��������t�@�C������������������w�� */
            for (StrList::iterator ite = filenameList_.begin(); ite != filenameList_.end(); ++ite) {
                StrLwrN(&(*ite)[0], ite->size());
            }
        }

        /* ���s */
        if (opts_.renbanEnd_ == 0) {
            for (StrList::iterator ite = filenameList_.begin(); ite != filenameList_.end(); ++ite) {
                char const* p = ite->c_str();
                convFmt_.setLineBuf(p);
                if (*p != '\\' && *p != '/' && p[1] != ':') {   /* ���΃p�X�̂Ƃ� */
                    strcpy(opts_.iname_, p);
                    strcpy(abxName_, opts_.ipath_);
                } else {                                        /* �t���p�X�̂Ƃ� */
                    strcpy(abxName_, p);
                    char const* s = STREND(p);
                    if (*s == '/' || *s == '\\')
                        strcat(abxName_, "*");
                }
                FIL_AddExt(abxName_, opts_.dfltExtp_);      /* �f�t�H���g�g���q�t�� */
                /* ���ۂ̃t�@�C�������Ƃ̐��� */
                fsrh_.FindAndDo(abxName_, opts_.fattr_, opts_.recFlg_,
                    opts_.zenFlg_, opts_.topN_, opts_.sort_, opts_.knjChk_, opts_.noFindFile_,
                    opts_.szmin_, opts_.szmax_, opts_.dtmin_, opts_.dtmax_, outFp_, &convFmt_, &ConvFmt::Write);
            }
        } else {
            opts_.noFindFile_ = 1;
            /* �A�Ԑ����ł̏����l�ݒ� */
            for (int num = opts_.renbanStart_; num < opts_.renbanEnd_; ++num) {
                convFmt_.setNum(num);
                sprintf(abxName_, "%d", num);
                convFmt_.setLineBuf( abxName_ );
                /* ���ۂ̃t�@�C�������Ƃ̐��� */
                fsrh_.FindAndDo(abxName_, opts_.fattr_, opts_.recFlg_,
                    opts_.zenFlg_, opts_.topN_, opts_.sort_, opts_.knjChk_, opts_.noFindFile_,
                    opts_.szmin_, opts_.szmax_, opts_.dtmin_, opts_.dtmax_, outFp_, &convFmt_, &ConvFmt::Write);
            }
        }

        /* ����o�̓e�L�X�g */
        for (StrList::iterator ite = afterStrList_.begin(); ite != afterStrList_.end(); ++ite) {
            convFmt_.WriteLine0(outFp_, ite->c_str());
        }
        if (opts_.batEx_)  /* �o�b�`���s�p�Ƀt�@�C������:END��t������ */
            fprintf(outFp_, ":END\n");

        if (opts_.outname_[0]) {
            fclose(outFp_);
            outFp_ = NULL;
        }

        return true;
    }

    bool execBat() {
        /* �o�b�`���s�̂Ƃ� */
        if (opts_.batFlg_) {
            char* p = getenv("COMSPEC");
            spawnl( _P_WAIT, p, p, "/c", opts_.outname_, NULL);
        }
        return true;
    }


private:
    char        abxName_[FIL_NMSZ];         /* ���O work */
    ConvFmt     convFmt_;
    Opts        opts_;
    ResCfgFile  resCfgFile_;
    FSrh        fsrh_;
    StrList     filenameList_;
    StrList     beforeStrList_;
    StrList     afterStrList_;
    char        fmtBuf_[FMTSIZ];           /* �ϊ�����������߂� */
    FILE*       outFp_;
};


/** start application
 */
int main(int argc, char *argv[]) {
    static App app;
    return app.main(argc, argv);
}