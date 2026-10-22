#pragma once

#include "../../ui/draw_context.h"

#include <cairo.h>

class Gtk3DrawContext final : public DrawContext {
public:
    explicit Gtk3DrawContext(cairo_t* cairo)
        : cairo_(cairo) {
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
    cairo_t* cairo_;
};
