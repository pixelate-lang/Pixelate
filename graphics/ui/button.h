#include "widget.h"
#include <functional>
#include <string>
#pragma once
class Button : public Widget {
public:
    explicit Button(std::wstring text);
    void setText(std::wstring text);
    const std::wstring& text() const;
    using ClickHandler = std::function<void()>;

    void setOnClick(ClickHandler handler);
    void click();

    void draw(DrawContext& context) override;
    bool onMouseDown(int x, int y) override;
    CursorType cursor() const override;

private:
    ClickHandler onClick_;
    std::wstring text_;
};