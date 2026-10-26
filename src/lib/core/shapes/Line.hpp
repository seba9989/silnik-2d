#pragma once

#include "../Shape.hpp"

/** @brief Odcinek, którego końce są określone względem pozycji obiektu. */
class Line : public Shape<Line> {
  public:
    /** @brief Ustawia początek odcinka. @param position Pozycja lokalna. @return Ten sam odcinek. */
    Line& From(Vector2 position) {
        from_ = position;
        return *this;
    }
    /** @brief Ustawia koniec odcinka. @param position Pozycja lokalna. @return Ten sam odcinek. */
    Line& To(Vector2 position) {
        to_ = position;
        return *this;
    }
    /** @brief Wybiera własny algorytm przyrostowy zamiast raylib.
     * @param value Czy włączyć algorytm własny. @return Ten sam odcinek. */
    Line& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }

    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointLine(point, World(from_), World(to_), kHitDistance);
    }

    /** @brief Tolerancja trafienia kursorem w odcinek, w pikselach. */
    static constexpr int kHitDistance = 4; // tolerancja trafienia w odcinek [px]

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Incremental(incremental_).Line(World(from_), World(to_));
    }

  private:
    Vector2 from_{};
    Vector2 to_{};
    bool incremental_ = false;
};
