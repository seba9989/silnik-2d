#pragma once

#include <raylib.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../render/PrimitiveRenderer.hpp"
#include "Input.hpp"

// Baza wszystkich obiektów sceny. Obiekty tworzą drzewo: każdy może mieć dzieci
// (Spawn<T>()), a dzieci kolejne dzieci. Pozycja dziecka jest względna wobec rodzica,
// więc przesunięcie rodzica przesuwa całe poddrzewo.
//
// Zdarzenia:
//   OnUpdate     - co klatkę
//   OnClick      - kliknięcie lewym przyciskiem myszy na obiekcie
//   OnMouseEnter - kursor wszedł na obiekt
//   OnMouseLeave - kursor zszedł z obiektu
//   OnKeyPressed - naciśnięcie konkretnego klawisza
// Typowane wersje (z dostępem do obiektu) dodaje Shape<T>.
/**
 * @brief Abstrakcyjna baza obiektów sceny.
 * @details Obiekty tworzą drzewo. Pozycja dziecka jest lokalna względem rodzica,
 * a zdarzenia myszy i klawiatury są obsługiwane podczas Update().
 */
class Entity {
  public:
    /** @brief Wirtualny destruktor umożliwiający usuwanie obiektów przez wskaźnik Entity. */
    virtual ~Entity() = default;

    // dziecko tego obiektu; zwraca referencję do dziecka, żeby je skonfigurować
    /** @brief Tworzy dziecko wybranego typu.
     * @tparam T Typ obiektu dziedziczącego po Entity.
     * @return Referencja do nowego dziecka. */
    template <typename T> T& Spawn() {
        auto child = std::make_unique<T>();
        T& ref = *child;
        child->parent_ = this;
        children_.push_back(std::move(child));
        return ref;
    }

    // pozycja względem rodzica oraz wynikowa pozycja na ekranie
    /** @brief Pobiera pozycję lokalną względem rodzica. @return Pozycja lokalna. */
    Vector2 Position() const { return position_; }
    /** @brief Oblicza pozycję obiektu w układzie ekranu. @return Pozycja światowa. */
    Vector2 WorldPosition() const {
        if (!parent_) return position_;
        const Vector2 origin = parent_->WorldPosition();
        return {origin.x + position_.x, origin.y + position_.y};
    }

    // punkt na ekranie (np. kursor) -> punkt względem pozycji obiektu
    /** @brief Przelicza punkt ekranowy na układ lokalny obiektu.
     * @param point Punkt ekranowy. @return Punkt lokalny. */
    Vector2 ToLocal(Vector2 point) const {
        const Vector2 origin = WorldPosition();
        return {point.x - origin.x, point.y - origin.y};
    }

    /** @brief Pobiera rodzica obiektu. @return Wskaźnik rodzica lub nullptr. */
    Entity* Parent() const { return parent_; }
    /** @brief Sprawdza, czy kursor znajduje się nad obiektem. @return Stan najechania. */
    bool Hovered() const { return hovered_; }

    // warstwa: wyższa rysowana jest na wierzchu, domyślnie 0.
    // Działa między rodzeństwem - dzieci zawsze rysowane są nad swoim rodzicem.
    /** @brief Pobiera warstwę używaną do kolejności rysowania rodzeństwa.
     * @return Numer warstwy. */
    int Layer() const { return layer_; }

    // czy punkt na ekranie (np. kursor) leży na obiekcie - potrzebne do zdarzeń myszy
    /** @brief Sprawdza trafienie punktem w obiekt.
     * @param point Punkt w układzie ekranu. @return true, gdy punkt należy do obiektu. */
    virtual bool Contains(Vector2 point) const = 0;

    // co klatkę: zdarzenia tego obiektu, potem jego dzieci
    /** @brief Aktualizuje obiekt i całe jego poddrzewo.
     * @param dt Czas od poprzedniej klatki w sekundach. @param input Stan wejścia. */
    void Update(float dt, const Input& input) {
        HandleEvents(dt, input);
        // pętla po indeksie: zdarzenie może wywołać Spawn() i powiększyć wektor
        for (std::size_t i = 0; i < children_.size(); ++i)
            children_[i]->Update(dt, input);
    }

    // rysuje obiekt, a nad nim jego dzieci (wg warstw)
    /** @brief Rysuje obiekt oraz dzieci w kolejności warstw.
     * @param renderer Renderer prymitywów. */
    void Render(PrimitiveRenderer& renderer) const {
        renderer.Reset().TargetScreen();
        Draw(renderer);
        for (const Entity* child : ByLayer(children_))
            child->Render(renderer);
    }

    // kolejność rysowania: od najniższej warstwy, w tej samej - w kolejności dodania
    /** @brief Sortuje obiekty po warstwie bez zmiany kolejności równych warstw.
     * @param list Obiekty do uporządkowania. @return Wskaźniki w kolejności rysowania. */
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
    // sam kształt, bez dzieci; współrzędne ekranu liczy się przez World()
    virtual void Draw(PrimitiveRenderer& renderer) const = 0;

    // punkt lokalny (względem pozycji obiektu) -> punkt na ekranie
    Vector2 World(Vector2 local) const {
        const Vector2 origin = WorldPosition();
        return {origin.x + local.x, origin.y + local.y};
    }
    template <typename Vertex>
    std::vector<Vertex> WorldVertices(const std::vector<Vertex>& local) const {
        std::vector<Vertex> world;
        world.reserve(local.size());
        for (const Vertex& vertex : local)
            world.push_back(Vertex{World(vertex)});
        return world;
    }

    Vector2 position_{};
    Color color_ = BLACK;
    bool filled_ = false;
    bool hovered_ = false;
    int layer_ = 0;

    std::function<void(float)> update_;
    std::function<void()> click_;
    std::function<void()> mouseEnter_;
    std::function<void()> mouseLeave_;
    std::vector<std::pair<int, std::function<void()>>> keys_;

  private:
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

    Entity* parent_ = nullptr;
    std::vector<std::unique_ptr<Entity>> children_;
};
