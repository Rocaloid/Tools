#include <stdio.h>
#include <unistd.h>
#include <RUtil2.h>
#include <RUCE.h>

/*
NAME
    GenRoto - Generate roto.json from word table.

SYNOPSIS
    genroto recfile
*/

#define Real Float

#include "../Commons.h"

int main(int ArgN, char** Arg)
{
    char* CRecFile = NULL;
    Array_Ctor(String, RecList);
    
    if(ArgN <= 1)
    {
        fprintf(stderr, "Missing argument 'recfile'.\n");
        fprintf(stderr, "Usage: genroto recfile\n");
        return 1;
    }
    
    CRecFile = Arg[1];
    
    if(ArgN > 2)
        fprintf(stderr, "Warning: redundant argument '%s'.\n", Arg[2]);
    
    String RecPath, RotoPath;
    RNew(String, & RecPath, & RotoPath);
    File RecFile, RotoFile;
    RNew(File, & RecFile, & RotoFile);
    
    String_SetChars(& RecPath, CRecFile);
    String_SetChars(& RotoPath, "Roto.json");
    
    if(! File_Open(& RecFile, & RecPath, READONLY))
    {
        fprintf(stderr, "Cannot load word table '%s'.\n", CRecFile);
        return 1;
    }
    
    ParseRecFile(& RecFile);
    
    RUCE_Roto Roto;
    RUCE_Roto_Entry RotoEntry;
    
    RUCE_Roto_Ctor(& Roto);
    RUCE_Roto_Entry_Ctor(& RotoEntry);
    
    int i;
    for(i = 0; i <= RecList_Index; i ++)
    {
        String_Copy(& RotoEntry.Name, RecList + i);
        RUCE_Roto_SetEntry(& Roto, & RotoEntry);
    }
    
    if(! RUCE_Roto_Write(& Roto, & RotoPath))
    {
        fprintf(stderr, "Cannot write to 'Roto.json'.\n");
        return 1;
    }
    
    File_Close(& RecFile);
    File_Close(& RotoFile);
    RDelete(& RecPath, & RotoPath, & RecFile, & RotoFile, & Roto, & RotoEntry);
    Array_ObjDtor(String, RecList);
    Array_Dtor(String, RecList);
    return 0;
}

