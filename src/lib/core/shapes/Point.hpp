#pragma once

#include "../Shape.hpp"
#include "Line.hpp"

/** @brief Pojedynczy punkt rysowany w pozycji obiektu. */
class Point : public Shape<Point> {
  public:
    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointCircle(point, WorldPosition(), Line::kHitDistance);
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Point(WorldPosition());
    }
};
