#include <unistd.h>
#include <RUtil2.h>
#include <RFNL.h>
#include <CVEDSP2.h>
#include <RUCE.h>
#include "../Commons.h"

#define Version "0.1.1.0"

static void PrintUsage()
{
    fprintf(stderr, "Usage: proberudb [-u] [-p] rudbfile\n");
}

static int GenOto;
static int ShowProperties;

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

int main(int ArgN, char** Arg)
{
    GenOto = ShowProperties = 0;
    char* CInPath;
    
    int c;
    while((c = getopt(ArgN, Arg, "up")) != -1)
    {
        switch(c)
        {
            case 'u':
                GenOto = 1;
            break;
            case 'p':
                ShowProperties = 1;
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
    
    String InPath, BaseName, UnitName, RUDBName;
    RNew(String, & InPath, & BaseName, & UnitName, & RUDBName);
    String_SetChars(& InPath, CInPath);
    String_FromChars(Dot, ".");
    
    RUCE_DB_Entry Entry;
    RUCE_DB_Entry_Ctor(& Entry);
    
    BaseFromFilePath(& BaseName, & InPath);
    int DotPos = String_InStrRev(& BaseName, & Dot);
    Left(& UnitName, & BaseName, DotPos);
    String_From(& RUDBName, & UnitName);
    String_JoinChars(& RUDBName, ".rudb");
    
    if(RUCE_RUDB_Load(& Entry, & RUDBName) != 1)
    {
        fprintf(stderr, "[Error] Cannot open '%s'.\n", CInPath);
        return 1;
    }
    
    /*
    int StartIndex, EndIndex;
    int VOTIndex = FirstIndexFromEntry(& Entry, Entry.VOT);
    */
    
    if(! (GenOto || ShowProperties))
    {
        RUCE_DB_PrintEntry(& Entry);
    }else if(GenOto)
    {
        //Generate UTAU oto configuration
        printf("%s.wav=,0,%f,0,%f,%f\n", String_GetChars(& UnitName),
            Entry.InvarLeft * 1000.0, Entry.VOT * 1000.0, Entry.VOT * 333.3);
    }else if(ShowProperties)
    {
        //Show RUDB properties
        printf("VOT = %f sec.\n", Entry.VOT);
        printf("SOT = %f sec.\n", Entry.SOT);
        printf("InvarLeft = %f sec.\n", Entry.InvarLeft);
        printf("InvarRight = %f sec.\n", Entry.InvarRight);
        
        printf("Consonant(residual) size = %d samples.\n", Entry.WaveSize);
        printf("Sample Rate = %dHz.\n", Entry.Samprate);
        printf("Hop size = %d samples.\n", Entry.HopSize);
        printf("Noise size = %d samples.\n", Entry.NoizSize);
        printf("Frame num = %d.\n", Entry.FrameList_Index + 1);
    }
    
    
    RDelete(& InPath, & Entry, & BaseName, & UnitName, & RUDBName,
            & Dot);
    return 0;
}

