#pragma once

#include "../Shape.hpp"

/** @brief Niewidoczny rodzic służący do grupowania i przesuwania dzieci. */
class Group : public Shape<Group> {
  public:
    /** @brief Grupa nie ma własnego obszaru trafienia. @return Zawsze false. */
    bool Contains(Vector2) const override { return false; }

  protected:
    void Draw(PrimitiveRenderer&) const override {}
};
