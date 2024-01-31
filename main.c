
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <commdlg.h>
#include <tlhelp32.h>

#define MAX_PROCESSES 200

enum injectionMethodEnum
{
    NONE,
    LOADLIBRARY,
    SETWINDOWSHOOK
};
enum injectionMethodEnum injectionMethod = LOADLIBRARY;

PROCESSENTRY32 processList[200];

PROCESSENTRY32 *targetProcess = NULL;

// Global variables for UI elements
HWND loadLibraryCheckbox, setWindowsHookCheckbox, injectButton;
HWND hwndPathDisplay, hwndList, injectButton;
char dllPath[MAX_PATH] = {0};

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL InitializeApplication(HINSTANCE hInstance);
BOOL CreateMainWindow(HINSTANCE hInstance, int nCmdShow);
BOOL isReadyToInject();
void PopulateProcessList(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if (!InitializeApplication(hInstance))
        return 0;
    return CreateMainWindow(hInstance, nCmdShow) ? GetMessageLoop() : 0;
}

BOOL InitializeApplication(HINSTANCE hInstance)
{
    const char CLASS_NAME[] = "Sample Window Class";
    WNDCLASS wc = {0};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    return RegisterClass(&wc) != 0;
}

BOOL CreateMainWindow(HINSTANCE hInstance, int nCmdShow)
{
    HWND hwnd = CreateWindowEx(0, "Sample Window Class", "DLL Injector",
                               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400,
                               NULL, NULL, hInstance, NULL);
    if (hwnd == NULL)
        return FALSE;
    ShowWindow(hwnd, nCmdShow);
    return TRUE;
}

int GetMessageLoop()
{
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        CreateUIElements(hwnd);
        PopulateProcessList(hwnd);
        break;
    case WM_COMMAND:
        HandleWM_COMMAND(hwnd, wParam, lParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        PaintWindow(hwnd);
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateUIElements(HWND hwnd)
{
    // AllocConsole();
    loadLibraryCheckbox = CreateWindow("BUTTON", "Load Library",
                                       WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
                                       10, 10, 150, 30,
                                       hwnd, (HMENU)1, NULL, NULL);

    CheckDlgButton(hwnd, 1, BST_CHECKED);

    setWindowsHookCheckbox = CreateWindow("BUTTON", "Set Windows Hook",
                                          WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
                                          10, 50, 150, 30,
                                          hwnd, (HMENU)2, NULL, NULL);

    injectButton = CreateWindow("BUTTON", "Inject",
                                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                10, 170, 100, 30,
                                hwnd, (HMENU)3, NULL, NULL);

    EnableWindow(injectButton, 0);

    CreateWindow("BUTTON", "Select a DLL",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 10, 90, 100, 30,
                 hwnd, (HMENU)4, NULL, NULL);

    hwndPathDisplay = CreateWindow("STATIC", "No DLL selected",
                                   WS_VISIBLE | WS_CHILD | SS_LEFT,
                                   120, 90, 300, 30,
                                   hwnd, (HMENU)5, NULL, NULL);

    hwndList = CreateWindow("LISTBOX", NULL,
                            WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_STANDARD,
                            120, 130, 200, 100,
                            hwnd, (HMENU)6, NULL, NULL);

    CreateWindow("STATIC", "Select Process",
                 WS_VISIBLE | WS_CHILD,
                 10, 130, 100, 25,
                 hwnd, NULL, NULL, NULL);
}

void PopulateProcessList(HWND hwnd)
{
    int pidCount = getProcessIdsXD();
    if (pidCount == 0)
    {
        MessageBox(hwnd, "No processes found!", "Error", MB_OK);
        PostQuitMessage(0);
        return 0;
    }
    else
    {
        for (int i = 0; i < pidCount; i++)
        {
            char buffer[64 + MAX_PATH];

            sprintf(buffer, "%s (ID: %lu)", processList[i].szExeFile, processList[i].th32ProcessID);

            int index = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)buffer);

            SendMessage(hwndList, LB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
        }
    }
}

void HandleWM_COMMAND(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    int wmId = LOWORD(wParam);
    switch (wmId)
    {
    case 1:
        injectionMethod = LOADLIBRARY;
        CheckDlgButton(hwnd, 1, BST_CHECKED);
        CheckDlgButton(hwnd, 2, BST_UNCHECKED);
        printf("Injection method: %d\n", injectionMethod);
        isReadyToInject();
        break;
    case 2:
        injectionMethod = SETWINDOWSHOOK;
        CheckDlgButton(hwnd, 2, BST_CHECKED);
        CheckDlgButton(hwnd, 1, BST_UNCHECKED);
        printf("Injection method: %d\n", injectionMethod);
        isReadyToInject();
        break;
    case 3:
        switch (injectionMethod)
        {
        case LOADLIBRARY:
            printf("LOADLIBRARY Injection method: %d\n", injectionMethod);
            loadLibraryInject(dllPath, targetProcess->th32ProcessID);
            break;
        case SETWINDOWSHOOK:
            printf("SETWINDOWSHOOK Injection method: %d\n", injectionMethod);
            setWindowsHookInject(dllPath, targetProcess->th32ProcessID);
            break;
        }
        break;
    case 4:
    {
        OPENFILENAME ofn;
        char szFile[260];
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = dllPath;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(dllPath);
        ofn.lpstrFilter = "Dll Files (*.dll)\0*.dll\0\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn) == TRUE)
        {
            printf("File selected: %s\n", ofn.lpstrFile);
            size_t path_length = strlen(ofn.lpstrFile);
            printf("Path length: %zu\n", path_length);

            const char *ext = strrchr(ofn.lpstrFile, '.');
            if (ext != NULL)
            {

                if (stricmp(ext, ".dll") == 0)
                {
                    isReadyToInject();
                    SetWindowText(hwndPathDisplay, ofn.lpstrFile);
                    break;
                }
                else
                {
                    printf("The selected file is not a DLL.\n");
                }
            }
            else
            {
                printf("No file extension found.\n");
            }
        }
        break;
    }

    case 6:
    {
        switch (HIWORD(wParam))
        {
        case LBN_SELCHANGE:
        {
            int index = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
            int processIndex = SendMessage(hwndList, LB_GETITEMDATA, index, 0);
            printf("Process index: %d name: %s\n", processIndex, processList[processIndex].szExeFile);
            targetProcess = &processList[processIndex];
            printf("Target process name: %s\n", targetProcess->szExeFile);
            isReadyToInject();
        }
        }

        break;
    }
    }
}

void PaintWindow(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
    EndPaint(hwnd, &ps);
}

BOOL isReadyToInject()
{
    if (dllPath[0] == '\0' || injectionMethod == NONE || targetProcess == NULL)
    {
        return FALSE;
    }
    EnableWindow(injectButton, 1);
    return TRUE;
}
int getProcessIdsXD()
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    int pidCount = 0;

    if (Process32First(hSnapshot, &pe32))
    {
        while (Process32Next(hSnapshot, &pe32) && pidCount < 200)
        {
            if (strcmp(pe32.szExeFile, "svchost.exe") != 0 && strcmp(pe32.szExeFile, "msedge.exe") != 0)
            {
                processList[pidCount] = pe32;
                pidCount++;
            }
        }
    }

    CloseHandle(hSnapshot);
    return pidCount;
}
