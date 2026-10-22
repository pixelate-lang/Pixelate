#include "../native_textbox.h"
#include "win32_text_style.h"

#include <windows.h>

class Win32NativeTextBox final : public NativeTextBox {
public:
    ~Win32NativeTextBox() override {
        if (edit_) {
            DestroyWindow(edit_);
        }
        if (font_) DeleteObject(font_);
    }

    void attach(
        void* parentHandle,
        const std::wstring& text,
        int x,
        int y,
        int width,
        int height
    ) override {
        HWND parent = static_cast<HWND>(parentHandle);
        edit_ = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            text.c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            x,
            y,
            width,
            height,
            parent,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr
        );

        if (!edit_) return;

        SendMessageW(
            edit_,
            WM_SETFONT,
            reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)),
            TRUE
        );

        SetWindowLongPtrW(
            edit_,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(this)
        );
        previousEditProc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
            edit_,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(&EditProc)
        ));

        showPlaceholder();
        setTextStyle(style_);
    }

    void setBounds(int x, int y, int width, int height) override {
        if (!edit_) return;
        SetWindowPos(
            edit_, nullptr, x, y, width, height,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
    }

    void setText(const std::wstring& text) override {
        text_ = text;
        showingPlaceholder_ = false;
        if (!edit_) return;

        SetWindowTextW(edit_, text_.c_str());
        if (text_.empty() && GetFocus() != edit_) {
            showPlaceholder();
        }
    }

    std::wstring text() const override {
        if (showingPlaceholder_ || !edit_) {
            return showingPlaceholder_ ? std::wstring{} : text_;
        }

        int length = GetWindowTextLengthW(edit_);
        std::wstring value(static_cast<std::size_t>(length) + 1, L'\0');
        GetWindowTextW(edit_, value.data(), length + 1);
        value.resize(static_cast<std::size_t>(length));
        return value;
    }

    void setPlaceholder(const std::wstring& placeholder) override {
        placeholder_ = placeholder;
        if (!edit_) return;

        if (GetFocus() == edit_) hidePlaceholder();
        else showPlaceholder();
    }

    void setTextStyle(const TextStyle& style) override {
        style_ = style;
        if (!edit_) return;
        if (font_) DeleteObject(font_);
        font_ = createWin32Font(style_);
        SendMessageW(edit_, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    }

    void setVisible(bool visible) override {
        if (edit_) ShowWindow(edit_, visible ? SW_SHOW : SW_HIDE);
    }

private:
    void showPlaceholder() {
        if (!edit_ || placeholder_.empty() || showingPlaceholder_ ||
            GetFocus() == edit_ || GetWindowTextLengthW(edit_) > 0) {
            return;
        }

        showingPlaceholder_ = true;
        SetWindowTextW(edit_, placeholder_.c_str());
    }

    void hidePlaceholder() {
        if (!edit_ || !showingPlaceholder_) return;
        showingPlaceholder_ = false;
        SetWindowTextW(edit_, text_.c_str());
    }

    static LRESULT CALLBACK EditProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    ) {
        auto* textBox = reinterpret_cast<Win32NativeTextBox*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA)
        );

        if (textBox && message == WM_SETFOCUS) {
            textBox->hidePlaceholder();
        }

        if (textBox && message == WM_SETCURSOR) {
            SetCursor(LoadCursorW(nullptr, MAKEINTRESOURCEW(32513)));
            return TRUE;
        }

        LRESULT result = textBox && textBox->previousEditProc_
            ? CallWindowProcW(
                textBox->previousEditProc_,
                hwnd,
                message,
                wParam,
                lParam
            )
            : DefWindowProcW(hwnd, message, wParam, lParam);

        if (textBox && message == WM_KILLFOCUS) {
            textBox->showPlaceholder();
        }

        return result;
    }

    HWND edit_ = nullptr;
    WNDPROC previousEditProc_ = nullptr;
    std::wstring text_;
    std::wstring placeholder_;
    bool showingPlaceholder_ = false;
    TextStyle style_;
    HFONT font_ = nullptr;
};

std::unique_ptr<NativeTextBox> createNativeTextBox() {
    return std::make_unique<Win32NativeTextBox>();
}
