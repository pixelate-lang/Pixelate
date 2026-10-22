#pragma once

#include "../language/ast.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <exception>

class App;
class Label;
class Widget;
struct MousePosition;

class PxlRuntime {
public:
    PxlRuntime();
    ~PxlRuntime();

    bool loadFile(const std::string& path, std::string& error);
    int run();

private:
    struct RuntimeView {
        std::string id;
        std::unique_ptr<App> app;
        std::unordered_map<std::string, Widget*> elements;
        std::vector<std::shared_ptr<TriggerStatement>> triggers;
    };

    VariableValue evaluateExpression(const ValueExpression& expression) const;
    bool evaluateCondition(const Condition& condition) const;
    void executeStatements(const std::vector<std::shared_ptr<Statement>>& statements);
    void executeStatement(const Statement& statement);
    void incrementVariable(const std::string& name, bool decrement = false);
    void incrementWidgetProperty(
        const std::string& target,
        const std::string& property,
        bool decrement = false
    );
    void applyUpdateAction(const UpdateAction& action);
    std::string renderText(const UpdateAction::TextTemplate& text) const;
    void checkTriggers();
    void fireMouseMoveTriggers();
    void fireKeyTriggers(const std::string& key);
    void fireCloseTriggers();
    void switchView(const std::string& windowId, bool destroyWindow);
    void configureView(RuntimeView& view, const WindowNode& window);
    void reportRuntimeError(const std::exception& exception) const;

    App* app_ = nullptr;
    std::vector<std::unique_ptr<RuntimeView>> views_;
    RuntimeView* activeView_ = nullptr;
    std::unordered_map<std::string, Widget*> elements_;
    std::unordered_map<std::string, VariableValue> variables_;
    std::vector<std::shared_ptr<Statement>> statements_;
    int currentLine_ = 0;
};
