#include <unistd.h>
#include <RUtil2.h>
#include <RFNL.h>
#include <CVEDSP2.h>
#include <RUCE.h>
#include "../Commons.h"

#define Version "1.0.0.0"

static void PrintUsage()
{
    fprintf(stderr, "Usage: rudb2oct [-h harmonicnum] [-n framenum]\n"
                    "                [-t position] [-r] [-a] [-f] [-p]\n"
                    "                [-v] rudbfile\n");
}

#define PTYPE_NUM 0
#define PTYPE_VOT 1
#define PTYPE_VHALF 2

static int HarmonicNum;
static int FrameNum;
static int Position;
static int FlagR, FlagA, FlagF, FlagP;
static int PType;

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
    HarmonicNum = 10;
    FrameNum = 100;
    Position = 0;
    FlagR = 0;
    FlagA = 0;
    FlagF = 0;
    FlagP = 0;
    PType = PTYPE_NUM;
    
    char* CInPath;
    
    int c;
    while((c = getopt(ArgN, Arg, "h:n:t:rafpv")) != -1)
    {
        switch(c)
        {
            case 'h':
                HarmonicNum = atoi(optarg);
            break;
            case 'n':
                FrameNum = atoi(optarg);
            break;
            case 't':
                if     (! strcmp(optarg, "vot"))
                    PType = PTYPE_VOT;
                else if(! strcmp(optarg, "half"))
                    PType = PTYPE_VHALF;
                else
                {
                    PType = PTYPE_NUM;
                    Position = atoi(optarg);
                }
            break;
            case 'r':
                FlagR = 1;
            break;
            case 'a':
                FlagA = 1;
            break;
            case 'f':
                FlagF = 1;
            break;
            case 'p':
                FlagP = 1;
            break;
            case 'v':
                printf("Rocaloid RUDB2Oct version " Version "\n");
                return 1;
            break;
            case '?':
                PrintUsage();
                return 1;
            default:
                abort();
        }
    }
    
    if(optind > ArgN - 1)
    {
        fprintf(stderr, "[Error] Missing argument 'rudbfile'.\n");
        PrintUsage();
        return 1;
    }
    if(optind < ArgN - 1)
    {
        fprintf(stderr, "[Error] Redundant argument '%s'.\n", Arg[optind + 1]);
    }
    
    if(! FlagA && ! FlagF)
    {
        fprintf(stderr, "[Error] At least one among [-a] [-f] should be "
            "specified.\n");
        return 1;
    }
    
    CInPath = Arg[optind];
    
    String InPath, DirPath, RotoPath, BaseName, UnitName;
    RNew(String, & InPath, & DirPath, & RotoPath, & BaseName, & UnitName);
    String_SetChars(& InPath, CInPath);
    String_FromChars(Dot, ".");
    
    RUCE_DB_Entry Entry;
    RUCE_DB_Entry_Ctor(& Entry);
    
    BaseFromFilePath(& BaseName, & InPath);
    DirFromFilePath(& DirPath, & InPath);
    int DotPos = String_InStr(& BaseName, & Dot);
    Left(& UnitName, & BaseName, DotPos);
    String_From(& RotoPath, & DirPath);
    String_JoinChars(& RotoPath, "/Roto.json");
    
    if(RUCE_DB_LoadEntry(& Entry, & UnitName, & DirPath, & RotoPath) != 1)
    {
        fprintf(stderr, "[Error] Cannot open '%s'.\n", CInPath);
        return 1;
    }
    
    int StartIndex, EndIndex;
    int VOTIndex = FirstIndexFromEntry(& Entry, Entry.VOT);
    
    if(! FlagR && PType == PTYPE_NUM)
    {
        StartIndex = FirstIndexFromEntry(& Entry, Position);
        EndIndex = Entry.FrameList_Index;
    }
    if(  FlagR && PType == PTYPE_NUM)
    {
        StartIndex = 0;
        EndIndex = FirstIndexFromEntry(& Entry, Position);
    }
    if(! FlagR && PType == PTYPE_VOT)
    {
        StartIndex = VOTIndex;
        EndIndex = Entry.FrameList_Index;
    }
    if(  FlagR && PType == PTYPE_VOT)
    {
        StartIndex = 0;
        EndIndex = VOTIndex;
    }
    if(! FlagR && PType == PTYPE_VHALF)
    {
        StartIndex = (VOTIndex + Entry.FrameList_Index) / 2;
        EndIndex = Entry.FrameList_Index;
    }
    if(  FlagR && PType == PTYPE_VHALF)
    {
        StartIndex = VOTIndex;
        EndIndex = (VOTIndex + Entry.FrameList_Index) / 2;
    }
    
    Array_Gtor(RUCE_DB_Frame, InterpList);
    Array_ObjResize(RUCE_DB_Frame, InterpList, FrameNum);
    InterpList_Index = FrameNum - 1;
    
    int MaxSize = MaxSizeFromEntry(& Entry);
    int i;
    for(i = 0; i < FrameNum; i ++)
    {
        Real IRatio = (Real)i / FrameNum;
        Real FIndex = (1.0 - IRatio) * StartIndex + IRatio * EndIndex;
        int  IntIndex = FIndex;
        Real Ratio = FIndex - IntIndex;
        InterpFetchDBFrame(& InterpList[i], & Entry, IntIndex, Ratio, MaxSize);
    }
    
    if(FlagP)
    {
        printf("# name: %s_position\n", String_GetChars(& UnitName));
        printf("# type: matrix\n");
        printf("# ndims: 2\n");
        printf("1 %d\n", FrameNum);
        for(i = 0; i < FrameNum; i ++)
        {
            printf("%d\n", InterpList[i].Position);
        }
        printf("\n");
    }
    
    #define GenericPrint(PostFix, Field, Transform) \
        printf("# name: %s_" PostFix "\n", String_GetChars(& UnitName)); \
        printf("# type: matrix\n"); \
        printf("# rows: %d\n", FrameNum); \
        printf("# columns: %d\n", HarmonicNum); \
        for(i = 0; i < FrameNum; i ++) \
        { \
            int j; \
            if(HarmonicNum < MaxSize) \
                for(j = 0; j < HarmonicNum; j ++) \
                    printf("%.4f ", Transform(InterpList[i].Field[j])); \
            else \
            { \
                for(j = 0; j < MaxSize; j ++) \
                    printf("%.4f ", Transform(InterpList[i].Field[j])); \
                for(; j < FrameNum; j ++) printf("0 "); \
            } \
            printf("\n"); \
        } \
        printf("\n")
    
    if(FlagA)
    {
        GenericPrint("ampl", Ampl, log);
    }
    if(FlagF)
    {
        GenericPrint("freq", Freq, +);
    }
    
    Array_ObjDtor(RUCE_DB_Frame, InterpList);
    Array_Dtor(RUCE_DB_Frame, InterpList);
    
    RDelete(& InPath, & DirPath, & RotoPath, & Entry, & BaseName, & UnitName,
            & Dot);
    return 0;
}

