#include <stdio.h>
#include <unistd.h>
#include <RUtil2.h>
#include <RUCE.h>

/*
NAME
    Context2Oct - Generate contextual training data in octave matrix format from 
                  audacity label tracks.

SYNOPSIS
    con2oct labelfile
*/

#define TYPE_NONE   -1

#define TYPE_X      0
#define TYPE_NP     1
#define TYPE_AP     2
#define TYPE_NA     3
#define TYPE_AA     4
#define TYPE_FR     5
#define TYPE_NS     6
#define TYPE_AX     7
#define TYPE_LA     8

#define TYPE_XV     100
#define TYPE_FN     101
#define TYPE_BN     102
#define TYPE_O      103
#define TYPE_M      104
#define TYPE_C      105

typedef struct
{
    float Time;
    int Identifier;
} ConEntry;

Array_Define(ConEntry, VTrack);
Array_Define(ConEntry, ETrack);
Array_Define(ConEntry, CTrack);

#include "../Commons.h"

static void ParseLabelFile(File* Sorc)
{
    int Size = File_GetLength(Sorc) - 1;
    
    ConEntry TempEntry;
    String Temp;
    String_Ctor(& Temp);
    while(File_GetPosition(Sorc) < Size)
    {
        float Pos1, Pos2;
        char  Flag[64];
        int   IFlag;
        
        File_ReadLine(Sorc, & Temp);
        int n = sscanf(String_GetChars(& Temp), "%f%f%s", & Pos1, & Pos2, Flag);
        
        if(n < 3)
            IFlag = TYPE_NONE;
        else if(! strcmp(Flag, "X"))
            IFlag = TYPE_X;
        else if(! strcmp(Flag, "NP"))
            IFlag = TYPE_NP;
        else if(! strcmp(Flag, "AP"))
            IFlag = TYPE_AP;
        else if(! strcmp(Flag, "NA"))
            IFlag = TYPE_NA;
        else if(! strcmp(Flag, "AA"))
            IFlag = TYPE_AA;
        else if(! strcmp(Flag, "FR"))
            IFlag = TYPE_FR;
        else if(! strcmp(Flag, "NS"))
            IFlag = TYPE_NS;
        else if(! strcmp(Flag, "AX"))
            IFlag = TYPE_AX;
        else if(! strcmp(Flag, "LA"))
            IFlag = TYPE_LA;
        else if(! strcmp(Flag, "XV"))
            IFlag = TYPE_XV;
        else if(! strcmp(Flag, "FN"))
            IFlag = TYPE_FN;
        else if(! strcmp(Flag, "BN"))
            IFlag = TYPE_BN;
        else if(! strcmp(Flag, "O"))
            IFlag = TYPE_O;
        else if(! strcmp(Flag, "M"))
            IFlag = TYPE_M;
        else if(! strcmp(Flag, "C"))
            IFlag = TYPE_C;
        else
            IFlag = TYPE_NONE;
        
        TempEntry.Time = Pos1;
        TempEntry.Identifier = IFlag;
        
        if(IFlag == TYPE_NONE)
            Array_Push(ConEntry, ETrack, TempEntry);
        else if(IFlag < 100)
            Array_Push(ConEntry, CTrack, TempEntry);
        else
            Array_Push(ConEntry, VTrack, TempEntry);
    }
    String_Dtor(& Temp);
}

int main(int ArgN, char** Arg)
{
    char* CLabelFile = NULL;
    
    if(ArgN <= 1)
    {
        fprintf(stderr, "Missing argument 'labelfile'.\n");
        fprintf(stderr, "Usage: con2oct labelfile\n");
        return 1;
    }
    
    CLabelFile = Arg[1];
    
    if(ArgN > 2)
        fprintf(stderr, "Warning: redundant argument '%s'.\n", Arg[2]);
    
    String LabelPath, BaseName, UnitName;
    RNew(String, & LabelPath, & BaseName, & UnitName);
    File LabelFile;
    RNew(File, & LabelFile);
    String_FromChars(Dot, ".");
    
    String_SetChars(& LabelPath, CLabelFile);
    BaseFromFilePath(& BaseName, & LabelPath);
    int DotPos = String_InStr(& BaseName, & Dot);
    Left(& UnitName, & BaseName, DotPos);
    
    if(! File_Open(& LabelFile, & LabelPath, READONLY))
    {
        fprintf(stderr, "Cannot load label track '%s'.\n", CLabelFile);
        return 1;
    }
    
    Array_Ctor(ConEntry, VTrack);
    Array_Ctor(ConEntry, ETrack);
    Array_Ctor(ConEntry, CTrack);
    
    ParseLabelFile(& LabelFile);
    if(VTrack_Index != CTrack_Index)
    {
        fprintf(stderr, "[Error] Unmatching label numbers.\n");
        printf("%d %d\n", VTrack_Index, CTrack_Index);
        return 1;
    }
    
    printf("# name: context_%s\n", String_GetChars(& UnitName));
    printf("# type: matrix\n");
    printf("# ndims: 2\n");
    printf("%d %d\n", VTrack_Index, 7);
    int i;
    for(i = 1; i <= VTrack_Index; i ++)
    {
        float Principle = VTrack[i].Time;
        int TV = VTrack[i].Identifier;
        int TC = CTrack[i].Identifier;
        
        int EIndex, j;
        EIndex = ETrack_Index;
        for(j = 0; j <= ETrack_Index; j ++)
            if(ETrack[j].Time > Principle)
            {
                EIndex = j - 1;
                break;
            }
        
        float E1 = Principle - ETrack[EIndex - 1].Time;
        float E2 = Principle - ETrack[EIndex    ].Time;
        float L1 = Principle - VTrack[i - 1].Time;
        float L2;
        if(i != VTrack_Index)
            L2 = VTrack[i + 1].Time - Principle;
        else
            L2 = TopOf(ETrack).Time - Principle;
        float C  = Principle - CTrack[i].Time;
        
        printf("%f %f %d %d %f %f %f\n", L1, L2, TV, TC, E1, E2, C);
    }
    
    Array_Dtor(ConEntry, VTrack);
    Array_Dtor(ConEntry, ETrack);
    Array_Dtor(ConEntry, CTrack);
    
    File_Close(& LabelFile);
    RDelete(& LabelPath, & LabelFile, & Dot, & BaseName, & UnitName);
    return 0;
}

