#pragma once

#include <string>
#include <unordered_set>

namespace parser_support {

inline bool isFontConstant(const std::string& value) {
    static const std::unordered_set<std::string> fonts = {
        "DEFAULT", "ARIAL", "CALIBRI", "CONSOLAS", "COURIERNEW",
        "TIMESNEWROMAN", "TAHOMA", "VERDANA"
    };
    return fonts.find(value) != fonts.end();
}

}

using parser_support::isFontConstant;
