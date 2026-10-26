#pragma once

#include <raylib.h>

#include <functional>
#include <utility>

#include "Entity.hpp"

// Wspólne settery i zdarzenia. Zwracają T&, żeby łańcuch nie "zgubił" typu,
// a funkcje zdarzeń dostają obiekt jako T&, np.:
//
//   scene.Spawn<Circle>().Radius(30).OnClick([](Circle& self) { self.Radius(50); });
//
// Dzieci można dodać w łańcuchu przez Children():
//
//   scene.Spawn<Rect>().Position({100, 100}).Size({200, 100}).Children([](Rect& self) {
//       self.Spawn<Circle>().Position({20, 20}).Radius(10);   // względem prostokąta
//   });
/**
 * @brief Wspólna konfiguracja kształtów i typowane funkcje zdarzeń.
 * @tparam T Konkretny kształt dziedziczący po Shape<T>.
 * @details Metody zwracają T&, więc łańcuch wywołań zachowuje typ figury.
 * Pozycje dzieci są względne wobec obiektu rodzica.
 */
template <typename T> class Shape : public Entity {
  public:
    using Entity::Position; // getter Position() z Entity, inaczej przesłoniłby go setter
    /** @brief Ustawia pozycję względem rodzica.
     * @param position Nowa pozycja lokalna. @return Ten sam kształt. */
    T& Position(Vector2 position) {
        position_ = position;
        return Self();
    }
    /** @brief Przesuwa kształt o wektor w układzie lokalnym.
     * @param offset Wektor przesunięcia. @return Ten sam kształt. */
    T& Move(Vector2 offset) {
        position_ = {position_.x + offset.x, position_.y + offset.y};
        return Self();
    }
    /** @brief Ustawia kolor rysowania. @param color Kolor. @return Ten sam kształt. */
    T& Paint(Color color) {
        color_ = color;
        return Self();
    }
    using Entity::Layer; // jak wyżej
    /** @brief Ustawia warstwę wśród obiektów tego samego rodzica.
     * @param layer Numer warstwy. @return Ten sam kształt. */
    T& Layer(int layer) {
        layer_ = layer;
        return Self();
    }
    /** @brief Włącza lub wyłącza wypełnienie figury.
     * @param value Czy figura ma być wypełniona. @return Ten sam kształt. */
    T& Fill(bool value = true) {
        filled_ = value;
        return Self();
    }

    // dzieci w łańcuchu: setup dostaje ten obiekt, a Children() zwraca go dalej
    /** @brief Wykonuje konfigurację dzieci i pozwala kontynuować łańcuch.
     * @param setup Funkcja otrzymująca ten kształt. @return Ten sam kształt. */
    T& Children(const std::function<void(T&)>& setup) {
        setup(Self());
        return Self();
    }

    /** @brief Ustawia funkcję aktualizacji wywoływaną co klatkę.
     * @param callback Funkcja otrzymująca obiekt i czas dt. @return Ten sam kształt. */
    T& OnUpdate(std::function<void(T&, float)> callback) {
        update_ = [this, callback = std::move(callback)](float dt) { callback(Self(), dt); };
        return Self();
    }
    /** @brief Ustawia reakcję na kliknięcie lewym przyciskiem wewnątrz figury.
     * @param callback Funkcja otrzymująca obiekt. @return Ten sam kształt. */
    T& OnClick(std::function<void(T&)> callback) {
        click_ = Bind(std::move(callback));
        return Self();
    }
    /** @brief Ustawia reakcję na wejście kursora w obszar figury.
     * @param callback Funkcja otrzymująca obiekt. @return Ten sam kształt. */
    T& OnMouseEnter(std::function<void(T&)> callback) {
        mouseEnter_ = Bind(std::move(callback));
        return Self();
    }
    /** @brief Ustawia reakcję na opuszczenie figury przez kursor.
     * @param callback Funkcja otrzymująca obiekt. @return Ten sam kształt. */
    T& OnMouseLeave(std::function<void(T&)> callback) {
        mouseLeave_ = Bind(std::move(callback));
        return Self();
    }
    /** @brief Ustawia reakcję na naciśnięcie wybranego klawisza.
     * @param key Kod klawisza raylib. @param callback Funkcja otrzymująca obiekt.
     * @return Ten sam kształt. */
    T& OnKeyPressed(int key, std::function<void(T&)> callback) {
        keys_.emplace_back(key, Bind(std::move(callback)));
        return Self();
    }

  protected:
    T& Self() { return static_cast<T&>(*this); }

  private:
    std::function<void()> Bind(std::function<void(T&)> callback) {
        return [this, callback = std::move(callback)] { callback(Self()); };
    }
};
