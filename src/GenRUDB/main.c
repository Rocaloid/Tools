#include <stdio.h>
#include <unistd.h>

#include <RUtil2.h>
#include <CVEDSP2.h>
#include <CVESVP.h>
#include <RUCE.h>

/*
NAME
    GenRUDB - Analyze voice waveform and generate RUCE voice bank.
    
SYNOPSIS
    genrudb [Other Options] [-n unitname] rotofile
    
OPTIONS
    
    Behaviour
    -n <unitname>
        Only generate the datafile for the specified vocal unit.
        Default: Disabled(batch process all units in roto)
    
    -a <sound intensity>
        Normalize the amplitude of utterance(in decibel).
        Default: No normalization
    
    Fundamental Frequency Estimation    
    -u <frequency>
        Upper bound for fundamental frequency estimation.
        Default: 700
    
    -l <frequency>
        Lower bound for fundamental frequency estimation.
        Default: 80
    
    -m <method>
        Fundamental frequency estimation method.
        Choices: YIN, SPECSTEP. (case-insensitive)
        Default: YIN    
    
    HNM Analysis
    -s <frequency>
        The upper bound of sinusoidal component.
        Default: 10000
    
    -h <hopsize>
        Hop size for HNM analysis.
        Default: 256    
    
    -z <size>
        Size of analysis window in integer power of 2.
        Default: 2048
    
    -w <window>
        Analysis window.
        Choices: hanning, hamming, blackman. (case-insensitive)
        Default: hanning
    
    -o <offset>
        The offset(in relation to VOT) of left bound for HNM analysis.
        Default: 500
    
    Other
    -v
        Verbose.
        Default: disabled
*/

#define Wave CDSP2_Wave_Float
#define Real Float

static void PrintUsage()
{
    fprintf(stderr, "Usuage: genrudb [-n unitname] [-a soundintensity]\n"
                    "                [-u freq] [-l freq] [-m method]\n"
                    "                [-s freq] [-h hopsize] [-z size]\n"
                    "                [-w window] [-o offset] [-v] "
                                    "rotofile\n");
}

int main(int ArgN, char** Arg)
{
    char* CRotoFile = NULL;
    char* CUnitName = NULL;
    float Intensity = -50;
    int   NormFlag = 0;
    float UFundFreq = 700;
    float LFundFreq = 80;
    char* CFundMethod = "YIN";
    float USinuFreq = 10000;
    int   HopSize = 256;
    int   WinSize = 2048;
    char* CWindow = "hanning";
    int   Offset = 500;
    int   VerboseFlag = 0;
    
    int c;
    while((c = getopt(ArgN, Arg, "n:a:u:l:m:s:h:z:w:o:v")) != -1)
    {
        switch(c)
        {
            case 'n':
                CUnitName = optarg;
            break;
            case 'a':
                Intensity = atof(optarg);
                NormFlag = 1;
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
            case 'o':
                Offset = atoi(optarg);
            break;
            case 'v':
                VerboseFlag = 1;
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
    String FundMethod, UnitName, WindowName;
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
    
    String RotoPath;
    String_Ctor(& RotoPath);
    String_SetChars(& RotoPath, CRotoFile);
    
    RUCE_Roto InRoto;
    if(! RUCE_Roto_CtorLoad(& InRoto, & RotoPath))
    {
        fprintf(stderr, "[Error] Cannot open '%s'.\n", CRotoFile);
        return 1;
    }
    printf("%ld\n", InRoto.Ptr);
    
    RDelete(& InRoto, & RotoPath);
    RDelete(& FundMethod, & UnitName, & WindowName);
    return 0;
}

