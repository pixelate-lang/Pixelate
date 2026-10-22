#pragma once

#include "../../ui/draw_context.h"

#include <windows.h>

class Win32DrawContext final : public DrawContext {
public:
    explicit Win32DrawContext(HDC deviceContext)
        : deviceContext_(deviceContext) {
    }

    void fillRectangle(int x, int y, int width, int height) override;
    void drawRectangle(int x, int y, int width, int height, int angle) override;
    void drawCircle(int x, int y, int radius) override;
    void drawTriangle(const std::array<PointNode, 3>& points, int angle) override;
    void drawText(
        const std::wstring& text,
        int x,
        int y,
        int width,
        int height,
        const TextStyle& style
    ) override;
    void drawParagraph(
        const std::wstring& text,
        int x,
        int y,
        int width,
        int height,
        const TextStyle& style
    ) override;

private:
    HDC deviceContext_;
};
