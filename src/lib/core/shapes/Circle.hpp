/**
 * @file Circle.hpp
 * @brief Kształt Circle - okrąg lub koło.
 */
#pragma once

#include "../Shape.hpp"

namespace shapes {

/** @brief Okrąg lub koło; pozycja obiektu oznacza środek. */
class Circle : public Shape<Circle> {
  public:
    /**
     * @brief Ustawia promień.
     * @param radius Promień w pikselach.
     * @return Ten sam okrąg.
     */
    Circle& Radius(float radius) {
        radius_ = radius;
        return *this;
    }

    /**
     * @brief Wybiera własny algorytm z ośmiokrotną symetrią zamiast raylib.
     * @param value true dla algorytmu własnego, false dla raylib.
     * @return Ten sam okrąg.
     */
    Circle& Symmetric(bool value = true) {
        symmetric_ = value;
        return *this;
    }

    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointCircle(point, WorldPosition(), radius_);
    }

  protected:
    /** @copydoc Entity::Draw */
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Symmetric(symmetric_).Circle(WorldPosition(), radius_);
    }

  private:
    float radius_ = 10.0f;   /**< Promień w pikselach. */
    bool symmetric_ = false; /**< Czy użyć własnego algorytmu. */
};

} // namespace shapes
