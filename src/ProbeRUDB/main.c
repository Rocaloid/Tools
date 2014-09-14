#include <unistd.h>
#include <RUtil2.h>
#include <RFNL.h>
#include <CVEDSP2.h>
#include <RUCE.h>
#include "../Commons.h"

#define Version "0.1.0.0"

static void PrintUsage()
{
    fprintf(stderr, "Usage: proberudb [-u] rudbfile\n");
}

static int GenOto;

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
    GenOto = 0;
    char* CInPath;
    
    int c;
    while((c = getopt(ArgN, Arg, "u")) != -1)
    {
        switch(c)
        {
            case 'u':
                GenOto = 1;
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
        
    CInPath = Arg[optind];
    
    String InPath, DirPath, BaseName, UnitName;
    RNew(String, & InPath, & DirPath, & BaseName, & UnitName);
    String_SetChars(& InPath, CInPath);
    String_FromChars(Dot, ".");
    
    RUCE_DB_Entry Entry;
    RUCE_DB_Entry_Ctor(& Entry);
    
    BaseFromFilePath(& BaseName, & InPath);
    DirFromFilePath(& DirPath, & InPath);
    int DotPos = String_InStrRev(& BaseName, & Dot);
    Left(& UnitName, & BaseName, DotPos);
    
    if(RUCE_DB_LoadEntry(& Entry, & UnitName, & DirPath) != 1)
    {
        fprintf(stderr, "[Error] Cannot open '%s'.\n", CInPath);
        return 1;
    }
    
    /*
    int StartIndex, EndIndex;
    int VOTIndex = FirstIndexFromEntry(& Entry, Entry.VOT);
    */
    
    if(! GenOto)
    {
        RUCE_DB_PrintEntry(& Entry);
    }else
    {
        //Generate UTAU oto configuration
        printf("%s.wav=,0,%f,0,%f,%f\n", String_GetChars(& UnitName),
            Entry.InvarLeft * 1000.0, Entry.VOT * 1000.0, Entry.VOT * 333.3);
    }
    
    RDelete(& InPath, & DirPath, & Entry, & BaseName, & UnitName,
            & Dot);
    return 0;
}

