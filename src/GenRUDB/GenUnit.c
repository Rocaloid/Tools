#include "GenUnit.h"
#include <stdlib.h>
#include <CVESVP.h>

Real ContourDiff(HNMContour* Sorc1, HNMContour* Sorc2)
{
    if(Sorc1 -> Size != Sorc2 -> Size) return -1;
    int HalfSize = Sorc1 -> Size / 2 + 1;
    
    int U1 = 0;
    int U2 = 0;
    int i;
    for(i = 0; i < HalfSize; i ++)
        if(Sorc1 -> Hmnc[i] < -100)
        {
            U1 = i;
            break;
        }
    for(i = 0; i < HalfSize; i ++)
        if(Sorc2 -> Hmnc[i] < -100)
        {
            U2 = i;
            break;
        }
    
    int U = (U1 < U2 ? U1 : U2) / 2;
    Real* Diff = RCall(RAlloc, Real)(U);
    RCall(CDSP2_VSub, Real)(Diff, Sorc1 -> Hmnc, Sorc2 -> Hmnc, U);
    RCall(CDSP2_VMul, Real)(Diff, Diff, Diff, U);
    Real Ret = RCall(CDSP2_VSum, Real)(Diff, 0, U) / (Real)U;
    
    RFree(Diff);
    return Ret;
}

Real sigmoid(Real x)
{
    return 1.0 / (1.0 + exp(- x));
}

int GenUnit(RUCE_Roto_Entry* Ret, RUCE_DB_Entry* Dest, Wave* Sorc)
{
    int FRet = 1;
    int WSize = Sorc -> Size;
    
    printf("Generating unit \'%s\'...\n", String_GetChars(& Ret -> Name));
    RCall(Wave, Resize)(Sorc, Sorc -> Size + WinSize * 2);
    
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
    
    if(VerboseFlag)
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
    if(EF0 == YIN)
        F0Iter.Option.Method = CSVP_F0_YIN;
    else
        F0Iter.Option.Method = CSVP_F0_SpecStep;
    
    int TempSize = WSize / 2 + 10000 > WSize ? WSize : WSize / 2 + 10000;
    RCall(F0Iterlyzer, SetHopSize)(& F0Iter, 256);
    RCall(F0Iterlyzer, SetWave)(& F0Iter, Sorc);
    RCall(F0Iterlyzer, SetPosition)(& F0Iter, WSize / 2);
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
    
    if(VerboseFlag)
    printf("Average fundamental frequency: %fHz\n",
        (Real)Sum / ((Real)F0Iter.F0List.Y_Index + 1.0));
    
    //PSOLA Analysis
    if(VerboseFlag)
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
    if(VerboseFlag)
    printf("HNM analysis...\n");
    RCall(HNMIterlyzer, CtorSize)(& HAna, WinSize);
    RCall(HNMIterlyzer, SetWave)(& HAna, Sorc);
    RCall(HNMIterlyzer, SetHopSize)(& HAna, HopSize);
    RCall(HNMIterlyzer, SetPosition)(& HAna, VOT + Offset + 2000);
    RCall(HNMIterlyzer, SetUpperFreq)(& HAna, USinuFreq);
    RCall(HNMIterlyzer, SetPitch)(& HAna, & F0Iter.F0List);
    HAna.GenPhase = 1;
    if(! RCall(HNMIterlyzer, PrevTo)(& HAna, VOT + Offset))
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
    if(VerboseFlag)
    printf("Converting data structure...\n");
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
        
        for(j = 0; j < WinSize / 2 + 1; j ++)
            SorcEntry -> Noiz[j] = SorcEntry -> Noiz[j] < -1000 ? -1000 :
                SorcEntry -> Noiz[j];
        RCall(CDSP2_Resample_Linear, Real)(DestEntry -> Noiz, SorcEntry -> Noiz,
            WinSize / 16, WinSize / 2 + 1);
        
        Array_Resize(float, DestEntry -> Freq, SorcPhase -> Size);
        Array_Resize(float, DestEntry -> Ampl, SorcPhase -> Size);
        Array_Resize(float, DestEntry -> Phse, SorcPhase -> Size);
        DestEntry -> Freq_Index = SorcPhase -> Size - 1;
        DestEntry -> Ampl_Index = SorcPhase -> Size - 1;
        DestEntry -> Phse_Index = SorcPhase -> Size - 1;
        
        for(j = 0; j < SorcPhase -> Size; j ++)
        {
            DestEntry -> Freq[j] = SorcEntry -> Hmnc.Freq[j];
            DestEntry -> Ampl[j] = SorcEntry -> Hmnc.Ampl[j];
            DestEntry -> Phse[j] = SorcPhase -> Data[j];
            
            //Harmonic correction
            if(HCorrThreshold < 200.0 && i > 0 && j > 1 && j < 15)
                if(fabs(DestEntry -> Freq[j] - (DestEntry -> Freq[j - 1] +
                    DestEntry -> Freq[0])) > HCorrThreshold)
                {
                    DestEntry -> Freq[j] = DestEntry -> Freq[j - 1] +
                                           DestEntry -> Freq[0];
                    DestEntry -> Ampl[j] = (DestEntry - 1) -> Ampl[j];
                }
        }
    }
    RDelete(& PAna);
    
    if(VerboseFlag)
    printf("Invariant region analysis...\n");
    //Generate Invar end points
    List_HNMContour ContourList;
    RCall(List_HNMContour, CtorSize)(& ContourList,
        HAna.HNMList.Frames_Index + 1, WinSize);
    ContourList.Frames_Index = HAna.HNMList.Frames_Index;
    
    for(i = 0; i <= ContourList.Frames_Index; i ++)
        RCall(HNMFrame, ToContour)(& HAna.HNMList.Frames[i],
            & ContourList.Frames[i]);
    
    Array_Define(Real, LocalDiff);
    Array_Ctor(Real, LocalDiff);
    Array_Resize(Real, LocalDiff, ContourList.Frames_Index + 1);
    Array_Define(Real, AvgDiff);
    Array_Ctor(Real, AvgDiff);
    
    LocalDiff_Index = ContourList.Frames_Index;
    for(i = 0; i <= LocalDiff_Index; i ++) LocalDiff[i] = 0;
    
    for(i = ContourList.Frames_Index * 0.02;
        i < ContourList.Frames_Index * 0.07;
        i ++)
    {
        HNMContour* Cmp1 = & ContourList.Frames[i * 10];
        for(j = 0; j <= LocalDiff_Index; j ++)
        {
            HNMContour* Cmp2 = & ContourList.Frames[j];
            Real Diff = ContourDiff(Cmp1, Cmp2);
            //Normalize
            LocalDiff[j] += sigmoid(Diff) / (Real)LocalDiff_Index;
        }
    }
    
    int AvgLen = WinSize / HopSize;
    if(AvgLen > LocalDiff_Index / 8) AvgLen = LocalDiff_Index / 8;
    Array_From(Real, AvgDiff, LocalDiff);
    for(j = AvgLen; j < LocalDiff_Index - AvgLen; j ++)
        AvgDiff[j] = RCall(CDSP2_VSum, Real)(LocalDiff, j - AvgLen, j + AvgLen)
                   / AvgLen / 2.0;
    
    Real Min = AvgDiff[0];
    Real MinIndex = 0;
    for(j = 0; j <= AvgDiff_Index; j ++)
        if(AvgDiff[j] < Min)
        {
            Min = AvgDiff[j];
            MinIndex = j;
        }
    
    Real InvarMin = Min + InvarThreshold;
    Dest -> InvarLeft  = HAna.PulseList.Frames[0];
    Ret  -> InvarLeft  = HAna.PulseList.Frames[0];
    Dest -> InvarRight = TopOf(HAna.PulseList.Frames);
    Ret  -> InvarRight = TopOf(HAna.PulseList.Frames);
    for(j = MinIndex; j > 0; j --)
        if(AvgDiff[j] > InvarMin)
        {
            Dest -> InvarLeft = HAna.PulseList.Frames[j];
            Ret  -> InvarLeft = Dest -> InvarLeft;
            break;
        }
    
    for(j = MinIndex; j <= LocalDiff_Index; j ++)
        if(AvgDiff[j] > InvarMin)
        {
            Dest -> InvarRight = HAna.PulseList.Frames[j];
            Ret  -> InvarRight = Dest -> InvarRight;
            break;
        }
    
    Array_Dtor(Real, LocalDiff);
    Array_Dtor(Real, AvgDiff);
    RDelete(& F0Iter, & HAna, & ContourList);
    return FRet;
}

