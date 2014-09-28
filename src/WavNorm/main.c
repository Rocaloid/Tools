#include <unistd.h>

#include <RUtil2.h>
#include <CVEDSP2.h>
#include <CVESVP.h>
#include <RUCE.h>
#include "../Commons.h"

#define Version "1.2.0.0"

int   VerboseFlag = 0;
char* CUnitName = NULL;
float Gain = 1.5;
float Intensity = -50;
int   WinSize = 1024;
char* CWavFile = NULL;
float Threshold = 0.005;
float EndingThreshold = 0.01;
int   TrimFlag = 0;

static void PrintUsage()
{
    fprintf(stderr, "Usage: wavnorm [-t] [-s noisegate] [-e endinggate]\n"
                    "               [-g gain] [-i intensity] [-z size]\n"
                    "               [-v] [-V] wavfile\n");
}

static void NormUnit(Wave* Dest, Wave* Sorc)
{
    Wave EWave;
    RCall(Wave, Ctor)(& EWave);
    RCall(Wave, Resize)(Dest, Sorc -> Size);
    
    RCall(CDSP2_EnergyCurveFromWaveDB, Real)(& EWave, Sorc, WinSize);
    Real Mean = RCall(CDSP2_MeanEnergyFromWaveDB, Real)(Sorc, 0,
        Sorc -> Size);
    Real WGain = pow(10, (Intensity - Mean) / 40.0) * Gain;
    if(VerboseFlag)
    {
        printf("Mean energy = %fdB.\n", Mean);
        printf("Gain = %f.\n", WGain);
    }
    
    Dest -> SampleRate = Sorc -> SampleRate;
    NormIterfector Normalizer;
    RCall(NormIterfector, Ctor)(& Normalizer);
    RCall(NormIterfector, SetWave)(& Normalizer, Sorc);
    RCall(NormIterfector, SetOutWave)(& Normalizer, Dest);
    RCall(NormIterfector, SetEnergyWave)(& Normalizer, & EWave);
    RCall(NormIterfector, SetPosition)(& Normalizer, 0);
    RCall(NormIterfector, SetGain)(& Normalizer, WGain);
    RCall(NormIterfector, SetIntensity)(& Normalizer, Intensity);
    
    if(VerboseFlag)
        printf("Normalizing...\n");
    RCall(NormIterfector, IterNextTo)(& Normalizer, Sorc -> Size);
    
    RCall(Wave, Dtor)(& EWave);
    RCall(NormIterfector, Dtor)(& Normalizer);
}

static int FindBound(Real* Sorc, int Size, Real Threshold, int Direction)
{
    int i;
    if(Direction < 0)
    {
        for(i = Size - 1; i >= 0; i --)
            if(Sorc[i] > Threshold || Sorc[i] < - Threshold)
                break;
    }else
    {
        for(i = 0; i < Size; i ++)
            if(Sorc[i] > Threshold || Sorc[i] < - Threshold)
                break;
        if(i == Size)
            i = -1;
    }
    return i;
}

static void TrimUnit(Wave* Dest, Wave* Sorc)
{
    int Size = Sorc -> Size;
    Real* Data = RCall(Wave, GetUnsafePtr)(Sorc);
    int LBound = FindBound(Data, Size, Threshold,  1);
    int RBound = FindBound(Data, Size, EndingThreshold, -1);
    if(LBound < 0 || RBound < 0 || RBound - LBound <= 0)
    {
        fprintf(stderr, "Boundary detection failed!\n");
        LBound = 0;
        RBound = Size;
    }
    else
        Size = RBound - LBound;
    RCall(Wave, Resize)(Dest, Size);
    RCall(Wave, Write)(Dest, Data + LBound, 0, Size);
}

int main(int ArgN, char** Arg)
{
    int c;
    while((c = getopt(ArgN, Arg, "ts:e:g:i:z:Vv")) != -1)
    {
        switch(c)
        {
            case 't':
                TrimFlag = 1;
            break;
            case 's':
                Threshold = atof(optarg);
            break;
            case 'e':
                EndingThreshold = atof(optarg);
            break;
            case 'n':
                CUnitName = optarg;
            break;
            case 'g':
                Gain = atof(optarg);
            break;
            case 'i':
                Intensity = atof(optarg);
            break;
            case 'z':
                WinSize = atoi(optarg);
            break;
            case 'v':
                printf("Rocaloid WavNorm version " Version "\n");
                return 1;
            break;
            case 'V':
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
        fprintf(stderr, "[Error] Missing argument 'wavfile'.\n");
        PrintUsage();
        return 1;
    }
    if(optind < ArgN - 1)
    {
        fprintf(stderr, "[Error] Redundant argument '%s'.\n", Arg[optind + 1]);
    }
    CWavFile = Arg[optind];
        
    Wave InWave, OutWave;
    RCall(Wave, Ctor)(& InWave);
    RCall(Wave, Ctor)(& OutWave);
    
    CDSP2_SetArch(CDSP2_Arch_Gnrc);
    Real* Window = RCall(RAlloc, Real)(WinSize);
    RCall(CDSP2_GenHanning, Real)(Window, WinSize);
    RCall(Wave, SetWindow)(& InWave, Window, WinSize);
    
    String WavName;
    String_Ctor(& WavName);
    String_SetChars(& WavName, CWavFile);
    
    //Generate single unit.
    int Status = RCall(Wave, FromFile)(& InWave, & WavName);
    if(Status < 1)
    {
        fprintf(stderr, "[Error] Cannot load '%s'.\n", CWavFile);
    }else
    {
        if(TrimFlag)
        {
            TrimUnit(& OutWave, & InWave);
            RCall(Wave, From)(& InWave, & OutWave);
        }
        NormUnit(& OutWave, & InWave);
        
        if(VerboseFlag)
            printf("Saving wav...\n");
        RCall(Wave, ToFile)(& OutWave, & WavName);
    }
    
    RFree(Window);
    RDelete(& InWave, & OutWave);
    RDelete(& WavName);
    return 0;
}

