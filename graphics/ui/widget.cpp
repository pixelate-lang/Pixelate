#include "widget.h"

void Widget::setPosition(int x, int y){
    x_ = x;
    y_ = y;
    syncNativeControl();
}
void Widget::setSize(int width, int height){
    width_= width;
    height_ = height;
    syncNativeControl();
}

void Widget::setFont(FontFamily family) {
    textStyle_.family = family;
    syncNativeControl();
}

void Widget::setFontSize(int size) {
    textStyle_.size = size;
    syncNativeControl();
}

void Widget::setTextStyle(TextStyle style) {
    textStyle_ = style;
    syncNativeControl();
}

bool Widget::containsPoint(int x, int y) const {
    return x >= x_
        && x <= x_ + width_
        && y >= y_
        && y <= y_ + height_;
}

bool Widget::onMouseDown(int, int) {
    return false;
}
