#pragma once

#include <string>

// These are platform-neutral values used by the public UI API. A backend
// translates them to native colors, cursors, and window operations.
struct AppColor {
    int red = 255;
    int green = 255;
    int blue = 255;
};

enum class FontFamily {
    Default,
    Arial,
    Calibri,
    Consolas,
    CourierNew,
    TimesNewRoman,
    Tahoma,
    Verdana
};

struct TextStyle {
    FontFamily family = FontFamily::Default;
    int size = 16;
};

enum class MouseShape {
    Arrow,
    IBeam,
    Wait,
    Cross,
    UpArrow,
    Size,
    Icon,
    SizeNWSE,
    SizeNESW,
    SizeWE,
    SizeNS,
    SizeAll,
    No,
    Hand,
    AppStarting,
    Help,
    Pin,
    Person
};

struct MousePosition {
    int x = 0;
    int y = 0;
};
