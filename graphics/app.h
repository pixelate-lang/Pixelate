#pragma once
#include <vector>
#include "ui/widget.h"
#include "platform/platform_types.h"
#include <memory>
#include <functional>
#include <string>

/**
 * Define your app's structure by settings it's options
 * Possible options:
 * - title: std::wstring
 * - width: int
 * - height: int
 * - iconPath: std:wstring
 */
struct AppOptions {
    std::wstring title = L"Pixelate App";
    /**The width of the window */
    int width = 800;
    /**The height of the window */
    int height = 600;
    /**Path to your app icon*/
    std::wstring iconPath = L"pixelate.ico";
    /**Window background color, with channels from 0 to 255*/
    AppColor backgroundColor{};
};

class App {
public:
    using MouseMoveHandler = std::function<void(MousePosition)>;
    using TickHandler = std::function<void()>;
    using KeyDownHandler = std::function<void(const std::string&)>;
    using CloseHandler = std::function<void()>;

    explicit App(AppOptions options = {});
    ~App();
    /**
     * Creates an app
     */
    bool create();
    int run();
    void add(std::unique_ptr<Widget> element);
    void requestRedraw();
    void setTitle(const std::wstring& title);
    void setBackgroundColor(AppColor color);
    void setMouseShape(MouseShape shape);
    MousePosition mousePosition() const;
    void setOnMouseMove(MouseMoveHandler handler);
    void setOnTick(TickHandler handler);
    void setOnKeyDown(KeyDownHandler handler);
    void setOnClose(CloseHandler handler);
    void close(bool quitMessage = true);
    void setVisible(bool visible);
    bool isCreated() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::vector<std::unique_ptr<Widget>> elements_;
};
