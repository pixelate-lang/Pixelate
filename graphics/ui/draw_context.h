#pragma once

#include <string>
#include <array>
#include "../../language/ast.h"
#include "../platform/platform_types.h"

class DrawContext {
public:
    virtual ~DrawContext() = default;

    virtual void fillRectangle(
        int x,
        int y,
        int width,
        int height
    ) = 0;

    virtual void drawRectangle(int x, int y, int width, int height, int angle) = 0;
    virtual void drawCircle(int x, int y, int radius) = 0;
    virtual void drawTriangle(const std::array<PointNode, 3>& points, int angle) = 0;

    virtual void drawText(
        const std::wstring& text,
        int x,
        int y,
        int width,
        int height,
        const TextStyle& style
    ) = 0;

    virtual void drawParagraph(
        const std::wstring& text,
        int x,
        int y,
        int width,
        int height,
        const TextStyle& style
    ) = 0;

protected:
    DrawContext() = default;
};
