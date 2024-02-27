# DLL-Injector
A DLL injector that can use various different injection methods.

# Injection methods
- Standard CreateRemoteThread injection: allocate memory in target process, write desired dll file path into it and then create remote thread in said process to load said dll. 
- SetWindowsHook: set a window hook on target process to load library, this enables you to execute code from within the hooked process without the use of CreateRemoteThread. Can also be combined with various different methods to load a DLL or execute code to avoid calling LoadLibrary.

Injector uses only WinAPI.
![image](https://github.com/Hampton33/C-DLL-INJECTOR/assets/145158981/f8da4797-d460-49aa-a513-13937c097cbd)
