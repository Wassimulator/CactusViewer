#include "main.h"
#include "source.cpp"

int main(int argc, char **argv)
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
    ScanFolder(argv[1]);
    if (argc > 1)
    {
        loaderthreadinputs Inputs = {argv[1], G->CurrentFileIndex, G->files_TYPE[G->CurrentFileIndex]};
        CreateThread(NULL, 0, LoaderThread, (LPVOID)&Inputs, 0, NULL);
    }

    while (Running)
    {
        MouseDetection = WindowHeight - 320;
        PollEvents();

        if (G->Droppedfile)
        {
            G->Loading_Droppedfile = true;
            ScanFolder(TempPath);
            G->loaded = false;
            loaderthreadinputs Inputs = {TempPath, G->CurrentFileIndex, G->files_TYPE[G->CurrentFileIndex], true};
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

        if (G->max_files > 0)
        {
            if (keyup(Key_Right) || G->signals.nextimage)
            {
                G->signals.nextimage = false;

                if (G->CurrentFileIndex < G->max_files - 1)
                {
                    G->CurrentFileIndex++;
                    G->loaded = false;
                    loaderthreadinputs Inputs = {G->files[G->CurrentFileIndex].path, G->CurrentFileIndex, G->files_TYPE[G->CurrentFileIndex], false};
                    CreateThread(NULL, 0, LoaderThread, (LPVOID)&Inputs, 0, NULL);
                }
            }
            if (keyup(Key_Left)|| G->signals.previmage)
            {
                G->signals.previmage = false;

                if (G->CurrentFileIndex > 0)
                {
                    G->CurrentFileIndex--;
                    G->loaded = false;
                    loaderthreadinputs Inputs = {G->files[G->CurrentFileIndex].path, G->CurrentFileIndex, G->files_TYPE[G->CurrentFileIndex], false};
                    CreateThread(NULL, 0, LoaderThread, (LPVOID)&Inputs, 0, NULL);
                }
            }
        }
        ResetInputs();
    }
    SaveSettings();
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int result = main(__argc, __argv);
    return result;
}