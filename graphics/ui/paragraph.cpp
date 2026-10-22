#include "paragraph.h"

#include "draw_context.h"

#include <utility>

Paragraph::Paragraph(std::wstring text)
    : text_(std::move(text)) {
}

void Paragraph::setText(std::wstring text) {
    text_ = std::move(text);
}

const std::wstring& Paragraph::text() const {
    return text_;
}

void Paragraph::draw(DrawContext& context) {
    context.drawParagraph(text_, x_, y_, width_, height_, textStyle_);
}
