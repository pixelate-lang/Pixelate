#include "widget.h"
#include <string>
#include <utility>

#pragma once
class Label : public Widget {
    public:
        explicit Label(std::wstring text);
        void setText(std::wstring text);
        const std::wstring& text() const;
        void draw(DrawContext& context) override;

    private:
        std::wstring text_;
};