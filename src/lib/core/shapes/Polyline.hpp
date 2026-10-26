#pragma once

#include <vector>

#include "../Shape.hpp"
#include "Line.hpp"

/** @brief Otwarta lub zamknięta łamana o lokalnych współrzędnych wierzchołków.
 * @details W przeciwieństwie do Polygon nie sprawdza samoprzecięć. */
class Polyline : public Shape<Polyline> {
  public:
    /** @brief Ustawia wierzchołki łamanej.
     * @param vertices Punkty w kolejności. @return Ta sama łamana. */
    Polyline& Vertices(std::vector<Vector2> vertices) {
        vertices_ = std::move(vertices);
        return *this;
    }
    /** @brief Ustawia domknięcie łamanej.
     * @param value Czy połączyć ostatni punkt z pierwszym. @return Ta sama łamana. */
    Polyline& Closed(bool value = true) {
        closed_ = value;
        return *this;
    }
    /** @brief Wybiera własny algorytm przyrostowy dla odcinków.
     * @param value Czy włączyć algorytm własny. @return Ta sama łamana. */
    Polyline& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }

    // zamknięta łamana: cały wielokąt, otwarta: tylko jej odcinki
    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        const std::vector<Vector2> world = WorldVertices(vertices_);
        const int count = static_cast<int>(world.size());
        if (closed_ && count >= 3) return ::CheckCollisionPointPoly(point, world.data(), count);
        for (int i = 0; i + 1 < count; ++i)
            if (::CheckCollisionPointLine(point, world[i], world[i + 1], Line::kHitDistance))
                return true;
        return false;
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_)
            .Incremental(incremental_)
            .Polyline(WorldVertices(vertices_), closed_);
    }

  private:
    std::vector<Vector2> vertices_;
    bool closed_ = false;
    bool incremental_ = false;
};
