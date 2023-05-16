#include "main.h"
#include "source.cpp"

int wmain(int argc, wchar_t **argv)
{

#ifdef _WIN32
#include <fileapi.h>
    char *appdata = getenv("APPDATA");
    APPDATA_FOLDER = (char *)malloc(strlen(appdata) + 15);
    sprintf(APPDATA_FOLDER, "%s\\CactusViewer", appdata);
    CreateDirectoryA(APPDATA_FOLDER, NULL);
#else
    APPDATA_FOLDER = "./";
#endif
	loaderthreadinputs Inputs;

    Main_Init();
    ScanFolder(argv[1]);
    if (argc > 1)
    {
        Inputs = {argv[1], G->CurrentFileIndex, G->Files[G->CurrentFileIndex].type};
        CreateThread(NULL, 0, LoaderThread, (LPVOID)&Inputs, 0, NULL);
    }

    while (Running)
    {
        bool gifmode = false;
        if (G->Files.Count > 0) gifmode = G->Files[G->CurrentFileIndex].type == 1;
        MouseDetection = WindowHeight - 140 - 60 * (gifmode);
        PollEvents();
        G->ShowUI = ShouldShowUI();

        if (G->Droppedfile)
        {
            G->Loading_Droppedfile = true;
            bool is_dir = ScanFolder(TempPath);
            G->loaded = false;
            Inputs = {TempPath, G->CurrentFileIndex, G->Files[G->CurrentFileIndex].type, true};
            if (is_dir)
                Inputs.File = G->Files[0].file.path;
            CreateThread(NULL, 0, LoaderThread, (LPVOID)&Inputs, 0, NULL);
            G->Droppedfile = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        GetWindowSize();

        if (G->signals.UpdatePass)
            G->signals.UpdatePass = false;
        UpdateLogic();
        UpdateGUI();
        Render();

        if (G->Files.Count > 0)
        {
            if ((!G->sorting && keyup(Key_Right)) || G->signals.nextimage)
            {
                G->signals.nextimage = false;

                if (G->CurrentFileIndex < G->Files.Count - 1)
                {
                    G->CurrentFileIndex++;
                    G->loaded = false;
                    Inputs = {G->Files[G->CurrentFileIndex].file.path, G->CurrentFileIndex, G->Files[G->CurrentFileIndex].type, false};
                    CreateThread(NULL, 0, LoaderThread, (LPVOID)&Inputs, 0, NULL);
                }
            }
            if ((!G->sorting && keyup(Key_Left))|| G->signals.previmage)
            {
                G->signals.previmage = false;

                if (G->CurrentFileIndex > 0)
                {
                    G->CurrentFileIndex--;
                    G->loaded = false;
                    Inputs = {G->Files[G->CurrentFileIndex].file.path, G->CurrentFileIndex, G->Files[G->CurrentFileIndex].type, false};
                    CreateThread(NULL, 0, LoaderThread, (LPVOID)&Inputs, 0, NULL);
                }
            }
        }
        ResetInputs();
    }
    SaveSettings();
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    if(AttachConsole (ATTACH_PARENT_PROCESS) != 0) {
        FILE *  fpstdin = stdin, *fpstdout = stdout, *fpstderr = stderr;  
        freopen_s (&fpstdin,  "CONIN$",  "r", stdin);  
        freopen_s (&fpstdout, "CONOUT$", "w", stdout);  
        freopen_s (&fpstderr, "CONOUT$", "w", stderr);  
    } 
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
    int result = wmain(argc, argv);
    return result;
}