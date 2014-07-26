#ifndef GENRUDB_COMMON_H
#define GENRUDB_COMMON_H

#define Version "1.0.0.3"

char* CRotoFile;
char* CUnitName;
int   ReadOnlyFlag;
float UFundFreq;
float LFundFreq;
char* CFundMethod;
float USinuFreq;
int   HopSize;
int   WinSize;
char* CWindow;
int   VOT;
int   VOTFlag;
int   Offset;
float InvarThreshold;
int   VerboseFlag;

String FundMethod, UnitName, WindowName;

enum
{
    Hanning = 0,
    Hamming,
    Blackman
} EWindow;

enum
{
    YIN = 0,
    SpecStep = 1
} EF0;

#endif

