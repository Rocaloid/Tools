#ifndef TOOLS_COMMONS_H
#define TOOLS_COMMONS_H

#define Wave CDSP2_Wave_Float
#define F0Iterlyzer CSVP_F0Iterlyzer_Float
#define PSOLAIterlyzer CSVP_PSOLAIterlyzer_Float
#define HNMIterlyzer CSVP_HNMIterlyzer_Float
#define HNMFrame CSVP_HNMFrame_Float
#define DataFrame CDSP2_DataFrame_Float
#define List_HNMContour CSVP_List_HNMContour_Float
#define HNMContour CSVP_HNMContour_Float
#define NormIterfector CDSP2_NormIterfector_Float
#define Real Float

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

