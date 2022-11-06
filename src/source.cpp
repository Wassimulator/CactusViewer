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
            layout(location = 4 ) uniform int PixelGrid;
            layout(location = 5 ) uniform float truescale;
            layout(location = 6 ) uniform vec2 ImageDim;
            layout(location = 7 ) uniform vec2 Window;
            layout(location = 8 ) uniform vec4 RGBAflags;

			vec2 uv_aa_smoothstep( vec2 uv, vec2 res, float width ) 
			{
			    vec2 pixels = uv * res;

			    vec2 pixels_floor = floor(pixels + 0.5);
			    vec2 pixels_fract = fract(pixels + 0.5);
			    vec2 pixels_aa = fwidth(pixels) * width * 0.5;
			    pixels_fract = smoothstep( vec2(0.5) - pixels_aa, vec2(0.5) + pixels_aa, pixels_fract );

			    return (pixels_floor + pixels_fract - 0.5) / res;
			}

            void main()
            {   
				vec2 uvActual = uv_aa_smoothstep(UV, textureSize(TextureInput, 0),1.5);

                if (PixelGrid == 1)
                {
                    vec2 uvFraction = 1.0 / ImageDim;
                    if ( any( lessThan(mod(uvActual, uvFraction), uvFraction / truescale) ) )
                        color = vec4(1,1,1,1) - texture(TextureInput, uvActual);
                    else
				    {  
                        color = texture(TextureInput, uvActual) * vec4(RGBAflags.rgb, 1);
                        if (RGBAflags.a == 0.0)
                            color.a = 1.0;
                    }
                }
                else
				{
                    color = texture(TextureInput, uvActual) * vec4(RGBAflags.rgb, 1);
                    if (RGBAflags.a == 0.0)
                        color.a = 1.0;
                }
                    
            }
    )###";

char *VertexBG = R"###(
            #version 430 core
            smooth out vec2 UV;

            void main()
            {
                uint x = gl_VertexID % 2;
                uint y = gl_VertexID / 2;
                UV = vec2(x, 1.0 - y);
                vec2 P = vec2(x,y);
                P = P * 2.0 - 1.0;
            
                gl_Position = vec4(P, 0.0, 1.0);
            } 
    )###";

char *FragmentBG = R"###(
                #version 430 core
                smooth in vec2 UV;
                out vec4 color;
                layout(location = 0 ) uniform vec2 Window;
                layout(location = 1 ) uniform float size;
                layout(location = 2 ) uniform vec3 color1;
                layout(location = 3 ) uniform vec3 color2;
                layout(location = 4 ) uniform vec4 bg;


                 void main()
                {  
                        vec2 pixel = UV * Window;
                        if ((mod(int(pixel.x/ size), 2) == 0) == (mod(int(pixel.y/ size), 2)  == 0))
                           color = vec4(mix(color1, bg.rgb, bg.a),1);
                        else
                           color = vec4(mix(color2, bg.rgb, bg.a),1);

  
                }
  )###";

static GLuint CreateGLProgram(char *Vertex, char *Fragment)
{
    GLuint Pipeline = glCreateProgram();
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glAttachShader(Pipeline, VertexShader);
    glAttachShader(Pipeline, FragmentShader);

    const GLint VertexCodeLength = strlen(Vertex);
    glShaderSource(VertexShader, 1, &Vertex, &VertexCodeLength);
    glCompileShader(VertexShader);

    const GLint FragmentCodeLength = strlen(Fragment);
    glShaderSource(FragmentShader, 1, &Fragment, &FragmentCodeLength);
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
        return Pipeline;
    }
    else
        return 0;
}
static void HelpMarker(const char *desc)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
static void InitLogoImage()
{
    glGenTextures(1, &G->Graphics.LogoTextureID);
    glBindTexture(GL_TEXTURE_2D, G->Graphics.LogoTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 50, 53, 0, GL_RGBA, GL_UNSIGNED_BYTE, minilogo);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
static void SaveSettings()
{
    FILE *F = fopen("config", "wb");
    if (F)
    {
        fwrite(&G->settings_resetpos, sizeof(int32_t), 1, F);
        fwrite(&G->settings_resetzoom, sizeof(int32_t), 1, F);
        fwrite(&Checkerboard_color1, sizeof(float), 3, F);
        fwrite(&Checkerboard_color2, sizeof(float), 3, F);
        fwrite(&BGcolor, sizeof(float), 4, F);
        fwrite(&G->settings_autoplayGIFs, sizeof(bool), 1, F);
        fwrite(&G->settings_movementmag, sizeof(float), 1, F);
        fwrite(&G->settings_shiftslowmag, sizeof(float), 1, F);
        fwrite(&G->settings_movementinvert, sizeof(bool), 1, F);
        fwrite(&G->nearest_filtering, sizeof(bool), 1, F);
        fwrite(&G->pixelgrid, sizeof(bool), 1, F);
        fclose(F);
    }
}
static void LoadSettings()
{
    FILE *F = fopen("config", "rb");
    if (F)
    {
        fread(&G->settings_resetpos, sizeof(int32_t), 1, F);
        fread(&G->settings_resetzoom, sizeof(int32_t), 1, F);
        fread(&Checkerboard_color1, sizeof(float), 3, F);
        fread(&Checkerboard_color2, sizeof(float), 3, F);
        fread(&BGcolor, sizeof(float), 4, F);
        fread(&G->settings_autoplayGIFs, sizeof(bool), 1, F);
        fread(&G->settings_movementmag, sizeof(float), 1, F);
        fread(&G->settings_shiftslowmag, sizeof(float), 1, F);
        fread(&G->settings_movementinvert, sizeof(bool), 1, F);
        fread(&G->nearest_filtering, sizeof(bool), 1, F);
        fread(&G->pixelgrid, sizeof(bool), 1, F);
        fclose(F);
    }
}

static void Main_Init()
{
    SDL_Init(SDL_INIT_EVENTS);

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

    glGenVertexArrays(1, &G->Graphics.VAO);

    G->Graphics.MainProgram = CreateGLProgram(VertexCode, FragmentCode);
    G->Graphics.BGProgram = CreateGLProgram(VertexBG, FragmentBG);

    InitLogoImage();

    BGcolor[0] = 0.15;
    BGcolor[1] = 0.15;
    BGcolor[2] = 0.15;
    BGcolor[3] = 1;
    float c1 = 0.47;
    float c2 = 0.76;
    Checkerboard_color1[0] = c1;
    Checkerboard_color1[1] = c1;
    Checkerboard_color1[2] = c1;
    Checkerboard_color2[0] = c2;
    Checkerboard_color2[1] = c2;
    Checkerboard_color2[2] = c2;

    Checkerboard_size = 50;

    RGBAflags[0] = 1;
    RGBAflags[1] = 1;
    RGBAflags[2] = 1;
    RGBAflags[3] = 1;
    G->Error.timer = 0;
    G->settings_autoplayGIFs = true;
    G->settings_movementmag = 2;
    G->settings_shiftslowmag = 9;
    LoadSettings();
}

static void PushError(char *string)
{
    int l = strlen(string) + 1;
    memcpy(G->Error.string, string, min(l, 256));
    G->Error.timer = 1;
};
struct loaderthreadinputs
{
    char *File;
    uint ID;
    int Type;
};

static void resetToNoFolder()
{
    if (G->Loading_Droppedfile)
        G->Loading_Droppedfile = false;
    G->max_files = 0;
}

static void SetToNoFile()
{
    G->Graphics.MainImage.w = 50;
    G->Graphics.MainImage.h = 53;
    G->req_truescale = 4;
    G->Position.x = 0;
    G->Position.y = 0;
    G->signals.update_truescale = true;
}

static int LoadImage(char *File, uint ID)
{
    int w, h, n;
    int result = 0;
    G->files_loading[ID] = true;
    unsigned char *data = stbi_load(File, &w, &h, &n, 4);
    G->files_loading[ID] = false;
    if (data == nullptr)
    {
        PushError("Loading the file failed");
        G->files_Failed[G->CurrentFileIndex] = true;
        G->loaded = true;
        G->signals.UpdatePass = true;
        SetToNoFile();
    }
    else
    {
        if (G->Graphics.MAX_GPU < w || G->Graphics.MAX_GPU < h)
        {
            PushError("Image is too large.");
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

void SavePPM()
{
    const int dimx = 800, dimy = 800;
    int i, j;
    FILE *fp = fopen("save.ppm", "wb"); /* b - binary mode */
    (void)fprintf(fp, "P6\n%d %d\n255\n", G->Graphics.MainImage.w, G->Graphics.MainImage.h);
    for (i = 0; i < G->Graphics.MainImage.w * G->Graphics.MainImage.h * 4; i += 4)
    {
        static unsigned char color[3];
        color[0] = G->Graphics.MainImage.data[i + 0]; /* red */
        color[1] = G->Graphics.MainImage.data[i + 1]; /* green */
        color[2] = G->Graphics.MainImage.data[i + 2]; /* blue */
        (void)fwrite(color, 1, 3, fp);
    }
    (void)fclose(fp);
}

void UnLoadGIF()
{
    G->GIF_frames = 0;
    free(G->GIF_buffer);
    free(G->GIF_FrameDelays);
}

void LoadGIF(char *File, uint ID)
{
    UnLoadGIF();
    int w, h;
    int frames;
    int *delays;

    G->files_loading[ID] = true;
    unsigned char *data = stbi_xload_file(File, &w, &h, &frames, &delays);
    G->files_loading[ID] = false;

    if (data != nullptr)
    {
        G->Graphics.MainImage.w = w;
        G->Graphics.MainImage.h = h;
        G->GIF_frames = frames;
        G->GIF_FrameDelays = delays;
        G->GIF_buffer = data;

        G->GIF_index = 0;
        G->GIF_Play = G->settings_autoplayGIFs;

        if (ID != G->CurrentFileIndex)
            UnLoadGIF();
        else
            G->signals.Initstep2 = true;
    }
    else
    {
        PushError("Loading GIF file failed");
        G->files_Failed[G->CurrentFileIndex] = true;
        G->loaded = true;
        resetToNoFolder();
        SetToNoFile();
    }
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

static void refreshdisplay()
{
    SDL_DisplayMode Display;
    SDL_GetCurrentDisplayMode(0, &Display);
    float aspect = (float)G->Graphics.MainImage.w / G->Graphics.MainImage.h;
    printf("%i %i", Display.w, Display.h);
    if (aspect < 1)
        SDL_SetWindowSize(Window, Display.h * 0.85 * aspect, Display.h * 0.85);
    else
        SDL_SetWindowSize(Window, Display.w * 0.85, Display.h * 0.85);
    SDL_SetWindowPosition(Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_GetWindowSize(Window, &WindowWidth, &WindowHeight);
}
static void ApplySettings()
{
    switch (G->settings_resetzoom)
    {
    case 0: // Do not reset zoom
        break;
    case 1: // Save zoom for each file
        G->req_truescale = G->files_scale[G->CurrentFileIndex];
        G->signals.update_truescale = true;
        break;
    case 2: // Fit Width
        G->Position = v2(0, 0);
        G->req_truescale = (float)WindowWidth / G->Graphics.MainImage.w;
        G->signals.UpdatePass = true;
        G->signals.update_truescale = true;
        break;
    case 3: // Fit Height
        G->Position = v2(0, 0);
        G->req_truescale = (float)WindowHeight / G->Graphics.MainImage.h;
        G->signals.UpdatePass = true;
        G->signals.update_truescale = true;
        break;
    case 4: // Zoom to 1:1
        G->Position = v2(0, 0);
        G->req_truescale = 1;
        G->signals.UpdatePass = true;
        G->signals.update_truescale = true;
        break;
    default:
        break;
    }
    switch (G->settings_resetpos)
    {
    case 0: // Persistent position across all files

        break;
    case 1: // Save position for each file
        G->Position = G->files_pos[G->CurrentFileIndex];
        break;
    case 2: // Reset to center
        G->Position = v2(0, 0);
        break;
    default:
        break;
    }
}

static void LoadImage_post()
{
    if (G->files_TYPE[G->CurrentFileIndex])
    {
        if (G->GIFTextureID != 0)
            glDeleteTextures(1, &G->GIFTextureID);

        if (G->GIFTextureID == 0)
            refreshdisplay();

        glGenTextures(1, &G->GIFTextureID);
        glBindTexture(GL_TEXTURE_2D, G->GIFTextureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, G->Graphics.MainImage.w, G->Graphics.MainImage.h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     G->GIF_buffer + G->GIF_index * G->Graphics.MainImage.w * G->Graphics.MainImage.h * 4); // indexing into the relevant gif frame
    }
    else
    {
        if (G->Graphics.MainImage.TextureID != 0)
            glDeleteTextures(1, &G->Graphics.MainImage.TextureID);

        if (G->Graphics.MainImage.TextureID == 0)
            refreshdisplay();

        glGenTextures(1, &G->Graphics.MainImage.TextureID);
        glBindTexture(GL_TEXTURE_2D, G->Graphics.MainImage.TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, G->Graphics.MainImage.w, G->Graphics.MainImage.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, G->Graphics.MainImage.data);
        // SavePPM();
        free(G->Graphics.MainImage.data);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    reducedfrac frac = ReduceFraction(G->Graphics.MainImage.w, G->Graphics.MainImage.h);
    G->Graphics.MainImage.frac1 = frac.n1;
    G->Graphics.MainImage.frac2 = frac.n2;
    G->Graphics.MainImage.aspectratio = (float)frac.n1 / frac.n2;

    ApplySettings();
}

static int LoaderThread(void *ptr)
{
    loaderthreadinputs *Inputs = (loaderthreadinputs *)ptr;
    if (!G->files_loading[Inputs->ID])
    {
        if (Inputs->Type == 1)
            LoadGIF(Inputs->File, Inputs->ID);
        else
            LoadImage(Inputs->File, Inputs->ID);
    }
    return 0;
}
static void BeginLoad(char *File)
{
    G->loaded = false;
    loaderthreadinputs Inputs = {File, G->CurrentFileIndex, G->files_TYPE[G->CurrentFileIndex]};
    SDL_CreateThread(LoaderThread, "LoaderThread", (void *)&Inputs);
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
    else if (strcmp(ext, ".gif") == 0)
        result = true;
    else if (strcmp(ext, ".jpeg") == 0)
        result = true;
    else if (strcmp(ext, ".bmp") == 0)
        result = true;

    free(ext);
    return result;
}
static int CheckOddExtention(char *EXT)
{
    const int length = strlen(EXT);
    char *ext = (char *)malloc(length + 1);
    ext[0] = '.';
    for (int i = 1; i < length + 1; i++)
        ext[i] = tolower(EXT[i]);

    int result = 0;
    if (strcmp(ext, ".gif") == 0)
        result = 1;

    free(ext);
    return result;
}
static void removechar(char *str, char ch)
{
    int len = strlen(str);

    for (int i = 0; i < len; i++)
    {
        if (str[i] == ch)
        {
            for (int j = i; j < len; j++)
            {
                str[j] = str[j + 1];
            }
            len--;
            i--;
        }
    }
}
static void ScanFolder(char *Path)
{
    if (Path == nullptr)
    {
        G->files = (cf_file_t *)realloc(G->files, sizeof(cf_file_t) * (1));
        G->files_loading = (bool *)realloc(G->files_loading, sizeof(bool) * (1));
        G->files_TYPE = (int *)realloc(G->files_TYPE, sizeof(int) * (1));
        G->files_Failed = (bool *)realloc(G->files_Failed, sizeof(bool) * (1));
        G->files_pos = (v2 *)realloc(G->files_pos, sizeof(v2) * (1));
        G->files_scale = (float *)realloc(G->files_scale, sizeof(float) * (1));
        G->files_TYPE[0] = 0;
        G->files_loading[0] = 0;

        G->max_files = 0;
        return;
    }
    int len = strlen(Path);
    char *BasePath = nullptr;
    char *FileName = nullptr;
    int newlen = len;

    removechar(Path, '/"');

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
            G->files_TYPE = (int *)realloc(G->files_TYPE, sizeof(int) * (G->allocated_files + 1));
            G->files_Failed = (bool *)realloc(G->files_Failed, sizeof(bool) * (G->allocated_files + 1));
            G->files_pos = (v2 *)realloc(G->files_pos, sizeof(v2) * (G->allocated_files + 1));
            G->files_scale = (float *)realloc(G->files_scale, sizeof(float) * (G->allocated_files + 1));

            G->allocated_files++;
        }

        G->files_TYPE[G->max_files] = CheckOddExtention(file_0.ext);

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
        G->files_Failed[i] = false;
    }
}

unsigned long createRGB(int r, int g, int b)
{
    return (r << 16) | (g << 8) | b;
}

static void GUIHelpMarker(const char *desc)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
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
    if (G->Error.timer > 0)
    {
        ImGui::Begin("Status", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        {
            ImGui::TextColored(ImVec4(1, 0.3, 0, 1), G->Error.string);
        }
        ImGui::End();
    }
    else
    {
        if (G->max_files > 0)
        {
            ImGui::Begin("Status", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
            {
                ImGui::Text("%i / %i | %s -", G->CurrentFileIndex + 1, G->max_files, G->files[G->CurrentFileIndex]);
                ImGui::SameLine();
                if (G->files_TYPE[G->CurrentFileIndex])
                    ImGui::Text("%d x %d - frames: %i -", G->Graphics.MainImage.w, G->Graphics.MainImage.h, G->GIF_frames);
                else
                    ImGui::Text("%d x %d -", G->Graphics.MainImage.w, G->Graphics.MainImage.h);
                ImGui::SameLine();
                ImGui::Text("%i:%i = %.3f - zoom: %.0f%% - Mouse: %i , %i",
                            G->Graphics.MainImage.frac1, G->Graphics.MainImage.frac2, G->Graphics.MainImage.aspectratio, G->truescale * 100,
                            (int)G->PixelMouse.x, (int)G->PixelMouse.y);
            }
            ImGui::End();
        }
        else
        {
            ImGui::Begin("Status", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
            {
                ImGui::Text("No file open. Drag and drop an image file to view it.");
            }
            ImGui::End();
        }
    }

    if (!G->loaded && G->max_files > 0)
    {

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Loading", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        ImGui::Text("Loading image...");
        ImGui::End();
    }

    if (G->Keys.Alt_Key || G->Keys.MouseMiddle)
    {
        ImGui::SetNextWindowPos(ImVec2(G->Keys.Mouse.x, G->Keys.Mouse.y));
        float colors[4];
        ImGui::Begin("color inspector", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
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

        if (G->Keys.MouseRightOnce)
        {
            char hexcolor[10];
            sprintf(hexcolor, "%02X%02X%02X", r, g, b);
            SDL_SetClipboardText(hexcolor);
        }
    }

    if (G->settings_visible)
    {
        FPS = MAX_FPS;
        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight * 0.5f), 0, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 500));
        ImGui::Begin("settings", NULL,ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Cactus Image Viewer ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "version %s", VERSION);
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 1.0f, 1.0f), "by Wassimulator");
        ImGui::Separator();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Supported file types:");
        ImGui::SameLine();
        ImGui::TextWrapped("PNG, JPG, JPEG, BMP, GIF");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "CONTROLS:");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Left Mouse button / WASD:");
        ImGui::SameLine();
        ImGui::Text("Pan");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Mouse Scroll / Q/E::");
        ImGui::SameLine();
        ImGui::Text("Zoom");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Shift:");
        ImGui::SameLine();
        ImGui::Text("Fine movement/zoom mode");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Middle Mouse button/ Alt(hold):");
        ImGui::SameLine();
        ImGui::Text("Pixel inspector (right click to copy)");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Left/Right Arrow buttons:");
        ImGui::SameLine();
        ImGui::Text("Next/ previous image");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Up/Down Arrow buttons:");
        ImGui::SameLine();
        ImGui::Text("GIF control: next/previous frame");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Space:");
        ImGui::SameLine();
        ImGui::Text("Pause/Play GIF");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "F:");
        ImGui::SameLine();
        ImGui::Text("Toggle between linear and nearest neighbor filtering");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "G:");
        ImGui::SameLine();
        ImGui::Text("Toggle pixel grid (if in nearest neighbor filtering is on)");
        ImGui::Separator();
        ImGui::Separator();

        const char *resetzoom_options[] = {"Do not reset zoom", "Save zoom for each file", "Fit Width", "Fit Height", "Zoom to 1:1"};
        const char *resetpos_options[] = {"Do not reset position", "Save position for each file", "Reset to center"};
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "SETTINGS:");
        ImGui::Text("When going from file to file:");
        ImGui::SetNextItemWidth(300);
        ImGui::Combo("image zoom", &G->settings_resetzoom, resetzoom_options, IM_ARRAYSIZE(resetzoom_options));
        ImGui::SetNextItemWidth(300);
        ImGui::Combo("image position", &G->settings_resetpos, resetpos_options, IM_ARRAYSIZE(resetpos_options));
        ImGui::Checkbox("Autoplay GIFs", &G->settings_autoplayGIFs);
        ImGui::ColorEdit3("Checkerboard color 1", (float *)&Checkerboard_color1, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();
        if (ImGui::Button("reset##1"))
        {
            float c1 = 0.47;
            Checkerboard_color1[0] = c1;
            Checkerboard_color1[1] = c1;
            Checkerboard_color1[2] = c1;
        }
        ImGui::ColorEdit3("Checkerboard color 2", (float *)&Checkerboard_color2, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();
        if (ImGui::Button("reset##2"))
        {
            float c2 = 0.76;
            Checkerboard_color2[0] = c2;
            Checkerboard_color2[1] = c2;
            Checkerboard_color2[2] = c2;
        }
        ImGui::SliderFloat("Keyboard WASD pan speed", &G->settings_movementmag, 0.1, 20);
        GUIHelpMarker("Adjust the movement speed when panning with WASD.");
        ImGui::SliderFloat("Shift slow mode magnitude", &G->settings_shiftslowmag, 0.1, 20);
        GUIHelpMarker("Holding SHIFT down allows for slower navigation. This adjusts how much slower the movement and zoom speed is when holding down SHIFT.");
        G->settings_movementmag = clamp(G->settings_movementmag, 0.1, 20);
        G->settings_shiftslowmag = clamp(G->settings_shiftslowmag, 0.1, 20);
        ImGui::Checkbox("Inverted pan movement with WASD", &G->settings_movementinvert);
        ImGui::Separator();
        ImGui::SetCursorPosX(220);
        if (ImGui::Button("close", ImVec2(60, 25)))
            G->settings_visible = false;
        ImGui::End();
    }
    if (G->max_files == 0)
        ImGui::BeginDisabled();
    if (G->Keys.Mouse.y > MouseDetection && !G->settings_visible)
    {
        if (G->files_Failed[G->CurrentFileIndex])
            ImGui::BeginDisabled();

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f - 125, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(65, 92));
        ImGui::Begin("fit", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse);
        if (ImGui::Button("fit W", ImVec2(50, 23)))
        {
            G->Position = v2(0, 0);
            G->req_truescale = (float)WindowWidth / G->Graphics.MainImage.w;
            G->signals.UpdatePass = true;
            G->signals.update_truescale = true;
        }
        if (ImGui::Button("fit H", ImVec2(50, 23)))
        {
            G->Position = v2(0, 0);
            G->req_truescale = (float)WindowHeight / G->Graphics.MainImage.h;
            G->signals.UpdatePass = true;
            G->signals.update_truescale = true;
        }
        if (ImGui::Button("1:1", ImVec2(50, 23)))
        {
            G->Position = v2(0, 0);
            G->req_truescale = 1;
            G->signals.UpdatePass = true;
            G->signals.update_truescale = true;
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f + 125, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(65, 92));
        ImGui::Begin("filter", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::Text("Nearest");
        ImGui::SetCursorPos(ImVec2(22, 25));
        if (ImGui::Checkbox("##", &G->nearest_filtering))
        {
            G->signals.update_filtering = true;
        }
        if (!G->nearest_filtering)
            ImGui::BeginDisabled();
        ImGui::Text(" Grid");
        ImGui::SetCursorPos(ImVec2(22, 65));
        ImGui::Checkbox("###", &G->pixelgrid);
        if (!G->nearest_filtering)
            ImGui::EndDisabled();
        ImGui::End();
        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f - 190, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(65, 92));
        ImGui::Begin("rgbasetting", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetCursorPosY(6);
        ImGui::Checkbox(" R", (bool *)&RGBAflags[0]);
        ImGui::SetCursorPosY(26 + 20 * 0);
        ImGui::Checkbox(" G", (bool *)&RGBAflags[1]);
        ImGui::SetCursorPosY(26 + 20 * 1);
        ImGui::Checkbox(" B", (bool *)&RGBAflags[2]);
        ImGui::SetCursorPosY(26 + 20 * 2);
        ImGui::Checkbox(" A", (bool *)&RGBAflags[3]);
        ImGui::End();
        if (G->files_Failed[G->CurrentFileIndex])
            ImGui::EndDisabled();

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f + 190, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(65, 92));
        ImGui::Begin("settingsmall", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetNextItemWidth(100);
        bool open_popup = false;
        ImGui::SetItemAllowOverlap();
        ImGui::SetCursorPos(ImVec2(7, 7));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(BGcolor[0], BGcolor[1], BGcolor[2], 0));
        ImGui::SetItemAllowOverlap();
        open_popup |= ImGui::Button("##", ImVec2(50, 50));
        ImGui::PopStyleColor();
        if (open_popup)
        {
            ImGui::OpenPopup("mypicker");
        }
        if (ImGui::BeginPopup("mypicker"))
        {
            ImGui::SliderFloat("##", &Checkerboard_size, WindowWidth * 0.01, WindowWidth * 0.5, "Checkerboard size");
            ImGui::ColorPicker4("##picker", (float *)&BGcolor, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaBar);
            ImGui::EndPopup();
        }
        ImGui::SetCursorPos(ImVec2(25, 15));
        ImGui::Text("bg");
        ImGui::SetCursorPos(ImVec2(15, 33));
        ImGui::Text("color");
        ImGui::SetCursorPosY(63);
        if (G->max_files == 0)
            ImGui::EndDisabled();
        if (ImGui::Button("config", ImVec2(50, 23)))
            G->settings_visible = true;
        if (G->max_files == 0)
            ImGui::BeginDisabled();
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(173, 92));
        ImGui::Begin("NextPrev", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
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
        int file = G->CurrentFileIndex;
        ImGui::SetNextItemWidth(158);
        ImGui::BeginDisabled();
        ImGui::SliderInt(" ", &file, 0, G->max_files - 1, " ");
        ImGui::EndDisabled();
        ImGui::PopStyleColor();
        ImGui::End();

        if (G->files_TYPE[G->CurrentFileIndex] == 1)
        {
            ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight - 158), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(173, 68));
            ImGui::Begin("Gif", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.1, 0.1, 1));

            int reqindex = G->GIF_index;
            if (G->GIF_Play || G->GIF_index == 0)
                ImGui::BeginDisabled(true);
            if (ImGui::Button("<<", ImVec2(33, 30)))
                reqindex--;
            if (G->GIF_Play || G->GIF_index == 0)
                ImGui::EndDisabled();

            ImGui::SameLine();
            if (G->GIF_Play)
            {
                if (ImGui::Button("Pause", ImVec2(75, 30)))
                    G->GIF_Play = !G->GIF_Play;
            }
            else
            {
                if (ImGui::Button("Play", ImVec2(75, 30)))
                    G->GIF_Play = !G->GIF_Play;
            }
            ImGui::SameLine();

            if (G->GIF_Play || G->GIF_index == G->GIF_frames - 1)
                ImGui::BeginDisabled(true);
            if (ImGui::Button(">>", ImVec2(33, 30)) || G->Keys.UpButton)
                reqindex++;
            if (G->GIF_Play || G->GIF_index == G->GIF_frames - 1)
                ImGui::EndDisabled();

            G->GIF_index = clamp(reqindex, 0, G->GIF_frames - 1);

            ImGui::SetNextItemWidth(158);
            if (G->GIF_Play)
                ImGui::BeginDisabled(true);
            char buff[128];
            sprintf(buff, "%i / %i", G->GIF_index, G->GIF_frames);
            ImGui::SliderInt("##", &G->GIF_index, 0, G->GIF_frames - 1);
            if (G->GIF_Play)
                ImGui::EndDisabled();

            ImGui::PopStyleColor();
            ImGui::End();
        }
    }
    if (G->max_files == 0)
        ImGui::EndDisabled();
}

static void UpdateLogic()
{

    if (G->Error.timer > 0)
        G->Error.timer++;
    if (G->Error.timer == 50)
        G->Error.timer = 0;

    if (G->Keys.F_Key)
    {
        G->signals.update_filtering = true;
        G->nearest_filtering = !G->nearest_filtering;
    }
    if (G->Keys.G_Key)
    {
        G->pixelgrid = !G->pixelgrid;
    }
    ImGuiIO &io = ImGui::GetIO();

    if (G->Keys.MouseLeft && !io.WantCaptureMouse)
    {
        G->Position += v2(G->Keys.xrel, G->Keys.yrel);
    }

    {
        v2 diff = v2(G->Keys.D_Key - G->Keys.A_Key, G->Keys.S_Key - G->Keys.W_Key) *
                  G->settings_movementmag / (1 + G->settings_shiftslowmag * G->Keys.Shift);
        if (G->settings_movementinvert)
            diff = -diff;
        G->Position += diff;
    }
    if (G->files_TYPE[G->CurrentFileIndex] == 1)
    {
        int reqindex = G->GIF_index;
        if (G->Keys.UpButton)
        {
            reqindex--;
            G->Keys.UpButton = false;
        }
        if (G->Keys.DownButton)
        {
            reqindex++;
            G->Keys.DownButton = false;
        }
        if (G->Keys.Space_Key)
        {
            G->Keys.Space_Key = false;
            G->GIF_Play = !G->GIF_Play;
        }
        G->GIF_index = clamp(reqindex, 0, G->GIF_frames - 1);
    }

    G->Graphics.aspect_wnd = (float)WindowWidth / WindowHeight;
    G->Graphics.aspect_img = (float)G->Graphics.MainImage.w / G->Graphics.MainImage.h;

    float prev_scale = G->scale;
    v2 prev_Position = G->Position;
    bool updatescalebar = false;
    if (G->max_files > 0)
    {
        if (!io.WantCaptureMouse)
            G->scale *= 1 + G->Keys.ScrollYdiff * 0.1 / (1 + G->settings_shiftslowmag * G->Keys.Shift);
        if (!G->files_Failed[G->CurrentFileIndex])
        {
            G->files_pos[G->CurrentFileIndex] = G->Position;
            G->files_scale[G->CurrentFileIndex] = G->truescale;
        }
    }

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
        if (G->Keys.Mouse.y > MouseDetection && !G->settings_visible)
        {
            ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(173, 92));
            ImGui::Begin("NextPrev", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
            if (G->max_files == 0 || G->files_Failed[G->CurrentFileIndex])
                ImGui::BeginDisabled();
            ImGui::SetNextItemWidth(158);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2, 0.2, 0.2, 1));
            if (ImGui::SliderFloat("  ", &TS, 0.1, 500, "zoom", ImGuiSliderFlags_Logarithmic) || G->signals.update_truescale) // the second and third check imposes the clamp on the scale when max or min zoom
            {
                updatescalebar = true;
            }
            ImGui::PopStyleColor();
            if (G->max_files == 0 || G->files_Failed[G->CurrentFileIndex])
                ImGui::EndDisabled();
            ImGui::End();
        }
        if (G->max_files > 0)
        {
            if (G->Keys.Q_Key)
            {
                TS *= 1 + 0.05 / (1 + G->settings_shiftslowmag * G->Keys.Shift);
                updatescalebar = true;
            }
            if (G->Keys.E_Key)
            {
                TS *= 1 - 0.05 / (1 + G->settings_shiftslowmag * G->Keys.Shift);
                updatescalebar = true;
            }
        }
        if (G->signals.update_truescale || updatescalebar)
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
    }
}

static void Render()
{


    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (G->signals.Initstep2 || G->loaded || G->max_files == 0)
    {

        glViewport(0, 0, WindowWidth, WindowHeight);
        if (G->signals.Initstep2)
        {
            LoadImage_post();
            G->signals.Initstep2 = false;
            G->signals.UpdatePass = true;
            G->loaded = true;
            if (G->Loading_Droppedfile)
            {
                G->Loading_Droppedfile = false;
                free(TempPath);
            }
        }

        if (G->max_files > 0 && !G->files_Failed[G->CurrentFileIndex]) // check if we have a folder open and no failed to load image
        {
            if (G->files_TYPE[G->CurrentFileIndex])
            {
                glBindTexture(GL_TEXTURE_2D, G->GIFTextureID);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                static uint32_t time = 0;
                uint32_t delta = SDL_GetTicks() - time;
                if (delta >= G->GIF_FrameDelays[G->GIF_index] && G->GIF_Play)
                {
                    G->GIF_index++;
                    if (G->GIF_index == G->GIF_frames)
                    {
                        G->GIF_index = 0;
                    }
                    time = SDL_GetTicks();
                }
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, G->Graphics.MainImage.w, G->Graphics.MainImage.h, GL_RGBA, GL_UNSIGNED_BYTE,
                                G->GIF_buffer + G->GIF_index * G->Graphics.MainImage.w * G->Graphics.MainImage.h * 4); // indexing into the relevant gif frame
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, G->Graphics.MainImage.TextureID);
            }
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, G->Graphics.LogoTextureID);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            G->nearest_filtering = true;
            SetToNoFile();
        }

        if (G->signals.update_filtering)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, G->nearest_filtering ? GL_NEAREST : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, G->nearest_filtering ? GL_NEAREST : GL_LINEAR);
            G->signals.update_filtering = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(BGcolor[0], BGcolor[1], BGcolor[2], 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(G->Graphics.VAO);
        glUseProgram(G->Graphics.BGProgram);
        glUniform2f(0, WindowWidth, WindowHeight);
        glUniform1f(1, Checkerboard_size);
        glUniform3f(2, Checkerboard_color1[0], Checkerboard_color1[1], Checkerboard_color1[2]);
        glUniform3f(3, Checkerboard_color2[0], Checkerboard_color2[1], Checkerboard_color2[2]);
        glUniform4f(4, BGcolor[0], BGcolor[1], BGcolor[2], BGcolor[3]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        G->Graphics.aspect_wnd = (float)WindowWidth / WindowHeight;
        G->Graphics.aspect_img = (float)G->Graphics.MainImage.w / G->Graphics.MainImage.h;

        glUseProgram(G->Graphics.MainProgram);
        glUniform1f(0, G->Graphics.aspect_img);
        glUniform1f(1, G->Graphics.aspect_wnd);
        glUniform1f(2, G->scale);
        v2 PositionGL = G->Position / v2(WindowWidth, -WindowHeight) * 2;
        glUniform2f(3, PositionGL.x, PositionGL.y);
        glUniform1i(4, (G->nearest_filtering && (G->truescale > 5) && G->pixelgrid) ? 1 : 0);
        glUniform1f(5, G->truescale);
        glUniform2f(6, G->Graphics.MainImage.w, G->Graphics.MainImage.h);
        glUniform2f(7, WindowWidth, WindowHeight);
        glUniform4f(8, (float)RGBAflags[0], (float)RGBAflags[1], (float)RGBAflags[2], (float)RGBAflags[3]);
        glActiveTexture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    if (G->Keys.Alt_Key || G->Keys.MouseMiddle)
    {
        glReadPixels(G->Keys.Mouse.x, WindowHeight - G->Keys.Mouse.y, 1, 1, GL_RGBA, GL_FLOAT, &G->InspectColors);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(Window);
}