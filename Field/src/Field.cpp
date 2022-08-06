
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Field.h"
#include "Filer.h"
#include "imgui.h"
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
typedef void (*FieldMainMarkerCallback)(const char* file, int line, const char* section, void* user_data);
extern FieldMainMarkerCallback  GFieldMainMarkerCallback;
extern void* GImGuiDemoMarkerCallbackUserData;
FieldMainMarkerCallback         GFieldMainMarkerCallback = NULL;
void* GFieldMainMarkerCallbackUserData = NULL;
#define FIELD_TEMPLATE_MARKER(section)  do { if (GFieldMainMarkerCallback != NULL) GFieldMainMarkerCallback(__FILE__, __LINE__, section, GFieldMainMarkerCallbackUserData); } while (0)





// Demonstrate most Dear ImGui features (this is big function!)
// You may execute this function to experiment with the UI and understand what it does.
// You may then search for keywords in the code when you are interested by a specific feature.
void Field::ShowMainWindow(bool* p_open)
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

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Main", p_open, window_flags))
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
  
    static ImVector<ImVec2> points;
    static ImVec2 scrolling(0.0f, 0.0f);
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = true;

    // Put UI Elements Here

    

    // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
    ImVec2 gridPoint0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    ImVec2 availSpace = ImGui::GetContentRegionAvail();   // Resize canvas to what's available

    // This is the aspect ratio 16 = x, 9 = y for a 16:9 ratio
    ImVec2 gridRatio;
    gridRatio.x = 20;
    gridRatio.y = 10;

    if (availSpace.x < 400.0f) 
        availSpace.x = 400.0f;
    if (availSpace.y < 200.0f)
        availSpace.y = 200.0f;

    ImVec2 availRatio = ImVec2(availSpace.x / gridRatio.x, availSpace.y / gridRatio.y);

    ImVec2 gridPoint1;


    if (availRatio.x < availRatio.y)
        gridPoint1 = ImVec2(gridPoint0.x + availSpace.x, (gridPoint0.y + ((availSpace.x / gridRatio.x) * gridRatio.y)));
    else
        gridPoint1 = ImVec2((gridPoint0.x + ((availSpace.y / gridRatio.y) * gridRatio.x)), gridPoint0.y + availSpace.y);



    gridPoint0 = ImVec2(gridPoint0.x + ((availSpace.x - (gridPoint1.x - gridPoint0.x)) / 2), gridPoint0.y);

    if (availRatio.x < availRatio.y)
        gridPoint1 = ImVec2(gridPoint0.x + availSpace.x, (gridPoint0.y + ((availSpace.x / gridRatio.x) * gridRatio.y)));
    else
        gridPoint1 = ImVec2((gridPoint0.x + ((availSpace.y / gridRatio.y) * gridRatio.x)), gridPoint0.y + availSpace.y);



    // Draw border and background color
    draw_list->AddRectFilled(gridPoint0, (gridPoint1), IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(gridPoint0, gridPoint1, IM_COL32(255, 255, 255, 255));




    // This will catch our interactions
    ImGui::InvisibleButton("canvas", availSpace, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool is_hovered = ImGui::IsItemHovered(); // Hovered
    const bool is_active = ImGui::IsItemActive();   // Held
    const ImVec2 origin(gridPoint0.x + scrolling.x, gridPoint0.y + scrolling.y); // Lock scrolled origin
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

    
    //Decides which way to shrink the grid
    //The divider is a multiplier of how many squares wanted (16:9 ratio divided by 10 would be 160:90 squares)
    float gridSpacer;
    int gridDivider = 10;

    if (availRatio.x < availRatio.y)
        gridSpacer = availRatio.x / gridDivider;
    else
        gridSpacer = availRatio.y / gridDivider;

    // Draw grid + all lines in the canvas
    draw_list->PushClipRect(gridPoint0, gridPoint1, true);



    int windowCenterX;
    int windowCenterY;

    windowCenterX = (gridPoint1.x + gridPoint0.x) / 2;
    windowCenterY = (gridPoint1.y + gridPoint0.y) / 2;

    ImVec2 Fiftyp1 = ImVec2(windowCenterX, gridPoint0.y);
    ImVec2 Fiftyp2 = ImVec2(windowCenterX, gridPoint1.y);

    auto LineDraw = [draw_list, windowCenterX, windowCenterY, gridSpacer](int x1, int y1, int x2, int y2, float th)
    {
        ImVec2 Linep1 = ImVec2(windowCenterX + (gridSpacer * x1), windowCenterY + ((gridSpacer * -1) * y1));
        ImVec2 Linep2 = ImVec2(windowCenterX + (gridSpacer * x2), windowCenterY + ((gridSpacer * -1) * y2));
        draw_list->AddLine(Linep1, Linep2, IM_COL32(255, 255, 255, 255), th);
    };


    //For vh variable 0 is vertical lines and 1 is horizontal
    auto HashDraw = [draw_list, windowCenterX, windowCenterY, gridSpacer](int x1, int y1, int x2, int y2,float sp, float th,int vh)
    {
        ImVec2 Hashp1 = ImVec2(windowCenterX + (gridSpacer * x1), windowCenterY + ((gridSpacer * -1) * y1));
        ImVec2 Hashp2 = ImVec2(windowCenterX + (gridSpacer * x2), windowCenterY + ((gridSpacer * -1) * y2));
        if(vh == 0)
            for (int h = 0; h <= (x2 / sp) * 2; h++) 
            {
                float x = h * (gridSpacer * sp);
                draw_list->AddLine(ImVec2(Hashp1.x + x, Hashp1.y), ImVec2(Hashp1.x + x, Hashp2.y), IM_COL32(255, 255, 255, 255), th);
            }
        if(vh == 1)
            for (int v = 0; v <= (x2 / sp) * 2; v++)
            {
                float y = v * (gridSpacer * sp);
                draw_list->AddLine(ImVec2(Hashp1.x, Hashp1.y + y), ImVec2(Hashp2.x, Hashp1.y + y), IM_COL32(255, 255, 255, 255), th);
            }
    };


    auto TextDraw = [draw_list, windowCenterX, windowCenterY, gridSpacer](int x, int y,const char * text)
    {
        ImVec2 Textp1 = ImVec2(windowCenterX + (gridSpacer * x), windowCenterY + ((gridSpacer * -1) * y));
        ImGui::SetWindowFontScale(gridSpacer/4.5);
        draw_list->AddText(Textp1, IM_COL32(255, 255, 255, 255),text);
        ImGui::SetWindowFontScale(1);
    };

    //DrawText(draw_list, windowCenterX, windowCenterY, gridSpacer, -2, 0, "50");

    
    
    for (int l = 1; l <= 100; l += 1) 
    {
        if (Filer::ContainsT("L", l)) 
        {
            float th = Filer::LoadN("L", l, "th");
            int x1 = Filer::LoadN("L", l, "x1");
            int y1 = Filer::LoadN("L", l, "y1");
            int x2 = Filer::LoadN("L", l, "x2");
            int y2 = Filer::LoadN("L", l, "y2");
            LineDraw(x1, y1, x2, y2, th);
        }
    }
        
    for (int h = 1; h <= 100; h += 1)
    {
        if (Filer::ContainsT("H", h))
        {
            float sp = Filer::LoadN("H", h, "sp");
            float th = Filer::LoadN("H", h, "th");
            int vh = Filer::LoadN("H", h, "vh");
            int x1 = Filer::LoadN("H", h, "x1");
            int y1 = Filer::LoadN("H", h, "y1");
            int x2 = Filer::LoadN("H", h, "x2");
            int y2 = Filer::LoadN("H", h, "y2");
            HashDraw(x1, y1, x2, y2, sp, th, vh);
        }
    }

    for (int t = 1; t <= 100; t += 1)
    {
        if (Filer::ContainsT("T", t))
        {
            int x = Filer::LoadN("T", t, "x");
            int y = Filer::LoadN("T", t, "y");
            std::string t1 = Filer::LoadS("T", t, "t");
            const char* t2 = t1.c_str();
            TextDraw(x, y, t2);
        }
    }
    
    


    if (opt_enable_grid)
    {
        const float GRID_STEP = gridSpacer;
        for (float x = fmodf(scrolling.x, GRID_STEP); x < availSpace.x; x += GRID_STEP)
            draw_list->AddLine(ImVec2(gridPoint0.x + x, gridPoint0.y), ImVec2(gridPoint0.x + x, gridPoint1.y), IM_COL32(200, 200, 200, 40));
            

        for (float y = fmodf(scrolling.y, GRID_STEP); y < availSpace.y; y += GRID_STEP)
            draw_list->AddLine(ImVec2(gridPoint0.x, gridPoint0.y + y), ImVec2(gridPoint1.x, gridPoint0.y + y), IM_COL32(200, 200, 200, 40));
    }

    

    for (int n = 0; n < points.Size; n += 2)
        draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
    draw_list->PopClipRect();

    ImGui::End();
};

void Field::DrawText(ImDrawList* draw_list, int windowCenterX, int windowCenterY, float gridSpacer, int x, int y, const char* text)
{
    ImVec2 Textp1 = ImVec2(windowCenterX + (gridSpacer * x), windowCenterY + ((gridSpacer * -1) * y));
    ImGui::SetWindowFontScale(gridSpacer / 5);
    draw_list->AddText(Textp1, IM_COL32(255, 255, 255, 255), text);
};




// End of code
#else

void ImGui::ShowAboutWindow(bool*) {}
void Field::ShowMainWindow(bool*) {}
void Field::LineDraw(int x1, int y1, int x2, int y2, float th)
void ImGui::ShowUserGuide() {}
void ImGui::ShowStyleEditor(ImGuiStyle*) {}

#endif

#endif // #ifndef IMGUI_DISABLE
