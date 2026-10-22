#pragma once

#include "../../platform/platform_types.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <pango/pango.h>
#include <sstream>
#include <string>

inline std::string gtk3Utf8(const std::wstring& value) {
    if (value.empty()) return {};
    GError* error = nullptr;
    gchar* converted = g_ucs4_to_utf8(
        reinterpret_cast<const gunichar*>(value.c_str()),
        static_cast<glong>(value.size()),
        nullptr,
        nullptr,
        &error
    );
    if (error) {
        g_error_free(error);
        return {};
    }
    std::string result = converted ? converted : "";
    g_free(converted);
    return result;
}

inline std::wstring gtk3Wide(const char* value) {
    if (!value) return {};
    GError* error = nullptr;
    gunichar* converted = g_utf8_to_ucs4(
        value,
        -1,
        nullptr,
        nullptr,
        &error
    );
    if (error) {
        g_error_free(error);
        return {};
    }
    std::wstring result;
    if (converted) {
        for (const gunichar* character = converted; *character; ++character) {
            result.push_back(static_cast<wchar_t>(*character));
        }
    }
    g_free(converted);
    return result;
}

inline const char* gtk3FontFamily(FontFamily family) {
    switch (family) {
        // Prefer common Linux equivalents when the Windows font is not
        // installed. Pango will still use the requested family if the user
        // has installed it locally.
        case FontFamily::Arial: return "Liberation Sans";
        case FontFamily::Calibri: return "Carlito";
        case FontFamily::Consolas: return "DejaVu Sans Mono";
        case FontFamily::CourierNew: return "Liberation Mono";
        case FontFamily::TimesNewRoman: return "Liberation Serif";
        case FontFamily::Tahoma: return "DejaVu Sans";
        case FontFamily::Verdana: return "DejaVu Sans";
        case FontFamily::Default: return "Sans";
    }
    return "Sans";
}

// Pixelate's fontSize values are pixel-oriented, like the Win32 backend.
// Pango's normal size API uses points, so use an absolute device size here.
inline double gtk3FontPixelSize(const TextStyle& style) {
    return static_cast<double>(style.size > 0 ? style.size : 16) * PANGO_SCALE;
}

inline void gtk3ApplyFont(
    GtkWidget* widget,
    GtkCssProvider*& provider,
    const TextStyle& style
) {
    if (!widget) return;
    GtkStyleContext* context = gtk_widget_get_style_context(widget);
    if (provider) {
        gtk_style_context_remove_provider(
            context,
            GTK_STYLE_PROVIDER(provider)
        );
        g_object_unref(provider);
        provider = nullptr;
    }

    provider = gtk_css_provider_new();
    std::ostringstream css;
    css << "* { font-family: \"" << gtk3FontFamily(style.family)
        << "\"; font-size: " << (style.size > 0 ? style.size : 16)
        << "px; }";
    gtk_css_provider_load_from_data(provider, css.str().c_str(), -1, nullptr);
    gtk_style_context_add_provider(
        context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

inline void gtk3ClearFont(GtkWidget* widget, GtkCssProvider*& provider) {
    if (!provider) return;
    if (widget) {
        gtk_style_context_remove_provider(
            gtk_widget_get_style_context(widget),
            GTK_STYLE_PROVIDER(provider)
        );
    }
    g_object_unref(provider);
    provider = nullptr;
}
