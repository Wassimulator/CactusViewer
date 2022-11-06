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
        SDL_CreateThread(LoaderThread, "LoaderThread", (void *)&Inputs);
    }

    while (Running)
    {
        MouseDetection = WindowHeight - 320;
        const int frameDelay = 1000 / FPS;
        Uint32 frameStart = SDL_GetTicks();
        PollEvents();

        if (G->Droppedfile)
        {
            G->Loading_Droppedfile = true;
            ScanFolder(TempPath);
            G->loaded = false;
            loaderthreadinputs Inputs = {TempPath, G->CurrentFileIndex, G->files_TYPE[G->CurrentFileIndex], true};
            SDL_CreateThread(LoaderThread, "LoaderThread", (void *)&Inputs);
            G->Droppedfile = false;
        }

        ImGui_ImplSDL2_NewFrame(Window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        SDL_GetWindowSize(Window, &WindowWidth, &WindowHeight);

        if (G->signals.UpdatePass)
            G->signals.UpdatePass = false;
        UpdateLogic();
        UpdateGUI();
        Render();

        if (G->max_files > 0)
        {
            if (G->Keys.RightButtonUp || G->signals.nextimage)
            {
                G->Keys.RightButtonUp = false;
                G->signals.nextimage = false;

                if (G->CurrentFileIndex < G->max_files - 1)
                {
                    G->CurrentFileIndex++;
                    G->loaded = false;
                    loaderthreadinputs Inputs = {G->files[G->CurrentFileIndex].path, G->CurrentFileIndex, G->files_TYPE[G->CurrentFileIndex], false};
                    SDL_CreateThread(LoaderThread, "LoaderThread", (void *)&Inputs);
                }
            }
            if (G->Keys.LeftButtonUp || G->signals.previmage)
            {
                G->Keys.LeftButtonUp = false;
                G->signals.previmage = false;

                if (G->CurrentFileIndex > 0)
                {
                    G->CurrentFileIndex--;
                    G->loaded = false;
                    loaderthreadinputs Inputs = {G->files[G->CurrentFileIndex].path, G->CurrentFileIndex, G->files_TYPE[G->CurrentFileIndex], false};
                    SDL_CreateThread(LoaderThread, "LoaderThread", (void *)&Inputs);
                }
            }
        }

        {
            int frameEnd = SDL_GetTicks();
            int frameTime = frameEnd - frameStart;
            if (frameTime < frameDelay)
            {
                SDL_Delay(frameDelay - frameTime);
            }
        }
    }
    SaveSettings();
    return 0;
}