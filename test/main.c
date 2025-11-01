#include <windows.h>
#include <wincon.h>

HANDLE std_out;
LPCSTR str = "Hello world\n";
DWORD len = 12;

int main()
{
    std_out = GetStdHandle(STD_OUTPUT_HANDLE);

    WriteConsoleA(std_out, str, len, NULL, NULL);

    ExitProcess(0);
}