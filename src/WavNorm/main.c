#include <unistd.h>

#include <RUtil2.h>
#include <CVEDSP2.h>
#include <CVESVP.h>
#include <RUCE.h>
#include "../Commons.h"

#define Version "1.0.0.0"

int   VerboseFlag = 0;
char* CUnitName = NULL;
float Gain = 1.5;
float Intensity = -50;
int   WinSize = 1024;
char* CRotoFile = NULL;

static void PrintUsage()
{
    fprintf(stderr, "Usuage: wavnorm [-n unitname]\n"
                    "                [-g gain] [-i intensity] [-z size]\n"
                    "                [-v] [-V] rotofile\n");
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

int main(int ArgN, char** Arg)
{
    int c;
    while((c = getopt(ArgN, Arg, "n:g:i:z:Vv")) != -1)
    {
        switch(c)
        {
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
        fprintf(stderr, "[Error] Missing argument 'rotofile'.\n");
        PrintUsage();
        return 1;
    }
    if(optind < ArgN - 1)
    {
        fprintf(stderr, "[Error] Redundant argument '%s'.\n", Arg[optind + 1]);
    }
    CRotoFile = Arg[optind];
    
    String UnitName;
    String_Ctor(& UnitName);
    if(CUnitName)
        String_SetChars(& UnitName, CUnitName);
    
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
    
    Wave InWave, OutWave;
    RUCE_Roto_Entry Entry;
    RUCE_Roto_Entry_Ctor(& Entry);
    RCall(Wave, Ctor)(& InWave);
    RCall(Wave, Ctor)(& OutWave);
    
    CDSP2_SetArch(CDSP2_Arch_Gnrc);
    Real* Window = RCall(RAlloc, Real)(WinSize);
    RCall(CDSP2_GenHanning, Real)(Window, WinSize);
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
            printf("Generating unit \'%s\'...\n", String_GetChars(& UnitName));
            NormUnit(& OutWave, & InWave);
            
            if(VerboseFlag)
                printf("Saving wav...\n");
            RCall(Wave, ToFile)(& OutWave, & DirName);
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
            
            printf("Generating unit \'%s\'...\n",
                String_GetChars(& Entry.Name));
            NormUnit(& OutWave, & InWave);
            
            if(VerboseFlag)
                printf("Saving wav...\n");
            RCall(Wave, ToFile)(& OutWave, & DirName);
        }
    }
    
    
    RFree(Window);
    RDelete(& InWave, & OutWave, & Entry, & DirName);
    RDelete(& RotoPath, & InRoto, & UnitName);
    return 0;
}

