#include <unistd.h>

#include <RUtil2.h>
#include <CVEDSP2.h>
#include <RUCE.h>
#include "../Commons.h"

#define Version "1.0.0.2"

int   VerboseFlag = 0;
char* CRotoFile = NULL;

static void PrintUsage()
{
    fprintf(stderr, "Usage: roto2oto [-v] [-V] rotofile\n");
}

int main(int ArgN, char** Arg)
{
    int c;
    while((c = getopt(ArgN, Arg, "Vv")) != -1)
    {
        switch(c)
        {
            case 'v':
                printf("Rocaloid Roto2Oto version " Version "\n");
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
    RUCE_Roto_Entry_Ctor(& Entry);
    RCall(Wave, Ctor)(& InWave);
    
    CDSP2_SetArch(CDSP2_Arch_Gnrc);
    
    String DirName;
    String_Ctor(& DirName);
    
    String OtoPath;
    String_Ctor(& OtoPath);
    DirFromFilePath(& OtoPath, & RotoPath);
    String_JoinChars(& OtoPath, "/oto.ini");
    
    File OtoFile;
    File_Ctor(& OtoFile);
    if(File_Open(& OtoFile, & OtoPath, CREATE) < 1)
    {
        fprintf(stderr, "[Error] Cannot create file '%s'.\n",
            String_GetChars(& OtoPath));
        return 1;
    }
    
    String_Dtor(& OtoPath);
    
    //Batch process all units.
    String LineBuff, WordBuff;
    RNew(String, & LineBuff, & WordBuff);
    
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
        int Status = RCall(Wave, FromFile)(& InWave, & DirName);
        if(Status < 1)
        {
            fprintf(stderr, "[Error] Cannot load '%s'.\n",
                String_GetChars(& DirName));
            continue;
        }
        
        /*
            Oto entry:
            Mapping, LeftBound, LeftInvar - LeftBound, Size - RightBound,
                VOT - LeftBound, Overlap
        */
        #define Smpl2ms(x) (Real)((x) * 1000.0 / InWave.SampleRate)
        String_From(& LineBuff, & Entry.Name);
        String_JoinChars(& LineBuff, ".wav=,0,");
        CStrFloat(& WordBuff, Smpl2ms(Entry.InvarLeft));
        String_Join(& LineBuff, & WordBuff);
        String_JoinChars(& LineBuff, ",0,");
        CStrFloat(& WordBuff, Smpl2ms(Entry.VOT));
        String_Join(& LineBuff, & WordBuff);
        String_JoinChars(& LineBuff, ",");
        CStrFloat(& WordBuff, Smpl2ms(Entry.VOT) / 3);
        String_Join(& LineBuff, & WordBuff);
        File_WriteLine(& OtoFile, & LineBuff);
        
        if(VerboseFlag)
            printf("Generating unit \'%s\'...\n",
                String_GetChars(& Entry.Name));
        
        
    }
    
    File_Flush(& OtoFile);
    File_Close(& OtoFile);
    
    RDelete(& InWave, & Entry, & DirName, & OtoFile, & LineBuff, & WordBuff);
    RDelete(& RotoPath, & InRoto);
    return 0;
}

