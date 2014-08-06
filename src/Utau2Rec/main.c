#include <RUtil2.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    String out;
    File f;
    String_Ctor(& out);
    File_Ctor(& f);
    
    if(argc == 2)
        String_SetChars(& out, "./rec.txt");
    else if(argc == 3)
        String_SetChars(& out, argv[2]);
    else
    {
        printf("Usage: %s Utau-Voicebank-Path [OutputFile]\n    By default, output file is rec.txt.\n", argv[0]);
        String_Dtor(& out);
        File_Dtor(& f);
        return 1;
    }
    
    File_Open(& f, & out, WRITEONLY);
    
    Directory d;
    Directory_Ctor(& d);
    
    String_FromChars(s, argv[1]);
    String_FromChars(WC, "*.wav");
    String o, tmp;
    String_Ctor(& o);
    String_Ctor(& tmp);
    
    File_OpenDir(& d, & s);
    File_SetDirFilter(& d, & WC);
    File_SetDirFlags(& d, FILEONLY);
    while(File_ReadDir(& d, & o) != 1)
    {
        Left(& tmp, & o, String_GetLength(& o) - 4);
        File_Write_String(& f, & tmp);
        File_Write_Chars(& f, " ");
    }
    File_CloseDir(& d);
    
    String_Dtor(& tmp);
    String_Dtor(& o);
    String_Dtor(& s);
    String_Dtor(& WC);
    Directory_Dtor(& d);
    
    File_Close(& f);
    
    String_Dtor(& out);
    File_Dtor(& f);
}