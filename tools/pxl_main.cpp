#include "../runtime/pxl_runtime.h"
#include "../language/docs.h"

#include <cctype>
#include <iostream>
#include <string>

namespace {

bool findDocumentation(const std::string& id, std::string& result) {
    const std::string& docs = PixelateDocs::embeddedPxldocs();
    std::size_t sectionStart = docs.find("---");

    while (sectionStart != std::string::npos) {
        const std::size_t metadataEnd = docs.find("\n---", sectionStart + 3);
        std::size_t sectionEnd = docs.find("\n---", metadataEnd + 4);
        if (sectionEnd == std::string::npos) {
            sectionEnd = docs.size();
        }

        const std::string section = docs.substr(sectionStart, sectionEnd - sectionStart);
        const std::string idPrefix = "\nid: ";
        const std::size_t idStart = section.find(idPrefix);

        if (idStart != std::string::npos) {
            const std::size_t valueStart = idStart + idPrefix.size();
            const std::size_t valueEnd = section.find('\n', valueStart);
            const std::string sectionId = section.substr(
                valueStart,
                valueEnd == std::string::npos ? std::string::npos : valueEnd - valueStart);

            std::string normalizedId = sectionId;
            if (!normalizedId.empty() && normalizedId.back() == '\r') {
                normalizedId.pop_back();
            }

            if (normalizedId == id) {
                result = section;
                return true;
            }
        }

        sectionStart = docs.find("---", sectionEnd + 1);
    }

    return false;
}

std::string formatDocumentation(const std::string& section) {
    std::string normalized = section;
    for (char& character : normalized) {
        if (character == '\r') {
            character = ' ';
        }
    }

    const std::size_t firstLineEnd = normalized.find('\n');
    const std::size_t descriptionStart = normalized.find("\n---", firstLineEnd + 1);
    if (firstLineEnd == std::string::npos || descriptionStart == std::string::npos) {
        return section;
    }

    std::string metadata = normalized.substr(
        firstLineEnd + 1,
        descriptionStart - firstLineEnd - 1);
    std::string description = normalized.substr(descriptionStart + 4);

    while (!metadata.empty() && std::isspace(static_cast<unsigned char>(metadata.back()))) {
        metadata.pop_back();
    }
    while (!description.empty() && std::isspace(static_cast<unsigned char>(description.front()))) {
        description.erase(description.begin());
    }
    while (!description.empty() && std::isspace(static_cast<unsigned char>(description.back()))) {
        description.pop_back();
    }

    return metadata + "\nDescription: " + description + '\n';
}

}

int main(int argc, char** argv) {
    if (argc >= 2 && std::string(argv[1]) == "--docs") {
        if (argc == 2) {
            std::cout << PixelateDocs::embeddedPxldocs();
            return 0;
        }

        if (argc == 3) {
            std::string documentation;
            if (findDocumentation(argv[2], documentation)) {
                std::cout << formatDocumentation(documentation);
                return 0;
            }

            std::cerr << "No documentation found for: " << argv[2] << '\n';
            return 1;
        }
    }

    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::cout << "Pixelate 0.1.0";
        return 0;
    }

    if (argc != 2) {
        std::cerr << "ERROR: No argument was given. \nUsage: pixelate <file.pxl>\nOptions:\npixelate --version\npixelate --docs [keyword]\n";
        return 1;
    }

    PxlRuntime runtime;
    std::string error;

    if (!runtime.loadFile(argv[1], error)) {
        std::cerr << "PXL error: " << error << '\n';
        return 1;
    }

    return runtime.run();
}
