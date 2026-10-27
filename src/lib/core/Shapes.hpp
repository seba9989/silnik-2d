/**
 * @file Shapes.hpp
 * @brief Zbiorczy nagłówek wszystkich wbudowanych kształtów.
 * @details Wygodny punkt włączenia dla kodu scen - dołącza go Engine.hpp.
 */
#pragma once

/**
 * @namespace shapes
 * @brief Kształty, które można tworzyć przez Scene::Spawn() i Entity::Spawn().
 * @details Tabela wszystkich kształtów z opisem i metodami jest w dokumentacji klasy
 * Shape. W kodzie wystarczy wpisać `Spawn<shapes::`, aby edytor podpowiedział typy.
 */

#include "shapes/Canvas.hpp"
#include "shapes/Circle.hpp"
#include "shapes/Ellipse.hpp"
#include "shapes/Group.hpp"
#include "shapes/Line.hpp"
#include "shapes/Lines.hpp"
#include "shapes/Point.hpp"
#include "shapes/Points.hpp"
#include "shapes/Polygon.hpp"
#include "shapes/Polyline.hpp"
#include "shapes/Rect.hpp"
#include "shapes/Triangle.hpp"
