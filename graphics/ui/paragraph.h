#pragma once

#include "widget.h"

#include <string>

class Paragraph : public Widget {
public:
    explicit Paragraph(std::wstring text);
    void setText(std::wstring text);
    const std::wstring& text() const;
    void draw(DrawContext& context) override;

private:
    std::wstring text_;
};
