#pragma once

#include "widget.h"

#include <memory>
#include <string>

class NativeTextBox;

class TextBox : public Widget {
public:
    explicit TextBox(std::wstring text);
    ~TextBox() override;

    void attachToWindow(void* parent) override;
    void setVisible(bool visible) override;
    void draw(DrawContext& context) override;

    void setText(std::wstring text);
    std::wstring text() const;
    void setPlaceholder(std::wstring placeholder);

protected:
    void syncNativeControl() override;

private:
    std::unique_ptr<NativeTextBox> nativeControl_;
    std::wstring text_;
    std::wstring placeholder_;
};
