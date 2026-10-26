{ pkgs, ... }:

{
  # Narzędzia dostępne w powłoce projektu
  packages = with pkgs; [
    raylib          # biblioteka graficzna (z pliku raylib-config.cmake korzysta CMake)
    cmake
    ninja
    pkg-config
    gdb             # debugowanie
    clang-tools     # clangd, clang-format, clang-tidy (linter)
    doxygen         # dokumentacja kodu (lab 6)
    graphviz        # diagramy hierarchii klas w Doxygenie

    # Edytor i serwery językowe, z których korzystają rozszerzenia Zeda
    zed-editor             # polecenie `zeditor`
    nixd                   # LSP dla plików .nix
    cmake-language-server  # LSP dla CMakeLists.txt
  ];

  languages.cplusplus.enable = true;

  # Skróty: wpisz w terminalu `build`, `run`, `lint`, `fmt`, `docs`, `clean`
  scripts.build.exec = ''
    cmake -S "$DEVENV_ROOT" -B "$DEVENV_ROOT/build" -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build "$DEVENV_ROOT/build"
    # clangd szuka compile_commands.json w katalogu głównym projektu
    ln -sf build/compile_commands.json "$DEVENV_ROOT/compile_commands.json"
  '';

  scripts.run.exec = ''
    build && "$DEVENV_ROOT/build/silnik" "$@"
  '';

  # Formatowanie wszystkich plików według .clang-format
  scripts.fmt.exec = ''
    find "$DEVENV_ROOT/src" -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \
      | xargs clang-format -i
  '';

  # Dokumentacja HTML wg doxygen/Doxyfile -> doxygen/html/index.html
  scripts.docs.exec = ''
    cd "$DEVENV_ROOT"
    doxygen doxygen/Doxyfile
  '';

  scripts.edit.exec = ''
    zeditor "$DEVENV_ROOT"
  '';

  scripts.clean.exec = ''
    rm -rf "$DEVENV_ROOT/build" "$DEVENV_ROOT/compile_commands.json"
  '';

  enterShell = ''
    echo "Silnik 2D (raylib ${pkgs.raylib.version})"
    echo "  build  - konfiguracja i kompilacja"
    echo "  run    - kompilacja i uruchomienie"
    echo "  fmt    - formatowanie kodu (.clang-format)"
    echo "  docs   - dokumentacja Doxygen"
    echo "  edit   - otwarcie projektu w Zedzie"
    echo "  clean  - usunięcie katalogu build"
  '';
}