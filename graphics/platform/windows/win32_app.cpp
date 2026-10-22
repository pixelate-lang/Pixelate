#include "../../app.h"
#include <utility>
#include <windows.h>
#include <utility>
#include "win32_draw_context.h"
#include "pixelate_resource.h"
#include <windowsx.h>
#include <cctype>
struct App::Impl {
    AppOptions options;
    App* owner = nullptr;
    HINSTANCE hInstance{};
    HWND hwnd{};
    HBRUSH backgroundBrush{};
    bool suppressQuitOnDestroy = false;
    MouseShape mouseShape = MouseShape::Arrow;
    bool mouseShapeConfigured = false;
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

    static LRESULT CALLBACK WindowProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    LRESULT handleMessage(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );
};

App::App(AppOptions options)
    : impl_(std::make_unique<Impl>()) {
    impl_->owner = this;
    impl_->options = std::move(options);
    impl_->hInstance = GetModuleHandleW(nullptr);
}

App::~App() = default;

App::Impl::~Impl() {
    if (backgroundBrush) {
        DeleteObject(backgroundBrush);
    }
}

bool App::create() {
    return impl_->create();
}

int App::run() {
    return impl_->run();
}

void App::add(std::unique_ptr<Widget> element) {
    elements_.push_back(std::move(element));
}

void App::requestRedraw() {
    if (impl_->hwnd) {
        // WM_PAINT clears and redraws the client area itself. Asking Windows
        // to erase the background first causes a visible erase/draw flash
        // when live triggers update frequently.
        InvalidateRect(impl_->hwnd, nullptr, FALSE);
    }
}

void App::setTitle(const std::wstring& title) {
    impl_->setTitle(title);
}

void App::setBackgroundColor(AppColor color) {
    impl_->setBackgroundColor(color);
}

void App::setMouseShape(MouseShape shape) {
    impl_->setMouseShape(shape);
}

MousePosition App::mousePosition() const {
    return impl_->mousePosition();
}

void App::setOnMouseMove(MouseMoveHandler handler) {
    impl_->setOnMouseMove(std::move(handler));
}

void App::setOnTick(TickHandler handler) {
    impl_->setOnTick(std::move(handler));
}

void App::setOnKeyDown(KeyDownHandler handler) {
    impl_->setOnKeyDown(std::move(handler));
}

void App::setOnClose(CloseHandler handler) {
    impl_->setOnClose(std::move(handler));
}

void App::close(bool quitMessage) {
    impl_->close(quitMessage);
}

void App::setVisible(bool visible) {
    if (impl_->hwnd) {
        ShowWindow(impl_->hwnd, visible ? SW_SHOW : SW_HIDE);
        if (visible) {
            SetForegroundWindow(impl_->hwnd);
        }
    }
}

bool App::isCreated() const {
    return impl_->hwnd != nullptr;
}

void App::Impl::setTitle(const std::wstring& title) {
    options.title = title;

    if (hwnd) {
        SetWindowTextW(hwnd, options.title.c_str());
    }
}

void App::Impl::setBackgroundColor(AppColor color) {
    HBRUSH newBrush = CreateSolidBrush(RGB(
        color.red,
        color.green,
        color.blue
    ));

    if (!newBrush) {
        return;
    }

    HBRUSH oldBrush = backgroundBrush;
    backgroundBrush = newBrush;
    options.backgroundColor = color;

    if (hwnd) {
        SetClassLongPtrW(
            hwnd,
            GCLP_HBRBACKGROUND,
            reinterpret_cast<LONG_PTR>(backgroundBrush)
        );
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    if (oldBrush) {
        DeleteObject(oldBrush);
    }
}

void App::Impl::setMouseShape(MouseShape shape) {
    mouseShape = shape;
    mouseShapeConfigured = true;

    if (hwnd) {
        InvalidateRect(hwnd, nullptr, FALSE);
    }
}

MousePosition App::Impl::mousePosition() const {
    POINT position{};
    GetCursorPos(&position);

    if (hwnd) {
        ScreenToClient(hwnd, &position);
    }

    return {position.x, position.y};
}

void App::Impl::setOnMouseMove(MouseMoveHandler handler) {
    mouseMoveHandler = std::move(handler);
}

void App::Impl::setOnTick(TickHandler handler) {
    tickHandler = std::move(handler);
}

void App::Impl::setOnKeyDown(KeyDownHandler handler) {
    keyDownHandler = std::move(handler);
}

void App::Impl::setOnClose(CloseHandler handler) {
    closeHandler = std::move(handler);
}

void App::Impl::close(bool quitMessage) {
    if (hwnd) {
        suppressQuitOnDestroy = !quitMessage;
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

static HCURSOR loadMouseCursor(MouseShape shape) {
    WORD resource = 32512;

    switch (shape) {
        case MouseShape::Arrow: resource = 32512; break;
        case MouseShape::IBeam: resource = 32513; break;
        case MouseShape::Wait: resource = 32514; break;
        case MouseShape::Cross: resource = 32515; break;
        case MouseShape::UpArrow: resource = 32516; break;
        case MouseShape::Size: resource = 32640; break;
        case MouseShape::Icon: resource = 32641; break;
        case MouseShape::SizeNWSE: resource = 32642; break;
        case MouseShape::SizeNESW: resource = 32643; break;
        case MouseShape::SizeWE: resource = 32644; break;
        case MouseShape::SizeNS: resource = 32645; break;
        case MouseShape::SizeAll: resource = 32646; break;
        case MouseShape::No: resource = 32648; break;
        case MouseShape::Hand: resource = 32649; break;
        case MouseShape::AppStarting: resource = 32650; break;
        case MouseShape::Help: resource = 32651; break;
        case MouseShape::Pin: resource = 32671; break;
        case MouseShape::Person: resource = 32672; break;
    }

    return LoadCursorW(nullptr, MAKEINTRESOURCEW(resource));
}

static std::string keyName(WPARAM key) {
    switch (key) {
        case VK_SPACE: return "space";
        case VK_ESCAPE: return "esc";
        case VK_RETURN: return "enter";
        case VK_SHIFT: return "shift";
        case VK_MENU: return "alt";
        case VK_UP: return "up";
        case VK_DOWN: return "down";
        case VK_LEFT: return "left";
        case VK_RIGHT: return "right";
        case VK_TAB: return "tab";
        default: break;
    }
    if (key >= 'A' && key <= 'Z') {
        const bool shifted = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        const bool capsLocked = (GetKeyState(VK_CAPITAL) & 1) != 0;
        const bool uppercase = shifted != capsLocked;
        return std::string(1, static_cast<char>(uppercase ? key : key + ('a' - 'A')));
    }
    return {};
}

static bool fileExists(const std::wstring& path) {
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
        !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static std::wstring executableDirectory() {
    std::wstring path(32768, L'\0');
    const DWORD length = GetModuleFileNameW(
        nullptr,
        path.data(),
        static_cast<DWORD>(path.size())
    );

    if (length == 0 || length >= path.size()) return {};
    path.resize(length);

    const std::size_t separator = path.find_last_of(L"\\/");
    return separator == std::wstring::npos
        ? std::wstring{}
        : path.substr(0, separator);
}

static std::wstring resolveIconPath(const std::wstring& requestedPath) {
    if (fileExists(requestedPath)) return requestedPath;

    const std::wstring directory = executableDirectory();
    if (directory.empty()) return requestedPath;

    const std::wstring besideExecutable = directory + L"\\" + requestedPath;
    if (fileExists(besideExecutable)) return besideExecutable;

    // Preserve compatibility with the old default path when the icon was
    // moved from window\pixelate.ico to the executable directory.
    if (requestedPath == L"window\\pixelate.ico") {
        const std::wstring fallback = directory + L"\\pixelate.ico";
        if (fileExists(fallback)) return fallback;
    }

    return requestedPath;
}

bool App::Impl::create() {
    WNDCLASSEXW windowClass{};

    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursorW(
        nullptr,
        MAKEINTRESOURCEW(32512)
    );
    const bool useEmbeddedIcon = options.iconPath == L"pixelate.ico";
    const std::wstring iconPath = resolveIconPath(options.iconPath);
    windowClass.hIcon = useEmbeddedIcon
        ? static_cast<HICON>(LoadImageW(
            hInstance,
            MAKEINTRESOURCEW(IDI_PIXELATE_ICON),
            IMAGE_ICON,
            0,
            0,
            LR_DEFAULTSIZE
        ))
        : static_cast<HICON>(LoadImageW(
            nullptr,
            iconPath.c_str(),
            IMAGE_ICON,
            0,
            0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE
        ));
    windowClass.hIconSm = useEmbeddedIcon
        ? static_cast<HICON>(LoadImageW(
            hInstance,
            MAKEINTRESOURCEW(IDI_PIXELATE_ICON),
            IMAGE_ICON,
            16,
            16,
            0
        ))
        : static_cast<HICON>(LoadImageW(
            nullptr,
            iconPath.c_str(),
            IMAGE_ICON,
            16,
            16,
            LR_LOADFROMFILE
        ));
    backgroundBrush = CreateSolidBrush(RGB(
        options.backgroundColor.red,
        options.backgroundColor.green,
        options.backgroundColor.blue
    ));

    if (!backgroundBrush) {
        MessageBoxW(
            nullptr,
            L"Background brush creation failed.",
            L"Error",
            MB_OK | MB_ICONERROR
        );

        return false;
    }

    windowClass.hbrBackground = nullptr;
    windowClass.lpszClassName = L"Pixelate";

    WNDCLASSEXW existingClass{};
    if (!GetClassInfoExW(hInstance, L"Pixelate", &existingClass) &&
        !RegisterClassExW(&windowClass)) {
        MessageBoxW(
            nullptr,
            L"Window registration failed.",
            L"Error",
            MB_OK | MB_ICONERROR
        );

        return false;
    }

    hwnd = CreateWindowExW(
        0,
        L"Pixelate",
        options.title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        options.width,
        options.height,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (!hwnd) {
        MessageBoxW(
            nullptr,
            L"Window creation failed.",
            L"Error",
            MB_OK | MB_ICONERROR
        );

        return false;
    }

    for (auto& element : owner->elements_) {
        element->attachToWindow(static_cast<void*>(hwnd));
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    SetTimer(hwnd, 1, 50, nullptr);

    return true;
}

int App::Impl::run() {
    MSG message{};

    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}

LRESULT CALLBACK App::Impl::WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
) {
    auto* app = reinterpret_cast<Impl*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA)
    );

    if (message == WM_NCCREATE) {
        auto* createStruct =
            reinterpret_cast<CREATESTRUCTW*>(lParam);

        app = static_cast<Impl*>(createStruct->lpCreateParams);

        SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(app)
        );

        app->hwnd = hwnd;
    }

    if (app) {
        return app->handleMessage(
            hwnd,
            message,
            wParam,
            lParam
        );
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT App::Impl::handleMessage(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
) {
    switch (message) {
        case WM_KEYDOWN:
            if (keyDownHandler) {
                const std::string keyNameValue = keyName(wParam);
                if (!keyNameValue.empty()) keyDownHandler(keyNameValue);
            }
            return 0;
        case WM_SETCURSOR: {
            if (mouseShapeConfigured) {
                SetCursor(loadMouseCursor(mouseShape));
                return TRUE;
            }

            POINT cursorPosition{};
            GetCursorPos(&cursorPosition);
            ScreenToClient(hwnd, &cursorPosition);

            for (auto iterator = owner->elements_.rbegin();
                iterator != owner->elements_.rend();
                ++iterator) {
                Widget& element = **iterator;
                if (!element.isVisible()) continue;

                if (element.containsPoint(
                        cursorPosition.x,
                        cursorPosition.y
                    )) {
                    if (element.cursor() == CursorType::Hand) {
                        HCURSOR handCursor = LoadCursorW(
                            nullptr,
                            MAKEINTRESOURCEW(32649)
                        );

                        SetCursor(handCursor);
                        return TRUE;
                    }

                    break;
                }
            }

            HCURSOR defaultCursor = LoadCursorW(
                nullptr,
                MAKEINTRESOURCEW(32512)
            );

            SetCursor(defaultCursor);
            return TRUE;
        }
        case WM_LBUTTONDOWN: {
            int mouseX = GET_X_LPARAM(lParam);
            int mouseY = GET_Y_LPARAM(lParam);

            for (auto iterator = owner->elements_.rbegin();
                iterator != owner->elements_.rend();
                ++iterator) {
                Widget& element = **iterator;
                if (!element.isVisible()) continue;

                if (element.onMouseDown(mouseX, mouseY)) {
                    InvalidateRect(hwnd, nullptr, FALSE);
                    break;
                }
            }

            return 0;
        }
        case WM_ERASEBKGND:
            // WM_PAINT fills the invalidated region before drawing widgets.
            // Prevent the separate erase pass, which is the source of the
            // white flash/flicker during rapid updates.
            return 1;
        case WM_MOUSEMOVE:
            if (mouseMoveHandler) {
                mouseMoveHandler({
                    GET_X_LPARAM(lParam),
                    GET_Y_LPARAM(lParam)
                });
            }
            return 0;
        case WM_TIMER:
            if (wParam == 1 && tickHandler) {
                tickHandler();
            }
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT paint{};
            HDC deviceContext = BeginPaint(hwnd, &paint);

            RECT clientArea{};
            GetClientRect(hwnd, &clientArea);

            // Render to an off-screen bitmap first. This prevents the user
            // from seeing the window halfway through a redraw while live
            // mouse triggers are changing labels.
            HDC bufferContext = CreateCompatibleDC(deviceContext);
            HBITMAP bufferBitmap = CreateCompatibleBitmap(
                deviceContext,
                clientArea.right - clientArea.left,
                clientArea.bottom - clientArea.top
            );

            if (bufferContext && bufferBitmap) {
                HGDIOBJ oldBitmap = SelectObject(bufferContext, bufferBitmap);

                FillRect(bufferContext, &clientArea, backgroundBrush);

                Win32DrawContext context(bufferContext);
                for (auto& element : owner->elements_) {
                    if (!element->isVisible()) continue;
                    element->draw(context);
                }

                BitBlt(
                    deviceContext,
                    paint.rcPaint.left,
                    paint.rcPaint.top,
                    paint.rcPaint.right - paint.rcPaint.left,
                    paint.rcPaint.bottom - paint.rcPaint.top,
                    bufferContext,
                    paint.rcPaint.left,
                    paint.rcPaint.top,
                    SRCCOPY
                );

                SelectObject(bufferContext, oldBitmap);
            } else {
                // Fall back to direct painting if Windows cannot allocate the
                // temporary drawing surface.
                FillRect(deviceContext, &clientArea, backgroundBrush);

                Win32DrawContext context(deviceContext);
                for (auto& element : owner->elements_) {
                    if (!element->isVisible()) continue;
                    element->draw(context);
                }
            }

            if (bufferBitmap) {
                DeleteObject(bufferBitmap);
            }
            if (bufferContext) {
                DeleteDC(bufferContext);
            }

            EndPaint(hwnd, &paint);
            return 0;
        }
        case WM_CLOSE:
            if (closeHandler) closeHandler();
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            if (!suppressQuitOnDestroy) {
                PostQuitMessage(0);
            }
            return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
