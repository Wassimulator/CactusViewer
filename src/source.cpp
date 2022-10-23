#pragma once
#include "main.h"
#include "structs.cpp"
#include "events.cpp"

char *VertexCode = R"###(
            #version 430 core
            smooth out vec2 UV;
            layout(location = 0 ) uniform float aspect_img;
            layout(location = 1 ) uniform float aspect_wnd;
            layout(location = 2 ) uniform float scale;
            layout(location = 3 ) uniform vec2 Position;

            void main()
            {
                uint x = gl_VertexID % 2;
                uint y = gl_VertexID / 2;
                UV = vec2(x, 1.0 - y);
                vec2 P = vec2(x,y);
                P = P * 2.0 - 1.0;
            

                if (aspect_img < aspect_wnd)
                {
                    P.x *= aspect_img/aspect_wnd;
                }
                else
                {
                   P.y *= aspect_wnd/aspect_img;
                }
                gl_Position = vec4(P * scale + Position, 0.0, 1.0);
            } 
    )###";

char *FragmentCode = R"###(
            #version 430 core
            smooth in vec2 UV;
            out vec4 color;
            uniform sampler2D TextureInput;

            void main()
            {   
				color = texture(TextureInput, UV);
            }
    )###";

static void
Main_Init()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    Window = SDL_CreateWindow("CactusViewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              500, 500, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    GLContext = SDL_GL_CreateContext(Window);
    gladLoadGL();

    G->Graphics.MainImage.TextureID = 0;

    GLint MAX_GPU = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MAX_GPU);
    G->Graphics.MAX_GPU = MAX_GPU;

    G->Mutex = SDL_CreateMutex();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(Window, GLContext);
    ImGui_ImplOpenGL3_Init();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenVertexArrays(1, &G->Graphics.VAO);
    glBindVertexArray(G->Graphics.VAO);

    GLuint Pipeline = glCreateProgram();
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glAttachShader(Pipeline, VertexShader);
    glAttachShader(Pipeline, FragmentShader);

    const GLint VertexCodeLength = strlen(VertexCode);
    glShaderSource(VertexShader, 1, &VertexCode, &VertexCodeLength);
    glCompileShader(VertexShader);

    const GLint FragmentCodeLength = strlen(FragmentCode);
    glShaderSource(FragmentShader, 1, &FragmentCode, &FragmentCodeLength);
    glCompileShader(FragmentShader);

    glLinkProgram(Pipeline);

    int NoErrors = 0;
    glGetProgramiv(Pipeline, GL_LINK_STATUS, &NoErrors);

    bool error = false;
    if (NoErrors == 0)
    {
        GLsizei BufferLength;
        GLchar Buffer[4096];
        glGetShaderInfoLog(FragmentShader, 4096, &BufferLength, Buffer);
        if (BufferLength > 0)
        {
            std::cerr << "Error (Shader):" << Buffer << std::endl;
            error = true;
        }
        glGetShaderInfoLog(VertexShader, 4096, &BufferLength, Buffer);
        if (BufferLength > 0)
        {
            std::cerr << "Error (Shader):" << Buffer << std::endl;
            error = true;
        }
        glGetProgramInfoLog(Pipeline, 4096, &BufferLength, Buffer);
        if (BufferLength > 0)
        {
            std::cerr << "Error(Program):" << Buffer << std::endl;
            error = true;
        }
    }
    if (!error)
    {
        G->Graphics.MainProgram = Pipeline;
        G->Graphics.Vertex = VertexShader;
        G->Graphics.Fragment = FragmentShader;
    }
}
struct loaderthreadinputs
{
    char *File;
    uint ID;
};

static int Load_Image(char *File, uint ID)
{
    int w, h, n;
    int result = 0;
    G->files_loading[ID] = true;
    unsigned char *data = stbi_load(File, &w, &h, &n, 4);
    G->files_loading[ID] = false;
    if (data == nullptr)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Loading the file failed", 0);
    }
    else
    {
        if (G->Graphics.MAX_GPU < w || G->Graphics.MAX_GPU < h)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Image is too large for yo ass", 0);
        }
        else
        {
            SDL_LockMutex(G->Mutex);
            if (ID == G->CurrentFileIndex)
            {
                G->Graphics.MainImage.w = w;
                G->Graphics.MainImage.h = h;
                G->Graphics.MainImage.n = n;
                G->Graphics.MainImage.data = data;
                G->signals.Initstep2 = true;
            }
            else
            {
                free(data);
            }
            SDL_UnlockMutex(G->Mutex);
            result = 1;
        }
    }
    return result;
}
struct reducedfrac
{
    int n1, n2;
};

int gcd(int n, int m)
{
    int gcd, rem;

    while (n != 0)
    {
        rem = m % n;
        m = n;
        n = rem;
    }
    gcd = m;

    return gcd;
}

reducedfrac ReduceFraction(int n1, int n2)
{
    reducedfrac Result;

    Result.n1 = n1 / gcd(n1, n2);
    Result.n2 = n2 / gcd(n1, n2);
    return Result;
}

static void LoadImage_post()
{
    if (G->Graphics.MainImage.TextureID != 0)
    {
        glDeleteTextures(1, &G->Graphics.MainImage.TextureID);
    }

    if (G->Graphics.MainImage.TextureID == 0)
    {
        SDL_DisplayMode Display;
        SDL_GetCurrentDisplayMode(0, &Display);
        float aspect = (float)G->Graphics.MainImage.w / G->Graphics.MainImage.h;
        printf("%i %i",  Display.w,  Display.h);
        SDL_SetWindowSize(Window, Display.h * 0.85 * aspect, Display.h * 0.85);
        SDL_SetWindowPosition(Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    glGenTextures(1, &G->Graphics.MainImage.TextureID);
    glBindTexture(GL_TEXTURE_2D, G->Graphics.MainImage.TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, G->Graphics.MainImage.w, G->Graphics.MainImage.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, G->Graphics.MainImage.data);
    free(G->Graphics.MainImage.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    reducedfrac frac = ReduceFraction(G->Graphics.MainImage.w, G->Graphics.MainImage.h);
    G->Graphics.MainImage.frac1 = frac.n1;
    G->Graphics.MainImage.frac2 = frac.n2;
    G->Graphics.MainImage.aspectratio = (float)frac.n1 / frac.n2;
}

static int LoaderThread(void *ptr)
{
    loaderthreadinputs *Inputs = (loaderthreadinputs *)ptr;
    if (!G->files_loading[Inputs->ID])
        Load_Image(Inputs->File, Inputs->ID);
    G->loaded = true;
    return 0;
}
static bool CheckValidExtention(char *EXT)
{
    const int length = strlen(EXT);
    char *ext = (char *)malloc(length + 1);
    ext[0] = '.';
    for (int i = 1; i < length + 1; i++)
        ext[i] = tolower(EXT[i]);

    bool result = false;
    if (strcmp(ext, ".png") == 0)
        result = true;
    else if (strcmp(ext, ".jpg") == 0)
        result = true;
    else if (strcmp(ext, ".jpeg") == 0)
        result = true;
    else if (strcmp(ext, ".bmp") == 0)
        result = true;
    else if (strcmp(ext, ".psd") == 0)
        result = true;
    else if (strcmp(ext, ".tga") == 0)
        result = true;
    else if (strcmp(ext, ".hdr") == 0)
        result = true;
    else if (strcmp(ext, ".pic") == 0)
        result = true;
    else if (strcmp(ext, ".pnm") == 0)
        result = true;

    free(ext);
    return result;
}

static void ScanFolder(char *Path)
{
    int len = strlen(Path);
    char *BasePath = nullptr;
    char *FileName = nullptr;
    int newlen = len;
    for (int i = len - 1; i > 0; i--)
    {
        if (Path[i] == '/' || Path[i] == '\\')
        {
            newlen = i + 1;
            break;
        }
    }
    BasePath = (char *)malloc(newlen + 1);
    FileName = (char *)malloc(len - newlen + 1);
    memcpy(FileName, &Path[newlen], len - newlen);
    memcpy(BasePath, Path, newlen);
    BasePath[newlen] = '\0';
    FileName[len - newlen] = '\0';

    cf_dir_t dir;
    cf_dir_open(&dir, BasePath);

    G->max_files = 0;
    while (dir.has_next)
    {
        cf_file_t file_0;
        cf_read_file(&dir, &file_0);

        if (!CheckValidExtention(file_0.ext))
        {
            cf_dir_next(&dir);
            continue;
        }

        if (G->max_files <= G->allocated_files)
        {
            G->files = (cf_file_t *)realloc(G->files, sizeof(cf_file_t) * (G->allocated_files + 1));
            G->files_loading = (bool *)realloc(G->files_loading, sizeof(bool) * (G->allocated_files + 1));
            G->allocated_files++;
        }
        cf_file_t *file = &G->files[G->max_files];
        *file = file_0;
        G->max_files++;

        printf("%s\n", file->name);
        cf_dir_next(&dir);
    }
    cf_dir_close(&dir);

    for (int i = 0; i < G->max_files; i++)
    {
        if (strcmp(FileName, G->files[i].name) == 0)
        {
            G->CurrentFileIndex = i;
        }
        G->files_loading[i] = false;
    }
}
unsigned long createRGB(int r, int g, int b)
{
    return (r << 16) | (g << 8) | b;
}

static void UpdateGUI()
{
    using namespace ImGui;
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2, 0.2, 0.2, 1);
    style.Colors[ImGuiCol_Button] = ImVec4(0.2, 0.2, 0.2, 1);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3, 0.3, 0.3, 1);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.4, 0.4, 0.4, 1);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3, 0.3, 0.3, 1);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4, 0.4, 0.4, 1);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.3, 0.3, 0.3, 1);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4, 0.4, 0.4, 1);
    G->signals.update_filtering = true;
    ImGui::SetNextWindowPos(ImVec2(0, WindowHeight - 30));
    ImGui::SetNextWindowSize(ImVec2(WindowWidth, 30));
    ImGui::Begin("Status", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    {
        ImGui::Text("%i / %i | %s - ", G->CurrentFileIndex + 1, G->max_files, G->files[G->CurrentFileIndex]);
        ImGui::SameLine();
        ImGui::Text("%d x %d - %i:%i = %.3f - zoom: %.0f%% - %i , %i", G->Graphics.MainImage.w, G->Graphics.MainImage.h,
                    G->Graphics.MainImage.frac1, G->Graphics.MainImage.frac2, G->Graphics.MainImage.aspectratio, G->truescale * 100,
                    (int)G->PixelMouse.x, (int)G->PixelMouse.y);
        ImGui::SameLine();

        // if (G->status.saved_count >= 0)
        // {
        //     ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        //     ImGui::SameLine();
        //     ImGui::Text("    Saved file at: %s", G->Track.Filepath);
        //     G->status.saved_count++;
        //     if (G->status.saved_count > 100)
        //         G->status.saved_count = -1;
        //     ImGui::PopStyleColor();
        // }
        ImGui::SameLine(ImGui::GetWindowWidth() - 250);
        // ImGui::Text("%i / %i", G->CurrentFileIndex + 1, G->max_files);
    }
    ImGui::End();

    if (G->files_loading[G->CurrentFileIndex])
    {

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Loading", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        ImGui::Text("Loading image...");
        ImGui::End();
    }

    if (G->Keys.Alt_Key)
    {
        ImGui::SetNextWindowPos(ImVec2(G->Keys.Mouse.x, G->Keys.Mouse.y));
        float colors[4];
        ImGui::Begin("color inspector", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(G->InspectColors[0], G->InspectColors[1], G->InspectColors[2], G->InspectColors[3]));
        ImGui::Button(" ", ImVec2(75, 50));
        ImGui::PopStyleColor();
        unsigned char r = G->InspectColors[0] * 255;
        unsigned char g = G->InspectColors[1] * 255;
        unsigned char b = G->InspectColors[2] * 255;
        ImGui::Text("%i , %i", (int)G->PixelMouse.x, (int)G->PixelMouse.y);
        ImGui::Text("0x%02X%02X%02X", r, g, b);
        ImGui::Text("R:%i", r);
        ImGui::Text("G:%i", g);
        ImGui::Text("B:%i", b);
        ImGui::End();

        if (G->Keys.MouseLeft_Click)
        {
            char hexcolor[10];
            sprintf(hexcolor, "0x%02X%02X%02X", r, g, b);
            SDL_SetClipboardText(hexcolor);
        }
    }

    // ImGui::BeginMainMenuBar();
    // if (ImGui::BeginMenu("View"))
    // {
    //     if (ImGui::MenuItem("reset view"))
    //     {
    //         G->Position = v2(0, 0);
    //         G->scale = 1;
    //     }
    //     ImGui::Separator();
    //     if (ImGui::MenuItem("Fit to width"))
    //     {
    //         G->Position = v2(0, 0);
    //         G->req_truescale = (float)WindowWidth / G->Graphics.MainImage.w;
    //         G->signals.update_truescale = true;
    //     }
    //     if (ImGui::MenuItem("Fit to height"))
    //     {
    //         G->Position = v2(0, 0);
    //         G->req_truescale = (float)WindowHeight / G->Graphics.MainImage.h;
    //         G->signals.update_truescale = true;
    //     }
    //     ImGui::EndMenu();
    // }
    // ImGui::EndMainMenuBar();

    if (G->Keys.Mouse.y > WindowHeight - 120)
    {
        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f - 125, WindowHeight - 65), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(65, 60));
        ImGui::Begin("fit", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize| ImGuiWindowFlags_NoScrollWithMouse);
        if (ImGui::Button("fit W", ImVec2(50, 20)))
        {
            G->Position = v2(0, 0);
            G->req_truescale = (float)WindowWidth / G->Graphics.MainImage.w;
            G->signals.update_truescale = true;
        }
        if (ImGui::Button("fit H", ImVec2(50, 20)))
        {
            G->Position = v2(0, 0);
            G->req_truescale = (float)WindowHeight / G->Graphics.MainImage.h;
            G->signals.update_truescale = true;
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f + 125, WindowHeight - 65), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(65, 60));
        ImGui::Begin("filter", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize| ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::Text("nearest");
        ImGui::SetCursorPos(ImVec2(22, 30));
        if (ImGui::Checkbox("##", &G->nearest_filtering))
        {
            G->signals.update_filtering = true;
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight - 65), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(173,68));
        ImGui::Begin("NextPrev", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.1, 0.1, 1));
        if (!(G->CurrentFileIndex > 0))
            ImGui::BeginDisabled(true);
        if (ImGui::Button("<< Prev", ImVec2(75, 30)))
            G->signals.previmage = true;
        if (!(G->CurrentFileIndex > 0))
            ImGui::EndDisabled();

        ImGui::SameLine();

        if (!(G->CurrentFileIndex < G->max_files - 1))
            ImGui::BeginDisabled(true);
        if (ImGui::Button("Next >>", ImVec2(75, 30)))
            G->signals.nextimage = true;
        if (!(G->CurrentFileIndex < G->max_files - 1))
            ImGui::EndDisabled();

        ImGui::PopStyleColor();
        ImGui::End();
    }
}

static void UpdateLogic()
{

    if (G->Keys.F_Key)
    {
        G->signals.update_filtering = true;
        G->nearest_filtering = !G->nearest_filtering;
    }
    ImGuiIO &io = ImGui::GetIO();

    if (G->Keys.MouseLeft && !io.WantCaptureMouse)
    {
        G->Position += v2(G->Keys.xrel, G->Keys.yrel);
    }

    G->Graphics.aspect_wnd = (float)WindowWidth / WindowHeight;
    G->Graphics.aspect_img = (float)G->Graphics.MainImage.w / G->Graphics.MainImage.h;

    float prev_scale = G->scale;
    v2 prev_Position = G->Position;
    if (!io.WantCaptureMouse)
        G->scale *= 1 + G->Keys.ScrollYdiff * 0.1;

    if (G->Graphics.MainImage.w > 0)
    {
        if (G->Graphics.aspect_img < G->Graphics.aspect_wnd)
        {
            G->truescale = (float)WindowHeight / G->Graphics.MainImage.h * G->scale;
        }
        else
        {
            G->truescale = (float)WindowWidth / G->Graphics.MainImage.w * G->scale;
        }
        v2 M = v2(G->Keys.Mouse.x, G->Keys.Mouse.y);
        G->PixelMouse = (M - (v2(WindowWidth, WindowHeight) - v2(G->Graphics.MainImage.w, G->Graphics.MainImage.h) * G->truescale) * 0.5 - G->Position) / G->truescale;
    }
    {
        v2 Mouse = v2(G->Keys.Mouse.x, G->Keys.Mouse.y) - v2(WindowWidth, WindowHeight) / 2;

        v2 M = v2(G->Keys.Mouse.x, G->Keys.Mouse.y);

        float TS = G->truescale;

        {
            G->Position -= Mouse;
            G->Position *= G->scale / prev_scale;
            G->Position += Mouse;
        }
        if (G->Keys.Mouse.y > WindowHeight - 120 || G->signals.update_truescale)
        {
            ImGui::Begin("NextPrev");
            ImGui::SetNextItemWidth(158);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2, 0.2, 0.2, 1));
            if (ImGui::SliderFloat(" ", &TS, 0.1, 500, "zoom", ImGuiSliderFlags_Logarithmic)||  G->signals.update_truescale) // the second and third check imposes the clamp on the scale when max or min zoom
            {
                if (G->signals.update_truescale)
                {
                    G->signals.update_truescale = false;
                    TS = G->req_truescale;
                }
                TS = clamp(TS, 0.1, 500);

                if (G->Graphics.aspect_img < G->Graphics.aspect_wnd)
                    G->scale = TS * (float)G->Graphics.MainImage.h / WindowHeight;
                else
                    G->scale = TS * (float)G->Graphics.MainImage.w / WindowWidth;

                G->Position *= G->scale / prev_scale;
            }
            ImGui::PopStyleColor();
            ImGui::End();
        }
    }
}

static void Render()
{
    glViewport(0, 0, WindowWidth, WindowHeight);

    // glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glrect) * 1, &G->Graphics.MainImage.Rect);
    if (G->loaded)
    {
        if (G->signals.Initstep2)
        {
            LoadImage_post();
            G->signals.Initstep2 = false;
        }

        glBindTexture(GL_TEXTURE_2D, G->Graphics.MainImage.TextureID);
        if (G->signals.update_filtering)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, G->nearest_filtering ? GL_NEAREST : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, G->nearest_filtering ? GL_NEAREST : GL_LINEAR);
            G->signals.update_filtering = false;
        }
        glBindVertexArray(G->Graphics.VAO);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(G->Graphics.MainProgram);
        glUniform1f(0, G->Graphics.aspect_img);
        glUniform1f(1, G->Graphics.aspect_wnd);
        glUniform1f(2, G->scale);
        v2 PositionGL = G->Position / v2(WindowWidth, -WindowHeight) * 2;
        glUniform2f(3, PositionGL.x, PositionGL.y);
        glActiveTexture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    if (G->Keys.Alt_Key)
    {
        glReadPixels(G->Keys.Mouse.x, WindowHeight - G->Keys.Mouse.y, 1, 1, GL_RGBA, GL_FLOAT, &G->InspectColors);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(Window);
}