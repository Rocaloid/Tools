#ifndef GENRUDB_COMMON_H
#define GENRUDB_COMMON_H

#define Version "1.2.1.0"

char* CRotoFile;
char* CUnitName;
int   ReadOnlyFlag;
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

String FundMethod, UnitName, WindowName;

enum
{
    Hanning = 0,
    Hamming,
    Blackman
} EWindow;

int EF0;

#endif

