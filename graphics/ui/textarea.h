#pragma once

#include "widget.h"

#include <memory>
#include <string>

class NativeTextArea;

class TextArea : public Widget {
public:
    explicit TextArea(std::wstring text);
    ~TextArea() override;

    void attachToWindow(void* parent) override;
    void setVisible(bool visible) override;
    void draw(DrawContext& context) override;

    void setText(std::wstring text);
    std::wstring text() const;
    void setPlaceholder(std::wstring placeholder);

protected:
    void syncNativeControl() override;

private:
    std::unique_ptr<NativeTextArea> nativeControl_;
    std::wstring text_;
    std::wstring placeholder_;
};
