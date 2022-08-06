#pragma once

// Includes

#include "imgui.h"

#include <float.h>                  // FLT_MIN, FLT_MAX
#include <stdarg.h>                 // va_list, va_start, va_end
#include <stddef.h>                 // ptrdiff_t, NULL
#include <string.h>                 // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp



#ifndef FIELD_API
#define FIELD_API
#endif

namespace Field
{
    // Demo, Debug, Information

    //
    FIELD_API void          ShowTemplateWindow(bool* p_open = NULL);        // create Demo window. demonstrate most ImGui features. call this to learn about the library! try to make it always available in your application!
    FIELD_API void          ShowMainWindow(bool* p_open = NULL);        // create Demo window. demonstrate most ImGui features. call this to learn about the library! try to make it always available in your application!
    FIELD_API void          DrawText(ImDrawList* draw_list, int windowCenterX, int windowCenterY, float gridSpacer, int x, int y, const char* text);
    


} // namespace Field

