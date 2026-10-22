#include "button.h"
#include "draw_context.h"
#include <utility>

Button::Button(std::wstring text)
    : text_(std::move(text)) {
}

void Button::setText(std::wstring text) {
    text_ = std::move(text);
}

const std::wstring& Button::text() const {
    return text_;
}

void Button::draw(DrawContext& context) {
    context.fillRectangle(
        x_,
        y_,
        width_,
        height_
    );
    context.drawText(text_, x_, y_, width_, height_, textStyle_);

}

void Button::setOnClick(ClickHandler handler) {
    onClick_ = std::move(handler);
}

void Button::click() {
    if (onClick_) {
        onClick_();
    }
}

bool Button::onMouseDown(int x, int y) {
    if (!containsPoint(x, y)) {
        return false;
    }

    click();
    return true;
}

CursorType Button::cursor() const {
    return CursorType::Hand;
}
