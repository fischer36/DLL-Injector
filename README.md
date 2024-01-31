# DLL-Injector
 DLL injector with multiple injection methods made in C. Made because I wanted to experiment with C and the windows API.

# Injection methods
- Standard CreateRemoteThread injection: allocate memory in target process, write desired DLL path into it and then create remote thread in said process to load said dll. 
- SetWindowsHook: set a window hook on target process that enables you to execute code internally without the use of CreateRemoteThread. Can also be combined with various different methods to load a DLL or execute code to avoid calling LoadLibrary.
