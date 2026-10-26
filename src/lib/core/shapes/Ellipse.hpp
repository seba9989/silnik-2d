#pragma once

#include "../Shape.hpp"

/** @brief Elipsa; pozycja obiektu oznacza środek, a promienie obie półosie. */
class Ellipse : public Shape<Ellipse> {
  public:
    /** @brief Ustawia promienie poziomy i pionowy.
     * @param radius Wektor {promień poziomy, promień pionowy}. @return Ta sama elipsa. */
    Ellipse& Radius(Vector2 radius) {
        radius_ = radius;
        return *this;
    }
    // własny algorytm z 4-krotną symetrią zamiast raylib
    /** @brief Wybiera własny algorytm czterokrotnej symetrii.
     * @param value true dla algorytmu własnego, false dla raylib.
     * @return Ta sama elipsa. */
    Ellipse& Symmetric(bool value = true) {
        symmetric_ = value;
        return *this;
    }

    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        const Vector2 local = ToLocal(point);
        const float x = local.x / radius_.x;
        const float y = local.y / radius_.y;
        return x * x + y * y <= 1.0f;
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_)
            .Fill(filled_)
            .Symmetric(symmetric_)
            .Ellipse(WorldPosition(), radius_);
    }

  private:
    Vector2 radius_{20, 10};
    bool symmetric_ = false;
};
