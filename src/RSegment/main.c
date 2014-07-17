#include <stdio.h>
#include <unistd.h>
#include <RUtil2.h>
#include <CVEDSP2.h>
#include <CVESVP.h>

/*
NAME
    RSegment - Split .wav according to audacity label track & recording table.

SYNOPSIS
    rsegment [-t] [-g noisegate] [-h] wavfile labelfile recfile

Options
    -t
        Automatically trim the output files.
    
    -g <noisegate>
        Specify the noise gate(the threshold to distinguish noise from vocals).
    
    -h
        Display help.
*/

#define Wave CDSP2_Wave_Float

Array_Define(String, RecList);
Array_Define(int, SegList);

static void ParseRecFile(File* Sorc)
{
    int Size = File_GetLength(Sorc) - 1;
    while(File_GetPosition(Sorc) < Size)
    {
        Array_PushNull(String, RecList);
        String_Ctor(& TopOf(RecList));
        File_ReadWord(Sorc, & TopOf(RecList));
    }
}

static void ParseLabelFile(File* Sorc, int SampleRate)
{
    int Size = File_GetLength(Sorc) - 1;
    
    String Temp;
    String_Ctor(& Temp);
    while(File_GetPosition(Sorc) < Size)
    {
        double Pos;
        File_ReadWord(Sorc, & Temp);
        Pos = CDoubleStr(& Temp);
        Array_Push(int, SegList, (int)(Pos * SampleRate));
        
        //Trash
        File_ReadLine(Sorc, & Temp);
    }
    String_Dtor(& Temp);
}

int main(int ArgN, char** Arg)
{
    float Threshold = 0.005; //Default
    int TrimFlag = 0;
    
    char* CWaveFile = NULL;
    char* CLabelFile = NULL;
    char* CRecFile = NULL;
    
    int c;
    
    while((c = getopt(ArgN, Arg, "thg:")) != -1)
    {
        switch(c)
        {
            case 't':
                TrimFlag = 1;
            break;
            case 'g':
                Threshold = atof(optarg);
            break;
            case '?':
                if(optopt == 'g')
                    fprintf(stderr, "Option -g requires an argument.\n");
                else
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                return 1;
            case 'h':
                printf("rsegment [-t] [-g noisegate] [-h] "
                           "wavfile labelfile recfile\n");
                return 0;
            default:
                abort();
        }
    }
    
    int i;
    for(i = optind; i < ArgN; i ++)
    {
        switch(i - optind)
        {
            case 0:
                CWaveFile = Arg[i];
            break;
            case 1:
                CLabelFile = Arg[i];
            break;
            case 2:
                CRecFile = Arg[i];
            break;
            default:
                fprintf(stderr, "Redundant argument '%s'.\n", Arg[i]);
        }
    }
    
    if(! CWaveFile)
    {
        fprintf(stderr, "Missing argument 'wavfile'.\n");
        return 1;
    }
    if(! CLabelFile)
    {
        fprintf(stderr, "Missing argument 'labelfile'.\n");
        return 1;
    }
    if(! CRecFile)
    {
        fprintf(stderr, "Missing argument 'recfile'.\n");
        return 1;
    }
    
    Wave SorcWave;
    File LabelFile;
    File RecFile;
    String WavePath, LabelPath, RecPath;
    
    RNew(String, & WavePath, & LabelPath, & RecPath);
    RCall(Wave, Ctor)(& SorcWave);
    RCall(File, Ctor)(& LabelFile);
    RCall(File, Ctor)(& RecFile);
    
    String_SetChars(& WavePath, CWaveFile);
    String_SetChars(& LabelPath, CLabelFile);
    String_SetChars(& RecPath, CRecFile);

    if(! RCall(Wave, FromFile)(& SorcWave, & WavePath))
    {
        fprintf(stderr, "Cannot load wave file.\n");
        return 1;
    }
    if(! RCall(File, Open)(& LabelFile, & LabelPath, READONLY))
    {
        fprintf(stderr, "Cannot load label file.\n");
        return 1;
    }
    if(! RCall(File, Open)(& RecFile, & RecPath, READONLY))
    {
        fprintf(stderr, "Cannot load word table.\n");
        return 1;
    }
    
    Array_Ctor(String, RecList);
    Array_Ctor(int, SegList);
    
    ParseRecFile(& RecFile);
    ParseLabelFile(& LabelFile, SorcWave.SampleRate);

    File_Close(& RecFile);
    File_Close(& LabelFile);

    Array_ObjDtor(String, RecList);
    Array_Dtor(String, RecList);
    Array_Dtor(int, SegList);    
    RDelete(& WavePath, & LabelPath, & RecPath);
    RDelete(& SorcWave, & LabelFile, & RecFile);
    return 0;
}

