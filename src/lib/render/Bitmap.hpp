#pragma once

#include <raylib.h>

// Bitmapa w pamięci RAM. W odróżnieniu od ekranu można z niej odczytywać kolory pikseli,
// czego wymagają algorytmy wypełniania przez spójność (boundary fill, flood fill).
// Na ekran trafia jako tekstura, wysyłana na kartę graficzną dopiero przy rysowaniu
// i tylko wtedy, gdy zawartość się zmieniła.
//
//   Bitmap bitmap{400, 300, WHITE};
//   renderer.Target(bitmap).Paint(BLACK).Circle({200, 150}, 80).TargetScreen();
//   renderer.Blit(bitmap, {100, 100});
/**
 * @brief Obraz rastrowy w pamięci RAM z teksturą aktualizowaną podczas rysowania.
 * @details Odczyt pikseli pozwala używać algorytmów flood fill i boundary fill.
 * Tekstura jest tworzona lub uaktualniana dopiero przy wywołaniu Texture().
 */
class Bitmap {
  public:
    /** @brief Tworzy pustą bitmapę. */
    Bitmap() = default;
    /** @brief Tworzy bitmapę o jednolitym kolorze.
     * @param width Szerokość w pikselach. @param height Wysokość w pikselach.
     * @param background Kolor początkowy. */
    Bitmap(int width, int height, Color background = BLANK);
    /** @brief Zwalnia obraz i ewentualną teksturę. */
    ~Bitmap();

    /** @brief Kopiowanie bitmapy jest zabronione. */
    Bitmap(const Bitmap&) = delete;
    /** @brief Przypisanie kopii bitmapy jest zabronione. */
    Bitmap& operator=(const Bitmap&) = delete;
    /** @brief Przenosi zasoby z innej bitmapy. @param other Bitmapa źródłowa. */
    Bitmap(Bitmap&& other) noexcept;
    /** @brief Przenosi zasoby z innej bitmapy.
     * @param other Bitmapa źródłowa. @return Ta sama bitmapa. */
    Bitmap& operator=(Bitmap&& other) noexcept;

    /** @brief Pobiera szerokość bitmapy. @return Liczba pikseli w poziomie. */
    int Width() const { return image_.width; }
    /** @brief Pobiera wysokość bitmapy. @return Liczba pikseli w pionie. */
    int Height() const { return image_.height; }
    /** @brief Sprawdza, czy piksel leży w granicach bitmapy.
     * @param x Współrzędna pozioma. @param y Współrzędna pionowa.
     * @return true dla poprawnych współrzędnych. */
    bool Contains(int x, int y) const { return x >= 0 && y >= 0 && x < Width() && y < Height(); }

    // kolor piksela; poza bitmapą zwraca BLANK, a zapis poza nią jest ignorowany
    /** @brief Odczytuje kolor piksela.
     * @param x Współrzędna pozioma. @param y Współrzędna pionowa.
     * @return Kolor piksela albo BLANK poza bitmapą. */
    Color Pixel(int x, int y) const;
    /** @brief Ustawia kolor piksela, ignorując współrzędne spoza bitmapy.
     * @param x Współrzędna pozioma. @param y Współrzędna pionowa.
     * @param color Nowy kolor. @return Ta sama bitmapa. */
    Bitmap& Pixel(int x, int y, Color color);

    /** @brief Wypełnia całą bitmapę jednym kolorem.
     * @param color Kolor wypełnienia. @return Ta sama bitmapa. */
    Bitmap& Clear(Color color);

    // dostęp do obrazu dla funkcji raylib ImageDraw*; oznacza bitmapę jako zmienioną
    /** @brief Udostępnia obraz raylib do modyfikacji i oznacza go jako zmieniony.
     * @return Referencja do obrazu w RAM. */
    Image& Edit() {
        dirty_ = true;
        return image_;
    }

    // tekstura z aktualną zawartością - wymaga otwartego okna
    /** @brief Pobiera teksturę odpowiadającą aktualnej zawartości bitmapy.
     * @return Referencja do tekstury raylib.
     * @pre Okno musi być otwarte przed utworzeniem tekstury. */
    const Texture2D& Texture() const;

  private:
    void Release();

    Image image_{};
    mutable Texture2D texture_{};
    mutable bool dirty_ = true;
};
