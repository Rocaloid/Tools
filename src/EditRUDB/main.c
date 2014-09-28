#include <unistd.h>
#include <RUtil2.h>
#include <RFNL.h>
#include <CVEDSP2.h>
#include <RUCE.h>
#include "../Commons.h"

#define Version "0.1.2.0"

/*
    editrudb param rudbfile [-t VOT] [-s SOT] [-r InvarRight] [-l InvarLeft]
             setnoise rudbfile wavfile
             gainnoise rudbfile [-g gain multiple]
             notchnoise rudbfile [-s SOT] [-r radius] [-h central height]
             mix rudbfile rudbfile(source) [-a positionA] [-b positionB]
             [-r radius]
*/
static void PrintUsage()
{
    fprintf(stderr, "Usage: editrudb command rudbfile [Options]\n");
}

#define DO_PARAM        0
#define DO_SETNOISE     1
#define DO_MIX          2
#define DO_NOTCHNOISE   3
#define DO_GAINNOISE    4

static int  Param_Operation;
static int  Set_t, Set_s, Set_r, Set_l, Set_a, Set_b, Set_h, Set_g;
static Real Param_t;
static Real Param_s;
static Real Param_r;
static Real Param_l;
static Real Param_a;
static Real Param_b;
static Real Param_h;
static Real Param_g;

static String RUDBPath;
static String WavPath;
static String RUDBPath2;

static int FirstIndexFromEntry(RUCE_DB_Entry* Sorc, int Position)
{
    int i;
    for(i = 0; i <= Sorc -> FrameList_Index; i ++)
        if(Sorc -> FrameList[i].Position > Position)
            break;
    return i;
}

static int MaxSizeFromEntry(RUCE_DB_Entry* Sorc)
{
    int i;
    int MSize = 0;
    for(i = 0; i <= Sorc -> FrameList_Index; i ++)
        if(Sorc -> FrameList[i].Freq_Index > MSize)
            MSize = Sorc -> FrameList[i].Freq_Index;
    return MSize + 1;
}

#define max(a, b) ((a) > (b) ? (a) : (b))
static void InterpFetchDBFrame(RUCE_DB_Frame* Dest, RUCE_DB_Entry* Sorc,
    int LIndex, Real Ratio, int MaxSize)
{
    RUCE_DB_Frame* L, *H;
    L = & Sorc -> FrameList[LIndex];
    H = & Sorc -> FrameList[LIndex + 1 > Sorc -> FrameList_Index ?
                                         Sorc -> FrameList_Index : LIndex + 1];
    
    int LSize = L -> Freq_Index + 1;
    int HSize = H -> Freq_Index + 1;
    
    Array_Resize(float, Dest -> Freq, MaxSize);
    Array_Resize(float, Dest -> Ampl, MaxSize);
    CDSP2_VSet_Float(Dest -> Freq, 0, MaxSize);
    CDSP2_VSet_Float(Dest -> Ampl, 0, MaxSize);
    
    CDSP2_VCMul_Float(Dest -> Freq, L -> Freq, 1.0 - Ratio, LSize);
    CDSP2_VCMul_Float(Dest -> Ampl, L -> Ampl, 1.0 - Ratio, LSize);
    CDSP2_VFCMA_Float(Dest -> Freq, H -> Freq, Ratio      , HSize);
    CDSP2_VFCMA_Float(Dest -> Ampl, H -> Ampl, Ratio      , HSize);
    
    Dest -> Position = (1.0 - Ratio) * L -> Position + Ratio * H -> Position;
}

int main(int ArgN, char** Arg)
{
    RNew(String, & RUDBPath, & WavPath, & RUDBPath2);
    
    Set_t = Set_s = Set_r = Set_l = Set_a = Set_b = Set_h = Set_g = 0;
    int c;
    if(ArgN < 2)
    {
        fprintf(stderr, "[Error] Too few arguments.\n");
        PrintUsage();
        return 1;
    }
    
    if      (! strcmp(Arg[1], "param"))
    {
        Param_Operation = DO_PARAM;
    }else if(! strcmp(Arg[1], "setnoise"))
    {
        Param_Operation = DO_SETNOISE;
    }else if(! strcmp(Arg[1], "notchnoise"))
    {
        Param_Operation = DO_NOTCHNOISE;
    }else if(! strcmp(Arg[1], "gainnoise"))
    {
        Param_Operation = DO_GAINNOISE;
    }else if(! strcmp(Arg[1], "mix"))
    {
        Param_Operation = DO_MIX;
    }else if(! strcmp(Arg[1], "-v"))
    {
        printf("Rocaloid EditRUDB version " Version "\n");
        return 1;
    }else
    {
        fprintf(stderr, "[Error] Invalid operation name '%s'.\n", Arg[1]);
        return 1;
    }
    
    #define CaseF(Id, Char) case Char: \
            _C1(Param_, Id) = atof(optarg); \
            _C1(Set_, Id) = 1; \
        break
    
    char* OptFilter = Param_Operation == DO_PARAM      ? "t:r:l:v" :
                      Param_Operation == DO_SETNOISE   ? "v"       :
                      Param_Operation == DO_GAINNOISE  ? "g:v"     :
                      Param_Operation == DO_NOTCHNOISE ? "s:r:h:v" :
                      Param_Operation == DO_MIX        ? "a:b:r:v" : "v";
    while((c = getopt(ArgN, Arg, OptFilter)) != -1)
    {
        if(c == 'v')
        {
            printf("Rocaloid EditRUDB version " Version "\n");
            return 1;
        }
        
        if(Param_Operation == DO_PARAM)
        switch(c)
        {
            CaseF(t, 't');
            CaseF(s, 's');
            CaseF(r, 'r');
            CaseF(l, 'l');
            case '?':
                PrintUsage();
                return 1;
            default:
                abort();
        }
        
        
        if(Param_Operation == DO_SETNOISE)
        switch(c)
        {
            case '?':
                PrintUsage();
                return 1;
            default:
                abort();
        }
        
        if(Param_Operation == DO_GAINNOISE)
        switch(c)
        {
            CaseF(g, 'g');
            case '?':
                PrintUsage();
                return 1;
            default:
                abort();
        }
        
        if(Param_Operation == DO_NOTCHNOISE)
        switch(c)
        {
            CaseF(s, 's');
            CaseF(r, 'r');
            CaseF(h, 'h');
            case '?':
                PrintUsage();
                return 1;
            default:
                abort();
        }
        
        if(Param_Operation == DO_MIX)
        switch(c)
        {
            CaseF(a, 'a');
            CaseF(b, 'b');
            CaseF(r, 'r');
            case '?':
                PrintUsage();
                return 1;
            default:
                abort();
        }
    }
    
    int NArgReq = Param_Operation == DO_PARAM      ? 2 :
                  Param_Operation == DO_SETNOISE   ? 3 :
                  Param_Operation == DO_GAINNOISE  ? 2 :
                  Param_Operation == DO_NOTCHNOISE ? 2 :
                  Param_Operation == DO_MIX        ? 3 : 0;
    if(optind > ArgN - NArgReq)
    {
        fprintf(stderr, "[Error] Missing argument.\n");
        PrintUsage();
        return 1;
    }
    if(optind < ArgN - NArgReq)
    {
        fprintf(stderr, "[Error] Redundant argument '%s'.\n", Arg[optind + 1]);
    }
    
    String_SetChars(& RUDBPath, Arg[optind + 1]);
    if(Param_Operation == DO_SETNOISE)
        String_SetChars(& WavPath, Arg[optind + 2]);
    if(Param_Operation == DO_GAINNOISE)
        String_SetChars(& WavPath, Arg[optind + 1]);
    if(Param_Operation == DO_NOTCHNOISE)
        String_SetChars(& WavPath, Arg[optind + 1]);
    if(Param_Operation == DO_MIX)
        String_SetChars(& RUDBPath2, Arg[optind + 2]);
    
    RUCE_DB_Entry Entry;
    RUCE_DB_Entry_Ctor(& Entry);
    
    if(RUCE_RUDB_Load(& Entry, & RUDBPath) != 1)
    {
        fprintf(stderr, "[Error] Cannot open '%s'.\n",
            String_GetChars(& RUDBPath));
        return 1;
    }
    
    if(Param_Operation == DO_PARAM)
    {
        if(Set_t)
            Entry.VOT = Param_t;
        if(Set_s)
            Entry.SOT = Param_s;
        if(Set_r)
            Entry.InvarRight = Param_r;
        if(Set_l)
            Entry.InvarLeft  = Param_l;
    }
    
    if(Param_Operation == DO_GAINNOISE)
    {
        float SOT = Entry.SOT;
        float Radius = 0.05;
        
        int Center = SOT * Entry.Samprate;
        int Length = Radius * Entry.Samprate;
        int i;
        
        #undef Wave
        CDSP2_VCMul_Float(Entry.Wave, Entry.Wave, Param_g, Center);
        for(i = 0; i < Length; i ++)
            Entry.Wave[i + Center] *= (Real)i / Length * (1.0 - Param_g)
                                    + Param_g;
        #define Wave _C(CDSP2_Wave_, Real)
    }
    
    if(Param_Operation == DO_NOTCHNOISE)
    {
        float SOT, Radius, Height;
        if(Set_s)
            SOT = Param_s;
        else
            SOT = Entry.SOT;
        if(Set_r)
            Radius = Param_r;
        else
            Radius = 0.02;
        if(Set_h)
            Height = Param_h;
        else
            Height = 0.2;
        int Center = SOT * Entry.Samprate;
        int Length = Radius * Entry.Samprate;
        int i = 0;
        
        if(Center - Length <= 0)
            i = Length - Center;
        #undef Wave
        for(; i < Length; i ++)
            Entry.Wave[i + Center - Length] *= (Real)i / Length 
                                             * (Height - 1.0) + 1.0;
        for(i = 0; i < Length; i ++)
            Entry.Wave[i + Center] *= (Real)i / Length 
                                    * (1.0 - Height) + Height;
        #define Wave _C(CDSP2_Wave_, Real)
    }
    
    if(RUCE_RUDB_Save(& Entry, & RUDBPath) != 1)
    {
        fprintf(stderr, "[Error] Cannot save '%s'.\n",
            String_GetChars(& RUDBPath));
        return 1;
    }
    
    RDelete(& RUDBPath, & WavPath, & RUDBPath2);
    RDelete(& Entry);
    /*
    String InPath, DirPath, BaseName, UnitName;
    RNew(String, & InPath, & DirPath, & BaseName, & UnitName);
    String_SetChars(& InPath, CInPath);
    String_FromChars(Dot, ".");
    
    BaseFromFilePath(& BaseName, & InPath);
    DirFromFilePath(& DirPath, & InPath);
    int DotPos = String_InStrRev(& BaseName, & Dot);
    Left(& UnitName, & BaseName, DotPos);
    
    if(RUCE_DB_LoadEntry(& Entry, & UnitName, & DirPath) != 1)
    {
        fprintf(stderr, "[Error] Cannot open '%s'.\n", CInPath);
        return 1;
    }
    */
    /*
    int StartIndex, EndIndex;
    int VOTIndex = FirstIndexFromEntry(& Entry, Entry.VOT);
    */
    /*
    if(! GenOto)
    {
        RUCE_DB_PrintEntry(& Entry);
    }else
    {
        //Generate UTAU oto configuration
        printf("%s.wav=,0,%f,0,%f,%f\n", String_GetChars(& UnitName),
            Entry.InvarLeft * 1000.0, Entry.VOT * 1000.0, Entry.VOT * 333.3);
    }
    */
    /*
    RDelete(& InPath, & DirPath, & Entry, & BaseName, & UnitName,
            & Dot);*/
    return 0;
}

