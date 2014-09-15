#include <unistd.h>
#include <RUtil2.h>
#include <RFNL.h>
#include <CVEDSP2.h>
#include <RUCE.h>
#include "../Commons.h"

#define Version "0.1.0.0"

/*
    editrudb param rudbfile [-t VOT] [-r InvarRight] [-l InvarLeft]
    editrudb setnoise rudbfile wavfile
    editrudb mix rudbfile rudbfile(source) [-a positionA] [-b positionB]
             [-r radius]
*/
static void PrintUsage()
{
    fprintf(stderr, "Usage: editrudb command rudbfile [Options]\n");
}

#define DO_PARAM        0
#define DO_SETNOISE     1
#define DO_MIX          2

static int  Param_Operation;
static int  Set_t, Set_r, Set_l, Set_a, Set_b;
static Real Param_t;
static Real Param_r;
static Real Param_l;
static Real Param_a;
static Real Param_b;

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
    
    Set_t = Set_r = Set_l = Set_a = Set_b = 0;
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
    
    char* OptFilter = Param_Operation == DO_PARAM    ? "t:r:l:v" :
                      Param_Operation == DO_SETNOISE ? "v"       :
                      Param_Operation == DO_MIX      ? "a:b:r:v" : "v";
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
            case 't':
                Param_t = atof(optarg);
                Set_t   = 1;
            break;
            case 'r':
                Param_r = atof(optarg);
                Set_r   = 1;
            break;
            case 'l':
                Param_l = atof(optarg);
                Set_l   = 1;
            break;
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
        
        if(Param_Operation == DO_MIX)
        switch(c)
        {
            case 'a':
                Param_a = atof(optarg);
                Set_a   = 1;
            break;
            case 'b':
                Param_b = atof(optarg);
                Set_b   = 1;
            break;
            case 'r':
                Param_r = atof(optarg);
                Set_r   = 1;
            break;
            case '?':
                PrintUsage();
                return 1;
            default:
                abort();
        }
    }
    
    int NArgReq = Param_Operation == DO_PARAM    ? 2 :
                  Param_Operation == DO_SETNOISE ? 3 :
                  Param_Operation == DO_MIX      ? 3 : 0;
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
        if(Set_r)
            Entry.InvarRight = Param_r;
        if(Set_l)
            Entry.InvarLeft  = Param_l;
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

