#pragma once

#include <raylib.h>

#include <optional>
#include <span>
#include <vector>

class PrimitiveRenderer;

// Punkt na płaszczyźnie. Zamienia się automatycznie z/na Vector2 z raylib,
// więc można go podawać wszędzie tam, gdzie silnik przyjmuje Vector2:
//
//   Point2D p{100, 50};
//   p.X(120).Y(80).Draw(renderer);
/** @brief Punkt dwuwymiarowy zgodny ze współrzędnymi raylib. */
class Point2D {
  public:
    /** @brief Tworzy punkt w początku układu. */
    constexpr Point2D() = default;
    /** @brief Tworzy punkt z dwóch współrzędnych.
     * @param x Współrzędna pozioma. @param y Współrzędna pionowa. */
    constexpr Point2D(float x, float y) : x_(x), y_(y) {}
    /** @brief Tworzy punkt z wektora raylib. @param vector Współrzędne wektora. */
    constexpr Point2D(Vector2 vector) : x_(vector.x), y_(vector.y) {}
    /** @brief Konwertuje punkt na wektor raylib. @return Wektor o tych samych współrzędnych. */
    constexpr operator Vector2() const { return {x_, y_}; }

    /** @brief Pobiera współrzędną poziomą. @return Wartość X. */
    float X() const { return x_; }
    /** @brief Pobiera współrzędną pionową. @return Wartość Y. */
    float Y() const { return y_; }
    /** @brief Zmienia współrzędną poziomą. @param x Nowa wartość X. @return Ten sam punkt. */
    Point2D& X(float x) {
        x_ = x;
        return *this;
    }
    /** @brief Zmienia współrzędną pionową. @param y Nowa wartość Y. @return Ten sam punkt. */
    Point2D& Y(float y) {
        y_ = y;
        return *this;
    }

    // rysuje punkt bieżącym kolorem renderera
    /** @brief Rysuje punkt bieżącym kolorem renderera.
     * @param renderer Renderer docelowy. @return Ten sam punkt. */
    const Point2D& Draw(PrimitiveRenderer& renderer) const;

    /** @brief Porównuje obie współrzędne punktów. */
    bool operator==(const Point2D&) const = default;

  private:
    float x_ = 0.0f;
    float y_ = 0.0f;
};

// Odcinek między dwoma punktami.
//
//   LineSegment edge{{0, 0}, {100, 50}};
//   edge.Draw(renderer);         // algorytm domyślny (raylib)
//   edge.Draw(renderer, true);   // algorytm przyrostowy
/** @brief Odcinek wyznaczony przez dwa punkty Point2D. */
class LineSegment {
  public:
    /** @brief Tworzy odcinek zerowy w początku układu. */
    constexpr LineSegment() = default;
    /** @brief Tworzy odcinek między dwoma punktami.
     * @param from Początek. @param to Koniec. */
    constexpr LineSegment(Point2D from, Point2D to) : from_(from), to_(to) {}

    /** @brief Pobiera początek odcinka. @return Punkt początkowy. */
    Point2D From() const { return from_; }
    /** @brief Pobiera koniec odcinka. @return Punkt końcowy. */
    Point2D To() const { return to_; }
    /** @brief Zmienia początek odcinka. @param from Nowy początek. @return Ten sam odcinek. */
    LineSegment& From(Point2D from) {
        from_ = from;
        return *this;
    }
    /** @brief Zmienia koniec odcinka. @param to Nowy koniec. @return Ten sam odcinek. */
    LineSegment& To(Point2D to) {
        to_ = to;
        return *this;
    }

    // czy odcinki mają choć jeden punkt wspólny (także styk końcami i nakładanie się)
    /** @brief Sprawdza przecięcie dwóch odcinków, także styk i nakładanie.
     * @param other Drugi odcinek. @return true, gdy odcinki mają wspólny punkt. */
    bool Intersects(const LineSegment& other) const;

    // rysuje odcinek bieżącym kolorem renderera wybranym algorytmem:
    // domyślnym z biblioteki (false) albo przyrostowym (true)
    /** @brief Rysuje odcinek wybranym algorytmem.
     * @param renderer Renderer docelowy. @param incremental true dla własnego algorytmu,
     * false dla raylib. @return Ten sam odcinek. */
    const LineSegment& Draw(PrimitiveRenderer& renderer, bool incremental = false) const;

  private:
    Point2D from_;
    Point2D to_;
};

// Czy wierzchołki tworzą łamaną zwyczajną zamkniętą: co najmniej 3 wierzchołki,
// a krawędzie niesąsiednie nie mają punktów wspólnych. Inaczej to łamana wiązana.
/** @brief Sprawdza, czy zamknięta łamana nie przecina się na krawędziach niesąsiednich.
 * @param vertices Wierzchołki w kolejności obwodu, co najmniej trzy.
 * @return false dla mniej niż trzech punktów lub przecięcia krawędzi niesąsiednich.
 * @note Powtórzone sąsiednie wierzchołki nie są osobno odrzucane. */
bool IsSimplePolygon(std::span<const Point2D> vertices);

// Zamienia zbiór odcinków na listę wierzchołków. Odcinki muszą tworzyć zamknięty
// łańcuch (koniec każdego = początek następnego, koniec ostatniego = początek pierwszego).
/** @brief Zamienia uporządkowany zamknięty łańcuch odcinków na wierzchołki.
 * @param edges Odcinki w kolejności obwodu.
 * @return Lista wierzchołków albo std::nullopt, gdy łańcuch jest pusty lub niedomknięty. */
std::optional<std::vector<Point2D>> PolygonVertices(std::span<const LineSegment> edges);
