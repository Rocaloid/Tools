#include "GenUnit.h"
#include <stdlib.h>

int GenUnit(RUCE_Roto_Entry* Ret, Wave* Sorc)
{
    int FRet = 1;
    
    printf("Generating unit \'%s\'...\n", String_GetChars(& Ret -> Name));
    
    //Some test codes
    Ret -> VOT = rand();
    
    return FRet;
}

