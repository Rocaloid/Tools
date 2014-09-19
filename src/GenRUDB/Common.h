#ifndef GENRUDB_COMMON_H
#define GENRUDB_COMMON_H

#define Version "1.3.0.1"

char* CWavFile;
int   QuitFlag;
float UFundFreq;
float LFundFreq;
char* CFundMethod;
float USinuFreq;
float HCorrThreshold;
int   HopSize;
int   WinSize;
char* CWindow;
int   VOT;
int   VOTFlag;
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

