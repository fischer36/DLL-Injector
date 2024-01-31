#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "user32.lib")

BOOL APIENTRY DllMain(HMODULE hModule, DWORD nReason, LPVOID lpReserved)
{
    switch (nReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

__declspec(dllexport) int LoadDllFromWithin()
{
    AllocConsole();
    MessageBox(NULL, "Hello from DLL loader hook!", "dll-loader", MB_OK);
    char path_buffer[260]; // 260 Windows max path constant
    FILE *file;

    file = fopen("C:\\Users\\joakr\\Desktop\\C-DLL-INJECTOR\\loader\\dll_path.txt", "r");
    if (file == NULL)
    {
        MessageBox(NULL, "Can't find txt file or txt file is empty", "dll-loader", MB_OK);
        printf("Can't find txt file or txt file is empty\n");
        return 0;
    }

    if (fgets(path_buffer, sizeof(path_buffer), file) != NULL)
    {
        size_t len = strlen(path_buffer);
        if (len > 0 && path_buffer[len - 1] == '\n')
        {
            path_buffer[len - 1] = '\0';
        }

        printf("Path: %s\n", path_buffer);
        fclose(file);

        HMODULE dllHandle = LoadLibrary(path_buffer);
        if (dllHandle == NULL)
        {
            printf("Failed to load DLL\n");
            return 0;
        }
        else
        {
            printf("DLL loaded successfully\n");
            return 1;
        }
    }
    else
    {
        printf("Error reading from file\n");
        fclose(file);
        return 0;
    }
}
