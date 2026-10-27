/**
 * @file Entity.hpp
 * @brief Abstrakcyjna baza obiektów sceny: hierarchia, warstwy, zdarzenia i rysowanie.
 */
#pragma once

#include <raylib.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../render/PrimitiveRenderer.hpp"
#include "Input.hpp"

/**
 * @brief Abstrakcyjna baza wszystkich obiektów sceny.
 * @details Obiekty tworzą drzewo: każdy może mieć dzieci (Spawn<T>()), a dzieci kolejne
 * dzieci. Pozycja dziecka jest względna wobec rodzica, więc przesunięcie rodzica przesuwa
 * całe poddrzewo. Dzieci są zawsze rysowane nad swoim rodzicem.
 *
 * Obsługiwane zdarzenia (sprawdzane w Update()):
 * - OnUpdate - co klatkę,
 * - OnClick - kliknięcie lewym przyciskiem myszy na obiekcie,
 * - OnMouseEnter / OnMouseLeave - kursor wszedł na obiekt / zszedł z niego,
 * - OnKeyPressed - naciśnięcie konkretnego klawisza.
 *
 * Metody ustawiające zdarzenia z dostępem do konkretnego typu obiektu dodaje Shape<T>.
 */
class Entity {
  public:
    /** @brief Wirtualny destruktor umożliwiający usuwanie obiektów przez wskaźnik Entity. */
    virtual ~Entity() = default;

    /**
     * @brief Tworzy dziecko wybranego typu.
     * @tparam T Kształt z przestrzeni nazw @ref shapes (lista w opisie klasy Shape)
     * albo własna klasa dziedzicząca po Shape<T>, np. `Spawn<shapes::Circle>()`.
     * @return Referencja do nowego dziecka, do dalszej konfiguracji.
     */
    template <typename T> T& Spawn() {
        auto child = std::make_unique<T>();
        T& ref = *child;
        child->parent_ = this;
        children_.push_back(std::move(child));
        return ref;
    }

    /**
     * @brief Pobiera pozycję lokalną względem rodzica.
     * @return Pozycja lokalna.
     */
    Vector2 Position() const { return position_; }

    /**
     * @brief Oblicza pozycję obiektu w układzie ekranu (z uwzględnieniem rodziców).
     * @return Pozycja na ekranie.
     */
    Vector2 WorldPosition() const {
        if (!parent_) return position_;
        const Vector2 origin = parent_->WorldPosition();
        return {origin.x + position_.x, origin.y + position_.y};
    }

    /**
     * @brief Przelicza punkt ekranowy (np. kursor) na układ lokalny obiektu.
     * @param point Punkt w układzie ekranu.
     * @return Punkt względem pozycji obiektu.
     */
    Vector2 ToLocal(Vector2 point) const {
        const Vector2 origin = WorldPosition();
        return {point.x - origin.x, point.y - origin.y};
    }

    /**
     * @brief Pobiera rodzica obiektu.
     * @return Wskaźnik rodzica albo nullptr dla obiektu na najwyższym poziomie sceny.
     */
    Entity* Parent() const { return parent_; }

    /**
     * @brief Sprawdza, czy kursor znajduje się nad obiektem.
     * @return Stan najechania z ostatniego Update().
     */
    bool Hovered() const { return hovered_; }

    /**
     * @brief Pobiera warstwę używaną do kolejności rysowania.
     * @details Wyższa warstwa jest rysowana na wierzchu, domyślnie 0. Warstwa działa między
     * rodzeństwem - dzieci zawsze są rysowane nad swoim rodzicem.
     * @return Numer warstwy.
     */
    int Layer() const { return layer_; }

    /**
     * @brief Sprawdza trafienie punktem (np. kursorem) w obiekt.
     * @param point Punkt w układzie ekranu.
     * @return true, gdy punkt należy do obiektu.
     */
    virtual bool Contains(Vector2 point) const = 0;

    /**
     * @brief Aktualizuje obiekt: najpierw jego zdarzenia, potem całe poddrzewo.
     * @param dt Czas od poprzedniej klatki w sekundach.
     * @param input Stan wejścia.
     */
    void Update(float dt, const Input& input) {
        HandleEvents(dt, input);
        // pętla po indeksie: zdarzenie może wywołać Spawn() i powiększyć wektor
        for (std::size_t i = 0; i < children_.size(); ++i)
            children_[i]->Update(dt, input);
    }

    /**
     * @brief Rysuje obiekt, a nad nim jego dzieci w kolejności warstw.
     * @details Przed rysowaniem obiektu pędzel jest resetowany, a celem staje się ekran.
     * @param renderer Renderer prymitywów.
     */
    void Render(PrimitiveRenderer& renderer) const {
        renderer.Reset().TargetScreen();
        Draw(renderer);
        for (const Entity* child : ByLayer(children_))
            child->Render(renderer);
    }

    /**
     * @brief Ustala kolejność rysowania: od najniższej warstwy, w tej samej - według dodania.
     * @param list Obiekty do uporządkowania.
     * @return Wskaźniki w kolejności rysowania.
     */
    static std::vector<const Entity*> ByLayer(const std::vector<std::unique_ptr<Entity>>& list) {
        std::vector<const Entity*> order;
        order.reserve(list.size());
        for (const auto& entity : list)
            order.push_back(entity.get());
        std::stable_sort(order.begin(), order.end(),
                         [](const Entity* a, const Entity* b) { return a->Layer() < b->Layer(); });
        return order;
    }

  protected:
    /**
     * @brief Rysuje sam kształt obiektu, bez dzieci.
     * @details Współrzędne ekranowe należy wyznaczać przez World() lub WorldPosition().
     * @param renderer Renderer z wyczyszczonym pędzlem i celem ustawionym na ekran.
     */
    virtual void Draw(PrimitiveRenderer& renderer) const = 0;

    /**
     * @brief Przelicza punkt lokalny (względem pozycji obiektu) na punkt ekranowy.
     * @param local Punkt lokalny.
     * @return Punkt w układzie ekranu.
     */
    Vector2 World(Vector2 local) const {
        const Vector2 origin = WorldPosition();
        return {origin.x + local.x, origin.y + local.y};
    }

    /**
     * @brief Przelicza listę wierzchołków lokalnych na ekranowe.
     * @tparam Vertex Typ wierzchołka (Vector2 albo Point2D).
     * @param local Wierzchołki lokalne.
     * @return Wierzchołki w układzie ekranu.
     */
    template <typename Vertex>
    std::vector<Vertex> WorldVertices(const std::vector<Vertex>& local) const {
        std::vector<Vertex> world;
        world.reserve(local.size());
        for (const Vertex& vertex : local)
            world.push_back(Vertex{World(vertex)});
        return world;
    }

    Vector2 position_{};   /**< Pozycja względem rodzica. */
    Color color_ = BLACK;  /**< Kolor rysowania. */
    bool filled_ = false;  /**< Czy figura jest wypełniona. */
    bool hovered_ = false; /**< Czy kursor był nad obiektem w ostatnim Update(). */
    int layer_ = 0;        /**< Warstwa wśród rodzeństwa. */

    std::function<void(float)> update_; /**< Funkcja OnUpdate. */
    std::function<void()> click_;       /**< Funkcja OnClick. */
    std::function<void()> mouseEnter_;  /**< Funkcja OnMouseEnter. */
    std::function<void()> mouseLeave_;  /**< Funkcja OnMouseLeave. */
    /** Pary (kod klawisza, funkcja) ustawione przez OnKeyPressed. */
    std::vector<std::pair<int, std::function<void()>>> keys_;

  private:
    /**
     * @brief Sprawdza zdarzenia myszy i klawiatury, a potem wywołuje OnUpdate.
     * @param dt Czas od poprzedniej klatki w sekundach.
     * @param input Stan wejścia.
     */
    void HandleEvents(float dt, const Input& input) {
        if (input.MouseEnabled()) {
            const bool inside = Contains(input.MousePosition());
            if (inside && !hovered_ && mouseEnter_) mouseEnter_();
            if (!inside && hovered_ && mouseLeave_) mouseLeave_();
            hovered_ = inside;
            if (inside && input.MousePressed(MOUSE_BUTTON_LEFT) && click_) click_();
        }
        for (const auto& [key, callback] : keys_)
            if (input.KeyPressed(key)) callback();
        if (update_) update_(dt);
    }

    Entity* parent_ = nullptr;                      /**< Rodzic albo nullptr. */
    std::vector<std::unique_ptr<Entity>> children_; /**< Dzieci obiektu. */
};
