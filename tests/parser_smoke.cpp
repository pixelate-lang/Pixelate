#include "../language/interpreter.h"

#include <filesystem>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: pixelate_parser_smoke <demo-directory>\n";
        return 2;
    }

    Interpreter interpreter;
    int parsed = 0;
    for (const auto& entry : std::filesystem::directory_iterator(argv[1])) {
        if (!entry.is_regular_file() || entry.path().extension() != ".pxl") continue;
        try {
            interpreter.parseFile(entry.path().string());
            ++parsed;
        } catch (const std::exception& exception) {
            std::cerr << entry.path() << ": " << exception.what() << '\n';
            return 1;
        }
    }

    if (parsed == 0) {
        std::cerr << "No .pxl files found in " << argv[1] << '\n';
        return 1;
    }

    try {
        interpreter.parseSource(
            "window { title: \"Legacy\", }"
        );
        std::cerr << "Legacy window blocks should be rejected.\n";
        return 1;
    } catch (const std::exception&) {
        // Expected: programs must declare named window objects.
    }

    try {
        interpreter.parseSource(
            "window main = window {} main.run(); "
            "update label.text = \"legacy\";"
        );
        std::cerr << "The legacy update command should be rejected.\n";
        return 1;
    } catch (const std::exception&) {
        // Expected: property assignments are now committed by window.update().
    }

    try {
        interpreter.parseSource(
            "window main = window {} "
            "label label_object = label { text: \"kept\", } "
            "trigger trigger_object = trigger (true) { } "
            "main.add(label_object); main.remove(label_object); "
            "main.add(trigger_object); main.remove(trigger_object); "
            "main.add(label_object); main.add(trigger_object); main.run();"
        );
    } catch (const std::exception& exception) {
        std::cerr << "Add/remove object references should remain reusable: "
                  << exception.what() << '\n';
        return 1;
    }

    try {
        interpreter.parseSource(
            "println(1 + 2); print(\"ok\");"
        );
    } catch (const std::exception& exception) {
        std::cerr << "Windowless print-only programs should parse: "
                  << exception.what() << '\n';
        return 1;
    }

    try {
        interpreter.parseSource(
            "window declared = window {} println(\"not run\");"
        );
    } catch (const std::exception& exception) {
        std::cerr << "Declared but unrun windows should parse: "
                  << exception.what() << '\n';
        return 1;
    }

    std::cout << "Parsed " << parsed << " demo programs.\n";
    return 0;
}
