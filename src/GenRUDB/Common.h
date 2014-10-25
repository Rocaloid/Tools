#ifndef GENRUDB_COMMON_H
#define GENRUDB_COMMON_H

#define Version "1.3.1.1"

char* CWavFile;
int   QuitFlag;
float UFundFreq;
float LFundFreq;
char* CFundMethod;
int   Stochastic;
float USinuFreq;
float HCorrThreshold;
int   HopSize;
int   WinSize;
char* CWindow;
float VOT;
int   VOTFlag;
float Alpha;
float InvarThreshold;
int   VerboseFlag;

String FundMethod, UnitName, WavName, WindowName;

enum
{
    Hanning = 0,
    Hamming,
    Blackman
} EWindow;

int EF0;

#endif

#ifdef PMatch
#undef PMatch
#endif
#define PMatch PMatch_Float_Float
