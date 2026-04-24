#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include "geokernel/geokernel.hpp"

using namespace geokernel;

int main() {
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(-1000.0, 1000.0);
    std::vector<Point2D> points;
    points.reserve(10000);
    for (int i = 0; i < 10000; ++i) {
        points.push_back({dist(rng), dist(rng)});
    }

    const auto start = std::chrono::steady_clock::now();
    const auto hull = convexHullAndrew(points);
    const auto closest = closestPair(points);
    const auto end = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "points=" << points.size()
              << " hull_vertices=" << hull.hull.size()
              << " closest_distance=" << closest.distance
              << " elapsed_ms=" << ms << '\n';
    return 0;
}
