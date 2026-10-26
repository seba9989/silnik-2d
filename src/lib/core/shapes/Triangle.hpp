/**
 * @file Triangle.hpp
 * @brief Kształt Triangle - trójkąt.
 */
#pragma once

#include "../Shape.hpp"

/** @brief Trójkąt z wierzchołkami lokalnymi względem pozycji obiektu. */
class Triangle : public Shape<Triangle> {
  public:
    /**
     * @brief Ustawia trzy wierzchołki trójkąta.
     * @param a Pierwszy wierzchołek.
     * @param b Drugi wierzchołek.
     * @param c Trzeci wierzchołek.
     * @return Ten sam trójkąt.
     */
    Triangle& Vertices(Vector2 a, Vector2 b, Vector2 c) {
        a_ = a;
        b_ = b;
        c_ = c;
        return *this;
    }

    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointTriangle(point, World(a_), World(b_), World(c_));
    }

  protected:
    /** @copydoc Entity::Draw */
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Triangle(World(a_), World(b_), World(c_));
    }

  private:
    Vector2 a_{}; /**< Pierwszy wierzchołek. */
    Vector2 b_{}; /**< Drugi wierzchołek. */
    Vector2 c_{}; /**< Trzeci wierzchołek. */
};
