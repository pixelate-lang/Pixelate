#pragma once

#include <memory>
#include <string>
#include "platform_types.h"

// Platform-neutral contract for an editable text control. The parent handle
// is intentionally opaque; each backend interprets it in its own way.
class NativeTextBox {
public:
    virtual ~NativeTextBox() = default;

    virtual void attach(
        void* parent,
        const std::wstring& text,
        int x,
        int y,
        int width,
        int height
    ) = 0;
    virtual void setBounds(int x, int y, int width, int height) = 0;
    virtual void setText(const std::wstring& text) = 0;
    virtual std::wstring text() const = 0;
    virtual void setPlaceholder(const std::wstring& text) = 0;
    virtual void setTextStyle(const TextStyle& style) = 0;
    virtual void setVisible(bool visible) = 0;
};

// The active platform supplies this factory. The current implementation is
// Win32; other platforms can provide the same function in their backend.
std::unique_ptr<NativeTextBox> createNativeTextBox();

class NativeTextArea {
public:
    virtual ~NativeTextArea() = default;

    virtual void attach(
        void* parent,
        const std::wstring& text,
        int x,
        int y,
        int width,
        int height
    ) = 0;
    virtual void setBounds(int x, int y, int width, int height) = 0;
    virtual void setText(const std::wstring& text) = 0;
    virtual std::wstring text() const = 0;
    virtual void setPlaceholder(const std::wstring& text) = 0;
    virtual void setTextStyle(const TextStyle& style) = 0;
    virtual void setVisible(bool visible) = 0;
};

std::unique_ptr<NativeTextArea> createNativeTextArea();
