/** @file Primitives.cpp
 * @brief Podstawowe prymitywy i własny algorytm przyrostowy odcinka.
 * @details Algorytm wybiera oś o większej zmianie współrzędnej, wykonuje na niej
 * kroki o jeden piksel i dodaje stały przyrost na drugiej osi. */

#include "../PrimitiveRenderer.hpp"

#include <cmath>
#include <utility>

namespace {

int Round(float value) {
    return static_cast<int>(std::lround(value));
}

// wspólna implementacja łamanej dla wierzchołków Vector2 i Point2D
template <typename Vertex>
void DrawPolyline(PrimitiveRenderer& renderer, std::span<const Vertex> vertices, bool closed) {
    if (vertices.size() < 2) return;
    for (std::size_t i = 0; i + 1 < vertices.size(); ++i)
        renderer.Line(vertices[i], vertices[i + 1]);
    if (closed) renderer.Line(vertices.back(), vertices.front());
}

} // namespace

PrimitiveRenderer& PrimitiveRenderer::Reset() {
    color_ = BLACK;
    filled_ = false;
    incremental_ = false;
    symmetric_ = false;
    return *this;
}

// ------------------------------------------------------------
// Piksele
// ------------------------------------------------------------

void PrimitiveRenderer::Plot(int x, int y) {
    if (target_)
        target_->Pixel(x, y, color_);
    else
        ::DrawPixel(x, y, color_);
}

void PrimitiveRenderer::Span(int y, int fromX, int toX) {
    if (fromX > toX) std::swap(fromX, toX);
    for (int x = fromX; x <= toX; ++x)
        Plot(x, y);
}

// ------------------------------------------------------------
// Pojedyncze prymitywy
// ------------------------------------------------------------

PrimitiveRenderer& PrimitiveRenderer::Point(Vector2 position) {
    Plot(Round(position.x), Round(position.y));
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Line(Vector2 from, Vector2 to) {
    if (incremental_)
        LineIncremental(from, to);
    else if (target_)
        ::ImageDrawLineV(&target_->Edit(), from, to, color_);
    else
        ::DrawLineV(from, to, color_);
    return *this;
}

// Algorytm przyrostowy:
//  1. m = dy/dx
//  2. x rośnie o 1 od lewej strony, y rośnie o m, piksel w (x, round(y))
// Dla |m| > 1 zamieniamy role osi (krok po y, x += 1/m), inaczej odcinek miałby dziury.
void PrimitiveRenderer::LineIncremental(Vector2 from, Vector2 to) {
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;

    if (dx == 0.0f && dy == 0.0f) { // odcinek zdegenerowany do punktu
        Plot(Round(from.x), Round(from.y));
        return;
    }

    if (std::fabs(dx) >= std::fabs(dy)) { // |m| <= 1 -> krok po x
        if (from.x > to.x) std::swap(from, to);
        const float m = (to.y - from.y) / (to.x - from.x);
        float y = from.y;
        for (int x = Round(from.x); x <= Round(to.x); ++x) {
            Plot(x, Round(y));
            y += m;
        }
    } else { // |m| > 1 -> krok po y
        if (from.y > to.y) std::swap(from, to);
        const float mInverse = (to.x - from.x) / (to.y - from.y);
        float x = from.x;
        for (int y = Round(from.y); y <= Round(to.y); ++y) {
            Plot(Round(x), y);
            x += mInverse;
        }
    }
}

PrimitiveRenderer& PrimitiveRenderer::Rect(Vector2 position, Vector2 size) {
    const Rectangle rectangle{position.x, position.y, size.x, size.y};
    if (target_) {
        if (filled_)
            ::ImageDrawRectangleRec(&target_->Edit(), rectangle, color_);
        else
            ::ImageDrawRectangleLines(&target_->Edit(), rectangle, 1, color_);
    } else {
        if (filled_)
            ::DrawRectangleRec(rectangle, color_);
        else
            ::DrawRectangleLinesEx(rectangle, 1.0f, color_);
    }
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Triangle(Vector2 a, Vector2 b, Vector2 c) {
    if (filled_) {
        // raylib wymaga wierzchołków przeciwnie do ruchu wskazówek zegara
        const float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        if (cross > 0.0f) std::swap(b, c);
        if (target_)
            ::ImageDrawTriangle(&target_->Edit(), a, b, c, color_);
        else
            ::DrawTriangle(a, b, c, color_);
    } else {
        if (target_)
            ::ImageDrawTriangleLines(&target_->Edit(), a, b, c, color_);
        else
            ::DrawTriangleLines(a, b, c, color_);
    }
    return *this;
}

// ------------------------------------------------------------
// Wersje tablicowe
// ------------------------------------------------------------

PrimitiveRenderer& PrimitiveRenderer::Points(std::span<const Vector2> positions) {
    for (const Vector2& position : positions)
        Point(position);
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Lines(std::span<const Vector2> vertices) {
    for (std::size_t i = 0; i + 1 < vertices.size(); i += 2)
        Line(vertices[i], vertices[i + 1]);
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Polyline(std::span<const Vector2> vertices, bool closed) {
    DrawPolyline(*this, vertices, closed);
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Polyline(std::span<const Point2D> vertices, bool closed) {
    DrawPolyline(*this, vertices, closed);
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Polyline(std::span<const LineSegment> segments) {
    for (const LineSegment& segment : segments)
        Line(segment.From(), segment.To());
    return *this;
}
