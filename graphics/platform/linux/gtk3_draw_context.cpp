#include "gtk3_draw_context.h"
#include "gtk3_text_utils.h"

#include <pango/pangocairo.h>

#include <cmath>

namespace {

PangoLayout* createLayout(
    cairo_t* cairo,
    const std::wstring& text,
    const TextStyle& style,
    int width,
    bool wrap
) {
    PangoLayout* layout = pango_cairo_create_layout(cairo);
    pango_layout_set_text(layout, gtk3Utf8(text).c_str(), -1);

    PangoFontDescription* font = pango_font_description_new();
    pango_font_description_set_family(font, gtk3FontFamily(style.family));
    pango_font_description_set_absolute_size(font, gtk3FontPixelSize(style));
    pango_layout_set_font_description(layout, font);
    pango_font_description_free(font);

    if (wrap) {
        pango_layout_set_width(layout, width * PANGO_SCALE);
        pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
    }

    return layout;
}

void drawLayout(cairo_t* cairo, PangoLayout* layout, int x, int y) {
    cairo_move_to(cairo, x, y);
    pango_cairo_show_layout(cairo, layout);
}

}

void Gtk3DrawContext::fillRectangle(
    int x,
    int y,
    int width,
    int height
) {
    cairo_set_source_rgb(cairo_, 70.0 / 255.0, 120.0 / 255.0, 220.0 / 255.0);
    cairo_rectangle(cairo_, x, y, width, height);
    cairo_fill(cairo_);
}

void Gtk3DrawContext::drawRectangle(int x, int y, int width, int height, int angle) {
    cairo_save(cairo_);
    cairo_translate(cairo_, x + width / 2.0, y + height / 2.0);
    cairo_rotate(cairo_, angle * 3.14159265358979323846 / 180.0);
    cairo_set_source_rgb(cairo_, 70.0 / 255.0, 120.0 / 255.0, 220.0 / 255.0);
    cairo_rectangle(cairo_, -width / 2.0, -height / 2.0, width, height);
    cairo_fill(cairo_);
    cairo_restore(cairo_);
}

void Gtk3DrawContext::drawCircle(int x, int y, int radius) {
    cairo_set_source_rgb(cairo_, 70.0 / 255.0, 120.0 / 255.0, 220.0 / 255.0);
    cairo_arc(cairo_, x, y, radius, 0, 2 * 3.14159265358979323846);
    cairo_fill(cairo_);
}

void Gtk3DrawContext::drawTriangle(const std::array<PointNode, 3>& points, int angle) {
    const double radians = angle * 3.14159265358979323846 / 180.0;
    const double centerX = (points[0].x + points[1].x + points[2].x) / 3.0;
    const double centerY = (points[0].y + points[1].y + points[2].y) / 3.0;
    cairo_set_source_rgb(cairo_, 70.0 / 255.0, 120.0 / 255.0, 220.0 / 255.0);
    cairo_save(cairo_);
    cairo_translate(cairo_, centerX, centerY);
    cairo_rotate(cairo_, radians);
    cairo_move_to(cairo_, points[0].x - centerX, points[0].y - centerY);
    cairo_line_to(cairo_, points[1].x - centerX, points[1].y - centerY);
    cairo_line_to(cairo_, points[2].x - centerX, points[2].y - centerY);
    cairo_close_path(cairo_);
    cairo_fill(cairo_);
    cairo_restore(cairo_);
}

void Gtk3DrawContext::drawText(
    const std::wstring& text,
    int x,
    int y,
    int width,
    int height,
    const TextStyle& style
) {
    PangoLayout* layout = createLayout(cairo_, text, style, width, false);
    int textWidth = 0;
    int textHeight = 0;
    pango_layout_get_pixel_size(layout, &textWidth, &textHeight);

    cairo_set_source_rgb(cairo_, 0.0, 0.0, 0.0);
    drawLayout(
        cairo_,
        layout,
        x + (width - textWidth) / 2,
        y + (height - textHeight) / 2
    );
    g_object_unref(layout);
}

void Gtk3DrawContext::drawParagraph(
    const std::wstring& text,
    int x,
    int y,
    int width,
    int height,
    const TextStyle& style
) {
    PangoLayout* layout = createLayout(cairo_, text, style, width, true);
    pango_layout_set_height(layout, height * PANGO_SCALE);
    cairo_set_source_rgb(cairo_, 0.0, 0.0, 0.0);
    drawLayout(cairo_, layout, x, y);
    g_object_unref(layout);
}
