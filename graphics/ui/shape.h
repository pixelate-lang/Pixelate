#pragma once

#include "widget.h"
#include <array>

enum class ShapeType { Rect, Circle, Triangle };

class ShapeWidget final : public Widget {
public:
    explicit ShapeWidget(ShapeType type);

    void draw(DrawContext& context) override;
    void setAngle(int angle) { angle_ = angle; }
    void setRadius(int radius) { radius_ = radius; }
    void setPoints(PointNode p1, PointNode p2, PointNode p3) {
        points_ = {p1, p2, p3};
    }
    int angle() const { return angle_; }
    int radius() const { return radius_; }
    const std::array<PointNode, 3>& points() const { return points_; }

private:
    ShapeType type_;
    int angle_ = 0;
    int radius_ = 10;
    std::array<PointNode, 3> points_{};
};
