#pragma once

#include <raylib.h>

#include <span>

#include "Bitmap.hpp"
#include "Geometry.hpp"

// Rysowanie prymitywów 2D. Działa jak pędzel: ma bieżący kolor, wypełnienie, algorytmy
// i cel rysowania (ekran albo bitmapa). Każda metoda zwraca *this, więc wywołania można łączyć:
//
//   renderer.Paint(RED).Fill().Circle({100, 100}, 40).Fill(false).Rect({200, 60}, {120, 80});
//
// Stan zostaje między wywołaniami - Reset() przywraca domyślny pędzel. Celu rysowania
// Reset() nie zmienia (robią to tylko Target() / TargetScreen()), więc wywołany w trakcie
// rysowania po bitmapie nie przeniesie przypadkiem rysowania na ekran.
/**
 * @brief Renderer prymitywów dwuwymiarowych ze stanem podobnym do pędzla.
 * @details Stan obejmuje kolor, wypełnienie, wybór algorytmu oraz cel rysowania:
 * ekran lub Bitmap. Metody zwracają referencję do tego samego obiektu, więc
 * wywołania można łączyć. Reset() przywraca ustawienia pędzla, lecz zachowuje cel.
 * @code{.cpp}
 * renderer.Paint(RED).Fill().Circle({100, 100}, 40)
 *         .Fill(false).Rect({200, 60}, {120, 80});
 * @endcode
 */
class PrimitiveRenderer {
  public:
    // stan pędzla
    /** @brief Ustawia bieżący kolor rysowania.
     * @param color Kolor raylib. @return Ten sam renderer. */
    PrimitiveRenderer& Paint(Color color) {
        color_ = color;
        return *this;
    }
    /** @brief Włącza lub wyłącza wypełnienie kolejnych figur.
     * @param value Czy rysować wnętrze. @return Ten sam renderer. */
    PrimitiveRenderer& Fill(bool value = true) {
        filled_ = value;
        return *this;
    }
    // odcinki (Line, Lines, Polyline, Polygon): własny algorytm przyrostowy zamiast raylib
    /** @brief Wybiera własny algorytm przyrostowy dla odcinków.
     * @param value true dla algorytmu własnego, false dla raylib.
     * @return Ten sam renderer. */
    PrimitiveRenderer& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }
    // okręgi i elipsy: własny algorytm z symetrią (8-krotną / 4-krotną) zamiast raylib
    /** @brief Wybiera własny algorytm symetryczny dla okręgów i elips.
     * @param value true dla algorytmu własnego, false dla raylib.
     * @return Ten sam renderer. */
    PrimitiveRenderer& Symmetric(bool value = true) {
        symmetric_ = value;
        return *this;
    }
    /** @brief Przywraca czarny kolor, brak wypełnienia i algorytmy raylib.
     * @details Nie zmienia celu rysowania. @return Ten sam renderer. */
    PrimitiveRenderer& Reset(); // czarny, bez wypełnienia, algorytmy raylib

    // cel rysowania: ekran (domyślnie) albo bitmapa w RAM
    /** @brief Kieruje następne operacje rysowania do bitmapy w RAM.
     * @param bitmap Bitmapa docelowa. @return Ten sam renderer. */
    PrimitiveRenderer& Target(Bitmap& bitmap) {
        target_ = &bitmap;
        return *this;
    }
    /** @brief Kieruje następne operacje rysowania na ekran.
     * @return Ten sam renderer. */
    PrimitiveRenderer& TargetScreen() {
        target_ = nullptr;
        return *this;
    }

    // pojedyncze prymitywy
    /** @brief Rysuje pojedynczy piksel.
     * @param position Pozycja piksela. @return Ten sam renderer. */
    PrimitiveRenderer& Point(Vector2 position);
    /** @brief Rysuje odcinek wybranym algorytmem.
     * @details Algorytm przyrostowy wykonuje krok po osi dominującej i dodaje
     * stały przyrost współrzędnej drugiej osi. Obsługuje też odcinek zerowy.
     * @param from Początek. @param to Koniec. @return Ten sam renderer. */
    PrimitiveRenderer& Line(Vector2 from, Vector2 to);
    /** @brief Rysuje kontur lub wypełnienie prostokąta.
     * @param position Lewy górny róg. @param size Szerokość i wysokość.
     * @return Ten sam renderer. */
    PrimitiveRenderer& Rect(Vector2 position, Vector2 size);
    /** @brief Rysuje okrąg albo koło wybranym algorytmem.
     * @details Algorytm własny liczy punkty dla kąta od 0 do pi/4 i odtwarza
     * resztę przez symetrię ośmiokrotną. Wypełnienie powstaje z poziomych odcinków.
     * @param center Środek. @param radius Promień. @return Ten sam renderer. */
    PrimitiveRenderer& Circle(Vector2 center, float radius);
    /** @brief Rysuje elipsę wybranym algorytmem.
     * @details Algorytm własny liczy ćwiartkę dla kąta od 0 do pi/2 i odtwarza
     * pozostałe punkty przez symetrię czterokrotną. Na bitmapie używany jest
     * zawsze algorytm własny, także gdy Symmetric(false).
     * @param center Środek. @param radius Promienie poziomy i pionowy.
     * @return Ten sam renderer. */
    PrimitiveRenderer& Ellipse(Vector2 center, Vector2 radius); // radius = {poziomy, pionowy}
    /** @brief Rysuje kontur lub wypełnienie trójkąta.
     * @param a Pierwszy wierzchołek. @param b Drugi wierzchołek.
     * @param c Trzeci wierzchołek. @return Ten sam renderer. */
    PrimitiveRenderer& Triangle(Vector2 a, Vector2 b, Vector2 c);

    // Wielokąt (łamana zwyczajna zamknięta). Gdy krawędzie się przecinają (łamana wiązana)
    // albo odcinki nie tworzą zamkniętego łańcucha, nic nie jest rysowane - można to
    // sprawdzić wcześniej przez IsSimplePolygon(). Wypełnienie: algorytm scanline z kontrolą
    // parzystości.
    /** @brief Rysuje wielokąt z uporządkowanych wierzchołków.
     * @details Przecięcie krawędzi niesąsiednich powoduje brak rysowania.
     * Wypełnienie używa poziomych wierszy i reguły parzystości.
     * @param vertices Wierzchołki w kolejności obwodu. @return Ten sam renderer. */
    PrimitiveRenderer& Polygon(std::span<const Point2D> vertices);
    /** @brief Rysuje wielokąt z uporządkowanego zamkniętego łańcucha odcinków.
     * @details Niedomknięty lub samoprzecinający się łańcuch nie jest rysowany.
     * @param edges Odcinki w kolejności obwodu. @return Ten sam renderer. */
    PrimitiveRenderer& Polygon(std::span<const LineSegment> edges);

    // wersje tablicowe (wiele prymitywów w jednej pętli)
    /** @brief Rysuje zbiór punktów.
     * @param positions Pozycje punktów. @return Ten sam renderer. */
    PrimitiveRenderer& Points(std::span<const Vector2> positions);
    /** @brief Rysuje niezależne odcinki z par kolejnych wierzchołków.
     * @param vertices Punkty (0,1), (2,3) itd.; końcowy punkt bez pary jest pomijany.
     * @return Ten sam renderer. */
    PrimitiveRenderer& Lines(std::span<const Vector2> vertices); // pary (0,1), (2,3), ...
    // Łamana otwarta lub zamknięta (closed = true łączy ostatni wierzchołek z pierwszym).
    // W odróżnieniu od Polygon() nie sprawdza przecięć - łamana może być wiązana.
    /** @brief Rysuje łamaną z wektorów raylib.
     * @param vertices Wierzchołki w kolejności. @param closed Czy połączyć koniec z początkiem.
     * @return Ten sam renderer. */
    PrimitiveRenderer& Polyline(std::span<const Vector2> vertices, bool closed = false);
    /** @brief Rysuje łamaną z punktów Point2D.
     * @param vertices Wierzchołki w kolejności. @param closed Czy domknąć łamaną.
     * @return Ten sam renderer. */
    PrimitiveRenderer& Polyline(std::span<const Point2D> vertices, bool closed = false);
    // łamana ze zbioru odcinków - otwarta lub zamknięta zależnie od samych odcinków
    /** @brief Rysuje podane odcinki bez sprawdzania ich ciągłości.
     * @param segments Zbiór odcinków. @return Ten sam renderer. */
    PrimitiveRenderer& Polyline(std::span<const LineSegment> segments);

    // Wypełnianie obszaru przez spójność (sąsiedztwo 4-punktowe, wersja iteracyjna ze stosem)
    // bieżącym kolorem. Wymaga celu-bitmapy, bo z ekranu nie da się odczytać koloru piksela.
    //   BoundaryFill - wypełnia aż do pikseli w kolorze krawędzi (wnętrze może być różnokolorowe)
    //   FloodFill    - wypełnia spójny obszar w kolorze piksela startowego (ziarna)
    /** @brief Wypełnia obszar aż do krawędzi o wskazanym kolorze.
     * @details Iteracyjny stos i czteropunktowe sąsiedztwo zapobiegają
     * przepełnieniu stosu wywołań. Wymaga celu typu Bitmap.
     * @param seed Piksel startowy. @param boundary Kolor krawędzi.
     * @return Ten sam renderer; bez ustawionej bitmapy niczego nie zmienia. */
    PrimitiveRenderer& BoundaryFill(Vector2 seed, Color boundary);
    /** @brief Wypełnia spójny obszar o kolorze piksela startowego.
     * @details Wymaga celu typu Bitmap. Używa iteracyjnego stosu i czterech sąsiadów.
     * @param seed Piksel startowy. @return Ten sam renderer;
     * bez ustawionej bitmapy lub dla punktu poza nią niczego nie zmienia. */
    PrimitiveRenderer& FloodFill(Vector2 seed);

    // wyświetla bitmapę na ekranie (lewy górny róg w position)
    /** @brief Wyświetla bitmapę na ekranie jako teksturę.
     * @param bitmap Bitmapa źródłowa. @param position Lewy górny róg na ekranie.
     * @return Ten sam renderer. */
    PrimitiveRenderer& Blit(const Bitmap& bitmap, Vector2 position);

  private:
    void Plot(int x, int y);              // pojedynczy piksel w bieżącym celu
    void Span(int y, int fromX, int toX); // poziomy odcinek pikseli (do wypełnień)

    void LineIncremental(Vector2 from, Vector2 to); // algorytm przyrostowy
    void CircleSymmetric(Vector2 center, float radius);
    void EllipseSymmetric(Vector2 center, Vector2 radius);
    void ScanlineFill(std::span<const Point2D> vertices);

    Color color_ = BLACK;
    bool filled_ = false;
    bool incremental_ = false;
    bool symmetric_ = false;
    Bitmap* target_ = nullptr; // nullptr = ekran
};
