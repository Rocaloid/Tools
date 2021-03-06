﻿#include <stdio.h>
#include <unistd.h>
#include <RUtil2.h>
#include <CVEDSP2.h>
#include <CVESVP.h>

/*
NAME
    RSegment - Split .wav according to audacity label track & recording table.

SYNOPSIS
    rsegment [-r]
             wavfile labelfile recfile

OPTIONS
    -r
        Regenerate audacity label track bounded with words from word tabel.
   
    NOTE: Trimming has been moved to WavNorm.
*/

Array_Define(int, SegList);

#include "../Commons.h"

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
    int RegenFlag = 0;
    
    char* CWaveFile = NULL;
    char* CLabelFile = NULL;
    char* CRecFile = NULL;
    char* CRegenFile = "RegeneratedTrack.txt";
    
    int c;
    
    while((c = getopt(ArgN, Arg, "r")) != -1)
    {
        switch(c)
        {
            case 'r':
                RegenFlag = 1;
            break;
            case '?':
                printf("Usage: rsegment [-r] "
                           "[-h] wavfile labelfile recfile\n");
                return 1;
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
                fprintf(stderr, "Warning: redundant argument '%s'.\n", Arg[i]);
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
    File RegenFile;
    String WavePath, LabelPath, RecPath, RegenPath;
    
    RNew(String, & WavePath, & LabelPath, & RecPath, & RegenPath);
    RNew(File, & LabelFile, & RecFile, & RegenFile);
    RCall(Wave, Ctor)(& SorcWave);
    
    String_SetChars(& WavePath, CWaveFile);
    String_SetChars(& LabelPath, CLabelFile);
    String_SetChars(& RecPath, CRecFile);
    String_SetChars(& RegenPath, CRegenFile);

    if(! RCall(Wave, FromFile)(& SorcWave, & WavePath))
    {
        fprintf(stderr, "Cannot load wave file.\n");
        return 1;
    }
    if(! File_Open(& LabelFile, & LabelPath, READONLY))
    {
        fprintf(stderr, "Cannot load label file.\n");
        return 1;
    }
    if(! File_Open(& RecFile, & RecPath, READONLY))
    {
        fprintf(stderr, "Cannot load word table.\n");
        return 1;
    }
    if(RegenFlag)
    {
        if(! File_Open(& RegenFile, & RegenPath, CREATE))
        {
            fprintf(stderr, "Cannot create '%s'.\n", CRegenFile);
            return 1;
        }
    }
    
    Array_Ctor(String, RecList);
    Array_Ctor(int, SegList);
    
    ParseRecFile(& RecFile);
    ParseLabelFile(& LabelFile, SorcWave.SampleRate);
    
    Wave SegWave;
    RCall(Wave, Ctor)(& SegWave);
    SegWave.SampleRate = SorcWave.SampleRate;
    
    String SegName, LineBuf, TempBuf;
    RNew(String, & SegName, & LineBuf, & TempBuf);
    for(i = 0; i <= SegList_Index; i ++)
    {
        int Start = SegList[i];
        int End;
        if(i == SegList_Index)
            End = SorcWave.Size;
        else
            End = SegList[i + 1];
        
        if(i > RecList_Index)
        {
            fprintf(stderr, "Insufficient records in word table.\n");
            fprintf(stderr, "Segmentation aborted at %fs.\n",
                (float)Start / SorcWave.SampleRate);
            return 1;
        }
        
        String* Name = & RecList[i];
        String_Copy(& SegName, Name);
        String_JoinChars(& SegName, ".wav");
        
        if(! RegenFlag)
        {
            int Size = End - Start;
            Real* Data = RCall(RAlloc, Real)(Size);
            RCall(Wave, Read)(& SorcWave, Data, Start, Size);
            
            RCall(Wave, Resize)(& SegWave, Size);
            RCall(Wave, Write)(& SegWave, Data, 0, Size);
            if(! RCall(Wave, ToFile)(& SegWave, & SegName))
                fprintf(stderr, "Exporting '%s' failed. Skipped.\n",
                    String_GetChars(& SegName));
            RFree(Data);
        }
        
        if(RegenFlag)
        {
            CStrFloat(& LineBuf, (float)SegList[i] / SorcWave.SampleRate);
            String_Copy(& TempBuf, & LineBuf);
            String_JoinChars(& LineBuf, "\t");
            String_Join(& LineBuf, & TempBuf);
            String_JoinChars(& LineBuf, "\t");
            String_Join(& LineBuf, Name);
            File_WriteLine(& RegenFile, & LineBuf);
        }
    }
    if(i <= RecList_Index)
        fprintf(stderr, "Warning: redundant records in word table starting from"
            " '%s'.\n", String_GetChars(& RecList[i]));
    
    if(RegenFlag)
        File_Flush(& RegenFile);
    File_Close(& RecFile);
    File_Close(& LabelFile);
    File_Close(& RegenFile);

    Array_ObjDtor(String, RecList);
    Array_Dtor(String, RecList);
    Array_Dtor(int, SegList);
    RDelete(& WavePath, & LabelPath, & RecPath, & RegenPath, & SegName,
        & LineBuf, & TempBuf);
    RDelete(& SorcWave, & LabelFile, & RecFile, & SegWave, & RegenFile);
    return 0;
}

