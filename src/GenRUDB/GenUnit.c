#include "GenUnit.h"
#include <stdlib.h>
#include <CVESVP.h>

int GenUnit(RUCE_Roto_Entry* Ret, Wave* Sorc)
{
    int FRet = 1;
    
    printf("Generating unit \'%s\'...\n", String_GetChars(& Ret -> Name));
        
    //VOT detection
    if(! VOTFlag)
        VOT = RCall(CSVP_VOTFromWave, Real)(Sorc, 0, Sorc -> Size);
    
    if(VOT > Sorc -> Size * 2 / 3)
    {
        fprintf(stderr, "[Error] VOT(%d) is too large for unit '%s'. "
                        "Skipped.\n",
            VOT, String_GetChars(& Ret -> Name));
        return 0;
    }
    
    printf("VOT = %d\n", VOT);
    Ret -> VOT = VOT;
    
    //F0 estimation
    F0Iterlyzer F0Iter;
    RCall(F0Iterlyzer, Ctor)(& F0Iter);
    F0Iter.Option.Adlib = 1;
    F0Iter.Option.LFreq = LFundFreq / 2;
    F0Iter.Option.HFreq = UFundFreq;
    F0Iter.Option.Threshold = 0.01;
    F0Iter.Option.YIN.W = 300;
    F0Iter.Option.YIN.Threshold = 0.2;
    if(EF0 = YIN)
        F0Iter.Option.Method = CSVP_F0_YIN;
    else
        F0Iter.Option.Method = CSVP_F0_SpecStep;
    
    RCall(F0Iterlyzer, SetHopSize)(& F0Iter, 256);
    RCall(F0Iterlyzer, SetWave)(& F0Iter, Sorc);
    RCall(F0Iterlyzer, SetPosition)(& F0Iter, VOT + 2000);
    if(RCall(F0Iterlyzer, PreAnalysisTo)(& F0Iter, VOT + 10000) < 1)
    {
        fprintf(stderr, "Fundamental frequency estimation failed. Skipped.\n");
        RDelete(& F0Iter);
        return 0;
    }
    RCall(F0Iterlyzer, PrevTo)(& F0Iter, 0);
    RCall(F0Iterlyzer, IterNextTo)(& F0Iter, Sorc -> Size);
    RCall(CSVP_F0PostProcess, Real)(& F0Iter.F0List, 4000, 0.15,
        LFundFreq, UFundFreq);
    
    Real Sum = RCall(CDSP2_VSum, Real)(F0Iter.F0List.Y, 0,
        F0Iter.F0List.Y_Index + 1);
    printf("Average fundamental frequency: %fHz\n",
        (Real)Sum / ((Real)F0Iter.F0List.Y_Index + 1.0));
    
    //PSOLA Analysis
    PSOLAIterlyzer PAna;
    RCall(PSOLAIterlyzer, Ctor)(& PAna);
    RCall(PSOLAIterlyzer, SetWave)(& PAna, Sorc);
    RCall(PSOLAIterlyzer, SetPosition)(& PAna, VOT + 3000);
    RCall(PSOLAIterlyzer, SetBound)(& PAna, VOT);
    RCall(PSOLAIterlyzer, SetPitch)(& PAna, & F0Iter.F0List);
    if(RCall(PSOLAIterlyzer, PreAnalysisTo)(& PAna, VOT + 7000) < 1)
    {
        fprintf(stderr, "PSOLA pre-analysis failed. Skipped.\n");
        RDelete(& F0Iter, & PAna);
        return 0;
    }
    if(RCall(PSOLAIterlyzer, IterNextTo)(& PAna, Sorc -> Size) < 1)
    {
        fprintf(stderr, "PSOLA forward analysis failed. Skipped.\n");
        RDelete(& F0Iter, & PAna);
        return 0;
    }
    if(RCall(PSOLAIterlyzer, PrevTo)(& PAna, 0) < 1)
    {
        fprintf(stderr, "PSOLA backward analysis failed. Skipped.\n");
        RDelete(& F0Iter, & PAna);
        return 0;
    }
    
    RDelete(& F0Iter, & PAna);
    return FRet;
}

