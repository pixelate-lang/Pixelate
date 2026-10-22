#include "../native_textbox.h"
#include "gtk3_text_utils.h"

#include <gtk/gtk.h>

class Gtk3NativeTextBox final : public NativeTextBox {
public:
    ~Gtk3NativeTextBox() override {
        gtk3ClearFont(entry_, fontProvider_);
        if (entry_) gtk_widget_destroy(entry_);
    }

    void attach(
        void* parent,
        const std::wstring& text,
        int x,
        int y,
        int width,
        int height
    ) override {
        auto* fixed = GTK_FIXED(parent);
        entry_ = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry_), gtk3Utf8(text).c_str());
        gtk_fixed_put(fixed, entry_, x, y);
        setBounds(x, y, width, height);
        setPlaceholder(placeholder_);
        setTextStyle(style_);
        gtk_widget_show(entry_);
    }

    void setBounds(int x, int y, int width, int height) override {
        if (!entry_) return;
        gtk_fixed_move(GTK_FIXED(gtk_widget_get_parent(entry_)), entry_, x, y);
        gtk_widget_set_size_request(entry_, width, height);
    }

    void setText(const std::wstring& text) override {
        text_ = text;
        if (entry_) gtk_entry_set_text(GTK_ENTRY(entry_), gtk3Utf8(text).c_str());
    }

    std::wstring text() const override {
        if (!entry_) return text_;
        return gtk3Wide(gtk_entry_get_text(GTK_ENTRY(entry_)));
    }

    void setPlaceholder(const std::wstring& text) override {
        placeholder_ = text;
        if (entry_) {
            gtk_entry_set_placeholder_text(
                GTK_ENTRY(entry_),
                placeholder_.empty() ? nullptr : gtk3Utf8(placeholder_).c_str()
            );
        }
    }

    void setTextStyle(const TextStyle& style) override {
        style_ = style;
        if (!entry_) return;
        gtk3ApplyFont(entry_, fontProvider_, style_);
    }

    void setVisible(bool visible) override {
        if (!entry_) return;
        if (visible) gtk_widget_show(entry_);
        else gtk_widget_hide(entry_);
    }

private:
    GtkWidget* entry_ = nullptr;
    std::wstring text_;
    std::wstring placeholder_;
    TextStyle style_;
    GtkCssProvider* fontProvider_ = nullptr;
};

std::unique_ptr<NativeTextBox> createNativeTextBox() {
    return std::make_unique<Gtk3NativeTextBox>();
}
