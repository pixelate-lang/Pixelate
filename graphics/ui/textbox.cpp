#include "textbox.h"

#include "../platform/native_textbox.h"

#include <utility>

TextBox::TextBox(std::wstring text)
    : nativeControl_(createNativeTextBox()),
      text_(std::move(text)) {
}

TextBox::~TextBox() = default;

void TextBox::attachToWindow(void* parent) {
    nativeControl_->attach(
        parent,
        text_,
        x_,
        y_,
        width_,
        height_
    );
    nativeControl_->setPlaceholder(placeholder_);
    nativeControl_->setTextStyle(textStyle_);
}

void TextBox::setVisible(bool visible) {
    Widget::setVisible(visible);
    nativeControl_->setVisible(visible);
}

void TextBox::draw(DrawContext&) {
    // The platform textbox draws itself.
}

void TextBox::setText(std::wstring text) {
    text_ = std::move(text);
    nativeControl_->setText(text_);
}

std::wstring TextBox::text() const {
    return nativeControl_->text();
}

void TextBox::setPlaceholder(std::wstring placeholder) {
    placeholder_ = std::move(placeholder);
    nativeControl_->setPlaceholder(placeholder_);
}

void TextBox::syncNativeControl() {
    nativeControl_->setBounds(x_, y_, width_, height_);
    nativeControl_->setTextStyle(textStyle_);
}
