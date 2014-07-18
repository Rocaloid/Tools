#ifndef TOOLS_COMMONS_H
#define TOOLS_COMMONS_H

Array_Define(String, RecList);

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

#endif

