#include <unistd.h>
#include <RUtil2.h>
#include <RFNL.h>
#include <CVEDSP2.h>
#include "../Commons.h"

#define Version "1.0.0.0"

static void PrintUsage()
{
    fprintf(stderr, "Usage: pcompdiff [-u] [-l level] [-v] inwave outwave\n");
}

static int UnFlag;
static int Level;

static int PreCompressDiff(Int16* Dest, int Size)
{
    int i;
    Int16 Temp = Dest[0], Temp2;
    for(i = 1; i < Size; i ++)
    {
        Temp2 = Dest[i];
        if((Int32)Dest[i] - Temp > 32767 || (Int32)Dest[i] - Temp < -32767)
            return 0;
        Dest[i] -= Temp;
        Temp = Temp2;
    }
    return 1;
}

static int DePreCompressDiff(Int16* Dest, int Size)
{
    int i;
    for(i = 1; i < Size; i ++)
    {
        Dest[i] += Dest[i - 1];
    }
    return 1;
}

int main(int ArgN, char** Arg)
{
    UnFlag = 0;
    Level  = 1;
    
    char* CInPath, *COutPath;
    
    int c;
    while((c = getopt(ArgN, Arg, "ul:v")) != -1)
    {
        switch(c)
        {
            case 'u':
                UnFlag = 1;
            break;
            case 'l':
                Level = atoi(optarg);
            break;
            case 'v':
                printf("Rocaloid PrecompDiff version " Version "\n");
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
        fprintf(stderr, "[Error] Missing argument 'inwave', 'outwave'.\n");
        PrintUsage();
        return 1;
    }
    if(optind > ArgN - 2)
    {
        fprintf(stderr, "[Error] Missing argument 'outwave'.\n");
        PrintUsage();
        return 1;
    }
    if(optind < ArgN - 2)
    {
        fprintf(stderr, "[Error] Redundant argument '%s'.\n", Arg[optind + 2]);
    }
    
    CInPath = Arg[optind];
    COutPath = Arg[optind + 1];
    
    String InPath, OutPath;
    RNew(String, & InPath, & OutPath);
    String_SetChars(& InPath, CInPath);
    String_SetChars(& OutPath, COutPath);
    
    WaveFile IOFile;
    WaveFile_Ctor(& IOFile);
    
    if(WaveFile_Open(& IOFile, & InPath) != 1)
    {
        fprintf(stderr, "[Error] Cannot open '%s'.\n", CInPath);
        return 1;
    }
    
    int Size = IOFile.Header.DataNum;
    Int16* Data = RAlloc_Int16(Size);
    WaveFile_FetchAll(& IOFile, (char*)Data);
    WaveFile_Close(& IOFile);
    
    int i;
    for(i = 0; i < Level; i ++)
    {
        if(! UnFlag)
        {
            if(! PreCompressDiff(Data, Size))
            {
                fprintf(stderr, "[Error] %dth precompress failed. "
                                "Please try smaller precompress level.\n",
                                i + 1);
                return 1;
            }
        }else
            DePreCompressDiff(Data, Size);
    }
    
    if(WaveFile_Save(& IOFile, & OutPath) != 1)
    {
        fprintf(stderr, "[Error] Cannot save to '%s'.\n", COutPath);
        return 1;
    }
    
    WaveFile_WriteAll(& IOFile, (char*)Data, Size * 2);
    RFree(Data);
    
    WaveFile_FinishWrite(& IOFile);
    WaveFile_Close(& IOFile);
    RDelete(& InPath, & OutPath, & IOFile);
    return 0;
}

