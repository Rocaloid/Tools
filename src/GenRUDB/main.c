#include <stdio.h>
#include <unistd.h>

#include <RUtil2.h>
#include <CVEDSP2.h>
#include <CVESVP.h>
#include <RUCE.h>

#include "Common.h"

static void PrintUsage()
{
    fprintf(stderr, "Usuage: genrudb [-n unitname] [-r]\n"
                    "                [-u freq] [-l freq] [-m method]\n"
                    "                [-s freq] [-h hopsize] [-z size]\n"
                    "                [-w window] [-t position] [-o offset]\n"
                    "                [-i threshold] [-v] [-V] rotofile\n");
}

int main(int ArgN, char** Arg)
{
    CRotoFile = NULL;
    CUnitName = NULL;
    ReadOnlyFlag = 0;
    UFundFreq = 700;
    LFundFreq = 80;
    CFundMethod = "YIN";
    USinuFreq = 10000;
    HopSize = 256;
    WinSize = 2048;
    CWindow = "hanning";
    VOT = 0;
    VOTFlag = 0;
    Offset = 500;
    InvarThreshold = 0.004;
    VerboseFlag = 0;
    
    int c;
    while((c = getopt(ArgN, Arg, "n:ru:l:m:s:h:z:w:t:o:i:Vv")) != -1)
    {
        switch(c)
        {
            case 'n':
                CUnitName = optarg;
            break;
            case 'r':
                ReadOnlyFlag = 1;
            break;
            case 'u':
                UFundFreq = atof(optarg);
            break;
            case 'l':
                LFundFreq = atof(optarg);
            break;
            case 'm':
                CFundMethod = optarg;
            break;
            case 's':
                USinuFreq = atof(optarg);
            break;
            case 'h':
                HopSize = atoi(optarg);
            break;
            case 'z':
                WinSize = atoi(optarg);
            break;
            case 'w':
                CWindow = optarg;
            break;
            case 't':
                VOT = atoi(optarg);
                VOTFlag = 1;
            break;
            case 'o':
                Offset = atoi(optarg);
            break;
            case 'v':
                printf("Rocaloid GenRUDB version " Version "\n");
                return 1;
            break;
            case 'V':
                VerboseFlag = 1;
            break;
            case 'i':
                InvarThreshold = atof(optarg);
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
        fprintf(stderr, "Missing argument 'rotofile'.\n");
        PrintUsage();
        return 1;
    }
    if(optind < ArgN - 1)
    {
        fprintf(stderr, "Redundant argument '%s'.\n", Arg[optind + 1]);
    }
    CRotoFile = Arg[optind];
    
    //String conversion
    String TempFundMethod, TempWindowName;
    RNew(String, & FundMethod, & UnitName, & WindowName,
                 & TempFundMethod, & TempWindowName);
    String_SetChars(& TempFundMethod, CFundMethod);
    String_SetChars(& TempWindowName, CWindow);
    if(CUnitName)
        String_SetChars(& UnitName, CUnitName);
    
    UpperCase(& FundMethod, & TempFundMethod);
    LowerCase(& WindowName, & TempWindowName);
    RDelete(& TempFundMethod, & TempWindowName);
    
    //Value validity checking
    if(UFundFreq < LFundFreq)
    {
        fprintf(stderr, "[Error] Upper bound of fundamental frequency lower "
                        "than lower bound.\n");
        return 1;
    }
    if(UFundFreq < 0 || LFundFreq < 0)
    {
        fprintf(stderr, "[Error] Invalid fundamental frequency bound.\n");
        return 1;
    }
    if(USinuFreq < 1500)
    {
        fprintf(stderr, "[Error] Upper bound of sinusoidal component is too "
                        "low.\n");
        return 1;
    }
    
    int LogHopSize = CDSP2_IntLogOf2(HopSize);
    if(pow(2, LogHopSize) != HopSize)
    {
        fprintf(stderr, "[Error] Hop size must be power of 2.\n");
        return 1;
    }
    if(HopSize <= 4)
    {
        fprintf(stderr, "[Error] Hop size is too small.\n");
        return 1;
    }
    if(HopSize >= 4096)
    {
        fprintf(stderr, "[Error] Hop size is too large.\n");
        return 1;
    }
    
    int LogWinSize = CDSP2_IntLogOf2(WinSize);
    if(pow(2, LogWinSize) != WinSize)
    {
        fprintf(stderr, "[Error] Size of analysis window must be power of 2"
                        ".\n");
        return 1;
    }
    if(WinSize <= 512 || WinSize <= HopSize / 2)
    {
        fprintf(stderr, "[Error] Analysis window is too small.\n");
        return 1;
    }
    if(WinSize >= 8192)
    {
        fprintf(stderr, "[Error] Analysis window is too large.\n");
        return 1;
    }
    
    if(VOTFlag && VOT < 0)
    {
        fprintf(stderr, "[Error] Invalid VOT.\n");
        return 1;
    }
    
    if(String_EqualChars(& WindowName, "hanning"))
    {
        EWindow = Hanning;
    }else if(String_EqualChars(& WindowName, "hamming"))
    {
        EWindow = Hamming;
    }else if(String_EqualChars(& WindowName, "blackman"))
    {
        EWindow = Blackman;
    }else
    {
        fprintf(stderr, "[Error] Invalid window '%s'.\n",
            String_GetChars(& WindowName));
        return 1;
    }
    
    if(String_EqualChars(& FundMethod, "YIN"))
    {
        EF0 = CSVP_F0_YIN;
    }else if(String_EqualChars(& FundMethod, "SPECSTEP"))
    {
        EF0 = CSVP_F0_SpecStep;
    }else
    {
        fprintf(stderr, "[Error] Invalid fundamental frequency estimation "
                        "method '%s'.\n",
            String_GetChars(& FundMethod));
        return 1;
    }
    
    //Roto loading
    String RotoPath;
    String_Ctor(& RotoPath);
    String_SetChars(& RotoPath, CRotoFile);
    
    RUCE_Roto InRoto;
    if(! RUCE_Roto_CtorLoad(& InRoto, & RotoPath))
    {
        fprintf(stderr, "[Error] Cannot open '%s'.\n", CRotoFile);
        return 1;
    }
    
    Wave InWave;
    RUCE_Roto_Entry Entry;
    RUCE_DB_Entry DBEntry;
    RUCE_Roto_Entry_Ctor(& Entry);
    RUCE_DB_Entry_Ctor(& DBEntry);
    RCall(Wave, Ctor)(& InWave);
    
    //RUDB Generation
    CDSP2_SetArch(CDSP2_Arch_Gnrc);
    #define _PrintConfig() \
            if(ReadOnlyFlag) \
            { \
                printf("Roto configuration for '%s':\n", \
                    String_GetChars(& Entry.Name)); \
                printf("  VOT = %d\n", Entry.VOT); \
                printf("  InvarLeft = %d\n", Entry.InvarLeft); \
                printf("  InvarRight = %d\n", Entry.InvarRight); \
            }
    
    Real* Window = RCall(RAlloc, Real)(WinSize);
    if(EWindow == Hanning)
        RCall(CDSP2_GenHanning, Real)(Window, WinSize);
    else if(EWindow == Hamming)
        RCall(CDSP2_GenHamming, Real)(Window, WinSize);
    else
        RCall(CDSP2_GenBlackman, Real)(Window, WinSize);
    RCall(Wave, SetWindow)(& InWave, Window, WinSize);
    
    String DirName;
    String_Ctor(& DirName);
    if(CUnitName)
    {
        //Generate single unit.
        int Status = RUCE_Roto_GetEntry(& InRoto, & Entry, & UnitName);
        if(Status < 1) printf("Entry '%s' does not exist.\n", CUnitName);
        String_Copy(& Entry.Name, & UnitName);
        DirFromFilePath(& DirName, & RotoPath);
        String_JoinChars(& DirName, "/");
        String_Join(& DirName, & UnitName);
        String_JoinChars(& DirName, ".wav");
        Status = RCall(Wave, FromFile)(& InWave, & DirName);
        if(Status < 1)
        {
            fprintf(stderr, "[Error] Cannot load '%s'.\n",
                String_GetChars(& DirName));
        }else
        {
            GenUnit(& Entry, & DBEntry, & InWave);
            if(Status < 1) printf("Creating entry '%s'...\n", CUnitName);
                RUCE_Roto_SetEntry(& InRoto, & Entry);
            
            if(VerboseFlag)
                printf("Saving rudb...\n");
            DirFromFilePath(& DirName, & RotoPath);
            String_JoinChars(& DirName, "/");
            String_Join(& DirName, & Entry.Name);
            String_JoinChars(& DirName, ".rudb");
            RUCE_RUDB_Save(& DBEntry, & DirName);
            _PrintConfig();
        }
    }else
    {
        //Batch process all units.
        int Num = RUCE_Roto_GetEntryNum(& InRoto);
        int i;
        for(i = 0; i < Num; i ++)
        {
            RUCE_Roto_GetEntryByIndex(& InRoto, & Entry, i);
            
            DirFromFilePath(& DirName, & RotoPath);
            String_JoinChars(& DirName, "/");
            String_Join(& DirName, & Entry.Name);
            String_JoinChars(& DirName, ".wav");
            RCall(Wave, Dtor)(& InWave);
            RCall(Wave, Ctor)(& InWave);
            RCall(Wave, SetWindow)(& InWave, Window, WinSize);
            int Status = RCall(Wave, FromFile)(& InWave, & DirName);
            if(Status < 1)
            {
                fprintf(stderr, "[Error] Cannot load '%s'.\n",
                    String_GetChars(& DirName));
                continue;
            }
            
            GenUnit(& Entry, & DBEntry, & InWave);
            RUCE_Roto_SetEntry(& InRoto, & Entry);
            
            if(VerboseFlag)
                printf("Saving rudb...\n");
            DirFromFilePath(& DirName, & RotoPath);
            String_JoinChars(& DirName, "/");
            String_Join(& DirName, & Entry.Name);
            String_JoinChars(& DirName, ".rudb");
            RUCE_RUDB_Save(& DBEntry, & DirName);
            
            _PrintConfig();
        }
    }
    
    if(! ReadOnlyFlag)
    {
        RUCE_Roto_Write(& InRoto, & RotoPath);
    }
    
    RFree(Window);
    RDelete(& InRoto, & RotoPath, & Entry, & InWave, & DirName, & DBEntry);
    RDelete(& FundMethod, & UnitName, & WindowName);
    return 0;
}

