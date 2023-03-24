#pragma once
#include "main.h"
#include "structs.cpp"
#include "events.cpp"
#include "font.cpp"


// Creates a hover help marker to the ImGui item before it, set by a certain delay
#define HELP_MARKER_GUI(x)                 \
    {                                      \
        {                                  \
            static int timer = 0;          \
            HelpMarkerPrev(&timer, x, 40); \
        }                                  \
    }
#define  HELP_MARKER_GUI_SIGN(desc)                                 \
{                                                                   \
    ImGui::SameLine();                                              \
    ImGui::TextDisabled("(?)");                                     \
    if (ImGui::IsItemHovered())                                     \
    {                                                               \
        ImGui::BeginTooltip();                                      \
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);       \
        ImGui::TextUnformatted(desc);                               \
        ImGui::PopTextWrapPos();                                    \
        ImGui::EndTooltip();                                        \
    }                                                               \
}

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
                UV = vec2(x, 1.0 - float(y));
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

            void main()
            {   
                if (PixelGrid == 1)
                {
                    vec2 uvFraction = 1.0 / ImageDim;
                    if ( any( lessThan(mod(UV, uvFraction), uvFraction / truescale) ) )
                        color = vec4(1,1,1,1) - texture(TextureInput, UV);
                    else
				    {  
                        color = texture(TextureInput, UV) * vec4(RGBAflags.rgb, 1);
                        if (RGBAflags.a == 0.0)
                            color.a = 1.0;
                    }
                }
                else
				{
                    color = texture(TextureInput, UV) * vec4(RGBAflags.rgb, 1);
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
                UV = vec2(x, 1.0 - float(y));
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
static void HelpMarkerPrev(int *hovertime, const char *desc, int delay)
{
    if (ImGui::IsItemHovered())
    {
        *hovertime = *hovertime + 1;
        if (*hovertime > delay)
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    else
        *hovertime = 0;
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
    char buffer[0x400];
    snprintf(buffer, sizeof(buffer), "%s\\config", APPDATA_FOLDER);
    FILE *F = fopen(buffer, "wb");
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
        fwrite(&G->settings_Sort, sizeof(bool), 1, F);
        fclose(F);
    }
}
static void LoadSettings()
{
    char buffer[0x400];
    snprintf(buffer, sizeof(buffer), "%s\\config", APPDATA_FOLDER);
    FILE *F = fopen(buffer, "rb");
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
        fread(&G->settings_Sort, sizeof(bool), 1, F);
        fclose(F);
    }
}

static uint32_t GetTicks()
{
    static LARGE_INTEGER frequency;
    static bool n = QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER  result;
    QueryPerformanceCounter(&result);
    return 1000 * result.QuadPart / frequency.QuadPart;
}
const ImWchar glyphranges[] = { 0x20, 0xFFFF, 0 }; 
ImFont *FONT;
DWORD WINAPI FontLoadThread(LPVOID lpParam)
{
    G->unicode_font_loaded = false;
    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig config;
    config.GlyphRanges = glyphranges;
    FONT = io.Fonts->AddFontFromMemoryCompressedTTF(juliamono_compressed_data, juliamono_compressed_size, 13, &config, glyphranges);
    io.Fonts->Build();
    G->unicode_font_loaded = true;
    return 0;
}
static void CenterWindow()
{
    v2 DisplaySize = v2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    RECT rect;
    GetClientRect (hwnd, &rect);
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    int x = (DisplaySize.x - w) / 2;
    int y = (DisplaySize.y - h) / 2;
    SetWindowPos(hwnd, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}
static void Main_Init()
{
    WindowWidth  = 500;
    WindowHeight = 500;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    const char* szTitle = "CactusViewer";
   
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "CactusViewer" ;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassEx(&wcex);
    RECT rc = { 0, 0,  WindowWidth, WindowHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, 0);
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, "CactusViewer", "CactusViewer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
    hdc = GetDC(hwnd);
    CenterWindow();
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    int iPixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, iPixelFormat, &pfd);
    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, 
        0 
    };
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    HGLRC glContext = wglCreateContextAttribsARB(hdc, NULL, attribs);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    wglMakeCurrent(hdc, glContext);
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(1);
    DragAcceptFiles(hwnd, TRUE);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)wcex.hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)wcex.hIcon);

    gladLoadGL();

    G->Graphics.MainImage.TextureID = 0;

    GLint MAX_GPU = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MAX_GPU);
    G->Graphics.MAX_GPU = MAX_GPU;

    InitializeCriticalSection(&G->Mutex);
    InitializeCriticalSection(&G->SortMutex);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
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

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = style.GrabRounding = 5;
    style.FrameBorderSize = 1;
    style.WindowRounding = 10;

    CreateThread(NULL, 0, FontLoadThread, NULL, 0, NULL);

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
    wchar_t *File;
    uint ID;
    int Type;
    bool dropped;
};

static void resetToNoFolder()
{
    if (G->Loading_Droppedfile)
        G->Loading_Droppedfile = false;
    G->Files.reset_count();
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

static int Load_Image(wchar_t *File, uint ID, bool dropped)
{
    int w, h, n;
    int result = 0;
    G->Files[ID].loading = true;
    int size = stbi_convert_wchar_to_utf8(0, 0, File);
	char *filename_utf8 = (char *)malloc(size);
	stbi_convert_wchar_to_utf8(filename_utf8, size, File);
    unsigned char *data = stbi_load(filename_utf8, &w, &h, &n, 4);
    G->Files[ID].loading = false;
    if (data == nullptr)
    {
        PushError("Loading the file failed");
        G->Files[G->CurrentFileIndex].failed = true;
        G->loaded = true;
        G->signals.UpdatePass = true;
        if (dropped)
            resetToNoFolder();
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
            EnterCriticalSection(&G->Mutex);
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
            LeaveCriticalSection(&G->Mutex);
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

static void UnLoad_GIF()
{
    G->GIF_frames = 0;
    free(G->GIF_buffer);
    G->GIF_buffer = nullptr;
    free(G->GIF_FrameDelays);
    G->GIF_FrameDelays = nullptr;
}

static int Load_GIF(wchar_t *File, uint ID, bool dropped)
{
    int w, h;
    int frames;
    int *delays;
    int result = 0;

    UnLoad_GIF();

    G->Files[ID].loading = true;
    unsigned char *data = stbi_xload_file(File, &w, &h, &frames, &delays);
    G->Files[ID].loading = false;

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
            UnLoad_GIF();
        else
            G->signals.Initstep2 = true;
        result = 1;
    }
    else
    {
        PushError("Loading GIF file failed");
        G->Files[G->CurrentFileIndex].failed = true;
        G->loaded = true;
        if (dropped)
            resetToNoFolder();
        SetToNoFile();
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



static void SetWindowSize(v2 DisplaySize, int w, int h)
{
    RECT rc = { 0, 0,  w, h };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, 0);
    int x = (DisplaySize.x - w) / 2;
    int y = (DisplaySize.y - h) / 2;
    SetWindowPos(hwnd, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

static void GetWindowSize()
{
    RECT rect;
    GetClientRect (hwnd, &rect);
    WindowWidth = rect.right - rect.left;
    WindowHeight = rect.bottom - rect.top;
}

static void refreshdisplay()
{
    v2 DisplaySize = v2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    float aspect = (float)G->Graphics.MainImage.w / G->Graphics.MainImage.h;
    // printf("%i %i", Display.w, Display.h);
    if (G->Graphics.MainImage.w > DisplaySize.x * 0.85 || G->Graphics.MainImage.h > DisplaySize.y* 0.85)
    {
        if (aspect < 1)
            SetWindowSize(DisplaySize, DisplaySize.y * 0.85 * aspect, DisplaySize.y * 0.85);
        else
            SetWindowSize(DisplaySize, DisplaySize.x * 0.85, DisplaySize.y * 0.85);
    }
    else
        SetWindowSize(DisplaySize, max(G->Graphics.MainImage.w, 500), max(G->Graphics.MainImage.h, 500));

    GetWindowSize();
}
static void ApplySettings()
{
    switch (G->settings_resetzoom)
    {
    case 0: // Do not reset zoom
        break;
    case 1: // Save zoom for each file
        G->req_truescale = G->Files[G->CurrentFileIndex].scale;
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
        G->Position = G->Files[G->CurrentFileIndex].pos;
        break;
    case 2: // Reset to center
        G->Position = v2(0, 0);
        break;
    default:
        break;
    }
    G->signals.setting_applied = true;
}

static void Load_Image_post()
{
    if (G->Files[G->CurrentFileIndex].type == 1)
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

DWORD WINAPI LoaderThread(LPVOID lpParam)
{
    loaderthreadinputs *Inputs = (loaderthreadinputs *)lpParam;
    if (!G->Files[Inputs->ID].loading)
    {
        if (Inputs->Type == 1)
            Load_GIF(Inputs->File, Inputs->ID, Inputs->dropped);
        else
            Load_Image(Inputs->File, Inputs->ID, Inputs->dropped);
    }
    return 0;
}

static bool CheckValidExtention(wchar_t *EXT)
{
    const int length = wcslen(EXT);
    wchar_t *ext = (wchar_t *)malloc((length + 1) * sizeof(wchar_t));
    ext[0] = '.';
    for (int i = 1; i < length + 1; i++)
        ext[i] = tolower(EXT[i]);

    bool result = false;
    if      (wcscmp(ext, L".png") == 0)
        result = true;
    else if (wcscmp(ext, L".jpg") == 0)
        result = true;
    else if (wcscmp(ext, L".gif") == 0)
        result = true;
    else if (wcscmp(ext, L".jpeg") == 0)
        result = true;
    else if (wcscmp(ext, L".bmp") == 0)
        result = true;

    free(ext);
    return result;
}
#define TYPE_GIF 1
static int CheckOddExtention(wchar_t *EXT)
{
    const int length = wcslen(EXT);
    wchar_t *ext = (wchar_t *)malloc((length + 1) * sizeof(wchar_t));
    ext[0] = '.';
    for (int i = 1; i < length + 1; i++)
        ext[i] = tolower(EXT[i]);

    int result = 0;

    if (wcscmp(ext, L".gif") == 0)
        result = TYPE_GIF;

    free(ext);
    return result;
}
static void removechar(wchar_t *str, wchar_t ch)
{
    int len = wcslen(str);

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

bool isValidWindowsPath(wchar_t* path) 
{
    bool isValid = false;
    
    if (PathFileExistsW(path)) {
        if (PathIsRelativeW(path) == FALSE) {
            wchar_t drive[3];
            wcsncpy(drive, path, 2);
            drive[2] = '\0';
            UINT driveType = GetDriveTypeW(drive);
            if (driveType == DRIVE_FIXED || driveType == DRIVE_CDROM || driveType == DRIVE_RAMDISK ||
                driveType == DRIVE_REMOTE || driveType == DRIVE_REMOVABLE) {
                isValid = true;
            }
        }
    }
    
    return isValid;
}

struct FolderEntry 
{ 
    wchar_t wpath[MAX_PATH];
    char *path;
};
FolderEntry *filesInFolder;

bool StringEqual(const wchar_t *a, const wchar_t *b) {
	while (true) {
		if (*a != *b) return false;
		if (*a == 0) return true;
		a++, b++;
	}
}

void StringCopy(wchar_t *d, const wchar_t *s) {
	while (true) {
		wchar_t c = *s++;
		*d++ = c;
		if (!c) break;
	}
}

void StringAppend(wchar_t *d, const wchar_t *s) {
	while (*d) d++;
	StringCopy(d, s);
}
wchar_t *GetWC(char *c)
{
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);

    return wc;
}
char* GetC(wchar_t* wc)
{
    const size_t wcSize = wcslen(wc) + 1;
    char* c = new char[wcSize];
    wcstombs(c, wc, wcSize);

    return c;
}
int itemsInFolder;

int cmp(const void* a, const void* b) 
{
    file_data* A = (file_data*)a;
    file_data* B = (file_data*)b;
    if      (A->index > B->index)  return  1; 
    else if (A->index < B->index)  return -1; 
    else                           return  0; 
}

static void sortfolder()
{
    for (int i =0; i < G->Files.Count; i++)
        for (int j =0; j < itemsInFolder; j++)
            if (wcscmp(G->Files[i].file.path, filesInFolder[j].wpath) == 0)
            {
                G->Files[i].index = j;
                break;
            }
    qsort(G->Files.Data, G->Files.Count, sizeof(file_data), cmp);
}

struct FolderSortThread_data
{
    wchar_t *path;
    wchar_t *FileName;
};

DWORD WINAPI FolderSortThread(LPVOID lpParam)
{
    EnterCriticalSection(&G->SortMutex);
    G->sorting = true;

    FolderSortThread_data *Data = (FolderSortThread_data *)lpParam;

    wchar_t *filePath = Data->path;
    wchar_t *FileName = Data->FileName;
	wchar_t pathBuffer[MAX_PATH + 4];
    static int indexInFolder;

    
	IShellWindows *shellWindows = NULL;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (S_OK != CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_IShellWindows, (void **) &shellWindows)) return 0;
	
	IDispatch *dispatch = NULL;
	VARIANT v = {};
	V_VT(&v) = VT_I4;
	
	for (V_I4(&v) = 0; S_OK == shellWindows->Item(v, &dispatch); V_I4(&v)++) 
    {
		bool success = false;
		
		IFolderView *folderView = NULL;	
		IWebBrowserApp *webBrowserApp = NULL;
		IServiceProvider *serviceProvider = NULL;
		IShellBrowser *shellBrowser = NULL;
		IShellView *shellView = NULL;
		IPersistFolder2 *persistFolder = NULL;
		ITEMIDLIST *folderPIDL = NULL;
		ITEMIDLIST *itemPIDL = NULL;
		PIDLIST_ABSOLUTE fullPIDL = NULL;
		
		int itemCount = 0, focusedItem = 0;
		pathBuffer[0] = 0;
		
		if (S_OK != dispatch->QueryInterface(IID_IWebBrowserApp, (void **) &webBrowserApp)) goto error;
		if (S_OK != webBrowserApp->QueryInterface(IID_IServiceProvider, (void **) &serviceProvider)) goto error;
		if (S_OK != serviceProvider->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void **) &shellBrowser)) goto error;
		if (S_OK != shellBrowser->QueryActiveShellView(&shellView)) goto error;
		if (S_OK != shellView->QueryInterface(IID_IFolderView, (void **) &folderView)) goto error;
		if (S_OK != folderView->GetFolder(IID_IPersistFolder2, (void **) &persistFolder)) goto error;
		if (S_OK != persistFolder->GetCurFolder(&folderPIDL)) goto error;
		if (S_OK != folderView->GetFocusedItem(&focusedItem)) goto error;
		if (S_OK != folderView->Item(focusedItem, &itemPIDL)) goto error;
		fullPIDL = ILCombine(folderPIDL, itemPIDL);
		if (!SHGetPathFromIDListW(fullPIDL, pathBuffer)) goto error;
		if (!StringEqual(filePath, pathBuffer)) goto error;
		if (S_OK != folderView->ItemCount(SVGIO_ALLVIEW, &itemCount)) goto error;
        for(int i = 0; i < itemsInFolder; i++)
            free(filesInFolder[i].path);
        free(filesInFolder);
		if (!(filesInFolder = (FolderEntry *) malloc(itemCount * sizeof(FolderEntry)))) goto error;
		
		for (int i = 0; i < itemCount; i++) 
        {
			filesInFolder[i].wpath[0] = 0;
			ITEMIDLIST *itemPIDL = NULL;
			if (S_OK != folderView->Item(i, &itemPIDL)) continue;
			PIDLIST_ABSOLUTE fullPIDL = ILCombine(folderPIDL, itemPIDL);
			SHGetPathFromIDListW(fullPIDL, filesInFolder[i].wpath);
			CoTaskMemFree(fullPIDL);
			CoTaskMemFree(itemPIDL);
		}
		
		itemsInFolder = itemCount;
		indexInFolder = focusedItem;
	
		success = true;
		error:;
		
		if (fullPIDL) CoTaskMemFree(fullPIDL);
		if (folderPIDL) CoTaskMemFree(folderPIDL);
		if (itemPIDL) CoTaskMemFree(itemPIDL);
		if (persistFolder) persistFolder->Release();
		if (folderView) folderView->Release();
		if (shellView) shellView->Release();
		if (shellBrowser) shellBrowser->Release();
		if (serviceProvider) serviceProvider->Release();
		if (webBrowserApp) webBrowserApp->Release();
		if (dispatch) dispatch->Release();
		
		if (success) break;
	}
	
	shellWindows->Release();

	if (filesInFolder)
    {
    
        for(int i = 0; i < itemsInFolder; i++)
        {
            filesInFolder[i].path = GetC(filesInFolder[i].wpath);
        }

        sortfolder();

        for (int i = 0; i < G->Files.Count; i++)
        {
            if (wcscmp(FileName, G->Files[i].file.name) == 0)
            {
                G->CurrentFileIndex = i;
            }
            G->Files[i].loading = false;
            G->Files[i].failed = false;
        }
    } 

    free(Data->FileName);
    free(Data->path);
    G->sorting = false;
    LeaveCriticalSection(&G->SortMutex);
    return 0;
}

FolderSortThread_data SortData;

static void ScanFolder(wchar_t *Path)
{
    if (Path == nullptr || !isValidWindowsPath(Path))
    {
        G->Files.reset_count();
        return;
    }
    int len = wcslen(Path);
    wchar_t *BasePath = nullptr;
    wchar_t *FileName = nullptr;
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

    BasePath = (wchar_t *)malloc((newlen + 1      ) * sizeof(wchar_t));
    FileName = (wchar_t *)malloc((len - newlen + 1) * sizeof(wchar_t));
    memcpy(FileName, &Path[newlen], (len - newlen) * sizeof(wchar_t));
    memcpy(BasePath, Path, newlen * sizeof(wchar_t));
    BasePath[newlen] = '\0';
    FileName[len - newlen] = '\0';

    cf_dir_t dir;
    cf_dir_open(&dir, BasePath);

    G->Files.reset_count();

    while (dir.has_next)
    {
        cf_file_t file_0;
        cf_read_file(&dir, &file_0);

        if (file_0.is_dir)
        {
            cf_dir_next(&dir);
            continue;
        }
        removechar(file_0.path, '/');

        if (!CheckValidExtention(file_0.ext))
        {
            cf_dir_next(&dir);
            continue;
        }

        G->Files.push_back(*new file_data);

        G->Files.back().type = CheckOddExtention(file_0.ext);

        cf_file_t *file = &G->Files.back().file;
        *file = file_0;
	    stbi_convert_wchar_to_utf8(file->name_utf8, 1024, file->name);

        //printf("%ws\n", file->name);
        cf_dir_next(&dir);
    }
    cf_dir_close(&dir);

    if (!G->sorting && G->settings_Sort)
    {   
        SortData.FileName = (wchar_t*)malloc((wcslen(FileName) + 1) * sizeof(wchar_t));
        memcpy(SortData.FileName, FileName,  (wcslen(FileName) + 1) * sizeof(wchar_t));
        SortData.path =     (wchar_t*)malloc((wcslen(Path) + 1)     * sizeof(wchar_t));
        memcpy(SortData.path, Path,          (wcslen(Path) + 1)     * sizeof(wchar_t));
        CreateThread(NULL, 0, FolderSortThread, (LPVOID)&SortData, 0, NULL);
    }

    for (int i = 0; i < G->Files.Count; i++)
    {
        if (wcscmp(FileName, G->Files[i].file.name) == 0)
        {
            G->CurrentFileIndex = i;
        }
        G->Files[i].loading = false;
        G->Files[i].failed = false;
    }
    free(BasePath);    
    free(FileName);    
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

void BasicFileOpen()
{
    IFileOpenDialog *pFileOpen;

    // Create the FileOpenDialog object.
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
    COMDLG_FILTERSPEC rgSpec[] =
    { 
        { L"Images", L"*.jpg;*.jpeg;*.png;*.bmp;*.gif" },
    };
    pFileOpen->SetFileTypes(1, rgSpec);

    if (SUCCEEDED(hr))
    {
        // Show the Open dialog box.
        hr = pFileOpen->Show(NULL);

        // Get the file name from the dialog box.
        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                // Display the file name to the user.
                if (SUCCEEDED(hr))
                {
                    if (!G->Loading_Droppedfile)
                    {
                        int l = wcslen(pszFilePath);
                        TempPath = (wchar_t *)malloc((l + 1) * sizeof(wchar_t));
                        memcpy(TempPath, pszFilePath, (l + 1) * sizeof(wchar_t));
                        TempPath[l] = 0;
                    }
                    G->Droppedfile = true;
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }
    CoUninitialize();
}

static void UpdateGUI()
{
    using namespace ImGui;
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2, 0.2, 0.2, 1);
    style.Colors[ImGuiCol_Button] = ImVec4(0.2, 0.2, 0.2, 1);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3, 0.3, 0.3, 1);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.4, 0.4, 0.3, 1);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3, 0.3, 0.3, 1);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4, 0.4, 0.4, 1);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.3, 0.3, 0.3, 1);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4, 0.4, 0.4, 1);
    G->signals.update_filtering = true;
    ImGui::SetNextWindowPos(ImVec2(0, WindowHeight - 30));
    ImGui::SetNextWindowSize(ImVec2(WindowWidth, 30));
    style.FrameRounding = style.GrabRounding = 0;
    style.FrameBorderSize = 1;
    style.WindowRounding = 0;
    
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
        if (G->Files.Count > 0)
        {
            ImGui::Begin("Status", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
            {

                ImGui::Text("%i / %i | ", G->CurrentFileIndex + 1, G->Files.Count, G->Files[G->CurrentFileIndex].file.name_utf8);
                if (G->unicode_font_loaded)
                {
                    ImGui::PushFont(FONT);
                    ImGui::SameLine(); ImGui::Text(u8"%s -", G->Files[G->CurrentFileIndex].file.name_utf8);
                    ImGui::PopFont();
                }
                ImGui::SameLine();
                if (G->Files[G->CurrentFileIndex].type == TYPE_GIF)
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
    style.FrameRounding = style.GrabRounding = 5;
    style.FrameBorderSize = 1;
    style.WindowRounding = 10;

    if (!G->loaded && G->Files.Count > 0)
    {

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Loading", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        ImGui::Text("Loading image...");
        ImGui::End();
    }

    if (keypress(Key_Alt) || keypress(MouseM))
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

        if (keyup(MouseR))
        {
            char hexcolor[10];
            sprintf(hexcolor, "%02X%02X%02X", r, g, b);
            ImGui::SetClipboardText(hexcolor);
        }
    }

    if (G->settings_visible)
    {
        FPS = MAX_FPS;
        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight * 0.5f), 0, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 525));
        ImGui::Begin("settings", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Cactus Image Viewer ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "version %s", VERSION);
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 1.0f, 1.0f), "by Wassim Alhajomar @wassimulator");
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
        ImGui::Checkbox("Sort according to folder", &G->settings_Sort);
        HELP_MARKER_GUI_SIGN("Sorts images in the folder according to how the window has them sorted, " 
                                "this might be slow for large folders and it blocks the 'next' and " 
                                "'previous' controls until it sorts, so it is optional")

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
    if (G->Files.Count == 0)
        ImGui::BeginDisabled();
    if (G->keep_menu || (G->Keys.Mouse.y > MouseDetection && !G->settings_visible))
    {
        if (G->Files.Count && G->Files[G->CurrentFileIndex].failed)
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
        HELP_MARKER_GUI("Zoom image to fit its width to the window width")
        if (ImGui::Button("fit H", ImVec2(50, 23)))
        {
            G->Position = v2(0, 0);
            G->req_truescale = (float)WindowHeight / G->Graphics.MainImage.h;
            G->signals.UpdatePass = true;
            G->signals.update_truescale = true;
        }
        HELP_MARKER_GUI("Zoom image to fit its height to the window height")
        if (ImGui::Button("1:1", ImVec2(50, 23)))
        {
            G->Position = v2(0, 0);
            G->req_truescale = 1;
            G->signals.UpdatePass = true;
            G->signals.update_truescale = true;
        }
        HELP_MARKER_GUI("Zoom image to 100% scale, one image pixel matching one screen pixel")

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
        HELP_MARKER_GUI("Toggle between nearest-neighbor and linear filtering")

        if (!G->nearest_filtering)
            ImGui::BeginDisabled();
        ImGui::Text(" Grid");
        ImGui::SetCursorPos(ImVec2(22, 65));
        ImGui::Checkbox("###", &G->pixelgrid);
        HELP_MARKER_GUI("Show pixel grid")

        if (!G->nearest_filtering)
            ImGui::EndDisabled();
        ImGui::End();
        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f - 190, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(65, 92));
        ImGui::Begin("rgbasetting", NULL, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetCursorPosY(6);
        ImGui::Checkbox("  R", (bool *)&RGBAflags[0]);
        HELP_MARKER_GUI("Toggle red channel")
        ImGui::SetCursorPosY(26 + 20 * 0);
        ImGui::Checkbox("  G", (bool *)&RGBAflags[1]);
        HELP_MARKER_GUI("Toggle green channel")
        ImGui::SetCursorPosY(26 + 20 * 1);
        ImGui::Checkbox("  B", (bool *)&RGBAflags[2]);
        HELP_MARKER_GUI("Toggle blue channel")
        ImGui::SetCursorPosY(26 + 20 * 2);
        ImGui::Checkbox("  A", (bool *)&RGBAflags[3]);
        HELP_MARKER_GUI("Toggle between premultiplied alpha and straight RGB")
        ImGui::End();
        if (G->Files.Count && G->Files[G->CurrentFileIndex].failed)
            ImGui::EndDisabled();

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f + 190, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(65, 92));
        ImGui::Begin("settingsmall", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetNextItemWidth(100);
        bool open_popup = false;
        ImGui::SetItemAllowOverlap();
        ImGui::SetCursorPosY(7);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(BGcolor[0], BGcolor[1], BGcolor[2], 1));
        ImGui::SetItemAllowOverlap();
        open_popup |= ImGui::Button("bg", ImVec2(50, 25));
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
            G->keep_menu = true;
        }
        else
        {
            G->keep_menu = false;
        }
        ImGui::SetCursorPosY(35);
        if (G->Files.Count == 0) ImGui::EndDisabled();
        if (G->Files.Count == 0)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(5.0/225, 120.0/225, 5.0/255, 1));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(5.0/225, 70.0/225, 5.0/255, 1));
        if(ImGui::Button("open", ImVec2(50, 25))) BasicFileOpen();
        ImGui::PopStyleColor();
        if (G->Files.Count == 0) ImGui::BeginDisabled();
        // ImGui::SetCursorPos(ImVec2(25, 15));
        // ImGui::Text("bg");
        // ImGui::SetCursorPos(ImVec2(15, 33));
        // ImGui::Text("color");
        ImGui::SetCursorPosY(63);
        if (G->Files.Count == 0)
            ImGui::EndDisabled();
        if (ImGui::Button("config", ImVec2(50, 23)))
            G->settings_visible = true;
        HELP_MARKER_GUI("Open help and settings page");
        if (G->Files.Count == 0)
            ImGui::BeginDisabled();
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(173, 92));
        ImGui::Begin("NextPrev", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
        if (G->sorting || !(G->CurrentFileIndex > 0)) ImGui::BeginDisabled(true);
        if (ImGui::Button("<< Prev", ImVec2(75, 30))) G->signals.previmage = true;
        if (G->sorting || !(G->CurrentFileIndex > 0)) ImGui::EndDisabled();
        ImGui::SameLine();

        if (G->sorting || !(G->CurrentFileIndex < G->Files.Count - 1)) ImGui::BeginDisabled(true);
        if (ImGui::Button("Next >>", ImVec2(75, 30))) G->signals.nextimage = true;
        if (G->sorting || !(G->CurrentFileIndex < G->Files.Count - 1)) ImGui::EndDisabled();
        int file = G->CurrentFileIndex;
        if (G->sorting) ImGui::BeginDisabled(true);
        if (ImGui::Button("Reload folder", ImVec2(157, 20)))
        {
            ScanFolder(G->Files[G->CurrentFileIndex].file.path);
        }
        if (G->sorting) ImGui::EndDisabled();
        HELP_MARKER_GUI("Click to reload the folder and scan for any changes");
        ImGui::SetCursorPosY(65);
        ImGui::SetNextItemWidth(157);
        ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1, 1, 1, 0.2));
        ImGui::SliderInt(" ", &file, 0, G->Files.Count - 1, " ");
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::EndDisabled();
        ImGui::SetCursorPosY(65);

        ImGui::End();

        if (G->Files.Count && G->Files[G->CurrentFileIndex].type == TYPE_GIF)
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
            if (ImGui::Button(">>", ImVec2(33, 30)) || keypress(Key_Up))
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
    if (G->Files.Count == 0)
        ImGui::EndDisabled();
}

static void UpdateLogic()
{
    ImGuiIO &io = ImGui::GetIO();

    if (G->Error.timer > 0)
        G->Error.timer++;
    if (G->Error.timer == 50)
        G->Error.timer = 0;

    if (keydn(Key_F))
    {
        G->signals.update_filtering = true;
        G->nearest_filtering = !G->nearest_filtering;
    }
    if (keydn(Key_G))
    {
        G->pixelgrid = !G->pixelgrid;
    }

    if (keypress(MouseL) && !io.WantCaptureMouse)
    {
        G->Position += G->Keys.Mouse_rel;
    }

    {
        v2 diff = v2(keypress(Key_D) - keypress(Key_A), keypress(Key_S) - keypress(Key_W)) *
                  G->settings_movementmag / (1 + G->settings_shiftslowmag * keypress(Key_Shift));
        if (G->settings_movementinvert)
            diff = -diff;
        G->Position += diff;
    }
    if (G->Files.Count > 0 && G->Files[G->CurrentFileIndex].type == TYPE_GIF)
    {
        int reqindex = G->GIF_index;
        if (keypress(Key_Up))
        {
            reqindex--;
            keyrelease(Key_Up);
        }
        if (keypress(Key_Down))
        {
            reqindex++;
            keyrelease(Key_Down);
        }
        if (keypress(Key_Space))
        {
            keyrelease(Key_Space);
            G->GIF_Play = !G->GIF_Play;
        }
        G->GIF_index = clamp(reqindex, 0, G->GIF_frames - 1);
    }

    G->Graphics.aspect_wnd = (float)WindowWidth / WindowHeight;
    G->Graphics.aspect_img = (float)G->Graphics.MainImage.w / G->Graphics.MainImage.h;

    float prev_scale = G->scale;
    v2 prev_Position = G->Position;
    bool updatescalebar = false;

    if (G->loaded && G->Files.Count > 0)
    {
        if (!io.WantCaptureMouse)
            G->scale *= 1 + G->Keys.ScrollYdiff * 0.1 / (1 + G->settings_shiftslowmag * keypress(Key_Shift));
        if (G->Files[G->CurrentFileIndex].type != 1)
        {
            G->Files[G->CurrentFileIndex].pos = G->Position;
            G->Files[G->CurrentFileIndex].scale = G->truescale;
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
        v2 M = G->Keys.Mouse;
        G->PixelMouse = (M - (v2(WindowWidth, WindowHeight) - v2(G->Graphics.MainImage.w, G->Graphics.MainImage.h) * G->truescale) * 0.5 - G->Position) / G->truescale;
    }
    {
        v2 Mouse = G->Keys.Mouse - v2(WindowWidth, WindowHeight) / 2;

        v2 M = G->Keys.Mouse;

        float TS = G->truescale;
        if (!G->signals.setting_applied)
        {
            G->Position -= Mouse;
            G->Position *= G->scale / prev_scale;
            G->Position += Mouse;
        }
        if (G->keep_menu || (G->Keys.Mouse.y > MouseDetection && !G->settings_visible))
        {
            ImGui::SetNextWindowPos(ImVec2(WindowWidth * 0.5f, WindowHeight - 78), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(173, 92));
            ImGui::Begin("NextPrev", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
            if (G->Files.Count == 0 || G->Files[G->CurrentFileIndex].failed) ImGui::BeginDisabled();
            ImGui::SetNextItemWidth(158);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2, 0.2, 0.2, 1));
            if (ImGui::SliderFloat("  ", &TS, 0.1, 500, "zoom", ImGuiSliderFlags_Logarithmic) || G->signals.update_truescale) // the second and third check imposes the clamp on the scale when max or min zoom
            {
                updatescalebar = true;
            }
            ImGui::PopStyleColor();
            if (G->Files.Count == 0 || G->Files[G->CurrentFileIndex].failed) ImGui::EndDisabled();
            ImGui::End();
        }
        if (G->Files.Count > 0)
        {
            if (keypress(Key_Q))
            {
                TS *= 1 + 0.05 / (1 + G->settings_shiftslowmag * keypress(Key_Shift));
                updatescalebar = true;
            }
            if (keypress(Key_E))
            {
                TS *= 1 - 0.05 / (1 + G->settings_shiftslowmag * keypress(Key_Shift));
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

            if (!G->signals.setting_applied) G->Position *= G->scale / prev_scale;
        }
    }
    G->signals.setting_applied = false;
}

static void Render()
{

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (G->signals.Initstep2 || G->loaded || G->Files.Count == 0)
    {

        glViewport(0, 0, WindowWidth, WindowHeight);
        if (G->signals.Initstep2)
        {
            Load_Image_post();
            G->signals.Initstep2 = false;
            G->signals.UpdatePass = true;
            G->loaded = true;
            if (G->Loading_Droppedfile)
            {
                G->Loading_Droppedfile = false;
                free(TempPath);
            }
        }

        if (G->Files.Count > 0 && !G->Files[G->CurrentFileIndex].failed) // check if we have a folder open and no failed to load image
        {
            if (G->Files[G->CurrentFileIndex].failed)
            {
                glBindTexture(GL_TEXTURE_2D, G->GIFTextureID);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                static uint32_t time = 0;
                uint32_t delta =GetTicks() - time;
                if (delta >= G->GIF_FrameDelays[G->GIF_index] && G->GIF_Play)
                {
                    G->GIF_index++;
                    if (G->GIF_index == G->GIF_frames)
                    {
                        G->GIF_index = 0;
                    }
                    time = GetTicks();
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

    if (keypress(Key_Alt) || keypress(MouseM))
    {
        glReadPixels(G->Keys.Mouse.x, WindowHeight - G->Keys.Mouse.y, 1, 1, GL_RGBA, GL_FLOAT, &G->InspectColors);
    }

    ImGui::Render();
    G->imgui_in_frame = false;
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SwapBuffers(hdc);
}