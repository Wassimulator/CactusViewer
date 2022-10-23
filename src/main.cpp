#include "main.h"
#include "source.cpp"

int main(int argc, char **argv)
{
    Main_Init();
    ScanFolder(argv[1]);
    if (argc > 1)
    {
        loaderthreadinputs Inputs = {argv[1], G->CurrentFileIndex};
        SDL_CreateThread(LoaderThread, "LoaderThread", (void *)&Inputs);
    }


    while (Running)
    {
        if (!G->loaded)
            glClearColor(0.15, 0.15, 0.15, 1);
        else
            glClearColor(0.3, 0.3, 0.3, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        PollEvents();
        ImGui_ImplSDL2_NewFrame(Window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        SDL_GetWindowSize(Window, &WindowWidth, &WindowHeight);

        if (G->Keys.RightButton || G->signals.nextimage)
        {
            G->Keys.RightButton = false;
            G->signals.nextimage = false;

            if (G->CurrentFileIndex < G->max_files - 1)
            {
                G->CurrentFileIndex++;
                G->loaded = false;
                loaderthreadinputs Inputs = {G->files[G->CurrentFileIndex].path, G->CurrentFileIndex};
                SDL_CreateThread(LoaderThread, "LoaderThread", (void *)&Inputs);
            }
        }
        if (G->Keys.LeftButton || G->signals.previmage)
        {
            G->Keys.LeftButton = false;
            G->signals.previmage = false;

            if (G->CurrentFileIndex > 0)
            {
                G->CurrentFileIndex--;
                G->loaded = false;
                loaderthreadinputs Inputs = {G->files[G->CurrentFileIndex].path, G->CurrentFileIndex};
                SDL_CreateThread(LoaderThread, "LoaderThread", (void *)&Inputs);
            }
        }

        UpdateGUI();
        UpdateLogic();
        Render();
    }
    return 0;
}