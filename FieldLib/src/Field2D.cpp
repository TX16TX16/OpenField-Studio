
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Field.h"
#include "Filer.h"
#include "imgui.h"
#include "implot.h"



#ifndef IMGUI_DISABLE

// System includes
#include <iostream>
#include <ctype.h>          // toupper
#include <limits.h>         // INT_MIN, INT_MAX
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>         // intptr_t
#else
#include <stdint.h>         // intptr_t
#endif


// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                     // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                           // yes, they are more terse.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"        // warning: 'xx' is deprecated: The POSIX name for this..   // for strdup used in demo code (so user can copy & paste the code)
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"       // warning: cast to 'void *' from smaller integer type
#pragma clang diagnostic ignored "-Wformat-security"                // warning: format string is not a string literal
#pragma clang diagnostic ignored "-Wexit-time-destructors"          // warning: declaration requires an exit-time destructor    // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wunused-macros"                  // warning: macro is not used                               // we define snprintf/vsnprintf on Windows so they are available, but not always used.
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                   // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wreserved-id-macro"              // warning: macro name is a reserved identifier
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                  // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"      // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat-security"          // warning: format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wmisleading-indentation"   // [__GNUC__ >= 6] warning: this 'if' clause does not guard this statement      // GCC 6.0+ only. See #883 on GitHub.
#endif

// Play it nice with Windows users (Update: May 2018, Notepad now supports Unix-style carriage returns!)
#ifdef _WIN32
#define IM_NEWLINE  "\r\n"
#else
#define IM_NEWLINE  "\n"
#endif

// Helpers
#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf    _snprintf
#endif
#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf   _vsnprintf
#endif

// Format specifiers, printing 64-bit hasn't been decently standardized...
// In a real application you should be using PRId64 and PRIu64 from <inttypes.h> (non-windows) and on Windows define them yourself.
#ifdef _MSC_VER
#define IM_PRId64   "I64d"
#define IM_PRIu64   "I64u"
#else
#define IM_PRId64   "lld"
#define IM_PRIu64   "llu"
#endif

// Helpers macros
// We normally try to not use many helpers in imgui_demo.cpp in order to make code easier to copy and paste,
// but making an exception here as those are largely simplifying code...
// In other imgui sources we can use nicer internal functions from imgui_internal.h (ImMin/ImMax) but not in the demo.
#define IM_MIN(A, B)            (((A) < (B)) ? (A) : (B))
#define IM_MAX(A, B)            (((A) >= (B)) ? (A) : (B))
#define IM_CLAMP(V, MN, MX)     ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifndef IMGUI_CDECL
#ifdef _MSC_VER
#define IMGUI_CDECL __cdecl
#else
#define IMGUI_CDECL
#endif
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward Declarations, Helpers
//-----------------------------------------------------------------------------

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)

// Forward Declarations
static void ShowExampleAppLog(bool* p_open);
static void ShowExampleAppLayout(bool* p_open);
static void ShowExampleAppPropertyEditor(bool* p_open);
static void ShowExampleAppLongText(bool* p_open);
static void ShowExampleAppAutoResize(bool* p_open);
static void ShowExampleAppConstrainedResize(bool* p_open);
static void ShowExampleAppSimpleOverlay(bool* p_open);
static void ShowExampleAppFullscreen(bool* p_open);
static void ShowExampleAppWindowTitles(bool* p_open);
static void ShowExampleAppCustomRendering(bool* p_open);
static void ShowExampleMenuFile();



// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
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

static void ShowDockingDisabledMessage()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
    ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::SmallButton("click here"))
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

// Helper to wire demo markers located in code to a interactive browser
typedef void (*FieldFeild2DMarkerCallback)(const char* file, int line, const char* section, void* user_data);
extern FieldFeild2DMarkerCallback  GFieldFeild2DMarkerCallback;
extern void* GImGuiDemoMarkerCallbackUserData;
FieldFeild2DMarkerCallback         GFieldFeild2DMarkerCallback = NULL;
void* GFieldFeild2DMarkerCallbackUserData = NULL;
#define FIELD_TEMPLATE_MARKER(section)  do { if (GFieldFeild2DMarkerCallback != NULL) GFieldFeild2DMarkerCallback(__FILE__, __LINE__, section, GFieldFeild2DMarkerCallbackUserData); } while (0)



// Demonstrate most Dear ImGui features (this is big function!)
// You may execute this function to experiment with the UI and understand what it does.
// You may then search for keywords in the code when you are interested by a specific feature.
void Field::ShowFeild2DWindow(bool* p_open)
{
    // Exceptionally add an extra assert here for people confused about initial Dear ImGui setup
    // Most ImGui functions would normally just crash if the context is missing.
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

    // Demonstrate the various window flags. Typically you would just use the default!
    static bool no_titlebar = false;
    static bool no_scrollbar = true;
    static bool no_menu = true;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = true;
    static bool no_close = true;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;
    static bool no_docking = false;
    static bool unsaved_document = false;

    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (no_docking)         window_flags |= ImGuiWindowFlags_NoDocking;
    if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;
    if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    // Feild2D body of the Demo window starts here.
    if (!ImGui::Begin("Feild2D", p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }


    // e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
    ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

    ImGuiIO& io = ImGui::GetIO();



    ImGui::PushItemWidth(-ImGui::GetFontSize() * 15);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();



    // Put UI Elements Here
    /*
    ImGui::BulletText("Click and drag each point.");
    static ImPlotDragToolFlags flags = ImPlotDragToolFlags_None;
    ImGui::CheckboxFlags("NoInput", (unsigned int*)&flags, ImPlotDragToolFlags_NoInputs);
    if (ImPlot::BeginPlot("##Bezier", ImVec2(-1, 0), ImPlotFlags_CanvasOnly)) {
        ImPlot::SetupAxes(0, 0);
        ImPlot::SetupAxesLimits(0, 1, 0, 1);
        static ImPlotPoint P[] = { ImPlotPoint(.05f,.05f), ImPlotPoint(0.2,0.4),  ImPlotPoint(0.8,0.6),  ImPlotPoint(.95f,.95f) };

        ImPlot::DragPoint(0, &P[0].x, &P[0].y, ImVec4(0, 0.9f, 0, 1), 4, flags);
        ImPlot::DragPoint(1, &P[1].x, &P[1].y, ImVec4(1, 0.5f, 1, 1), 4, flags);
        ImPlot::DragPoint(2, &P[2].x, &P[2].y, ImVec4(0, 0.5f, 1, 1), 4, flags);
        ImPlot::DragPoint(3, &P[3].x, &P[3].y, ImVec4(0, 0.9f, 0, 1), 4, flags);

        static ImPlotPoint B[100];
        for (int i = 0; i < 100; ++i) {
            double t = i / 99.0;
            double u = 1 - t;
            double w1 = u * u * u;
            double w2 = 3 * u * u * t;
            double w3 = 3 * u * t * t;
            double w4 = t * t * t;
            B[i] = ImPlotPoint(w1 * P[0].x + w2 * P[1].x + w3 * P[2].x + w4 * P[3].x, w1 * P[0].y + w2 * P[1].y + w3 * P[2].y + w4 * P[3].y);
        }


        ImPlot::SetNextLineStyle(ImVec4(1, 0.5f, 1, 1));
        ImPlot::PlotLine("##h1", &P[0].x, &P[0].y, 2, 0, 0, sizeof(ImPlotPoint));
        ImPlot::SetNextLineStyle(ImVec4(0, 0.5f, 1, 1));
        ImPlot::PlotLine("##h2", &P[2].x, &P[2].y, 2, 0, 0, sizeof(ImPlotPoint));
        ImPlot::SetNextLineStyle(ImVec4(0, 0.9f, 0, 1), 2);
        ImPlot::PlotLine("##bez", &B[0].x, &B[0].y, 100, 0, 0, sizeof(ImPlotPoint));

        ImPlot::EndPlot();
    } 
    */
    
    static float constraints[4] = { 88,46,16,176};

    static ImPlotDragToolFlags flags = ImPlotDragToolFlags_None;
    ImGui::CheckboxFlags("NoInput", (unsigned int*)&flags, ImPlotDragToolFlags_NoInputs);

    static ImPlotFlags plotFlags = ImPlotFlags_CanvasOnly;

    // Initalize Grid
    if (ImPlot::BeginPlot("##Bezier", ImVec2(-1, -1), plotFlags)) {

        // Axis Labels
        ImPlot::SetupAxes(0, 0);

        ImPlot::SetupAxisScale(ImAxis_X1,0,0,0);

        ImPlot::SetupAxesLimits(-88, 88, -46, 46);

        // Max Grid Size
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, -(constraints[0]), constraints[0]);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, -(constraints[1]), constraints[1]);

        // Max Zoom
        ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 17.2, 172);
        ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, 9.2, 92);


        static ImPlotPoint P[] = { ImPlotPoint(.05f,.05f), ImPlotPoint(0.2,0.4),  ImPlotPoint(0.8,0.6),  ImPlotPoint(.95f,.95f) };

        ImPlot::DragPoint(0, &P[0].x, &P[0].y, ImVec4(0, 0.9f, 0, 1), 4, flags);
        ImPlot::DragPoint(1, &P[1].x, &P[1].y, ImVec4(1, 0.5f, 1, 1), 4, flags);
        ImPlot::DragPoint(2, &P[2].x, &P[2].y, ImVec4(0, 0.5f, 1, 1), 4, flags);
        ImPlot::DragPoint(3, &P[3].x, &P[3].y, ImVec4(0, 0.9f, 0, 1), 4, flags);

        static ImPlotPoint B[1000];
        for (int i = 0; i < 1000; ++i) {
            double t = i / 99.0;
            double u = 1 - t;
            double w1 = u * u * u;
            double w2 = 3 * u * u * t;
            double w3 = 3 * u * t * t;
            double w4 = t * t * t;
            B[i] = ImPlotPoint(w1 * P[0].x + w2 * P[1].x + w3 * P[2].x + w4 * P[3].x, w1 * P[0].y + w2 * P[1].y + w3 * P[2].y + w4 * P[3].y);
        }

        //ImPlotFlags_Equal

        ImPlot::SetNextLineStyle(ImVec4(1, 0.5f, 1, 1));
        ImPlot::PlotLine("##h1", &P[0].x, &P[0].y, 2, 0, 0, sizeof(ImPlotPoint));
        ImPlot::SetNextLineStyle(ImVec4(0, 0.5f, 1, 1));
        ImPlot::PlotLine("##h2", &P[2].x, &P[2].y, 2, 0, 0, sizeof(ImPlotPoint));
        ImPlot::SetNextLineStyle(ImVec4(0, 0.9f, 0, 1), 2);
        ImPlot::PlotLine("##bez", &B[0].x, &B[0].y, 100, 0, 0, sizeof(ImPlotPoint));

        ImPlot::EndPlot();
    }

    
    /*
    ImGui::DragFloat2("Limits Constraints", &constraints[0], 0.01);
    ImGui::DragFloat2("Zoom Constraints", &constraints[2], 0.01);
    if (ImPlot::BeginPlot("##AxisConstraints", ImVec2(-1, 0))) {
        ImPlot::SetupAxes("X", "Y", flags, flags);
        ImPlot::SetupAxesLimits(-1, 1, -1, 1);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, constraints[0], constraints[1]);
        ImPlot::SetupAxisZoomConstraints(ImAxis_X1, constraints[2], constraints[3]);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, constraints[0], constraints[1]);
        ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, constraints[2], constraints[3]);
        ImPlot::EndPlot();
    }
    */

    static bool clamp = false;
    ImGui::Checkbox("Clamp", &clamp);

    if (ImPlot::BeginPlot("##Annotations")) {
        ImPlot::SetupAxesLimits(0, 2, 0, 1);
        static float p[] = { 0.25f, 0.25f, 0.75f, 0.75f, 0.25f };
        ImPlot::PlotScatter("##Points", &p[0], &p[1], 4);
        ImVec4 col = ImPlot::GetLastItemColor();
        ImPlot::Annotation(0.25, 0.25, col, ImVec2(-15, 15), clamp, "BL");
        ImPlot::Annotation(0.75, 0.25, col, ImVec2(15, 15), clamp, "BR");
        ImPlot::Annotation(0.75, 0.75, col, ImVec2(15, -15), clamp, "TR");
        ImPlot::Annotation(0.25, 0.75, col, ImVec2(-15, -15), clamp, "TL");
        ImPlot::Annotation(0.5, 0.5, col, ImVec2(0, 0), clamp, "Center");

        ImPlot::Annotation(1.25, 0.75, ImVec4(0, 1, 0, 1), ImVec2(0, 0), clamp);

        float bx[] = { 1.2f,1.5f,1.8f };
        float by[] = { 0.25f, 0.5f, 0.75f };
        ImPlot::PlotBars("##Bars", bx, by, 3, 0.2);
        for (int i = 0; i < 3; ++i)
            ImPlot::Annotation(bx[i], by[i], ImVec4(0, 0, 0, 0), ImVec2(0, -5), clamp, "B[%d]=%.2f", i, by[i]);
        ImPlot::EndPlot();
    }

    static double xs[1001], ys1[1001], ys2[1001];
    for (int i = 0; i < 1001; ++i) {
        xs[i] = i * 0.1f - 50;
        ys1[i] = sin(xs[i]);
        ys2[i] = i * 0.002 - 1;
    }
    if (ImPlot::BeginPlot("SymLog Plot", ImVec2(-1, 0))) {
        ImPlot::SetupAxisScale(ImAxis_X1, 4);
        ImPlot::PlotLine("f(x) = a*x+b", xs, ys2, 1001);
        ImPlot::PlotLine("f(x) = sin(x)", xs, ys1, 1001);
        ImPlot::EndPlot();
    }
 

    ImGui::End();
};





// End of code
#else

void ImGui::ShowAboutWindow(bool*) {}
void Field::ShowFeild2DWindow(bool*) {}
void Field::LineDraw(int x1, int y1, int x2, int y2, float th)
void ImGui::ShowUserGuide() {}
void ImGui::ShowStyleEditor(ImGuiStyle*) {}

#endif

#endif // #ifndef IMGUI_DISABLE
