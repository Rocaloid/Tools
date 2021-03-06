#include <stdio.h>
#include <unistd.h>

#include <RUtil2.h>
#include <CVEDSP2.h>
#include <CVESVP.h>
#include <RUCE.h>

#include "GenUnit.h"
#include "Common.h"
#include "../Commons.h"

static void PrintUsage()
{
    fprintf(stderr, "Usage: genrudb [-u freq] [-l freq] [-m method] [-S]\n"
                    "               [-s freq] [-h hopsize] [-z size]\n"
                    "               [-c threshold] [-w window] [-t position]\n"
                    "               [-a alpha] [-i threshold] [-v] [-V]\n"
                    "               [-q] wavfile\n");
}

int main(int ArgN, char** Arg)
{
    CWavFile = NULL;
    QuitFlag = 0;
    UFundFreq = 700;
    LFundFreq = 80;
    CFundMethod = "YIN";
    Stochastic = 0;
    USinuFreq = 10000;
    HopSize = 256;
    WinSize = 2048;
    HCorrThreshold = 30.0;
    CWindow = "hanning";
    VOT = 0;
    VOTFlag = 0;
    InvarThreshold = 0.003;
    VerboseFlag = 0;
    Alpha = 0.005;
    
    int c;
    while((c = getopt(ArgN, Arg, "qu:l:m:Ss:h:z:w:t:a:i:Vv")) != -1)
    {
        switch(c)
        {
            case 'q':
                QuitFlag = 1;
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
            case 'S':
                Stochastic = 1;
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
            case 'c':
                HCorrThreshold = atof(optarg);
            break;
            case 'w':
                CWindow = optarg;
            break;
            case 't':
                VOT = atof(optarg);
                VOTFlag = 1;
            break;
            case 'a':
                Alpha = atof(optarg);
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
        fprintf(stderr, "[Error] Missing argument 'wavfile'.\n");
        PrintUsage();
        return 1;
    }
    if(optind < ArgN - 1)
    {
        fprintf(stderr, "[Error] Redundant argument '%s'.\n", Arg[optind + 1]);
    }
    CWavFile = Arg[optind];
    
    //String conversion
    String TempFundMethod, TempWindowName, TempFileName;
    RNew(String, & FundMethod, & UnitName, & WavName, & WindowName,
                 & TempFundMethod, & TempWindowName, & TempFileName);
    String_SetChars(& TempFundMethod, CFundMethod);
    String_SetChars(& TempWindowName, CWindow);
    if(CWavFile)
        String_SetChars(& WavName, CWavFile);
    String_FromChars(Dot, ".");
    BaseFromFilePath(& TempFileName, & WavName);
    int DotPos = InStrRev(& TempFileName, & Dot);
    Left(& UnitName, & TempFileName, DotPos);
    
    UpperCase(& FundMethod, & TempFundMethod);
    LowerCase(& WindowName, & TempWindowName);
    RDelete(& TempFundMethod, & TempWindowName, & Dot, & TempFileName);
    
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
    
    Wave InWave;
    RUCE_DB_Entry DBEntry;
    RUCE_DB_Entry_Ctor(& DBEntry);
    RCall(Wave, Ctor)(& InWave);
    
    //RUDB Generation
    CDSP2_SetArch(CDSP2_Arch_Gnrc);
    
    Real* Window = RCall(RAlloc, Real)(WinSize);
    if(EWindow == Hanning)
        RCall(CDSP2_GenHanning, Real)(Window, WinSize);
    else if(EWindow == Hamming)
        RCall(CDSP2_GenHamming, Real)(Window, WinSize);
    else
        RCall(CDSP2_GenBlackman, Real)(Window, WinSize);
    RCall(Wave, SetWindow)(& InWave, Window, WinSize);
    
    int Status = RCall(Wave, FromFile)(& InWave, & WavName);
    if(Status < 1)
    {
        fprintf(stderr, "[Error] Cannot load '%s'.\n", CWavFile);
    }else
    {
        GenUnit(& DBEntry, & InWave);
        if(VerboseFlag)
            printf("Saving rudb...\n");
        String_JoinChars(& UnitName, ".rudb");
        RUCE_RUDB_Save(& DBEntry, & UnitName);
    }
    
    RFree(Window);
    RDelete(& InWave, & DBEntry);
    RDelete(& FundMethod, & UnitName, & WavName, & WindowName);
    return 0;
}
