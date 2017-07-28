#include "stdafx.h"
#include "demo.h"
#include "floats.h"
#include "shader_code.h"

/*
// final color image
unsigned int finalImage[IMAGE_SIZE * IMAGE_SIZE] = { 0 };

//////////////////////////////////////////////////////////////////////////

static const BITMAPINFO bmi =
{
    { sizeof(BITMAPINFOHEADER), IMAGE_SIZE, IMAGE_SIZE, 1, 32, BI_RGB, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
};

static const char* windowClassName = "edit";

__declspec(naked)
void entrypoint(void)
{
    __asm
    {
        xor eax, eax;
        mov ecx, IMAGE_SIZE;

        // arguments for StretchDIBits
        push 0x00CC0020; //SRCCOPY
        push eax;        // 0 == DIB_RGB_COLORS
        push offset bmi;
        push offset finalImage;
        push ecx;        // IMAGE_SIZE
        push ecx;        // IMAGE_SIZE
        push eax;        // 0
        push eax;        // 0
        push ecx;        // IMAGE_SIZE
        push ecx;        // IMAGE_SIZE
        push eax;        // 0
        push eax;        // 0
        // ...

            // arguments for CreateWindowExA
            push eax;    // 0
            push eax;    // 0
            push eax;    // 0
            push eax;    // 0
            push ecx;    // IMAGE_SIZE
            push ecx;    // IMAGE_SIZE
            push eax;    // 0
            push eax;    // 0
            push WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
            push eax;    // 0
            push windowClassName;
            push eax;    // 0
            call CreateWindowExA;

            push eax;    // return of CreateWindowExA
            call GetDC;

        // ...
        push eax;        // return of GetDC (HDC)
        call StretchDIBits;

    label_loop:
        push 0x1B;      // VK_ESCAPE
        call GetAsyncKeyState;
        test ax, ax;
        jz label_loop;

        ret
    }
    //HDC hDC = GetDC(CreateWindowExA(0, windowClassName, 0, WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, IMAGE_SIZE, IMAGE_SIZE, 0, 0, 0, 0));
    //StretchDIBits(hDC, 0, 0, IMAGE_SIZE, IMAGE_SIZE, 0, 0, IMAGE_SIZE, IMAGE_SIZE, finalImage, &bmi, DIB_RGB_COLORS, SRCCOPY);
    //do { } while (!GetAsyncKeyState(VK_ESCAPE));
}

*/

static PIXELFORMATDESCRIPTOR pfd =
{
    sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_SUPPORT_OPENGL, PFD_TYPE_RGBA,
    32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
};


__declspec(align(16)) static __m128 gCoords = { -1.0f, -1.0f, -p0d99, 1.0f };
__declspec(align(16)) static const __m128 gCoordsOffset = { p0d01, 0.0f, p0d01, 0.0f };


// TODO rewrite in assembler
__declspec(naked) void entrypoint()
{
    {
        // create window and device context
        const HDC hDC = GetDC(CreateWindowA("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0));

        // initialize OpenGL
        SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
        wglMakeCurrent(hDC, wglCreateContext(hDC));

        // init shader (requires OpenGL 4.1)
        auto glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv");
        const GLuint prog = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, (const GLchar**)&shader_glsl);
        ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(prog);

        // rendering loop
        for (;;)
        {
            // render region
            glRectfv(gCoords.m128_f32, gCoords.m128_f32 + 2);

            // move rendering region
            gCoords = _mm_add_ps(gCoords, gCoordsOffset);

            // display
            glFlush();
            SwapBuffers(hDC);

            if (GetAsyncKeyState(VK_ESCAPE))
                ExitProcess(0);
        }
    }
}
