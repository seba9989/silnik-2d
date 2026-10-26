#pragma once

#include <utility>
#include <vector>

#include "../Shape.hpp"
#include "Line.hpp"

/** @brief Zbiór punktów, których pozycje są lokalne względem obiektu. */
class Points : public Shape<Points> {
  public:
    /** @brief Ustawia pozycje punktów.
     * @param positions Pozycje lokalne. @return Ten sam zbiór. */
    Points& Positions(std::vector<Vector2> positions) {
        positions_ = std::move(positions);
        return *this;
    }

    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        for (const Vector2& position : positions_)
            if (::CheckCollisionPointCircle(point, World(position), Line::kHitDistance))
                return true;
        return false;
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Points(WorldVertices(positions_));
    }

  private:
    std::vector<Vector2> positions_;
};
