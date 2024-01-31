#include <windows.h>
#include <stdio.h>

HANDLE hThread;

BOOL test = FALSE;

BOOL loadLibraryInject(char *dllPath, DWORD processID);

BOOL setWindowsHookInject(char *dllPath, DWORD processID);

BOOL loadLibraryInject(char *dllPath, DWORD processID)
{

    printf("Injecting dll: %s\n", dllPath);
    printf("Process ID: %ld\n", processID);
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (hProcess == NULL)
    {
        printf("Failed to open process: %d\n", GetLastError());
        return FALSE;
    }

    PVOID pRemoteBuffer = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteBuffer == NULL)
    {
        printf("Failed to allocate memory: %d\n", GetLastError());
        return FALSE;
    }
    if (WriteProcessMemory(hProcess, pRemoteBuffer, (LPVOID)dllPath, strlen(dllPath) + 1, NULL) == FALSE)
    {
        printf("Failed to write memory: %d\n", GetLastError());
        return FALSE;
    }

    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pRemoteBuffer, 0, NULL);
    if (hThread == NULL)
    {
        printf("Failed to create remote thread: %d\n", GetLastError());
        return FALSE;
    }

    Sleep(1000);
    VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    CloseHandle(hThread);
    printf("Injected DLL into process id: %d\n", processID);
    return TRUE;
}

BOOL setWindowsHookInject(char *dllPath, DWORD processID)

{

    if (test == TRUE)
    {
        printf("test is TRUE\n");
        return FALSE;
    }
    test == TRUE;
    HWND hWindowList[5];
    int WindowCount = 0;

    HWND hLocalHwnd = FindWindowEx(NULL, NULL, NULL, NULL);
    while (hLocalHwnd != NULL && WindowCount < 5)
    {
        DWORD localProcessID = 0;
        GetWindowThreadProcessId(hLocalHwnd, &localProcessID);
        if (localProcessID == processID)
        {
            hWindowList[WindowCount] = hLocalHwnd;
            WindowCount++;
        }

        hLocalHwnd = FindWindowEx(NULL, hLocalHwnd, NULL, NULL);
    }

    if (WindowCount == 0)
    {
        printf("No window found for process id: %d\n", processID);
        return FALSE;
    }

    hLocalHwnd = hWindowList[0];
    printf("Found window for process id: %d", processID);

    DWORD hWindowThread = GetWindowThreadProcessId(hLocalHwnd, NULL);
    if (hWindowThread == 0)
    {
        printf("Failed to get window thread: %d", GetLastError());
        return FALSE;
    }

    FILE *file = fopen("C:\\Users\\joakr\\Desktop\\C-DLL-INJECTOR\\loader\\dll_path.txt", "w");
    if (file == NULL)
    {
        int errnum = errno;
        fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
        return FALSE;
    }

    fprintf(file, "%s", dllPath);

    printf("DLL Path written into txt file for loader to read");
    fclose(file);

    HMODULE dllLoader = LoadLibraryA("C:\\Users\\joakr\\Desktop\\C-DLL-INJECTOR\\loader\\loader.dll");
    if (dllLoader == NULL)
    {
        printf("Error loading the loader dll");
    }

    HOOKPROC hookProc = (HOOKPROC)GetProcAddress(dllLoader, "LoadDllFromWithin");
    HHOOK hHook = SetWindowsHookEx(WH_GETMESSAGE, hookProc, dllLoader, hWindowThread);
    if (hHook == NULL)
    {
        printf("Failed to set windows hook: %d", GetLastError());
        return FALSE;
    }

    PostThreadMessage(hWindowThread, WM_NULL, 0, 0);
    Sleep(10);
    UnhookWindowsHookEx(hHook);
    return TRUE;
}
