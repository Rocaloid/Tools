#include <libgen.h>
#include <stdlib.h>
#include <RUtil2.h>

void DirFromFilePath(String* Dest, String* Sorc)
{
    char* Temp = strdup(String_GetChars(Sorc));
    char* DirName = dirname(Temp);
    String_SetChars(Dest, DirName);
    free(Temp);
}

void BaseFromFilePath(String* Dest, String* Sorc)
{
    char* Temp = strdup(String_GetChars(Sorc));
    char* DirName = basename(Temp);
    String_SetChars(Dest, DirName);
    free(Temp);
}

