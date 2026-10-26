/**
 * @mainpage Silnik 2D
 *
 * @section cel Cel projektu
 * Silnik 2D napisany w C++20 z użyciem raylib. Biblioteka udostępnia sceny,
 * obiekty z hierarchią dzieci, wejście z klawiatury i myszy oraz renderer
 * prymitywów. Program zawiera demonstrację funkcji z laboratoriów 1-3.
 *
 * @section uruchomienie Kompilacja i uruchomienie
 * W katalogu głównym projektu należy wykonać:
 * @code{.sh}
 * cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
 * cmake --build build
 * @endcode
 * Program wynikowy to `build/silnik.exe` na Windows albo `build/silnik` na Linux.
 * Jeśli raylib nie jest dostępny lokalnie, CMake pobiera wersję 5.5.
 *
 * @section budowa Najważniejsze części
 * - @ref Engine zarządza oknem, pętlą i przełączaniem scen.
 * - @ref Scene przechowuje obiekty aktywnego ekranu.
 * - @ref Entity i @ref Shape opisują hierarchię figur oraz zdarzenia.
 * - @ref PrimitiveRenderer rysuje na ekranie albo na @ref Bitmap.
 * - @ref Canvas udostępnia bitmapę do algorytmów wypełniania.
 *
 * @section algorytmy Algorytmy laboratoriów
 * Własny algorytm przyrostowy odcinka, okrąg z ośmiokrotną symetrią, elipsa
 * z czterokrotną symetrią, kontrola przecięć wielokąta, wypełnianie scanline
 * oraz iteracyjne flood fill i boundary fill są opisane przy metodach
 * @ref PrimitiveRenderer. W scenie `figury` porównano krzywe z raylib.
 *
 * @section obsluga Sceny demonstracyjne
 * Klawisze 1, 2 i 3 wybierają odpowiednio `menu`, `prymitywy` i `figury`.
 * Na scenie `prymitywy` strzałki przesuwają prostokąt, L zmienia warstwę,
 * PPM przesuwa koniec odcinka, a spacja przełącza jego algorytm.
 * Na scenie `figury` LPM uruchamia flood fill na płótnie, Shift+LPM
 * uruchamia boundary fill, a R przywraca kontury.
 */

#include <string>

#include "lib/core/Engine.hpp"

// Sceny rejestrują się same - patrz SCENE(...) w src/scenes/*.cpp
int main() {
    Engine& engine = Engine::Get()
                         .Title("Silnik 2D - lab 3")
                         .VSync(false)           // podwójne buforowanie z synchronizacją pionową
                         .Window(1000, 700) // albo .Fullscreen()
                         .Background(RAYWHITE)
                         .StartScene("menu")
                         .Init();

    if (!engine.Ready()) return 1;

    return engine
        .OnUpdate([&](float) {
            if (engine.KeyPressed(KEY_ONE)) engine.SwitchScene("menu");
            if (engine.KeyPressed(KEY_TWO)) engine.SwitchScene("prymitywy");
            if (engine.KeyPressed(KEY_THREE)) engine.SwitchScene("figury");
        })
        .OnDraw([&](PrimitiveRenderer&) {
            const std::string hud =
                "Scena: " + engine.ActiveScene() + "   [1] menu  [2] prymitywy  [3] figury";
            ::DrawText(hud.c_str(), 20, 665, 20, DARKGRAY);
        })
        .Run();
}
