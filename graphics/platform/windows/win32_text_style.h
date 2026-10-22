#pragma once

#include "../platform_types.h"

#include <windows.h>

inline const wchar_t* win32FontName(FontFamily family) {
    switch (family) {
        case FontFamily::Arial: return L"Arial";
        case FontFamily::Calibri: return L"Calibri";
        case FontFamily::Consolas: return L"Consolas";
        case FontFamily::CourierNew: return L"Courier New";
        case FontFamily::TimesNewRoman: return L"Times New Roman";
        case FontFamily::Tahoma: return L"Tahoma";
        case FontFamily::Verdana: return L"Verdana";
        case FontFamily::Default: return L"Segoe UI";
    }
    return L"Segoe UI";
}

inline HFONT createWin32Font(const TextStyle& style) {
    const int size = style.size > 0 ? style.size : 9;
    return CreateFontW(
        -size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        win32FontName(style.family)
    );
}
