#include "stdafx.h"
#include "shader_code.h"

#include <stdio.h>

#include <string>
#include <fstream>
#include <streambuf>

//////////////////////////////////////////////////////////////////////////

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

const char* gWindowClassName = "myWindowClass";

typedef struct
{
    HINSTANCE   hInstance;
    HDC         hDC;
    HGLRC       hRC;
    HWND        hWnd;
    int         full;
} WININFO;

static WININFO wininfo = { 0, 0, 0, 0, 0 };

static PIXELFORMATDESCRIPTOR pfd =
{
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
    PFD_TYPE_RGBA,
    32,
    0, 0, 0, 0, 0, 0, 8, 0,
    32, 0, 0, 0, 0,
    0, // depth
    0,
    0,
    PFD_MAIN_PLANE,
    0, 0, 0, 0
};

static time_t shaderFileModificationDate = 0;
static GLuint currentShader = 0;

static const int blockSize = 128;
static int xOffset = 0;
static int yOffset = 0;

//////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_SYSCOMMAND && (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER))
        return 0;

    if (uMsg == WM_CLOSE || uMsg == WM_DESTROY || (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE))
    {
        PostQuitMessage(0);
        return 0;
    }

    if (uMsg == WM_SIZE)
    {
        xOffset = 0;
        yOffset = 0;
        return 0;
    }

    if (uMsg == WM_CHAR || uMsg == WM_KEYDOWN)
    {
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static void window_end()
{
    if (wininfo.hRC)
    {
        wglMakeCurrent(0, 0);
        wglDeleteContext(wininfo.hRC);
    }

    if (wininfo.hDC) ReleaseDC(wininfo.hWnd, wininfo.hDC);
    if (wininfo.hWnd) DestroyWindow(wininfo.hWnd);

    UnregisterClassA(gWindowClassName, wininfo.hInstance);

    if (wininfo.full)
    {
        ChangeDisplaySettings(0, 0);
        while (ShowCursor(1) < 0);
    }
}

static int window_init()
{
    unsigned int PixelFormat;
    DWORD dwExStyle, dwStyle;
    DEVMODE dmScreenSettings;
    RECT rec;
    WNDCLASSA wc;

    ZeroMemory(&wc, sizeof(WNDCLASSA));
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = wininfo.hInstance;
    wc.lpszClassName = gWindowClassName;
    wc.hbrBackground = 0;

    if (!RegisterClassA(&wc))
        return(0);

    if (wininfo.full)
    {
        dmScreenSettings.dmSize = sizeof(DEVMODE);
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmPelsWidth = WINDOW_WIDTH;
        dmScreenSettings.dmPelsHeight = WINDOW_HEIGHT;

        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            return(0);

        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_VISIBLE | WS_POPUP;

        while (ShowCursor(0) >= 0);    // hide cursor
    }
    else
    {
        dwExStyle = 0;
        dwStyle = WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_OVERLAPPED;
        dwStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_POPUP;

    }

    rec.left = 0;
    rec.top = 0;
    rec.right = WINDOW_WIDTH;
    rec.bottom = WINDOW_HEIGHT;

    AdjustWindowRect(&rec, dwStyle, 0);

    wininfo.hWnd = CreateWindowExA(dwExStyle, wc.lpszClassName, "Demo", dwStyle,
                                   (GetSystemMetrics(SM_CXSCREEN) - rec.right + rec.left) >> 1,
                                   (GetSystemMetrics(SM_CYSCREEN) - rec.bottom + rec.top) >> 1,
                                   rec.right - rec.left, rec.bottom - rec.top, 0, 0, wininfo.hInstance, 0);

    if (!wininfo.hWnd)
        return(0);

    if (!(wininfo.hDC = GetDC(wininfo.hWnd)))
        return(0);

    if (!(PixelFormat = ChoosePixelFormat(wininfo.hDC, &pfd)))
        return(0);

    if (!SetPixelFormat(wininfo.hDC, PixelFormat, &pfd))
        return(0);

    if (!(wininfo.hRC = wglCreateContext(wininfo.hDC)))
        return(0);

    if (!wglMakeCurrent(wininfo.hDC, wininfo.hRC))
        return(0);

    return 1;
}

PFNGLDELETESHADERPROC glDeleteShader;
PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameteriv;
PFNGLGETINFOLOGARBPROC glGetInfoLog;

static bool load_shader()
{
    const char* shaderFilePath = "shader.glsl";

    struct stat fileStat;
    if (stat(shaderFilePath, &fileStat) == 0)
    {
        if (shaderFileModificationDate != 0 && shaderFileModificationDate == fileStat.st_mtime)
        {
            // file not modified
            return false;
        }

        shaderFileModificationDate = fileStat.st_mtime;
    }
    else
    {
        MessageBoxA(0, "stat()", "error", 0);
        return false;
    }

    std::ifstream shaderFile(shaderFilePath);
    if (!shaderFile.good())
    {
        MessageBoxA(0, "Failed to read shader file", "error", 0);
        return false;
    }

    const std::string shaderString((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
    const char* shaders[] = { shaderString.c_str() };

    GLuint newShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, (const GLchar**)shaders);

    const int maxMessageLen = 4096;
    int result;
    char info[maxMessageLen];

    glGetObjectParameteriv(newShader, GL_OBJECT_COMPILE_STATUS_ARB, &result);
    glGetInfoLog(newShader, maxMessageLen, NULL, (char*)info);
    if (!result)
    {
        OutputDebugStringA(info);
        return false;
    }

    glGetObjectParameteriv(newShader, GL_OBJECT_LINK_STATUS_ARB, &result);
    glGetInfoLog(newShader, maxMessageLen, NULL, (char*)info);
    if (!result)
    {
        OutputDebugStringA(info);
        return false;
    }

    if (currentShader != 0)
    {
        glDeleteShader(currentShader);
    }

    OutputDebugStringA("NEW SHADER LOADED\n");
    currentShader = newShader;
    return true;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    bool done = false;

    wininfo.hInstance = GetModuleHandle(0);

    if (!window_init())
    {
        window_end();
        MessageBoxA(0, "window_init()!", "error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
    glGetObjectParameteriv = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
    glGetInfoLog = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");

    if (!load_shader())
    {
        return 1;
    }

    while (!done)
    {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                done = true;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (load_shader())
        {
            xOffset = 0;
            yOffset = 0;
        }

        if (xOffset < WINDOW_WIDTH && yOffset < WINDOW_HEIGHT)
        {
            glViewport(xOffset, yOffset, blockSize, blockSize);
            xOffset += blockSize;
            if (xOffset >= WINDOW_WIDTH)
            {
                xOffset = 0;
                yOffset += blockSize;
            }

            glUseProgram(currentShader);
            glRecti(-1, -1, 1, 1);
        }
        else
        {
            Sleep(20);
        }

        glFlush();
        SwapBuffers(wininfo.hDC);
    }

    window_end();
    return 0;
}
