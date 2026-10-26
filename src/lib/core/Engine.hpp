#pragma once

#include <raylib.h>

#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <utility>

#include "../render/Bitmap.hpp"
#include "../render/PrimitiveRenderer.hpp"
#include "Input.hpp"
#include "Scene.hpp"
#include "Shapes.hpp"

/**
 * @brief Główny obiekt silnika zarządzający oknem, scenami, wejściem i pętlą gry.
 * @details Silnik jest singletonem: Engine::Get() zwraca zawsze tę samą instancję.
 * Metody konfiguracji zwracają Engine&, dlatego można je łączyć w łańcuch.
 * Ustawienia okna można zmieniać także po Init().
 *
 * Przykład użycia:
 * @code{.cpp}
 * Engine& engine = Engine::Get().Title("Gra").Window(800, 600).Fps(60).Init();
 * if (!engine.Ready()) return 1;
 * return engine.Run();
 * @endcode
 */
// Silnik gry. Konfiguracja, scena i pętla gry przez method chaining:
//
//   Engine& engine = Engine::Get().Title("Gra").Window(800, 600).Fps(60).Init();
//   if (!engine.Ready()) return 1;
//
//   engine.CreateScene("menu", [&](Scene& scene) {   // pierwsza scena staje się aktywna
//       scene.Spawn<Circle>().Position({400, 300}).Radius(20).Paint(ORANGE).Fill();
//   });
//
//   engine.CreateScene("gra", [&](Scene& scene) {
//       scene.OnUpdate([&](float dt) { ... });
//   });
//
//   return engine
//       .OnUpdate([&](float) {
//           if (engine.KeyPressed(KEY_ENTER)) engine.SwitchScene("gra");
//       })
//       .Run();
//
// Settery okna działają też po Init() - zmieniają okno na bieżąco.
class Engine {
  public:
    /** @brief Zwraca jedyną instancję silnika. @return Referencja do silnika. */
    // Singleton: jedyna instancja, tworzona przy pierwszym wywołaniu.
    static Engine& Get() {
        static Engine instance;
        return instance;
    }

    /** @brief Kopiowanie silnika jest zabronione. */
    Engine(const Engine&) = delete;
    /** @brief Przypisanie kopii silnika jest zabronione. */
    Engine& operator=(const Engine&) = delete;

    // konfiguracja okna
    /** @brief Ustawia tytuł okna. @param title Nowy tytuł. @return Ten sam silnik. */
    Engine& Title(std::string title);
    /** @brief Ustawia rozmiar okna i wyłącza tryb pełnoekranowy.
     * @param width Szerokość w pikselach. @param height Wysokość w pikselach.
     * @return Ten sam silnik. */
    Engine& Window(int width, int height);
    /** @brief Włącza lub wyłącza tryb pełnoekranowy.
     * @param value Czy włączyć ten tryb. @return Ten sam silnik. */
    Engine& Fullscreen(bool value = true);
    /** @brief Ustawia docelową liczbę klatek na sekundę.
     * @param fps Docelowa liczba klatek. @return Ten sam silnik. */
    Engine& Fps(int fps);
    // Wielokrotne buforowanie: raylib (przez GLFW) zawsze tworzy okno z podwójnym buforem
    // ramki - rysujemy w tylnym buforze, a EndDrawing() zamienia go z przednim. VSync()
    // synchronizuje tę zamianę z odświeżaniem monitora, co usuwa rozrywanie obrazu (tearing).
    /** @brief Włącza lub wyłącza synchronizację pionową.
     * @details Podwójne buforowanie okna działa niezależnie od tego ustawienia.
     * @param value Czy włączyć VSync. @return Ten sam silnik. */
    Engine& VSync(bool value = true);
    /** @brief Ustawia kolor czyszczenia ekranu na początku każdej klatki.
     * @param color Kolor tła. @return Ten sam silnik. */
    Engine& Background(Color color) {
        background_ = color;
        return *this;
    }

    // konfiguracja wejścia i logów
    /** @brief Włącza lub wyłącza odczyt klawiatury.
     * @param value Czy włączyć klawiaturę. @return Ten sam silnik. */
    Engine& Keyboard(bool value = true) {
        input_.Keyboard(value);
        return *this;
    }
    /** @brief Włącza lub wyłącza odczyt myszy.
     * @param value Czy włączyć mysz. @return Ten sam silnik. */
    Engine& Mouse(bool value = true) {
        input_.Mouse(value);
        return *this;
    }
    /** @brief Ustawia ścieżkę pliku diagnostycznego.
     * @param path Ścieżka pliku; pusty napis wyłącza zapis do pliku.
     * @return Ten sam silnik. */
    Engine& LogFile(std::string path) {
        logFile_ = std::move(path);
        return *this;
    }

    // logika globalna wywoływana co klatkę, niezależnie od aktywnej sceny
    /** @brief Ustawia globalną funkcję aktualizacji.
     * @param update Funkcja wywoływana co klatkę z czasem dt w sekundach.
     * @return Ten sam silnik. */
    Engine& OnUpdate(std::function<void(float dt)> update) {
        update_ = std::move(update);
        return *this;
    }
    /** @brief Ustawia globalną funkcję rysowania.
     * @param draw Funkcja wywoływana co klatkę z rendererem.
     * @return Ten sam silnik. */
    Engine& OnDraw(std::function<void(PrimitiveRenderer&)> draw) {
        draw_ = std::move(draw);
        return *this;
    }

    // cykl życia: Init() otwiera okno, Run() kręci pętlę aż do zamknięcia i sprząta.
    // Run() zwraca kod wyjścia dla main (0 - ok, 1 - błąd inicjalizacji).
    /** @brief Otwiera okno i inicjalizuje silnik, jeśli jeszcze nie jest gotowy.
     * @return Ten sam silnik; wynik można sprawdzić przez Ready(). */
    Engine& Init();
    /** @brief Sprawdza wynik inicjalizacji. @return true, gdy okno jest gotowe. */
    bool Ready() const { return initialized_; }
    /** @brief Uruchamia pętlę gry aż do zamknięcia okna.
     * @return 0 po normalnym zakończeniu albo 1 po błędzie inicjalizacji. */
    int Run();
    /** @brief Zwalnia sceny, tekstury i okno. Ponowne wywołanie jest bezpieczne. */
    void Shutdown();

    // sceny: aktualizowana i rysowana jest tylko aktywna.
    // SwitchScene() zmienia scenę na końcu bieżącej klatki.
    // CreateScene(name, setup) tworzy scenę, od razu wywołuje setup(scena) i zwraca silnik,
    // więc sceny można łączyć w jeden łańcuch. Wersja bez setup zwraca samą scenę.
    /** @brief Tworzy scenę i natychmiast wykonuje jej konfigurację.
     * @param name Nazwa sceny. @param setup Funkcja otrzymująca utworzoną scenę.
     * @return Ten sam silnik. */
    Engine& CreateScene(const std::string& name, const std::function<void(Scene&)>& setup);
    /** @brief Tworzy scenę albo zwraca już istniejącą o tej nazwie.
     * @param name Nazwa sceny. @return Referencja do sceny. */
    Scene& CreateScene(const std::string& name);

    // RegisterScene() tylko zapamiętuje setup - scena powstaje w Run(), już po otwarciu okna.
    // Zwykle używane przez makro SCENE (niżej). StartScene() wybiera scenę startową.
    /** @brief Rejestruje konfigurację sceny do wykonania po otwarciu okna.
     * @param name Nazwa sceny. @param setup Funkcja konfigurująca scenę.
     * @return Ten sam silnik. */
    Engine& RegisterScene(std::string name, std::function<void(Scene&)> setup);
    /** @brief Wybiera scenę startową spośród zarejestrowanych scen.
     * @param name Nazwa sceny. @return Ten sam silnik. */
    Engine& StartScene(std::string name) {
        start_ = std::move(name);
        return *this;
    }
    /** @brief Pobiera scenę o podanej nazwie.
     * @param name Nazwa sceny. @return Referencja do sceny.
     * @throw std::out_of_range Gdy scena nie istnieje. */
    Scene& GetScene(const std::string& name) { return scenes_.at(name); }
    /** @brief Zleca zmianę aktywnej sceny na koniec bieżącej klatki.
     * @param name Nazwa istniejącej sceny. @return Ten sam silnik.
     * Nieznana nazwa jest zapisywana jako błąd w logu. */
    Engine& SwitchScene(const std::string& name);
    /** @brief Podaje nazwę aktywnej sceny. @return Referencja do nazwy. */
    const std::string& ActiveScene() const { return active_; }

    // klawiatura i mysz
    /** @brief Sprawdza, czy klawisz jest aktualnie wciśnięty. @param key Kod klawisza raylib. @return Stan klawisza. */
    bool KeyDown(int key) const { return input_.KeyDown(key); }
    /** @brief Sprawdza, czy klawisz naciśnięto w bieżącej klatce. @param key Kod klawisza raylib. @return Stan naciśnięcia. */
    bool KeyPressed(int key) const { return input_.KeyPressed(key); }
    /** @brief Sprawdza, czy przycisk myszy jest wciśnięty. @param button Kod przycisku raylib. @return Stan przycisku. */
    bool MouseDown(int button) const { return input_.MouseDown(button); }
    /** @brief Sprawdza, czy przycisk myszy naciśnięto w bieżącej klatce. @param button Kod przycisku raylib. @return Stan naciśnięcia. */
    bool MousePressed(int button) const { return input_.MousePressed(button); }
    /** @brief Pobiera pozycję kursora w oknie. @return Współrzędne kursora. */
    Vector2 MousePosition() const { return input_.MousePosition(); }

    // czyszczenie ekranu / bitmapy
    /** @brief Czyści aktualny cel rysowania wskazanym kolorem.
     * @param color Kolor czyszczenia. @return Ten sam silnik. */
    Engine& Clear(Color color) {
        ::ClearBackground(color);
        return *this;
    }
    /** @brief Czyści teksturę renderowaną.
     * @param target Tekstura docelowa. @param color Kolor czyszczenia.
     * @return Ten sam silnik. */
    Engine& Clear(RenderTexture2D& target, Color color);
    /** @brief Czyści bitmapę w pamięci RAM.
     * @param bitmap Bitmapa docelowa. @param color Kolor czyszczenia.
     * @return Ten sam silnik. */
    Engine& Clear(Bitmap& bitmap, Color color) {
        bitmap.Clear(color);
        return *this;
    }

    // rysowanie prymitywów
    /** @brief Udostępnia renderer prymitywów silnika. @return Referencja do renderera. */
    PrimitiveRenderer& Primitives() { return primitives_; }

    // błędy / diagnostyka
    /** @brief Zapisuje komunikat do stderr i opcjonalnego pliku logu.
     * @param message Treść komunikatu. @return Ten sam silnik. */
    Engine& Log(const std::string& message);

  private:
    Engine() = default;
    ~Engine() { Shutdown(); }

    std::string title_ = "Gra";
    int width_ = 800;
    int height_ = 600;
    bool fullscreen_ = false;
    bool vsync_ = false;
    int fps_ = 60;
    Color background_ = BLACK;
    Input input_;
    std::string logFile_ = "engine.log";

    std::function<void(float)> update_;
    std::function<void(PrimitiveRenderer&)> draw_;

    std::ofstream log_;
    bool initialized_ = false;

    std::map<std::string, Scene> scenes_;
    std::string active_;
    std::string pending_; // scena, na którą przełączamy po zakończeniu klatki
    std::string start_;   // scena startowa wybrana przez StartScene()
    std::map<std::string, std::function<void(Scene&)>> registered_;
    PrimitiveRenderer primitives_;

    void CreateRegisteredScenes();
};

// ------------------------------------------------------------
// Samorejestracja scen. W dowolnym pliku .cpp:
//
//   SCENE("menu") {
//       scene.Spawn<Rect>().Position({300, 200}).Size({400, 300});
//   }
//
// Wewnątrz bloku dostępna jest zmienna `scene` (Scene&). Scena zostanie utworzona w Run(),
// a scenę startową wybiera engine.StartScene("menu") w main.
// ------------------------------------------------------------
/** @brief Pomocniczy rejestrator scen wykorzystywany przez makro SCENE. */
struct SceneRegistration {
    /** @brief Zapamiętuje funkcję konfigurującą scenę.
     * @param name Nazwa sceny. @param setup Funkcja konfigurująca. */
    SceneRegistration(const char* name, void (*setup)(Scene&)) {
        Engine::Get().RegisterScene(name, setup);
    }
};

#define SCENE_CONCAT_(a, b) a##b
#define SCENE_CONCAT(a, b) SCENE_CONCAT_(a, b)
#define SCENE_IMPL_(name, setup)                                                                   \
    static void setup(Scene& scene);                                                               \
    static const SceneRegistration SCENE_CONCAT(setup, _registration){name, setup};                \
    static void setup(Scene& scene)
/** @def SCENE(name)
 * @brief Rejestruje scenę z bloku kodu wykonywanego po inicjalizacji okna.
 * @param name Nazwa sceny używana przez StartScene() i SwitchScene().
 */
#define SCENE(name) SCENE_IMPL_(name, SCENE_CONCAT(scene_setup_, __LINE__))
