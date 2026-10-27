/**
 * @file Shape.hpp
 * @brief Szablon Shape<T> ze wspólnymi setterami i typowanymi zdarzeniami kształtów.
 */
#pragma once

#include <raylib.h>

#include <functional>
#include <utility>

#include "Entity.hpp"

// Tabela w dokumentacji poniżej - formatter łamałby jej wiersze.
// clang-format off
/**
 * @brief Baza wszystkich kształtów, które można tworzyć przez Spawn<T>() (CRTP).
 * @tparam T Konkretny kształt dziedziczący po Shape<T>.
 *
 * @details Każdy kształt z przestrzeni nazw @ref shapes dziedziczy po Shape<T>, więc można go
 * dodać do sceny (Scene::Spawn()) albo jako dziecko innego obiektu (Entity::Spawn()).
 * W edytorze wystarczy wpisać `Spawn<shapes::`, aby zobaczyć listę dostępnych typów.
 *
 * @section shape_lista Kształty dostępne w Spawn<T>()
 *
 * | Typ                   | Co rysuje                             | Własne metody                         |
 * |-----------------------|---------------------------------------|---------------------------------------|
 * | @ref shapes::Point    | pojedynczy piksel                     | -                                     |
 * | @ref shapes::Points   | chmurę punktów                        | `Positions()`                         |
 * | @ref shapes::Line     | odcinek                               | `From()`, `To()`                      |
 * | @ref shapes::Lines    | niezależne odcinki z par wierzchołków | `Vertices()`                          |
 * | @ref shapes::Polyline | łamaną otwartą lub zamkniętą          | `Vertices()`, `Closed()`              |
 * | @ref shapes::Rect     | prostokąt                             | `Size()`                              |
 * | @ref shapes::Triangle | trójkąt                               | `Vertices()`                          |
 * | @ref shapes::Circle   | okrąg / koło                          | `Radius()`, `Symmetric()`             |
 * | @ref shapes::Ellipse  | elipsę                                | `Radius()`, `Symmetric()`             |
 * | @ref shapes::Polygon  | wielokąt (wypełnianie scanline)       | `Vertices()`, `Edges()`               |
 * | @ref shapes::Canvas   | bitmapę w RAM (flood/boundary fill)   | `Size()`, `Background()`, `Content()` |
 * | @ref shapes::Group    | nic - grupuje i przesuwa dzieci       | -                                     |
 *
 * Metody wspólne (z tej klasy) ma każdy kształt: Position(), Move(), Paint(), Layer(),
 * Children() oraz zdarzenia OnUpdate(), OnClick(), OnMouseEnter(), OnMouseLeave()
 * i OnKeyPressed(). Fill() działa w Rect, Triangle, Circle, Ellipse i Polygon, a Incremental()
 * (własny algorytm odcinka) w Line, Lines, Polyline i Polygon.
 *
 * @code{.cpp}
 * scene.Spawn<shapes::Circle>()
 *     .Position({400, 300})
 *     .Radius(30)
 *     .OnClick([](shapes::Circle& self) { self.Radius(50); });
 *
 * scene.Spawn<shapes::Rect>()
 *     .Position({100, 100})
 *     .Size({200, 100})
 *     .Children([](shapes::Rect& self) {
 *         self.Spawn<shapes::Circle>().Position({20, 20}).Radius(10); // względem prostokąta
 *     });
 * @endcode
 *
 * @section shape_wlasne Własny kształt
 * Własny typ też można tworzyć przez Spawn<T>(), jeśli dziedziczy po Shape<T>
 * i nadpisuje Entity::Contains() oraz Entity::Draw(). Umieszczenie go w przestrzeni nazw
 * `shapes` sprawia, że edytor podpowie go razem z wbudowanymi.
 * @code{.cpp}
 * namespace shapes {
 * class Star : public Shape<Star> {
 *   public:
 *     bool Contains(Vector2 point) const override;
 *   protected:
 *     void Draw(PrimitiveRenderer& renderer) const override;
 * };
 * } // namespace shapes
 *
 * scene.Spawn<shapes::Star>();
 * @endcode
 */
// clang-format on
template <typename T> class Shape : public Entity {
  public:
    /** Getter Position() z Entity - inaczej przesłoniłby go setter o tej samej nazwie. */
    using Entity::Position;

    /**
     * @brief Ustawia pozycję względem rodzica.
     * @param position Nowa pozycja lokalna.
     * @return Ten sam kształt.
     */
    T& Position(Vector2 position) {
        position_ = position;
        return Self();
    }

    /**
     * @brief Przesuwa kształt o wektor w układzie lokalnym.
     * @param offset Wektor przesunięcia.
     * @return Ten sam kształt.
     */
    T& Move(Vector2 offset) {
        position_ = {position_.x + offset.x, position_.y + offset.y};
        return Self();
    }

    /**
     * @brief Ustawia kolor rysowania.
     * @param color Kolor.
     * @return Ten sam kształt.
     */
    T& Paint(Color color) {
        color_ = color;
        return Self();
    }

    /** Getter Layer() z Entity - inaczej przesłoniłby go setter o tej samej nazwie. */
    using Entity::Layer;

    /**
     * @brief Ustawia warstwę wśród obiektów tego samego rodzica.
     * @param layer Numer warstwy.
     * @return Ten sam kształt.
     */
    T& Layer(int layer) {
        layer_ = layer;
        return Self();
    }

    /**
     * @brief Włącza lub wyłącza wypełnienie figury.
     * @param value Czy figura ma być wypełniona.
     * @return Ten sam kształt.
     */
    T& Fill(bool value = true) {
        filled_ = value;
        return Self();
    }

    /**
     * @brief Wykonuje konfigurację dzieci i pozwala kontynuować łańcuch.
     * @param setup Funkcja otrzymująca ten kształt (dzieci tworzy się przez Spawn<T>()).
     * @return Ten sam kształt.
     */
    T& Children(const std::function<void(T&)>& setup) {
        setup(Self());
        return Self();
    }

    /**
     * @brief Ustawia funkcję aktualizacji wywoływaną co klatkę.
     * @param callback Funkcja otrzymująca obiekt i czas dt w sekundach.
     * @return Ten sam kształt.
     */
    T& OnUpdate(std::function<void(T&, float)> callback) {
        update_ = [this, callback = std::move(callback)](float dt) { callback(Self(), dt); };
        return Self();
    }

    /**
     * @brief Ustawia reakcję na kliknięcie lewym przyciskiem wewnątrz figury.
     * @param callback Funkcja otrzymująca obiekt.
     * @return Ten sam kształt.
     */
    T& OnClick(std::function<void(T&)> callback) {
        click_ = Bind(std::move(callback));
        return Self();
    }

    /**
     * @brief Ustawia reakcję na wejście kursora w obszar figury.
     * @param callback Funkcja otrzymująca obiekt.
     * @return Ten sam kształt.
     */
    T& OnMouseEnter(std::function<void(T&)> callback) {
        mouseEnter_ = Bind(std::move(callback));
        return Self();
    }

    /**
     * @brief Ustawia reakcję na opuszczenie figury przez kursor.
     * @param callback Funkcja otrzymująca obiekt.
     * @return Ten sam kształt.
     */
    T& OnMouseLeave(std::function<void(T&)> callback) {
        mouseLeave_ = Bind(std::move(callback));
        return Self();
    }

    /**
     * @brief Ustawia reakcję na naciśnięcie wybranego klawisza.
     * @details Można podpiąć kilka różnych klawiszy.
     * @param key Kod klawisza raylib.
     * @param callback Funkcja otrzymująca obiekt.
     * @return Ten sam kształt.
     */
    T& OnKeyPressed(int key, std::function<void(T&)> callback) {
        keys_.emplace_back(key, Bind(std::move(callback)));
        return Self();
    }

  protected:
    /**
     * @brief Zwraca ten obiekt jako konkretny typ kształtu.
     * @return Referencja typu T.
     */
    T& Self() { return static_cast<T&>(*this); }

  private:
    /**
     * @brief Opakowuje funkcję zdarzenia tak, by otrzymywała obiekt jako T&.
     * @param callback Funkcja zdarzenia.
     * @return Funkcja bez argumentów do zapisania w Entity.
     */
    std::function<void()> Bind(std::function<void(T&)> callback) {
        return [this, callback = std::move(callback)] { callback(Self()); };
    }
};
