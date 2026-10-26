#pragma once

#include <vector>

#include "../../render/Geometry.hpp"
#include "../Shape.hpp"

// Wielokąt (łamana zwyczajna zamknięta), wierzchołki względem pozycji obiektu.
// Łamana wiązana (przecinające się krawędzie) nie jest rysowana - patrz Simple().
// Fill() wypełnia go algorytmem scanline z kontrolą parzystości.
/** @brief Wielokąt o wierzchołkach lokalnych względem pozycji obiektu.
 * @details Przecinające się krawędzie niesąsiednie uniemożliwiają rysowanie.
 * Fill() uruchamia wypełnianie poziomymi wierszami z regułą parzystości. */
class Polygon : public Shape<Polygon> {
  public:
    /** @brief Ustawia wierzchołki obwodu.
     * @param vertices Punkty w kolejności obwodu. @return Ten sam wielokąt. */
    Polygon& Vertices(std::vector<Point2D> vertices) {
        vertices_ = std::move(vertices);
        return *this;
    }
    // zbiór odcinków tworzących zamknięty łańcuch; inny zbiór daje pusty wielokąt
    /** @brief Buduje wielokąt z uporządkowanego łańcucha odcinków.
     * @param edges Odcinki stykające się kolejno końcami i domknięte.
     * @return Ten sam wielokąt; dla niepoprawnego łańcucha bez wierzchołków. */
    Polygon& Edges(const std::vector<LineSegment>& edges) {
        vertices_ = PolygonVertices(edges).value_or(std::vector<Point2D>{});
        return *this;
    }
    /** @brief Wybiera algorytm przyrostowy dla konturu.
     * @param value Czy włączyć algorytm własny. @return Ten sam wielokąt. */
    Polygon& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }

    /** @brief Sprawdza przecięcia niesąsiednich krawędzi.
     * @return Wynik IsSimplePolygon() dla bieżących wierzchołków. */
    bool Simple() const { return IsSimplePolygon(vertices_); }

    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        if (!Simple()) return false;
        std::vector<Vector2> world;
        for (const Point2D& vertex : WorldVertices(vertices_))
            world.push_back(vertex);
        return ::CheckCollisionPointPoly(point, world.data(), static_cast<int>(world.size()));
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_)
            .Fill(filled_)
            .Incremental(incremental_)
            .Polygon(WorldVertices(vertices_));
    }

  private:
    std::vector<Point2D> vertices_;
    bool incremental_ = false;
};
