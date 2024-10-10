#ifndef MEMORY_H
#define MEMORY_H

#include <Windows.h>
#include <Psapi.h>

#include "Array.h"

typedef struct _ThreadArgs {
	HANDLE hProcess;
	int tId;
	Array* mbiAddrArr;
	Array* foundAddrArr;
	int* patternBytes;
	int patternBytesSize;
} ThreadArgs;

int GetTotalThreads();
int StrLen(const char* string);
int HexToInt(const char* hex);
HANDLE GetProcessHandle(const char* processName);
DWORD WINAPI ThreadTask(LPVOID lpParam);
void FindPattern(HANDLE hProcess, char pattern[], Array* foundAddrArr);
int ReadInt(HANDLE hProcess, UINT64 address);

#endif