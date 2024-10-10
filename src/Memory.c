#include "Memory.h"

int GetTotalThreads() {
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwNumberOfProcessors;
}

int StrLen(const char* string) {
	int stringLength = -1;
	while (string[++stringLength] != '\0');
	return stringLength;
}

int HexToInt(const char* hex) {
	static const char* base16Upper = "0123456789ABCDEF";
	static const char* base16Lower = "0123456789abcdef";
	
	const int hexLength = StrLen(hex);

	int intValue = 0;
	int basePower = 1;

	for (int i = hexLength - 1; i >= 0; i--) {
		char hexBit = hex[i];
		int hexBitValue = 0;

		for (int j = 0; j < 16; j++) {
			if (hexBit == base16Upper[j] || hexBit == base16Lower[j]) {
				hexBitValue = j;
				break;
			}
		}

		intValue += basePower * hexBitValue;
		basePower *= 16;
	}

	return intValue;
}

HANDLE GetProcessHandle(const char* processName) {
	HANDLE hProcess;
	HMODULE hModule;
	TCHAR szProcessName[MAX_PATH];
	DWORD lpidProcess[1024], cbNeeded;

	if (!EnumProcesses(lpidProcess, sizeof(lpidProcess), &cbNeeded)) return NULL;
	
	const int totalProcesses = (int)(cbNeeded / sizeof(DWORD));
	const int processNameLength = StrLen(processName);

	for (int i = 0; i < totalProcesses; i++) {
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, lpidProcess[i]);

		if (hProcess != NULL && EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded) != 0) {
			const int baseNameLength = GetModuleBaseName(hProcess, hModule, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
			BOOL foundProcess = TRUE;

			for (int j = 0; j < baseNameLength; j++) {
				if (baseNameLength != processNameLength || (char)szProcessName[j] != processName[j]) {
					foundProcess = FALSE;
					break;
				}
			}

			if (foundProcess) return hProcess;

			CloseHandle(hProcess);
		}
	}

	return NULL;
}

DWORD WINAPI ThreadTask(LPVOID lpParam) {
	ThreadArgs* args = (ThreadArgs*)lpParam;
	
	const int totalThreads = GetTotalThreads();
	const int mbiArrActualSize = args->mbiAddrArr->size >> 1;
	
	const int iterationsRemainder = mbiArrActualSize % totalThreads;
	const int iterationsPerThread = (mbiArrActualSize - iterationsRemainder) / totalThreads;

	for (int i = args->tId * iterationsPerThread; i < (args->tId + 1) * iterationsPerThread + (args->tId == totalThreads - 1 ? iterationsRemainder : 0); i++) {
		LPVOID baseAddress = (LPVOID)args->mbiAddrArr->data[2 * i];
		SIZE_T regionSize = (SIZE_T)args->mbiAddrArr->data[2 * i + 1];

		BYTE* byteBuffer = (BYTE*)malloc(regionSize * sizeof(BYTE));
		SIZE_T bytesRead;

		if (byteBuffer == NULL) continue;

		if (ReadProcessMemory(args->hProcess, baseAddress, byteBuffer, regionSize, &bytesRead)) {
			for (int j = 0; j < (int)bytesRead - args->patternBytesSize; j++) {
				BOOL found = TRUE;

				for (int k = 0; k < args->patternBytesSize; k++) {
					if (args->patternBytes[k] != -1 && byteBuffer[j + k] != (BYTE)args->patternBytes[k]) {
						found = FALSE;
						break;
					}
				}
				
				if (found) PushBack(args->foundAddrArr, (UINT64)baseAddress + j);
			}
		}

		free(byteBuffer);
	}
	
	return 0;
}

void FindPattern(HANDLE hProcess, char pattern[], Array* foundAddrArr) {
	SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
	
    LPVOID address = (LPVOID)hProcess;
    MEMORY_BASIC_INFORMATION mbi;
	
	char* patternCopy = (char*)malloc(StrLen(pattern) * sizeof(char));
	memcpy(patternCopy, pattern, StrLen(pattern));
	
	int patternBytes[512];
	int patternBytesSize = 0;
	
	char* nextToken = NULL;
	char* token = strtok_s(patternCopy, " ", &nextToken);

	while (token != NULL) {
		patternBytes[patternBytesSize++] = (char)token[0] == '?' ? -1 : HexToInt(token);
		token = strtok_s(NULL, " ", &nextToken);
	}
	
	free(patternCopy);
		
	Array mbiAddrArr;
	InitArray(&mbiAddrArr);
	
    while (address < sysInfo.lpMaximumApplicationAddress) {
		if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_READONLY || mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_WRITECOPY || mbi.Protect == PAGE_EXECUTE_READ || mbi.Protect == PAGE_EXECUTE_READWRITE)) {
				PushBack(&mbiAddrArr, (Array_T)mbi.BaseAddress);
				PushBack(&mbiAddrArr, (Array_T)mbi.RegionSize);
			}

            address = (LPBYTE)address + mbi.RegionSize;
        }
    }
		
	int totalThreads = GetTotalThreads();
	
	HANDLE* threads = (HANDLE*)malloc(totalThreads * sizeof(HANDLE));
	ThreadArgs* threadArgs = (ThreadArgs*)malloc(totalThreads * sizeof(ThreadArgs));
	
	ClearArray(foundAddrArr, 0);
	
	for (int i = 0; i < totalThreads; i++) {
		threadArgs[i].hProcess = hProcess;
		threadArgs[i].tId = i;
		threadArgs[i].mbiAddrArr = &mbiAddrArr;
		threadArgs[i].foundAddrArr = foundAddrArr;
		threadArgs[i].patternBytes = patternBytes;
		threadArgs[i].patternBytesSize = patternBytesSize;
		
		threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadTask, (LPVOID)&threadArgs[i], 0, NULL);
	}
	
	WaitForMultipleObjects(totalThreads, threads, TRUE, INFINITE);
	
	for (int i = 0; i < totalThreads; i++) {
		CloseHandle(threads[i]);
	}

	free(threads);
	free(threadArgs);
	FreeArray(&mbiAddrArr);
}

int ReadInt(HANDLE hProcess, UINT64 address) {
	BYTE byteBuffer[4];
	SIZE_T bytesRead;

	int value = 0;

	if (ReadProcessMemory(hProcess, (LPCVOID)address, byteBuffer, 4, &bytesRead) && bytesRead == 4) {
		for (int i = 3; i >= 0; i--) {
			value = (value << 8) | byteBuffer[i];
		}
	}

	return value;
}
