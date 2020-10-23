#include "windows.h"

extern int main(int argc, char **argv);

int __stdcall WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
	int lIRc = main(__argc, __argv);
	exit(lIRc);
}