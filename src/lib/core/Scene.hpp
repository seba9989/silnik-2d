#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../render/PrimitiveRenderer.hpp"
#include "Entity.hpp"
#include "Input.hpp"

/** @brief Zbiór obiektów i funkcji logicznych jednego ekranu silnika.
 * @details Silnik aktualizuje i rysuje wyłącznie aktywną scenę. Obiekty dodane
 * przez Spawn() są korzeniami drzew i mogą tworzyć własne dzieci. */
class Scene {
  public:
    /** @brief Dodaje obiekt do sceny.
     * @tparam T Typ obiektu sceny. @return Referencja do nowego obiektu. */
    template <typename T> T& Spawn() {
        entities_.push_back(std::make_unique<T>());
        return static_cast<T&>(*entities_.back());
    }

    /** @brief Ustawia logikę sceny wykonywaną co klatkę.
     * @param update Funkcja otrzymująca czas dt w sekundach. @return Ta sama scena. */
    Scene& OnUpdate(std::function<void(float dt)> update) {
        update_ = std::move(update);
        return *this;
    }
    /** @brief Ustawia dodatkowe rysowanie po obiektach sceny.
     * @param draw Funkcja otrzymująca renderer. @return Ta sama scena. */
    Scene& OnDraw(std::function<void(PrimitiveRenderer&)> draw) {
        draw_ = std::move(draw);
        return *this;
    }

    /** @brief Aktualizuje logikę sceny i jej obiekty.
     * @param dt Czas od poprzedniej klatki. @param input Stan wejścia. */
    void Update(float dt, const Input& input) {
        if (update_) update_(dt);
        // pętla po indeksie: zdarzenie obiektu może wywołać Spawn() i powiększyć wektor
        for (std::size_t i = 0; i < entities_.size(); ++i)
            entities_[i]->Update(dt, input);
    }

    // rysuje drzewa obiektów od najniższej warstwy; w tej samej - w kolejności dodania
    /** @brief Rysuje obiekty według warstw, a następnie wywołuje OnDraw().
     * @param renderer Renderer prymitywów. */
    void Draw(PrimitiveRenderer& renderer) const {
        for (const Entity* entity : Entity::ByLayer(entities_))
            entity->Render(renderer);
        if (draw_) draw_(renderer.Reset().TargetScreen());
    }

  private:
    std::vector<std::unique_ptr<Entity>> entities_;
    std::function<void(float)> update_;
    std::function<void(PrimitiveRenderer&)> draw_;
};
