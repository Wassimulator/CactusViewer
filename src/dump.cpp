#if 0
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
    SDL_Delay(7000);
    LPWSTR *szArglist;
    int argc;
    szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
    int length = wcslen((const wchar_t *)szArglist);
    char *argv1 = (char *)malloc(length);
    wcstombs(argv1, szArglist[1], length);
#endif