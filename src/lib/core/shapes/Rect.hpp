#pragma once

#include "../Shape.hpp"

/** @brief Prostokąt, którego pozycja oznacza lewy górny róg. */
class Rect : public Shape<Rect> {
  public:
    /** @brief Ustawia szerokość i wysokość prostokąta.
     * @param size Wymiary w pikselach. @return Ten sam prostokąt. */
    Rect& Size(Vector2 size) {
        size_ = size;
        return *this;
    }

    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        const Vector2 corner = WorldPosition();
        return ::CheckCollisionPointRec(point, {corner.x, corner.y, size_.x, size_.y});
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Rect(WorldPosition(), size_);
    }

  private:
    Vector2 size_{10, 10};
};
