#include <RUtil2.h>
#include <stdio.h>

void PrintUsage(char *argv[0])
{
    printf("Usage: %s Utau-Voicebank-Path [OutputFile]\n    By default, "
           "output file is rec.txt.\n", argv[0]);   
}

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
        PrintUsage(argv);
        String_Dtor(& OutPath);
        File_Dtor(& OutFile);
        return 1;
    }
    
    File_Open(& OutFile, & OutPath, WRITEONLY);
    
    Directory SrcDir;
    Directory_Ctor(& SrcDir);
    
    String_FromChars(CDirPath, argv[1]);
    String_FromChars(Match, "*.wav");
    String FileName, Temp;
    String_Ctor(& FileName);
    String_Ctor(& Temp);
    
    if(File_OpenDir(& SrcDir, & CDirPath) != 1)
    {
        fprintf(stderr, "Error: Utau-Voicebank-Path must be a directory!\n\n");
        PrintUsage(argv);
        return 8;
    }
    File_SetDirFilter(& SrcDir, & Match);
    File_SetDirFlags(& SrcDir, FILEONLY);
    while(File_ReadDir(& SrcDir, & FileName) != 1)
    {
        Left(& Temp, & FileName, String_GetLength(& FileName) - 4);
        File_Write_String(& OutFile, & Temp);
        File_Write_Chars(& OutFile, " ");
    }
    File_Write_Chars(& OutFile, "\n");
    File_CloseDir(& SrcDir);
    
    String_Dtor(& Temp);
    String_Dtor(& FileName);
    String_Dtor(& CDirPath);
    String_Dtor(& Match);
    Directory_Dtor(& SrcDir);
    
    File_Close(& OutFile);
    
    String_Dtor(& OutPath);
    File_Dtor(& OutFile);
}
