#include "../../app.h"
#include "gtk3_draw_context.h"
#include "gtk3_text_utils.h"

#include <gtk/gtk.h>

#include <algorithm>
#include <utility>
#include <cctype>

struct App::Impl {
    AppOptions options;
    App* owner = nullptr;
    GtkWidget* window = nullptr;
    GtkWidget* fixed = nullptr;
    GtkWidget* drawingArea = nullptr;
    MouseShape mouseShape = MouseShape::Arrow;
    bool mouseShapeConfigured = false;
    bool suppressQuitOnDestroy = false;
    MousePosition lastMousePosition{};
    MouseMoveHandler mouseMoveHandler;
    TickHandler tickHandler;
    KeyDownHandler keyDownHandler;
    CloseHandler closeHandler;

    ~Impl();

    bool create();
    int run();
    void setTitle(const std::wstring& title);
    void setBackgroundColor(AppColor color);
    void setMouseShape(MouseShape shape);
    MousePosition mousePosition() const;
    void setOnMouseMove(MouseMoveHandler handler);
    void setOnTick(TickHandler handler);
    void setOnKeyDown(KeyDownHandler handler);
    void setOnClose(CloseHandler handler);
    void close(bool quitMessage);

    static gboolean onDraw(GtkWidget*, cairo_t*, gpointer);
    static gboolean onButtonPress(GtkWidget*, GdkEventButton*, gpointer);
    static gboolean onMotion(GtkWidget*, GdkEventMotion*, gpointer);
    static gboolean onKeyPress(GtkWidget*, GdkEventKey*, gpointer);
    static gboolean onDelete(GtkWidget*, GdkEvent*, gpointer);
    static void onDestroy(GtkWidget*, gpointer);
    static gboolean onTick(gpointer);

    void updateCursor(double x, double y);
};

namespace {

const gchar* cursorName(MouseShape shape) {
    switch (shape) {
        case MouseShape::IBeam: return "text";
        case MouseShape::Wait: return "wait";
        case MouseShape::Cross: return "crosshair";
        case MouseShape::Hand: return "pointer";
        case MouseShape::No: return "not-allowed";
        case MouseShape::SizeWE: return "ew-resize";
        case MouseShape::SizeNS: return "ns-resize";
        case MouseShape::SizeNWSE: return "nwse-resize";
        case MouseShape::SizeNESW: return "nesw-resize";
        case MouseShape::SizeAll: return "move";
        case MouseShape::Help: return "help";
        default: return "default";
    }
}

void setCursor(GtkWidget* widget, const gchar* name) {
    if (!widget || !gtk_widget_get_window(widget)) return;
    GdkCursor* cursor = gdk_cursor_new_from_name(
        gdk_window_get_display(gtk_widget_get_window(widget)),
        name
    );
    gdk_window_set_cursor(gtk_widget_get_window(widget), cursor);
    if (cursor) g_object_unref(cursor);
}

}

App::App(AppOptions options)
    : impl_(std::make_unique<Impl>()) {
    impl_->owner = this;
    impl_->options = std::move(options);
    if (!gtk_init_check(nullptr, nullptr)) {
        impl_->window = nullptr;
    }
}

App::~App() = default;

App::Impl::~Impl() {
    if (window) {
        suppressQuitOnDestroy = true;
        gtk_widget_destroy(window);
        window = nullptr;
    }
}

bool App::create() { return impl_->create(); }
int App::run() { return impl_->run(); }

void App::add(std::unique_ptr<Widget> element) {
    elements_.push_back(std::move(element));
}

void App::requestRedraw() {
    if (impl_->drawingArea) gtk_widget_queue_draw(impl_->drawingArea);
}

void App::setTitle(const std::wstring& title) { impl_->setTitle(title); }
void App::setBackgroundColor(AppColor color) { impl_->setBackgroundColor(color); }
void App::setMouseShape(MouseShape shape) { impl_->setMouseShape(shape); }
MousePosition App::mousePosition() const { return impl_->mousePosition(); }
void App::setOnMouseMove(MouseMoveHandler handler) {
    impl_->setOnMouseMove(std::move(handler));
}
void App::setOnTick(TickHandler handler) { impl_->setOnTick(std::move(handler)); }
void App::setOnKeyDown(KeyDownHandler handler) { impl_->setOnKeyDown(std::move(handler)); }
void App::setOnClose(CloseHandler handler) { impl_->setOnClose(std::move(handler)); }
void App::close(bool quitMessage) { impl_->close(quitMessage); }

void App::setVisible(bool visible) {
    if (!impl_->window) return;
    if (visible) gtk_widget_show_all(impl_->window);
    else gtk_widget_hide(impl_->window);
}

bool App::isCreated() const { return impl_->window != nullptr; }

void App::Impl::setTitle(const std::wstring& title) {
    options.title = title;
    if (window) gtk_window_set_title(GTK_WINDOW(window), gtk3Utf8(title).c_str());
}

void App::Impl::setBackgroundColor(AppColor color) {
    options.backgroundColor = color;
    if (drawingArea) gtk_widget_queue_draw(drawingArea);
}

void App::Impl::setMouseShape(MouseShape shape) {
    mouseShape = shape;
    mouseShapeConfigured = true;
    updateCursor(lastMousePosition.x, lastMousePosition.y);
}

MousePosition App::Impl::mousePosition() const { return lastMousePosition; }

void App::Impl::setOnMouseMove(MouseMoveHandler handler) {
    mouseMoveHandler = std::move(handler);
}

void App::Impl::setOnTick(TickHandler handler) { tickHandler = std::move(handler); }
void App::Impl::setOnKeyDown(KeyDownHandler handler) { keyDownHandler = std::move(handler); }
void App::Impl::setOnClose(CloseHandler handler) { closeHandler = std::move(handler); }

void App::Impl::close(bool quitMessage) {
    if (!window) return;
    suppressQuitOnDestroy = !quitMessage;
    gtk_widget_destroy(window);
}

bool App::Impl::create() {
    if (window) return true;
    if (!gtk_init_check(nullptr, nullptr)) return false;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), gtk3Utf8(options.title).c_str());
    gtk_window_set_default_size(GTK_WINDOW(window), options.width, options.height);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    fixed = gtk_fixed_new();
    drawingArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawingArea, options.width, options.height);
    gtk_fixed_put(GTK_FIXED(fixed), drawingArea, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), fixed);

    gtk_widget_add_events(
        drawingArea,
        GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK
    );
    g_signal_connect(drawingArea, "draw", G_CALLBACK(onDraw), this);
    g_signal_connect(drawingArea, "button-press-event", G_CALLBACK(onButtonPress), this);
    g_signal_connect(drawingArea, "motion-notify-event", G_CALLBACK(onMotion), this);
    g_signal_connect(drawingArea, "key-press-event", G_CALLBACK(onKeyPress), this);
    gtk_widget_set_can_focus(drawingArea, TRUE);
    gtk_widget_grab_focus(drawingArea);
    g_signal_connect(window, "delete-event", G_CALLBACK(onDelete), this);
    g_signal_connect(window, "destroy", G_CALLBACK(onDestroy), this);

    for (auto& element : owner->elements_) {
        element->attachToWindow(static_cast<void*>(fixed));
    }

    gtk_widget_show_all(window);
    g_timeout_add(50, onTick, this);
    return true;
}

int App::Impl::run() {
    gtk_main();
    return 0;
}

gboolean App::Impl::onDraw(GtkWidget*, cairo_t* cairo, gpointer data) {
    auto* app = static_cast<Impl*>(data);
    cairo_set_source_rgb(
        cairo,
        app->options.backgroundColor.red / 255.0,
        app->options.backgroundColor.green / 255.0,
        app->options.backgroundColor.blue / 255.0
    );
    cairo_paint(cairo);

    Gtk3DrawContext context(cairo);
    for (auto& element : app->owner->elements_) {
        if (element->isVisible()) element->draw(context);
    }
    return FALSE;
}

gboolean App::Impl::onButtonPress(
    GtkWidget* widget,
    GdkEventButton* event,
    gpointer data
) {
    if (event->button != 1) return FALSE;
    auto* app = static_cast<Impl*>(data);
    for (auto iterator = app->owner->elements_.rbegin();
         iterator != app->owner->elements_.rend();
         ++iterator) {
        if (!(*iterator)->isVisible()) continue;
        if ((*iterator)->onMouseDown(
                static_cast<int>(event->x),
                static_cast<int>(event->y))) {
            gtk_widget_queue_draw(widget);
            return TRUE;
        }
    }
    return FALSE;
}

gboolean App::Impl::onMotion(
    GtkWidget* widget,
    GdkEventMotion* event,
    gpointer data
) {
    auto* app = static_cast<Impl*>(data);
    app->lastMousePosition = {
        static_cast<int>(event->x),
        static_cast<int>(event->y)
    };
    app->updateCursor(event->x, event->y);
    if (app->mouseMoveHandler) app->mouseMoveHandler(app->lastMousePosition);
    return FALSE;
}

gboolean App::Impl::onKeyPress(
    GtkWidget*, GdkEventKey* event, gpointer data
) {
    auto* app = static_cast<Impl*>(data);
    std::string key;
    switch (event->keyval) {
        case GDK_KEY_space: key = "space"; break;
        case GDK_KEY_Escape: key = "esc"; break;
        case GDK_KEY_Return: key = "enter"; break;
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R: key = "shift"; break;
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R: key = "alt"; break;
        case GDK_KEY_Up: key = "up"; break;
        case GDK_KEY_Down: key = "down"; break;
        case GDK_KEY_Left: key = "left"; break;
        case GDK_KEY_Right: key = "right"; break;
        case GDK_KEY_Tab: key = "tab"; break;
        default: {
            gunichar character = gdk_keyval_to_unicode(event->keyval);
            if (character < 128 && std::isalpha(static_cast<unsigned char>(character))) {
                key = std::string(1, static_cast<char>(character));
            }
            break;
        }
    }
    if (app->keyDownHandler && !key.empty()) app->keyDownHandler(key);
    return FALSE;
}

gboolean App::Impl::onDelete(GtkWidget*, GdkEvent*, gpointer data) {
    auto* app = static_cast<Impl*>(data);
    if (app->closeHandler) app->closeHandler();
    return FALSE;
}

void App::Impl::onDestroy(GtkWidget*, gpointer data) {
    auto* app = static_cast<Impl*>(data);
    app->window = nullptr;
    app->fixed = nullptr;
    app->drawingArea = nullptr;
    if (!app->suppressQuitOnDestroy) gtk_main_quit();
}

gboolean App::Impl::onTick(gpointer data) {
    auto* app = static_cast<Impl*>(data);
    if (!app->window) return G_SOURCE_REMOVE;
    if (app->tickHandler) app->tickHandler();
    return G_SOURCE_CONTINUE;
}

void App::Impl::updateCursor(double x, double y) {
    if (!drawingArea) return;
    if (mouseShapeConfigured) {
        setCursor(drawingArea, cursorName(mouseShape));
        return;
    }

    MouseShape shape = MouseShape::Arrow;
    for (auto iterator = owner->elements_.rbegin();
         iterator != owner->elements_.rend();
         ++iterator) {
        Widget& element = **iterator;
        if (!element.isVisible()) continue;
        if (element.containsPoint(static_cast<int>(x), static_cast<int>(y))) {
            if (element.cursor() == CursorType::Hand) shape = MouseShape::Hand;
            break;
        }
    }
    setCursor(drawingArea, cursorName(shape));
}
