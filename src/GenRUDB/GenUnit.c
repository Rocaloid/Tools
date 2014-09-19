#include "GenUnit.h"
#include <stdlib.h>
#include <CVESVP.h>

Real SinusoidDiff(Sinusoid* Sorc1, Sinusoid* Sorc2, int HNum)
{
    int i;
    Real* Diff = RCall(RAlloc, Real)(HNum);
    RCall(CDSP2_VSet, Real)(Diff, 0.0000001, HNum);
    
    if(Sorc1 -> Ampl_Index < HNum - 1)
        RCall(CDSP2_VCopy, Real)(Diff, Sorc1 -> Ampl,
            Sorc1 -> Ampl_Index + 1);
    else
        RCall(CDSP2_VCopy, Real)(Diff, Sorc1 -> Ampl, HNum);
    
    for(i = 0; i < HNum; i ++)
        Diff[i] = log(Diff[i]);
    
    if(Sorc2 -> Ampl_Index < HNum - 1)
    {
        for(i = 0; i <= Sorc2 -> Ampl_Index; i ++)
            Diff[i] -= log(Sorc2 -> Ampl[i]);
    }else
    {
        for(i = 0; i < HNum; i ++)
            Diff[i] -= log(Sorc2 -> Ampl[i]);
    }
    
    for(i = 0; i < HNum; i ++)
        if(fabs(Diff[i]) > 10) Diff[i] = 0;
    
    RCall(CDSP2_VMul, Real)(Diff, Diff, Diff, HNum);
    Real Ret = RCall(CDSP2_VSum, Real)(Diff, 0, HNum) / HNum / HNum;
    
    RFree(Diff);
    return Ret;
}

int MaxElmtIndex(Real* Sorc, int Size)
{
    Real Max = Sorc[0];
    int  Ret = 0;
    int i;
    
    for(i = 0; i < Size; i ++)
        if(Sorc[i] > Max)
        {
            Ret = i;
            Max = Sorc[i];
        }
    
    return Ret;
}

int GenUnit(RUCE_DB_Entry* Dest, Wave* Sorc)
{
    int FRet = 1;
    int WSize = Sorc -> Size;
    
    RCall(Wave, Resize)(Sorc, Sorc -> Size + WinSize * 2);
    
    //F0 estimation
    F0Iterlyzer F0Iter;
    RCall(F0Iterlyzer, Ctor)(& F0Iter);
    F0Iter.Option.Adlib = 1;
    F0Iter.Option.LFreq = LFundFreq / 2;
    F0Iter.Option.HFreq = UFundFreq;
    F0Iter.Option.Threshold = 0.01;
    F0Iter.Option.YIN.W = 300;
    F0Iter.Option.YIN.Threshold = 0.2;
    F0Iter.Option.Method = EF0;
    
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
    
    if(! VOTFlag)
        VOT = RCall(CSVP_VOTFromF0Match, Real)(& F0Iter.F0List, 30, 3, 1000);
    
    if(VerboseFlag)
    printf("VOT = %d\n", VOT);
    
    Real Sum = RCall(CDSP2_VSum, Real)(F0Iter.F0List.Y, 0,
        F0Iter.F0List.Y_Index + 1);
    Real AvgF0 = (Real)Sum / ((Real)F0Iter.F0List.Y_Index + 1.0);
    
    if(VerboseFlag)
    printf("Average fundamental frequency: %fHz (%s)\n",
        AvgF0,
        EF0 == CSVP_F0_YIN ? "YIN" : "SPECSTEP");
    
    //Noise Analysis
    if(VerboseFlag)
        printf("Noise analysis...\n");
    Wave ConWave, VowWave;
    RNew(Wave, & ConWave, & VowWave);
    SinusoidIterlyzer SAna;
    RCall(SinusoidIterlyzer, Ctor)(& SAna);
    SAna.GenPhase = 1;
    SAna.LeftBound = VOT + 1500;
    SAna.DThreshold = AvgF0 / 3.0;
    SAna.DFThreshold = AvgF0 / 5.0;
    
    RCall(SinusoidIterlyzer, SetHopSize)(& SAna, 128);
    RCall(SinusoidIterlyzer, SetWave)(& SAna, Sorc);
    RCall(SinusoidIterlyzer, SetPosition)(& SAna, SAna.LeftBound);
    RCall(SinusoidIterlyzer, SetUpperFreq)(& SAna, 9000);
    RCall(SinusoidIterlyzer, SetPitch)(& SAna, & F0Iter.F0List);
    
    RCall(SinusoidIterlyzer, PrevTo)(& SAna, 0);
    RCall(SinusoidIterlyzer, IterNextTo)(& SAna, SAna.LeftBound + 3000);
    
    RCall(CSVP_SinusoidalFromSinuList, Real)(& VowWave,
        & SAna.PulseList, & SAna.SinuList, & SAna.PhseList);
    
    int i;
    for(i = 0; i <= VowWave.Size; i ++)
        if(VowWave.Data[i] > 0.015)
            break;
    int  VOTSelDest = i + 1500;
    Real VOTSelMax = RCall(CDSP2_VMaxElmt, Real)(VowWave.Data, i, VOTSelDest);
    for(; i < VOTSelDest; i ++)
        if(VowWave.Data[i] > VOTSelMax / 2)
            break;
    VOT = i;
    if(VerboseFlag)
        printf("Refined VOT estimation: %d\n", VOT);
    
    RCall(CSVP_NoiseTurbFromWave, Real)(& ConWave, Sorc, & VowWave);
    
    RCall(SinusoidIterlyzer, Dtor)(& SAna);
    RDelete(& VowWave);
    
    //HNM Analysis
    HNMIterlyzer HAna;
    if(VerboseFlag)
        printf("HNM analysis...\n");
    RCall(HNMIterlyzer, CtorSize)(& HAna, WinSize);
    RCall(HNMIterlyzer, SetWave)(& HAna, Sorc);
    RCall(HNMIterlyzer, SetHopSize)(& HAna, HopSize);
    RCall(HNMIterlyzer, SetPosition)(& HAna, VOT + 2000);
    RCall(HNMIterlyzer, SetUpperFreq)(& HAna, USinuFreq);
    RCall(HNMIterlyzer, SetPitch)(& HAna, & F0Iter.F0List);
    HAna.GenPhase = 1;
    HAna.LeftBound = VOT + 1500;
    HAna.DThreshold = AvgF0 / 3.0;
    HAna.DFThreshold = AvgF0 / 5.0;
    if(VerboseFlag)
        printf("Backward HNM analysis...\n");
    if(! RCall(HNMIterlyzer, PrevTo)(& HAna, 0))
    {
        fprintf(stderr, "[Error] HNM backward analysis failed. Skipped.\n");
        RDelete(& F0Iter, & HAna);
        return 0;
    }
    if(VerboseFlag)
        printf("Forward analysis...\n");
    if(! RCall(HNMIterlyzer, IterNextTo)(& HAna, WSize))
    {
        fprintf(stderr, "[Error] HNM forward analysis failed. Skipped.\n");
        RDelete(& F0Iter, & HAna);
        return 0;
    }
    
    if(VerboseFlag)
        printf("%d HNM frames generated.\n", HAna.HNMList.Frames_Index + 1);
    
    //Filling in
    if(VerboseFlag)
    printf("Converting data structure...\n");
    int j;
    
    Dest -> HopSize = HopSize;
    Dest -> NoizSize = WinSize / 16;
    
    #undef Wave
    Dest -> Wave = realloc(Dest -> Wave, ConWave.Size * 4);
    for(i = 0; i < ConWave.Size; i ++)
        Dest -> Wave[i] = ConWave.Data[i];
    Dest -> WaveSize = ConWave.Size;
    Dest -> Samprate = Sorc -> SampleRate;
    #define Wave CDSP2_Wave_Float
    
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
            
            if(DestEntry -> Ampl[j] <= 0)
                DestEntry -> Ampl[j] = 0.0000001;
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
    
    if(VerboseFlag)
    printf("Invariant region analysis...\n");
    //Generate Invar end points
    
    #define DiffProcess() \
        for(i = 3; i < DiffSize - 3; i ++) \
        { \
            Real s1 = LocalDiff[i - 3] + LocalDiff[i - 2] + LocalDiff[i - 1]; \
            Real s2 = LocalDiff[i + 1] + LocalDiff[i + 2] + LocalDiff[i + 3]; \
            JumpDiff[i] = (s1 - s2) / 4.0; \
        } do {} while(0)
    
    int VOTIndex = CDSP2_List_Int_IndexAfter(& HAna.PulseList, VOT);
    int VHalfIndex = (HAna.HNMList.Frames_Index + VOTIndex) / 2;
    int DiffSize = HAna.HNMList.Frames_Index - VHalfIndex - 4;
    
    HNMFrame* EndingFrame = & HAna.HNMList.Frames
                             [HAna.HNMList.Frames_Index - DiffSize / 10];
    HNMFrame* MiddleFrame = & HAna.HNMList.Frames[VHalfIndex];
    
    Array_Define(Real, LocalDiff);
    Array_Ctor(Real, LocalDiff);
    Array_Define(Real, JumpDiff);
    Array_Ctor(Real, JumpDiff);
    Array_Resize(Real, LocalDiff, DiffSize);
    Array_Resize(Real, JumpDiff , DiffSize);
    LocalDiff_Index = DiffSize - 1;
    JumpDiff_Index  = DiffSize - 1;
    
    RCall(CDSP2_VSet, Real)(LocalDiff, 0, DiffSize);
    RCall(CDSP2_VSet, Real)(JumpDiff , 0, DiffSize);
    
    for(i = 0; i < DiffSize; i ++)
    {
        HNMFrame* CompareFrame = & HAna.HNMList.Frames[i + VHalfIndex];
        LocalDiff[i] = SinusoidDiff(& EndingFrame -> Hmnc,
                                    & CompareFrame -> Hmnc, 8);
    }
    
    DiffProcess();
    
    int FinalIndex = MaxElmtIndex(JumpDiff, DiffSize);
    
    DiffSize = VHalfIndex - VOTIndex + 1;
    Array_Resize(Real, LocalDiff, DiffSize);
    Array_Resize(Real, JumpDiff , DiffSize);
    RCall(CDSP2_VSet, Real)(LocalDiff, 0, DiffSize);
    RCall(CDSP2_VSet, Real)(JumpDiff , 0, DiffSize);
    
    for(i = 0; i < DiffSize; i ++)
    {
        HNMFrame* CompareFrame = & HAna.HNMList.Frames[i + VOTIndex];
        LocalDiff[i] = SinusoidDiff(& MiddleFrame -> Hmnc,
                                    & CompareFrame -> Hmnc, 8);
    }
    
    DiffProcess();
    
    int LeftIndex = MaxElmtIndex(JumpDiff, DiffSize);
    
    Dest -> VOT = (Real)VOT / Sorc -> SampleRate;
    Dest -> SOT = Dest -> VOT;
    Dest -> InvarLeft  = HAna.PulseList.Frames[VOTIndex   + LeftIndex];
    Dest -> InvarRight = HAna.PulseList.Frames[VHalfIndex + FinalIndex];
    Dest -> InvarLeft  /= Sorc -> SampleRate;
    Dest -> InvarRight /= Sorc -> SampleRate;
    
    Array_Dtor(Real, LocalDiff);
    Array_Dtor(Real, JumpDiff);
    RDelete(& F0Iter, & HAna, & ConWave);
    return FRet;
}

