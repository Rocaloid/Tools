#include <RUtil2.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    String OutPath;
    File OutFile;
    String_Ctor(& OutPath);
    File_Ctor(& OutFile);
    
    if(argc == 2)
        String_SetChars(& OutPath, "./rec.txt");
    else if(argc == 3)
        String_SetChars(& OutPath, argv[2]);
    else
    {
        printf("Usage: %s Utau-Voicebank-Path [OutputFile]\n    By default, "
               "output file is rec.txt.\n", argv[0]);
        String_Dtor(& OutPath);
        File_Dtor(& OutFile);
        return 1;
    }
    
    File_Open(& OutFile, & OutPath, WRITEONLY);
    
    Directory d;
    Directory_Ctor(& d);
    
    String_FromChars(CDirPath, argv[1]);
    String_FromChars(Match, "*.wav");
    String FileName, Temp;
    String_Ctor(& FileName);
    String_Ctor(& Temp);
    
    File_OpenDir(& d, & CDirPath);
    File_SetDirFilter(& d, & Match);
    File_SetDirFlags(& d, FILEONLY);
    while(File_ReadDir(& d, & FileName) != 1)
    {
        Left(& Temp, & FileName, String_GetLength(& FileName) - 4);
        File_Write_String(& OutFile, & Temp);
        File_Write_Chars(& OutFile, " ");
    }
    File_CloseDir(& d);
    
    String_Dtor(& Temp);
    String_Dtor(& FileName);
    String_Dtor(& CDirPath);
    String_Dtor(& Match);
    Directory_Dtor(& d);
    
    File_Close(& OutFile);
    
    String_Dtor(& OutPath);
    File_Dtor(& OutFile);
}