#include "stdafx.h"
#include "demo.h"
#include "floats.h"
#include "shader_code.h"

#define WINDOW_WIDTH 1366
#define WINDOW_HEIGHT 768


static PIXELFORMATDESCRIPTOR pfd =
{
    0, // HACK, should be sizeof(PIXELFORMATDESCRIPTOR),
    0,
    0, // will be PFD_SUPPORT_OPENGL
    0, // PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
};

static const char* glCreateShaderProgramvName = "glCreateShaderProgramv";
static const char* glUseProgramName = "glUseProgram";

__declspec(naked)
void entrypoint(void)
{
    __asm
    {
        // EPIC HACK - exploit EAX==0 from crinkler-generated DLL import code
        // (restore if crashes)
        // xor eax, eax   

        push eax    // for "pop esi"

    create_window:
        push eax    // 0
        push eax    // 0
        push eax    // 0
        push eax    // 0
        push eax    // 0
        push eax    // 0
        push eax    // 0
        push eax    // 0
        push WS_POPUP | WS_VISIBLE | WS_MAXIMIZE;
        push eax    // 0
        push 0xC019 // atom for "static", http://www.pouet.net/topic.php?which=9894
        push eax    // 0
        call CreateWindowExA

    init_ogl:
        push eax    // window handle (result of CreateWindowExA)
        call GetDC

        mov edi, eax            // store device context in EDI        
        mov eax, offset pfd
        mov[eax + 0x4], 0x20    // pdf.dwFlags = PFD_SUPPORT_OPENGL

        push eax    // 'pfd' for SetPixelFormat
            push eax
            push edi    // device context
            call ChoosePixelFormat
        push eax    // pixel format
        push edi    // device context
        call SetPixelFormat

        push eax
        push edi    // device context
        call wglCreateContext

        push eax    // NOP - makes the code structure regular (push eax; push edi; call)
        push edi    // device context
        call wglMakeCurrent

        // initialize offset of rendering region
        pop esi     // from "push eax", equal to "mov esi, 0"

    load_shader:
        push offset shader_glsl
        push 1      // num of shaders
        push GL_FRAGMENT_SHADER
            push glCreateShaderProgramvName
            call wglGetProcAddress
        call eax    // call glCreateShaderProgramv

        push eax    // shader program
            push glUseProgramName
            call wglGetProcAddress
        call eax    // call glUseProgram

    render_loop:
        push 0x1B       // VK_ESCAPE
            push PM_REMOVE
            push 0
            push 0
            push 0
            push 0
                push edi    // device context
                    push 1
                    push 1
                    push -1
                    push -1
                        push WINDOW_HEIGHT
                        push 8    // step size
                        push 0
                        push esi
                        call glViewport
                    call glRects
                call SwapBuffers
                call glFlush    // TODO not needed when drawing full-screen quad ???
            call PeekMessageA   
        call GetAsyncKeyState
            add esi, 8  // move rendering region
        test ax, ax     // GetAsyncKeyState(VK_ESCAPE) == 0
        jz render_loop

        // HACK: "push 0" omitted, potential access violation
        call ExitProcess    // TODO get rid of this
    }
}