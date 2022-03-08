// DLL-Injector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

using namespace std;


DWORD GetProcId(const char* procName)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_stricmp(procEntry.szExeFile, procName))
                {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}



int main()
{
    const char* dllPath = "c:\\csgointernal\\Debug\\csgo.dll";
    const char* procName = "csgo.exe";
    // two slashes for character escape

    DWORD procId = 0;

    while (!procId) {
        procId = GetProcId(procName);
        Sleep(50);
    }
    // get full permissions the process
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

    // ensure process is not null or invalid handle 
    if (hProc && hProc != INVALID_HANDLE_VALUE) {
        // allocate memory in the process
        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        // add path to the target process so it can be set as a parameter
        WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);
        // call LoadLibrary API with DLL path set as parameter
        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

        if (hThread) {
            CloseHandle(hThread);
        }

    }
    if (hProc) {
        CloseHandle(hProc);
    }
    
    return 0;
}

