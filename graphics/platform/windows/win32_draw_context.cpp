#include "win32_draw_context.h"

#include <array>
#include <cmath>

namespace {

const wchar_t* fontName(FontFamily family) {
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

HFONT createFont(const TextStyle& style) {
    const int size = style.size > 0 ? style.size : 9;
    return CreateFontW(-size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontName(style.family));
}

}

void Win32DrawContext::fillRectangle(
    int x,
    int y,
    int width,
    int height
) {
    RECT rectangle{x, y, x + width, y + height};
    HBRUSH brush = CreateSolidBrush(RGB(70, 120, 220));

    FillRect(deviceContext_, &rectangle, brush);
    DeleteObject(brush);
}

void Win32DrawContext::drawRectangle(int x, int y, int width, int height, int angle) {
    const double radians = angle * 3.14159265358979323846 / 180.0;
    const double centerX = x + width / 2.0;
    const double centerY = y + height / 2.0;
    const std::array<std::pair<double, double>, 4> corners = {{
        {-width / 2.0, -height / 2.0}, {width / 2.0, -height / 2.0},
        {width / 2.0, height / 2.0}, {-width / 2.0, height / 2.0}
    }};
    POINT points[4]{};
    for (int index = 0; index < 4; ++index) {
        const double rotatedX = corners[index].first * std::cos(radians) -
            corners[index].second * std::sin(radians);
        const double rotatedY = corners[index].first * std::sin(radians) +
            corners[index].second * std::cos(radians);
        points[index] = {
            static_cast<LONG>(std::lround(centerX + rotatedX)),
            static_cast<LONG>(std::lround(centerY + rotatedY))
        };
    }
    HBRUSH brush = CreateSolidBrush(RGB(70, 120, 220));
    HGDIOBJ oldBrush = SelectObject(deviceContext_, brush);
    Polygon(deviceContext_, points, 4);
    SelectObject(deviceContext_, oldBrush);
    DeleteObject(brush);
}

void Win32DrawContext::drawCircle(int x, int y, int radius) {
    HBRUSH brush = CreateSolidBrush(RGB(70, 120, 220));
    HGDIOBJ oldBrush = SelectObject(deviceContext_, brush);
    Ellipse(deviceContext_, x - radius, y - radius, x + radius, y + radius);
    SelectObject(deviceContext_, oldBrush);
    DeleteObject(brush);
}

void Win32DrawContext::drawTriangle(const std::array<PointNode, 3>& points, int angle) {
    const double radians = angle * 3.14159265358979323846 / 180.0;
    const double centerX = (points[0].x + points[1].x + points[2].x) / 3.0;
    const double centerY = (points[0].y + points[1].y + points[2].y) / 3.0;
    POINT nativePoints[3]{};
    for (int index = 0; index < 3; ++index) {
        const double dx = points[index].x - centerX;
        const double dy = points[index].y - centerY;
        nativePoints[index] = {
            static_cast<LONG>(std::lround(centerX + dx * std::cos(radians) - dy * std::sin(radians))),
            static_cast<LONG>(std::lround(centerY + dx * std::sin(radians) + dy * std::cos(radians)))
        };
    }
    HBRUSH brush = CreateSolidBrush(RGB(70, 120, 220));
    HGDIOBJ oldBrush = SelectObject(deviceContext_, brush);
    Polygon(deviceContext_, nativePoints, 3);
    SelectObject(deviceContext_, oldBrush);
    DeleteObject(brush);
}

void Win32DrawContext::drawText(
    const std::wstring& text,
    int x,
    int y,
    int width,
    int height,
    const TextStyle& style
) {
    RECT area{x, y, x + width, y + height};
    int oldBackgroundMode = SetBkMode(deviceContext_, TRANSPARENT);
    COLORREF oldTextColor = SetTextColor(
        deviceContext_,
        RGB(0, 0, 0)
    );
    HFONT font = createFont(style);
    HFONT oldFont = static_cast<HFONT>(SelectObject(deviceContext_, font));

    DrawTextW(
        deviceContext_,
        text.c_str(),
        -1,
        &area,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SetBkMode(deviceContext_, oldBackgroundMode);
    SetTextColor(deviceContext_, oldTextColor);
    SelectObject(deviceContext_, oldFont);
    DeleteObject(font);
}

void Win32DrawContext::drawParagraph(
    const std::wstring& text,
    int x,
    int y,
    int width,
    int height,
    const TextStyle& style
) {
    RECT area{x, y, x + width, y + height};
    int oldBackgroundMode = SetBkMode(deviceContext_, TRANSPARENT);
    COLORREF oldTextColor = SetTextColor(deviceContext_, RGB(0, 0, 0));
    HFONT font = createFont(style);
    HFONT oldFont = static_cast<HFONT>(SelectObject(deviceContext_, font));

    DrawTextW(
        deviceContext_,
        text.c_str(),
        -1,
        &area,
        DT_LEFT | DT_TOP | DT_WORDBREAK
    );

    SetBkMode(deviceContext_, oldBackgroundMode);
    SetTextColor(deviceContext_, oldTextColor);
    SelectObject(deviceContext_, oldFont);
    DeleteObject(font);
}
