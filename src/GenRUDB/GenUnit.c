#include "GenUnit.h"
#include <stdlib.h>
#include <CVESVP.h>

int GenUnit(RUCE_Roto_Entry* Ret, RUCE_DB_Entry* Dest, Wave* Sorc)
{
    int FRet = 1;
    int WSize = Sorc -> Size;
    
    printf("Generating unit \'%s\'...\n", String_GetChars(& Ret -> Name));
    RCall(Wave, Resize)(Sorc, Sorc -> Size + WinSize);
    
    //VOT detection
    if(! VOTFlag)
        VOT = RCall(CSVP_VOTFromWave, Real)(Sorc, 0, WSize);
    
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
    
    int TempSize = VOT + 10000 > WSize ? WSize : VOT + 10000;
    RCall(F0Iterlyzer, SetHopSize)(& F0Iter, 256);
    RCall(F0Iterlyzer, SetWave)(& F0Iter, Sorc);
    RCall(F0Iterlyzer, SetPosition)(& F0Iter, VOT + 2000);
    if(RCall(F0Iterlyzer, PreAnalysisTo)(& F0Iter, TempSize) < 1)
    {
        fprintf(stderr, "[Error] Fundamental frequency estimation failed. "
                        "Skipped.\n");
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
    printf("PSOLA analysis...\n");
    PSOLAIterlyzer PAna;
    RCall(PSOLAIterlyzer, Ctor)(& PAna);
    RCall(PSOLAIterlyzer, SetWave)(& PAna, Sorc);
    RCall(PSOLAIterlyzer, SetPosition)(& PAna, VOT + 3000);
    RCall(PSOLAIterlyzer, SetBound)(& PAna, VOT);
    RCall(PSOLAIterlyzer, SetPitch)(& PAna, & F0Iter.F0List);
    if(RCall(PSOLAIterlyzer, PreAnalysisTo)(& PAna, TempSize) < 1)
    {
        fprintf(stderr, "[Error] PSOLA pre-analysis failed. Skipped.\n");
        RDelete(& F0Iter, & PAna);
        return 0;
    }
    if(RCall(PSOLAIterlyzer, IterNextTo)(& PAna, WSize) < 1)
    {
        fprintf(stderr, "[Error] PSOLA forward analysis failed. Skipped.\n");
        RDelete(& F0Iter, & PAna);
        return 0;
    }
    if(RCall(PSOLAIterlyzer, PrevTo)(& PAna, 0) < 1)
    {
        fprintf(stderr, "[Error] PSOLA backward analysis failed. Skipped.\n");
        RDelete(& F0Iter, & PAna);
        return 0;
    }
    
    //HNM Analysis
    HNMIterlyzer HAna;
    printf("HNM analysis...\n");
    RCall(HNMIterlyzer, CtorSize)(& HAna, WinSize);
    RCall(HNMIterlyzer, SetWave)(& HAna, Sorc);
    RCall(HNMIterlyzer, SetHopSize)(& HAna, HopSize);
    RCall(HNMIterlyzer, SetPosition)(& HAna, VOT + 2000);
    RCall(HNMIterlyzer, SetUpperFreq)(& HAna, USinuFreq);
    RCall(HNMIterlyzer, SetPitch)(& HAna, & F0Iter.F0List);
    HAna.GenPhase = 1;
    if(! RCall(HNMIterlyzer, PrevTo)(& HAna, VOT))
    {
        fprintf(stderr, "[Error] HNM backward analysis failed. Skipped.\n");
        RDelete(& F0Iter, & PAna, & HAna);
        return 0;
    }
    if(! RCall(HNMIterlyzer, IterNextTo)(& HAna, WSize))
    {
        fprintf(stderr, "[Error] HNM forward analysis failed. Skipped.\n");
        RDelete(& F0Iter, & PAna, & HAna);
        return 0;
    }
    
    //Filling in
    int i, j;
    
    Dest -> HopSize = HopSize;
    Dest -> NoizSize = WinSize / 16;
    
    Array_From(int, Dest -> PulseList, PAna.PulseList.Frames);
    Array_ObjResize(RUCE_DB_Frame, Dest -> FrameList,
        HAna.HNMList.Frames_Index + 1);
    Dest -> FrameList_Index = HAna.HNMList.Frames_Index;
    for(i = 0; i <= Dest -> FrameList_Index; i ++)
    {
        RUCE_DB_Frame* DestEntry = & Dest -> FrameList[i]; 
        DestEntry -> Position = HAna.PulseList.Frames[i];
        DestEntry -> Noiz = realloc(DestEntry -> Noiz, Dest -> NoizSize * 4);
        HNMFrame*  SorcEntry = & HAna.HNMList.Frames[i];
        DataFrame* SorcPhase = & HAna.PhseList.Frames[i];
        
        RCall(CDSP2_Resample_Linear, Real)(DestEntry -> Noiz, SorcEntry -> Noiz,
            WinSize / 16, WinSize / 2);
        
        Array_Resize(float, DestEntry -> Freq, SorcPhase -> Size);
        Array_Resize(float, DestEntry -> Ampl, SorcPhase -> Size);
        Array_Resize(float, DestEntry -> Phse, SorcPhase -> Size);
        DestEntry -> Freq_Index = SorcPhase -> Size - 1;
        DestEntry -> Ampl_Index = SorcPhase -> Size - 1;
        DestEntry -> Phse_Index = SorcPhase -> Size - 1;
        
        for(j = 0; j < SorcPhase -> Size; j ++)
        {
            DestEntry -> Freq[i] = SorcEntry -> Hmnc.Freq[i];
            DestEntry -> Ampl[i] = SorcEntry -> Hmnc.Ampl[i];
            DestEntry -> Phse[i] = SorcPhase -> Data[i];
        }
    }
    
    RDelete(& F0Iter, & PAna, & HAna);
    return FRet;
}

