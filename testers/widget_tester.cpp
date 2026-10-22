#include "../graphics/app.h"
#include "../graphics/ui/button.h"
#include "../graphics/ui/label.h"
#include <memory>
#include <utility>

int main() {
    AppOptions options;
    options.width = 800;
    options.height = 600;
    options.title = L"Pixelate Test widget";

    App app(options);

    auto button = std::make_unique<Button>(L"Click me!");
    auto label = std::make_unique<Label>(L"Hello world");
    button->setPosition(400,300);
    button->setSize(200, 60);
    
    label->setPosition(50, 50);
    label->setSize(200,100);
    Label* labelPointer = label.get();

    button->setOnClick([labelPointer] {
        labelPointer->setText(L"Updated text");
    });


    app.add(std::move(button));
    app.add(std::move(label));

    if (!app.create()) {
        return 1;
    }

    return app.run();
}
