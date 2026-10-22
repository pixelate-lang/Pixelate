#include "textarea.h"

#include "../platform/native_textbox.h"

#include <utility>

TextArea::TextArea(std::wstring text)
    : nativeControl_(createNativeTextArea()),
      text_(std::move(text)) {
}

TextArea::~TextArea() = default;

void TextArea::attachToWindow(void* parent) {
    nativeControl_->attach(parent, text_, x_, y_, width_, height_);
    nativeControl_->setPlaceholder(placeholder_);
    nativeControl_->setTextStyle(textStyle_);
}

void TextArea::setVisible(bool visible) {
    Widget::setVisible(visible);
    nativeControl_->setVisible(visible);
}

void TextArea::draw(DrawContext&) {
    // The platform textarea draws itself.
}

void TextArea::setText(std::wstring text) {
    text_ = std::move(text);
    nativeControl_->setText(text_);
}

std::wstring TextArea::text() const {
    return nativeControl_->text();
}

void TextArea::setPlaceholder(std::wstring placeholder) {
    placeholder_ = std::move(placeholder);
    nativeControl_->setPlaceholder(placeholder_);
}

void TextArea::syncNativeControl() {
    nativeControl_->setBounds(x_, y_, width_, height_);
    nativeControl_->setTextStyle(textStyle_);
}
