#pragma once
#include "../platform/platform_types.h"
#include "../../language/ast.h"

class DrawContext;

enum class CursorType {
    Default,
    Hand
};

class Widget {
public:
    virtual ~Widget() = default;

    virtual void draw(DrawContext& context) = 0;
    // The platform backend supplies an opaque native parent handle. Generic
    // widgets do not need to know which window system produced it.
    virtual void attachToWindow(void* parent) {
        (void)parent;
    }
    virtual void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }

    void setPosition(int x, int y);
    void setSize(int width, int height);
    int x() const { return x_; }
    int y() const { return y_; }
    int width() const { return width_; }
    int height() const { return height_; }
    void setFont(FontFamily family);
    void setFontSize(int size);
    void setTextStyle(TextStyle style);
    TextStyle textStyle() const { return textStyle_; }
    
    bool containsPoint(int x, int y) const;

    virtual bool onMouseDown(int x, int y);
    virtual CursorType cursor() const {
        return CursorType::Default;
    }
protected:
    virtual void syncNativeControl() {}
    int x_ = 0;
    int y_ = 0;
    int width_ = 100;
    int height_ = 30;
    TextStyle textStyle_;
    bool visible_ = true;
};
