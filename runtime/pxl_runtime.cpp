#include "pxl_runtime.h"

#include "../language/interpreter.h"
#include "../graphics/app.h"
#include "../graphics/ui/button.h"
#include "../graphics/ui/label.h"
#include "../graphics/ui/paragraph.h"
#include "../graphics/ui/textbox.h"
#include "../graphics/ui/textarea.h"
#include "../graphics/ui/shape.h"

#include <exception>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cmath>
#include <limits>
#include <iomanip>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <unordered_map>

namespace {

std::wstring widen(const std::string& value) {
    return std::wstring(value.begin(), value.end());
}

FontFamily fontFamilyFromName(const std::string& name) {
    static const std::unordered_map<std::string, FontFamily> fonts = {
        {"DEFAULT", FontFamily::Default},
        {"ARIAL", FontFamily::Arial},
        {"CALIBRI", FontFamily::Calibri},
        {"CONSOLAS", FontFamily::Consolas},
        {"COURIERNEW", FontFamily::CourierNew},
        {"TIMESNEWROMAN", FontFamily::TimesNewRoman},
        {"TAHOMA", FontFamily::Tahoma},
        {"VERDANA", FontFamily::Verdana}
    };
    auto font = fonts.find(name);
    return font == fonts.end() ? FontFamily::Default : font->second;
}

std::string fontFamilyName(FontFamily family) {
    switch (family) {
        case FontFamily::Arial: return "ARIAL";
        case FontFamily::Calibri: return "CALIBRI";
        case FontFamily::Consolas: return "CONSOLAS";
        case FontFamily::CourierNew: return "COURIERNEW";
        case FontFamily::TimesNewRoman: return "TIMESNEWROMAN";
        case FontFamily::Tahoma: return "TAHOMA";
        case FontFamily::Verdana: return "VERDANA";
        case FontFamily::Default: return "DEFAULT";
    }
    return "DEFAULT";
}

void configureTextStyle(Widget& widget, const ElementNode& node) {
    widget.setTextStyle({fontFamilyFromName(node.font), node.fontSize});
}

std::string valueToString(const VariableValue& value) {
    return std::visit([](const auto& item) {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::string>) {
            return item;
        } else if constexpr (std::is_same_v<Type, bool>) {
            return item ? std::string("true") : std::string("false");
        } else if constexpr (std::is_same_v<Type, char>) {
            return std::string(1, item);
        } else if constexpr (std::is_floating_point_v<Type>) {
            std::ostringstream output;
            output << std::setprecision(std::numeric_limits<Type>::max_digits10)
                   << item;
            std::string result = output.str();
            if (result.find('.') == std::string::npos &&
                result.find('e') == std::string::npos &&
                result.find('E') == std::string::npos) {
                result += ".0";
            }
            return result;
        } else {
            return std::to_string(item);
        }
    }, value);
}

bool isFloating(const VariableValue& value) {
    return std::holds_alternative<float>(value) ||
        std::holds_alternative<double>(value);
}

long double numericValue(const VariableValue& value) {
    return std::visit([](const auto& item) -> long double {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, bool>) return item ? 1 : 0;
        else if constexpr (std::is_same_v<Type, char>) {
            return static_cast<unsigned char>(item);
        } else if constexpr (std::is_arithmetic_v<Type>) {
            return static_cast<long double>(item);
        } else {
            throw std::runtime_error("Expected a numeric value.");
        }
    }, value);
}

}

VariableValue PxlRuntime::evaluateExpression(
    const ValueExpression& expression
) const {
    if (expression.arithmetic) {
        const VariableValue left = evaluateExpression(
            expression.arithmetic->left
        );
        const VariableValue right = evaluateExpression(
            expression.arithmetic->right
        );

        if (std::holds_alternative<std::string>(left) ||
            std::holds_alternative<std::string>(right) ||
            (std::holds_alternative<char>(left) &&
             std::holds_alternative<char>(right) &&
             expression.arithmetic->operation == '+')) {
            if (expression.arithmetic->operation != '+') {
                throw std::runtime_error("Only '+' is supported for strings.");
            }
            return valueToString(left) + valueToString(right);
        }

        const long double leftNumber = numericValue(left);
        const long double rightNumber = numericValue(right);
        const char operation = expression.arithmetic->operation;
        if ((operation == '/' || operation == '%') && rightNumber == 0) {
            throw std::runtime_error("Division by zero.");
        }

        long double result = 0;
        if (operation == '+') result = leftNumber + rightNumber;
        else if (operation == '-') result = leftNumber - rightNumber;
        else if (operation == '*') result = leftNumber * rightNumber;
        else if (operation == '/') result = leftNumber / rightNumber;
        else result = std::fmod(leftNumber, rightNumber);

        if (isFloating(left) || isFloating(right)) {
            const bool fitsFloat = std::isfinite(static_cast<double>(result)) &&
                std::abs(result) <= std::numeric_limits<float>::max();
            return fitsFloat && !std::holds_alternative<double>(left) &&
                    !std::holds_alternative<double>(right)
                ? VariableValue{static_cast<float>(result)}
                : VariableValue{static_cast<double>(result)};
        }

        if (result >= std::numeric_limits<int>::min() &&
            result <= std::numeric_limits<int>::max()) {
            return VariableValue{static_cast<int>(result)};
        }
        return VariableValue{static_cast<long long>(result)};
    }

    if (expression.literal) {
        return *expression.literal;
    }

    if (expression.variable == "mouse.x" || expression.variable == "mouse.y") {
        if (!app_) {
            throw std::runtime_error("Mouse position is unavailable.");
        }

        const MousePosition position = app_->mousePosition();
        return expression.variable == "mouse.x" ? position.x : position.y;
    }

    const std::size_t dot = expression.variable.find('.');
    if (dot != std::string::npos &&
        expression.variable.substr(dot + 1) == "text") {
        const std::string id = expression.variable.substr(0, dot);
        auto element = elements_.find(id);

        if (element != elements_.end()) {
            if (auto* textBox = dynamic_cast<TextBox*>(element->second)) {
                const std::wstring value = textBox->text();
                return std::string(value.begin(), value.end());
            }
            if (auto* textArea = dynamic_cast<TextArea*>(element->second)) {
                const std::wstring value = textArea->text();
                return std::string(value.begin(), value.end());
            }
        }
    }

    if (dot != std::string::npos &&
        (expression.variable.substr(dot + 1) == "font" ||
         expression.variable.substr(dot + 1) == "fontSize" ||
         expression.variable.substr(dot + 1) == "angle" ||
         expression.variable.substr(dot + 1) == "radius" ||
         expression.variable.substr(dot + 1) == "x" ||
         expression.variable.substr(dot + 1) == "y" ||
         expression.variable.substr(dot + 1) == "width" ||
         expression.variable.substr(dot + 1) == "height")) {
        const std::string id = expression.variable.substr(0, dot);
        auto element = elements_.find(id);
        if (element != elements_.end()) {
            const TextStyle style = element->second->textStyle();
            if (expression.variable.substr(dot + 1) == "font") {
                return fontFamilyName(style.family);
            }
            if (expression.variable.substr(dot + 1) == "fontSize") {
                return style.size;
            }

            if (auto* shape = dynamic_cast<ShapeWidget*>(element->second)) {
                if (expression.variable.substr(dot + 1) == "angle") {
                    return shape->angle();
                }
                if (expression.variable.substr(dot + 1) == "radius") {
                    return shape->radius();
                }
            }

            const std::string property = expression.variable.substr(dot + 1);
            if (property == "x") return element->second->x();
            if (property == "y") return element->second->y();
            if (property == "width") return element->second->width();
            return element->second->height();
        }
    }

    auto variable = variables_.find(expression.variable);
    if (variable == variables_.end()) {
        throw std::runtime_error("Unknown variable: " + expression.variable);
    }

    return variable->second;
}

bool PxlRuntime::evaluateCondition(const Condition& condition) const {
    if (condition.logical != Condition::LogicalOperator::None) {
        const bool left = evaluateCondition(*condition.leftCondition);
        if (condition.logical == Condition::LogicalOperator::And) {
            return left && evaluateCondition(*condition.rightCondition);
        }
        return left || evaluateCondition(*condition.rightCondition);
    }

    const VariableValue left = evaluateExpression(condition.left);

    if (!condition.hasComparison) {
        bool truthy = std::visit([](const auto& value) {
            using Type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<Type, bool>) {
                return value;
            } else if constexpr (std::is_same_v<Type, std::string>) {
                return !value.empty();
            } else {
                return value != 0;
            }
        }, left);

        return condition.negated ? !truthy : truthy;
    }

    const VariableValue right = evaluateExpression(condition.right);

    auto numeric = [](const VariableValue& value, double& result) {
        return std::visit([&](const auto& item) {
            using Type = std::decay_t<decltype(item)>;
            if constexpr (std::is_arithmetic_v<Type>) {
                result = static_cast<double>(item);
                return true;
            } else {
                return false;
            }
        }, value);
    };

    double leftNumber = 0.0;
    double rightNumber = 0.0;
    const bool numericValues = numeric(left, leftNumber) && numeric(right, rightNumber);

    if (condition.operation == ComparisonOperator::Equal) {
        return numericValues ? leftNumber == rightNumber : left == right;
    }

    if (condition.operation == ComparisonOperator::NotEqual) {
        return numericValues ? leftNumber != rightNumber : left != right;
    }

    if (!numericValues) {
        throw std::runtime_error("Relational comparisons require numeric values.");
    }

    switch (condition.operation) {
        case ComparisonOperator::Less: return leftNumber < rightNumber;
        case ComparisonOperator::LessEqual: return leftNumber <= rightNumber;
        case ComparisonOperator::Greater: return leftNumber > rightNumber;
        case ComparisonOperator::GreaterEqual: return leftNumber >= rightNumber;
        default: return false;
    }
}

void PxlRuntime::applyUpdateAction(const UpdateAction& action) {
    if (action.targetId == "mouse" && action.property == "shape") {
        const auto* shape = std::get_if<std::string>(&action.value);
        if (!shape || !app_) return;

        static const std::unordered_map<std::string, MouseShape> shapes = {
            {"ARROW", MouseShape::Arrow},
            {"IBEAM", MouseShape::IBeam},
            {"WAIT", MouseShape::Wait},
            {"CROSS", MouseShape::Cross},
            {"UPARROW", MouseShape::UpArrow},
            {"SIZE", MouseShape::Size},
            {"ICON", MouseShape::Icon},
            {"SIZENWSE", MouseShape::SizeNWSE},
            {"SIZENESW", MouseShape::SizeNESW},
            {"SIZEWE", MouseShape::SizeWE},
            {"SIZENS", MouseShape::SizeNS},
            {"SIZEALL", MouseShape::SizeAll},
            {"NO", MouseShape::No},
            {"HAND", MouseShape::Hand},
            {"APPSTARTING", MouseShape::AppStarting},
            {"HELP", MouseShape::Help},
            {"PIN", MouseShape::Pin},
            {"PERSON", MouseShape::Person}
        };

        auto cursor = shapes.find(*shape);
        if (cursor != shapes.end()) {
            app_->setMouseShape(cursor->second);
        }
        return;
    }

    auto targetWindow = std::find_if(
        views_.begin(),
        views_.end(),
        [&](const auto& view) { return view->id == action.targetId; }
    );
    if (action.targetId == "window" || targetWindow != views_.end()) {
        App* targetApp = action.targetId == "window"
            ? app_
            : targetWindow->get()->app.get();
        if (!targetApp) return;

        if (action.property == "title") {
            if (const auto* title = std::get_if<std::string>(&action.value)) {
                targetApp->setTitle(widen(*title));
            } else if (const auto* formatted =
                       std::get_if<UpdateAction::TextTemplate>(&action.value)) {
                targetApp->setTitle(widen(renderText(*formatted)));
            }
        } else if (action.property == "background") {
            if (const auto* color = std::get_if<ColorNode>(&action.value)) {
                targetApp->setBackgroundColor({
                    color->red,
                    color->green,
                    color->blue
                });
            }
        }
        return;
    }

    auto element = elements_.find(action.targetId);
    if (element == elements_.end()) return;

    Widget* widget = element->second;

    auto numericValue = [this, &action]() -> std::optional<int> {
        if (const auto* number = std::get_if<int>(&action.value)) {
            return *number;
        }
        if (const auto* expression =
                std::get_if<ValueExpression>(&action.value)) {
            const VariableValue value = evaluateExpression(*expression);
            if (const auto* integer = std::get_if<int>(&value)) {
                return *integer;
            }
            if (const auto* decimal = std::get_if<double>(&value)) {
                return static_cast<int>(*decimal);
            }
        }
        return std::nullopt;
    };

    if (action.property == "x" || action.property == "y" ||
        action.property == "width" || action.property == "height") {
        const auto number = numericValue();
        if (!number) return;

        int x = widget->x();
        int y = widget->y();
        int width = widget->width();
        int height = widget->height();

        if (action.property == "x") x = *number;
        if (action.property == "y") y = *number;
        if (action.property == "width") width = *number;
        if (action.property == "height") height = *number;

        if (width > 0 && height > 0) {
            widget->setPosition(x, y);
            widget->setSize(width, height);
        }
        return;
    }

    if (auto* shape = dynamic_cast<ShapeWidget*>(widget)) {
        if (action.property == "angle") {
            const auto number = numericValue();
            if (number) shape->setAngle(*number);
            return;
        }
        if (action.property == "radius") {
            const auto number = numericValue();
            if (number && *number > 0) shape->setRadius(*number);
            return;
        }
    }

    if (action.property == "font" || action.property == "fontSize") {
        TextStyle style = widget->textStyle();
        if (action.property == "font") {
            const auto* font = std::get_if<std::string>(&action.value);
            if (!font) return;
            style.family = fontFamilyFromName(*font);
        } else {
            const auto size = numericValue();
            if (!size || *size <= 0) return;
            style.size = *size;
        }
        widget->setTextStyle(style);
        return;
    }

    if (action.property != "text") return;

    std::string text;
    if (const auto* plainText = std::get_if<std::string>(&action.value)) {
        text = *plainText;
    } else if (const auto* formatted =
               std::get_if<UpdateAction::TextTemplate>(&action.value)) {
        text = renderText(*formatted);
    } else {
        return;
    }

    if (auto* label = dynamic_cast<Label*>(element->second)) {
        label->setText(widen(text));
    } else if (auto* paragraph = dynamic_cast<Paragraph*>(element->second)) {
        paragraph->setText(widen(text));
    } else if (auto* button = dynamic_cast<Button*>(element->second)) {
        button->setText(widen(text));
    } else if (auto* textBox = dynamic_cast<TextBox*>(element->second)) {
        textBox->setText(widen(text));
    } else if (auto* textArea = dynamic_cast<TextArea*>(element->second)) {
        textArea->setText(widen(text));
    }
}

std::string PxlRuntime::renderText(
    const UpdateAction::TextTemplate& text
) const {
    std::string result;

    for (std::size_t index = 0; index < text.text.size();) {
        if (text.text[index] != '{') {
            result.push_back(text.text[index++]);
            continue;
        }

        const std::size_t end = text.text.find('}', index + 1);
        if (end == std::string::npos) {
            throw std::runtime_error("Unclosed formatted string expression.");
        }

        ValueExpression expression;
        expression.variable = text.text.substr(index + 1, end - index - 1);
        VariableValue value = evaluateExpression(expression);

        std::visit([&result](const auto& item) {
            using Type = std::decay_t<decltype(item)>;
            if constexpr (std::is_same_v<Type, std::string>) {
                result += item;
            } else if constexpr (std::is_same_v<Type, bool>) {
                result += item ? "true" : "false";
            } else if constexpr (std::is_same_v<Type, char>) {
                result.push_back(item);
            } else {
                result += std::to_string(item);
            }
        }, value);

        index = end + 1;
    }

    return result;
}

void PxlRuntime::fireMouseMoveTriggers() {
    if (!activeView_) return;
    for (const auto& trigger : activeView_->triggers) {
        if (trigger->attached && trigger->mouseMove) {
            executeStatements(trigger->body);
        }
    }
}

void PxlRuntime::fireKeyTriggers(const std::string& key) {
    if (!activeView_) return;
    for (const auto& trigger : activeView_->triggers) {
        if (!trigger->attached || !trigger->key) continue;
        const std::string& expected = *trigger->key;
        bool matches = expected.size() == key.size();
        for (std::size_t index = 0; matches && index < key.size(); ++index) {
            const auto lhs = static_cast<unsigned char>(expected[index]);
            const auto rhs = static_cast<unsigned char>(key[index]);
            matches = trigger->keyCaseInsensitive
                ? std::tolower(lhs) == std::tolower(rhs)
                : lhs == rhs;
        }
        if (matches) executeStatements(trigger->body);
    }
}

void PxlRuntime::fireCloseTriggers() {
    if (!activeView_) return;
    for (const auto& trigger : activeView_->triggers) {
        if (trigger->attached && trigger->closeTrigger) {
            executeStatements(trigger->body);
        }
    }
}

void PxlRuntime::checkTriggers() {
    if (!activeView_) return;
    for (const auto& trigger : activeView_->triggers) {
        if (!trigger->attached || trigger->mouseMove || trigger->closeTrigger || trigger->key ||
            !trigger->condition) continue;

        const bool active = evaluateCondition(*trigger->condition);
        if (active) {
            executeStatements(trigger->body);
        }
    }
}

void PxlRuntime::incrementVariable(const std::string& name, bool decrement) {
    auto variable = variables_.find(name);
    if (variable == variables_.end()) {
        throw std::runtime_error("Unknown variable: " + name);
    }

    std::visit([decrement](auto& value) {
        using Type = std::decay_t<decltype(value)>;
        if constexpr (std::is_arithmetic_v<Type> && !std::is_same_v<Type, bool>) {
            if (decrement) --value;
            else ++value;
        } else {
            throw std::runtime_error("Only numeric variables can be incremented.");
        }
    }, variable->second);
}

void PxlRuntime::incrementWidgetProperty(
    const std::string& target,
    const std::string& property,
    bool decrement
) {
    auto element = elements_.find(target);
    if (element == elements_.end()) {
        throw std::runtime_error("Unknown widget: " + target);
    }

    if (auto* shape = dynamic_cast<ShapeWidget*>(element->second)) {
        if (property == "x" || property == "y") {
            int x = shape->x();
            int y = shape->y();
            const int amount = decrement ? -1 : 1;
            if (property == "x") x += amount;
            else y += amount;
            shape->setPosition(x, y);
            return;
        }
        if (property == "radius") {
            const int radius = shape->radius() + (decrement ? -1 : 1);
            if (radius > 0) shape->setRadius(radius);
            return;
        }
        if (property == "angle") {
            shape->setAngle(shape->angle() + (decrement ? -1 : 1));
            return;
        }
    }

    if (property == "x" || property == "y") {
        int x = element->second->x();
        int y = element->second->y();
        const int amount = decrement ? -1 : 1;
        if (property == "x") x += amount;
        else y += amount;
        element->second->setPosition(x, y);
        return;
    }

    if (property != "fontSize") {
        throw std::runtime_error("Only fontSize, radius, and angle can be incremented on widgets.");
    }

    TextStyle style = element->second->textStyle();
    if (decrement) --style.size;
    else ++style.size;
    element->second->setTextStyle(style);
}

void PxlRuntime::executeStatement(const Statement& statement) {
    currentLine_ = statement.line;
    if (const auto* declaration = std::get_if<VariableDeclaration>(&statement.value)) {
        variables_[declaration->name] = declaration->value;
        return;
    }

    if (const auto* print = std::get_if<PrintStatement>(&statement.value)) {
        std::string output;
        if (print->formatted) {
            for (const FormattedPart& part : print->parts) {
                if (const auto* text = std::get_if<std::string>(&part.value)) {
                    output += *text;
                } else {
                    output += valueToString(
                        evaluateExpression(std::get<ValueExpression>(part.value))
                    );
                }
            }
        } else if (!print->parts.empty() || print->expression.literal ||
                   !print->expression.variable.empty() ||
                   print->expression.arithmetic) {
            output = valueToString(evaluateExpression(print->expression));
        }
        std::cout << output;
        if (print->newline) std::cout << '\n';
        std::cout.flush();
        return;
    }

    if (const auto* assignment = std::get_if<VariableAssignment>(&statement.value)) {
        if (variables_.find(assignment->name) == variables_.end()) {
            throw std::runtime_error("Unknown variable: " + assignment->name);
        }
        variables_[assignment->name] = evaluateExpression(assignment->value);
        return;
    }

    if (const auto* update = std::get_if<UpdateAction>(&statement.value)) {
        applyUpdateAction(*update);
        return;
    }

    if (const auto* increment = std::get_if<IncrementStatement>(&statement.value)) {
        if (increment->property.empty()) {
            incrementVariable(increment->target, increment->decrement);
        } else {
            incrementWidgetProperty(
                increment->target,
                increment->property,
                increment->decrement
            );
        }
        return;
    }

    if (const auto* switchStatement = std::get_if<SwitchViewStatement>(
            &statement.value)) {
        switchView(switchStatement->windowId, true);
        return;
    }

    if (const auto* addStatement = std::get_if<AddViewStatement>(
            &statement.value)) {
        switchView(addStatement->windowId, false);
        return;
    }

    if (const auto* updateWindow = std::get_if<WindowUpdateStatement>(
            &statement.value)) {
        auto view = std::find_if(
            views_.begin(),
            views_.end(),
            [&](const auto& candidate) {
                return candidate->id == updateWindow->windowId;
            }
        );
        if (view == views_.end()) {
            throw std::runtime_error(
                "Unknown window object: " + updateWindow->windowId
            );
        }
        view->get()->app->requestRedraw();
        return;
    }

    if (const auto* object = std::get_if<WindowObjectStatement>(
            &statement.value)) {
        auto view = std::find_if(
            views_.begin(),
            views_.end(),
            [&](const auto& candidate) {
                return candidate->id == object->windowId;
            }
        );
        if (view == views_.end()) {
            throw std::runtime_error("Unknown window object: " + object->windowId);
        }

        auto element = view->get()->elements.find(object->objectId);
        if (element != view->get()->elements.end()) {
            element->second->setVisible(object->add);
            if (view->get() == activeView_) {
                app_ = view->get()->app.get();
                app_->requestRedraw();
            }
            return;
        }

        auto trigger = std::find_if(
            view->get()->triggers.begin(),
            view->get()->triggers.end(),
            [&](const auto& candidate) {
                return candidate->id == object->objectId;
            }
        );
        if (trigger != view->get()->triggers.end()) {
            (*trigger)->attached = object->add;
            return;
        }

        throw std::runtime_error(
            "Object is not attached to window: " + object->objectId
        );
    }

    if (const auto* ifStatement = std::get_if<std::shared_ptr<IfStatement>>(
            &statement.value)) {
        if (evaluateCondition((*ifStatement)->condition)) {
            executeStatements((*ifStatement)->body);
        } else {
            executeStatements((*ifStatement)->elseBody);
        }
        return;
    }

    if (const auto* whileStatement = std::get_if<std::shared_ptr<WhileStatement>>(
            &statement.value)) {
        std::size_t iterations = 0;
        while (evaluateCondition((*whileStatement)->condition)) {
            if (++iterations > 100000) {
                throw std::runtime_error("While loop exceeded 100000 iterations.");
            }
            executeStatements((*whileStatement)->body);
        }
        return;
    }

    const auto& forStatement = std::get<std::shared_ptr<ForStatement>>(statement.value);
    variables_[forStatement->initializer.name] = forStatement->initializer.value;

    std::size_t iterations = 0;
    while (evaluateCondition(forStatement->condition)) {
        if (++iterations > 100000) {
            throw std::runtime_error("For loop exceeded 100000 iterations.");
        }
        executeStatements(forStatement->body);
        incrementVariable(forStatement->incrementVariable);
    }
}

void PxlRuntime::reportRuntimeError(const std::exception& exception) const {
    if (currentLine_ > 0) {
        std::cerr << "PXL runtime error on line " << currentLine_ << ": "
                  << exception.what() << '\n';
    } else {
        std::cerr << "PXL runtime error: " << exception.what() << '\n';
    }
}

void PxlRuntime::executeStatements(
    const std::vector<std::shared_ptr<Statement>>& statements
) {
    for (const auto& statement : statements) {
        executeStatement(*statement);
    }
}

PxlRuntime::PxlRuntime() = default;
PxlRuntime::~PxlRuntime() = default;

void PxlRuntime::configureView(
    RuntimeView& view,
    const WindowNode& window
) {
    AppOptions options;
    options.title = widen(window.title);
    options.width = window.width;
    options.height = window.height;
    options.iconPath = widen(window.iconPath);
    options.backgroundColor = {
        window.background.red,
        window.background.green,
        window.background.blue
    };

    view.id = window.id;
    view.app = std::make_unique<App>(options);
    view.elements.clear();
    view.triggers = window.triggers;

    activeView_ = &view;
    app_ = view.app.get();
    elements_.clear();

    RuntimeView* viewPointer = &view;
    view.app->setOnClose([this, viewPointer] {
        try {
            activeView_ = viewPointer;
            app_ = viewPointer->app.get();
            elements_ = viewPointer->elements;
            fireCloseTriggers();
            viewPointer->elements = elements_;
        } catch (const std::exception& exception) {
            reportRuntimeError(exception);
        }
    });

    for (const ElementNode& node : window.elements) {
        std::string elementText;
        if (const auto* plainText = std::get_if<std::string>(&node.text)) {
            elementText = *plainText;
        } else {
            elementText = renderText(
                std::get<UpdateAction::TextTemplate>(node.text)
            );
        }

        if (node.type == ElementType::Label) {
            auto label = std::make_unique<Label>(widen(elementText));
            label->setPosition(node.x, node.y);
            label->setSize(node.width, node.height);
            configureTextStyle(*label, node);
            elements_[node.id] = label.get();
            view.app->add(std::move(label));
            continue;
        }

        if (node.type == ElementType::Paragraph) {
            auto paragraph = std::make_unique<Paragraph>(widen(elementText));
            paragraph->setPosition(node.x, node.y);
            paragraph->setSize(node.width, node.height);
            configureTextStyle(*paragraph, node);
            elements_[node.id] = paragraph.get();
            view.app->add(std::move(paragraph));
            continue;
        }

        if (node.type == ElementType::TextBox) {
            auto textBox = std::make_unique<TextBox>(widen(elementText));
            textBox->setPlaceholder(widen(node.placeholder));
            textBox->setPosition(node.x, node.y);
            textBox->setSize(node.width, node.height);
            configureTextStyle(*textBox, node);
            elements_[node.id] = textBox.get();
            view.app->add(std::move(textBox));
            continue;
        }

        if (node.type == ElementType::TextArea) {
            auto textArea = std::make_unique<TextArea>(widen(elementText));
            textArea->setPlaceholder(widen(node.placeholder));
            textArea->setPosition(node.x, node.y);
            textArea->setSize(node.width, node.height);
            configureTextStyle(*textArea, node);
            elements_[node.id] = textArea.get();
            view.app->add(std::move(textArea));
            continue;
        }

        if (node.type == ElementType::Rect || node.type == ElementType::Circle ||
            node.type == ElementType::Triangle) {
            const ShapeType shapeType = node.type == ElementType::Rect
                ? ShapeType::Rect
                : node.type == ElementType::Circle
                    ? ShapeType::Circle
                    : ShapeType::Triangle;
            auto shape = std::make_unique<ShapeWidget>(shapeType);
            shape->setSize(node.width, node.height);
            shape->setAngle(node.angle);
            shape->setRadius(node.radius);
            if (node.type == ElementType::Triangle) {
                PointNode p1 = node.p1;
                PointNode p2 = node.p2;
                PointNode p3 = node.p3;
                bool usePosition = false;
                if (!node.p1Specified) {
                    // Equilateral triangle with side 10, centroid at (0, 0),
                    // and its apex pointing upward.
                    p1 = {0, -6};
                    p2 = {-5, 3};
                    p3 = {5, 3};
                    usePosition = true;
                } else if (node.xSpecified) {
                    const double centerX = (p1.x + p2.x + p3.x) / 3.0;
                    const double centerY = (p1.y + p2.y + p3.y) / 3.0;
                    const int offsetX = node.x - static_cast<int>(centerX);
                    const int offsetY = node.y - static_cast<int>(centerY);
                    p1.x += offsetX; p2.x += offsetX; p3.x += offsetX;
                    p1.y += offsetY; p2.y += offsetY; p3.y += offsetY;
                }
                shape->setPoints(p1, p2, p3);
                shape->setPosition(usePosition ? node.x : 0, usePosition ? node.y : 0);
            } else {
                shape->setPosition(node.x, node.y);
                shape->setPoints(node.p1, node.p2, node.p3);
            }
            elements_[node.id] = shape.get();
            view.app->add(std::move(shape));
            continue;
        }

        auto button = std::make_unique<Button>(widen(elementText));
        button->setPosition(node.x, node.y);
        button->setSize(node.width, node.height);
        configureTextStyle(*button, node);
        elements_[node.id] = button.get();

        if (node.onClick) {
            std::vector<std::shared_ptr<Statement>> actions = *node.onClick;
            RuntimeView* viewPointer = &view;

            button->setOnClick([this, actions, viewPointer] {
                try {
                    activeView_ = viewPointer;
                    app_ = viewPointer->app.get();
                    elements_ = viewPointer->elements;
                    executeStatements(actions);
                    viewPointer->elements = elements_;
                } catch (const std::exception& exception) {
                    reportRuntimeError(exception);
                }
            });
        }

        view.app->add(std::move(button));
    }

    view.elements = elements_;
}

void PxlRuntime::switchView(const std::string& windowId, bool destroyWindow) {
    auto target = std::find_if(
        views_.begin(),
        views_.end(),
        [&](const auto& view) { return view->id == windowId; }
    );

    if (target == views_.end()) {
        throw std::runtime_error("Unknown window id: " + windowId);
    }

    RuntimeView* oldView = activeView_;
    RuntimeView* newView = target->get();

    if (!newView->app->isCreated() && !newView->app->create()) {
        throw std::runtime_error(
            "Could not recreate window: " + windowId
        );
    }

    if (oldView && oldView != newView) {
        oldView->elements = elements_;
    }

    // Activate the destination before closing the source. This is important
    // when switch_view is called from a click handler belonging to the source
    // window: the destination is already visible before that window is torn
    // down.
    activeView_ = newView;
    app_ = newView->app.get();
    elements_ = newView->elements;
    app_->setVisible(true);
    app_->requestRedraw();

    if (oldView && oldView != newView && destroyWindow) {
        oldView->app->close(false);
    }
}

bool PxlRuntime::loadFile(
    const std::string& path,
    std::string& error
) {
    try {
        Interpreter interpreter;
        ProgramNode program = interpreter.parseFile(path);

        views_.clear();
        activeView_ = nullptr;
        app_ = nullptr;
        elements_.clear();
        variables_.clear();
        statements_.clear();

        for (const VariableDeclaration& variable : program.variables) {
            if (variables_.find(variable.name) != variables_.end()) {
                throw std::runtime_error(
                    "Variable already declared: " + variable.name
                );
            }

            variables_[variable.name] = variable.value;
        }

        for (const WindowNode& window : program.windows) {
            auto view = std::make_unique<RuntimeView>();
            configureView(*view, window);
            views_.push_back(std::move(view));
        }

        if (!views_.empty() && !program.entryWindow.empty()) {
            auto entryView = std::find_if(
                views_.begin(),
                views_.end(),
                [&](const auto& view) {
                    return view->id == program.entryWindow;
                }
            );

            if (entryView == views_.end()) {
                throw std::runtime_error(
                    "Could not find entry window: " + program.entryWindow
                );
            }

            activeView_ = entryView->get();
            app_ = activeView_->app.get();
            elements_ = activeView_->elements;
        } else {
            // Window declarations are only definitions until a .run() call
            // selects one. Do not let configureView's temporary context make
            // an unrun window look active to run().
            activeView_ = nullptr;
            app_ = nullptr;
            elements_.clear();
        }

        statements_ = std::move(program.statements);
        return true;
    } catch (const std::exception& exception) {
        error = exception.what();
        views_.clear();
        activeView_ = nullptr;
        app_ = nullptr;
        elements_.clear();
        variables_.clear();
        statements_.clear();
        return false;
    }
}

int PxlRuntime::run() {
    if (views_.empty() || activeView_ == nullptr) {
        try {
            executeStatements(statements_);
            return 0;
        } catch (const std::exception& exception) {
            reportRuntimeError(exception);
            return 1;
        }
    }

    for (auto& view : views_) {
        if (!view->app->create()) return 1;
        view->app->setVisible(view.get() == activeView_);

        RuntimeView* viewPointer = view.get();
        view->app->setOnMouseMove([this, viewPointer](MousePosition) {
            try {
                activeView_ = viewPointer;
                app_ = viewPointer->app.get();
                elements_ = viewPointer->elements;
                fireMouseMoveTriggers();
                viewPointer->elements = elements_;
            } catch (const std::exception& exception) {
                reportRuntimeError(exception);
            }
        });
        view->app->setOnKeyDown([this, viewPointer](const std::string& key) {
            try {
                activeView_ = viewPointer;
                app_ = viewPointer->app.get();
                elements_ = viewPointer->elements;
                fireKeyTriggers(key);
                viewPointer->elements = elements_;
            } catch (const std::exception& exception) {
                reportRuntimeError(exception);
            }
        });
        view->app->setOnTick([this, viewPointer] {
            try {
                activeView_ = viewPointer;
                app_ = viewPointer->app.get();
                elements_ = viewPointer->elements;
                checkTriggers();
                viewPointer->elements = elements_;
            } catch (const std::exception& exception) {
                reportRuntimeError(exception);
            }
        });
    }

    try {
        executeStatements(statements_);
    } catch (const std::exception& exception) {
        reportRuntimeError(exception);
        return 1;
    }

    return app_->run();
}
