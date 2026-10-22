#include "label.h"
#include "draw_context.h"
#include <utility>

Label::Label(std::wstring text)
    : text_(std::move(text)) {
}

void Label::draw(DrawContext& context) {
    context.drawText(text_, x_, y_, width_, height_, textStyle_);
}

void Label::setText(std::wstring text) {
    text_ = std::move(text);
}

const std::wstring& Label::text() const {
    return text_;
}
