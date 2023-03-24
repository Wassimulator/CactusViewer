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

    Main_Init();
    ScanFolder(argv[0]);
    if (argc > 0)
    {
        loaderthreadinputs Inputs = {argv[0], G->CurrentFileIndex, G->Files[G->CurrentFileIndex].type};
        CreateThread(NULL, 0, LoaderThread, (LPVOID)&Inputs, 0, NULL);
    }

    while (Running)
    {
        MouseDetection = WindowHeight - 140;
        PollEvents();
        G->ShowUI = ShouldShowUI();

        if (G->Droppedfile)
        {
            G->Loading_Droppedfile = true;
            ScanFolder(TempPath);
            G->loaded = false;
            loaderthreadinputs Inputs = {TempPath, G->CurrentFileIndex, G->Files[G->CurrentFileIndex].type, true};
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
                    loaderthreadinputs Inputs = {G->Files[G->CurrentFileIndex].file.path, G->CurrentFileIndex, G->Files[G->CurrentFileIndex].type, false};
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
                    loaderthreadinputs Inputs = {G->Files[G->CurrentFileIndex].file.path, G->CurrentFileIndex, G->Files[G->CurrentFileIndex].type, false};
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
    int argc;
    LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);
    int result = wmain(argc, argv);
    return result;
}