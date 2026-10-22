#include "../native_textbox.h"
#include "gtk3_text_utils.h"

#include <gtk/gtk.h>

class Gtk3NativeTextArea final : public NativeTextArea {
public:
    ~Gtk3NativeTextArea() override {
        gtk3ClearFont(view_, viewFontProvider_);
        gtk3ClearFont(placeholderLabel_, placeholderFontProvider_);
        if (frame_) gtk_widget_destroy(frame_);
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
        frame_ = gtk_frame_new(nullptr);
        overlay_ = gtk_overlay_new();
        view_ = gtk_text_view_new();
        placeholderLabel_ = gtk_label_new("");
        buffer_ = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view_));
        gtk_text_buffer_set_text(buffer_, gtk3Utf8(text).c_str(), -1);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view_), GTK_WRAP_WORD_CHAR);
        gtk_container_add(GTK_CONTAINER(frame_), overlay_);
        gtk_container_add(GTK_CONTAINER(overlay_), view_);
        gtk_overlay_add_overlay(
            GTK_OVERLAY(overlay_),
            placeholderLabel_
        );
        gtk_widget_set_halign(placeholderLabel_, GTK_ALIGN_START);
        gtk_widget_set_valign(placeholderLabel_, GTK_ALIGN_START);
        gtk_widget_set_margin_start(placeholderLabel_, 6);
        gtk_widget_set_margin_top(placeholderLabel_, 5);
        gtk_widget_set_sensitive(placeholderLabel_, FALSE);
        gtk_label_set_xalign(GTK_LABEL(placeholderLabel_), 0.0f);

        g_signal_connect(
            buffer_,
            "changed",
            G_CALLBACK(onBufferChanged),
            this
        );
        g_signal_connect(
            view_,
            "focus-in-event",
            G_CALLBACK(onFocusChanged),
            this
        );
        g_signal_connect(
            view_,
            "focus-out-event",
            G_CALLBACK(onFocusChanged),
            this
        );

        gtk_fixed_put(fixed, frame_, x, y);
        setBounds(x, y, width, height);
        setPlaceholder(placeholder_);
        setTextStyle(style_);
        gtk_widget_show_all(frame_);
    }

    void setBounds(int x, int y, int width, int height) override {
        if (!frame_) return;
        gtk_fixed_move(GTK_FIXED(gtk_widget_get_parent(frame_)), frame_, x, y);
        gtk_widget_set_size_request(frame_, width, height);
    }

    void setText(const std::wstring& text) override {
        text_ = text;
        if (buffer_) gtk_text_buffer_set_text(buffer_, gtk3Utf8(text).c_str(), -1);
        updatePlaceholderVisibility();
    }

    std::wstring text() const override {
        if (!buffer_) return text_;
        GtkTextIter start;
        GtkTextIter end;
        gtk_text_buffer_get_start_iter(buffer_, &start);
        gtk_text_buffer_get_end_iter(buffer_, &end);
        gchar* value = gtk_text_buffer_get_text(buffer_, &start, &end, FALSE);
        std::wstring result = gtk3Wide(value);
        g_free(value);
        return result;
    }

    void setPlaceholder(const std::wstring& text) override {
        placeholder_ = text;
        if (placeholderLabel_) {
            gtk_label_set_text(
                GTK_LABEL(placeholderLabel_),
                gtk3Utf8(placeholder_).c_str()
            );
        }
        updatePlaceholderVisibility();
    }

    void setTextStyle(const TextStyle& style) override {
        style_ = style;
        if (!view_) return;
        gtk3ApplyFont(view_, viewFontProvider_, style_);
        gtk3ApplyFont(placeholderLabel_, placeholderFontProvider_, style_);
    }

    void setVisible(bool visible) override {
        if (!view_) return;
        if (visible) gtk_widget_show(view_);
        else gtk_widget_hide(view_);
    }

private:
    static void onBufferChanged(GtkTextBuffer*, gpointer data) {
        static_cast<Gtk3NativeTextArea*>(data)->updatePlaceholderVisibility();
    }

    static gboolean onFocusChanged(GtkWidget*, GdkEventFocus*, gpointer data) {
        static_cast<Gtk3NativeTextArea*>(data)->updatePlaceholderVisibility();
        return FALSE;
    }

    void updatePlaceholderVisibility() {
        if (!placeholderLabel_ || !buffer_ || !view_) return;
        const bool empty = gtk_text_buffer_get_char_count(buffer_) == 0;
        const bool focused = gtk_widget_has_focus(view_);
        gtk_widget_set_visible(
            placeholderLabel_,
            empty && !focused && !placeholder_.empty()
        );
    }

    GtkWidget* frame_ = nullptr;
    GtkWidget* overlay_ = nullptr;
    GtkWidget* view_ = nullptr;
    GtkWidget* placeholderLabel_ = nullptr;
    GtkTextBuffer* buffer_ = nullptr;
    std::wstring text_;
    std::wstring placeholder_;
    TextStyle style_;
    GtkCssProvider* viewFontProvider_ = nullptr;
    GtkCssProvider* placeholderFontProvider_ = nullptr;
};

std::unique_ptr<NativeTextArea> createNativeTextArea() {
    return std::make_unique<Gtk3NativeTextArea>();
}
