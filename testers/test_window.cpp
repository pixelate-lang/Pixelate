#include "../graphics/app.h"

int main() {
    AppOptions options;
    options.width = 1200;
    options.height = 800;
    options.title = L"Pixelate Test Window";
    // options.iconPath = L"graphics\\pixelate.ico";

    App app(options);

    if (!app.create()) {
        return 1;
    }

    return app.run();
}
