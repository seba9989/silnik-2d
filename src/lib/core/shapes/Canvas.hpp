#pragma once

#include <functional>

#include "../../render/Bitmap.hpp"
#include "../Shape.hpp"

// Płótno: bitmapa w RAM wyświetlana w pozycji obiektu (lewy górny róg). Można po niej rysować
// i wypełniać obszary (BoundaryFill / FloodFill), bo w odróżnieniu od ekranu da się z niej
// odczytać kolory pikseli. Współrzędne rysowania są względem lewego górnego rogu płótna.
//
//   scene.Spawn<Canvas>().Position({50, 50}).Size({300, 200}).Background(WHITE)
//       .Content([](PrimitiveRenderer& r) { r.Paint(BLACK).Circle({150, 100}, 80); });
/** @brief Płótno z bitmapą RAM wyświetlaną jako obiekt sceny.
 * @details Pozwala odczytywać piksele i stosować wypełnianie przez spójność.
 * Współrzędne Content() są liczone od lewego górnego rogu płótna. */
class Canvas : public Shape<Canvas> {
  public:
    /** @brief Ustawia rozmiar, tworząc bitmapę od nowa.
     * @param size Szerokość i wysokość w pikselach. @return To samo płótno. */
    Canvas& Size(Vector2 size) {
        bitmap_ = Bitmap{static_cast<int>(size.x), static_cast<int>(size.y), background_};
        return *this;
    }
    /** @brief Ustawia kolor tła i czyści nim bitmapę.
     * @param color Kolor tła. @return To samo płótno. */
    Canvas& Background(Color color) {
        background_ = color;
        bitmap_.Clear(color);
        return *this;
    }
    /** @brief Czyści płótno bieżącym kolorem tła. @return To samo płótno. */
    Canvas& Clear() {
        bitmap_.Clear(background_);
        return *this;
    }
    // rysowanie po płótnie - renderer ma już ustawiony cel na bitmapę płótna
    /** @brief Wykonuje rysowanie na bitmapie płótna.
     * @param paint Funkcja otrzymująca renderer skierowany na bitmapę.
     * @return To samo płótno. */
    Canvas& Content(const std::function<void(PrimitiveRenderer&)>& paint) {
        PrimitiveRenderer renderer;
        renderer.Target(bitmap_);
        paint(renderer);
        return *this;
    }

    /** @brief Udostępnia bitmapę płótna. @return Referencja do bitmapy. */
    Bitmap& Pixels() { return bitmap_; }

    /** @copydoc Entity::Contains */
    bool Contains(Vector2 point) const override {
        const Vector2 corner = WorldPosition();
        return ::CheckCollisionPointRec(point,
                                        {corner.x, corner.y, static_cast<float>(bitmap_.Width()),
                                         static_cast<float>(bitmap_.Height())});
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        if (bitmap_.Width() > 0) renderer.Blit(bitmap_, WorldPosition());
    }

  private:
    Bitmap bitmap_;
    Color background_ = WHITE;
};
