#include "shape.h"
#include "draw_context.h"

ShapeWidget::ShapeWidget(ShapeType type) : type_(type) {}

void ShapeWidget::draw(DrawContext& context) {
    switch (type_) {
        case ShapeType::Rect:
            context.drawRectangle(x_, y_, width_, height_, angle_);
            break;
        case ShapeType::Circle:
            context.drawCircle(x_, y_, radius_);
            break;
        case ShapeType::Triangle:
            {
                auto translated = points_;
                for (auto& point : translated) {
                    point.x += x_;
                    point.y += y_;
                }
                context.drawTriangle(translated, angle_);
            }
            break;
    }
}
