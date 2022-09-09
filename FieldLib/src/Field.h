#pragma once

// Includes

#include "imgui.h"

#include <float.h>                  // FLT_MIN, FLT_MAX
#include <stdarg.h>                 // va_list, va_start, va_end
#include <stddef.h>                 // ptrdiff_t, NULL
#include <string.h>                 // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp
#include <iostream>
#include <fstream>

namespace Field
{
    // Demo, Debug, Information

    //
    void    ShowTemplateWindow(bool* p_open = NULL);   
    void    ShowMainWindow(bool* p_open = NULL);        
    void    ShowToolsWindow(bool* p_open = NULL);
    void    ShowInfoWindow(bool* p_open = NULL);
    void    ShowFeild2DWindow(bool* p_open = NULL);


} // namespace Field

extern ImVec2 gridPoint0;
extern ImVec2 gridPoint1;
extern float gridInfoSpacer;