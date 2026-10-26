/**
 * @file Group.hpp
 * @brief Kształt Group - niewidoczny rodzic do grupowania obiektów.
 */
#pragma once

#include "../Shape.hpp"

/**
 * @brief Niewidoczny rodzic służący do grupowania i przesuwania dzieci.
 * @details Przydaje się np. dla postaci złożonej z kilku figur albo układu obiektów
 * poruszających się razem.
 */
class Group : public Shape<Group> {
  public:
    /**
     * @brief Grupa nie ma własnego obszaru trafienia.
     * @return Zawsze false.
     */
    bool Contains(Vector2) const override { return false; }

  protected:
    /** @brief Grupa nie rysuje niczego - widoczne są tylko jej dzieci. */
    void Draw(PrimitiveRenderer&) const override {}
};
