#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include "geokernel/geokernel.hpp"

using namespace geokernel;

template <typename Fn>
long long elapsedMs(Fn&& fn) {
    const auto start = std::chrono::steady_clock::now();
    fn();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

std::vector<Segment2D> generateSegments(std::size_t count) {
    std::mt19937 rng(7);
    std::uniform_real_distribution<double> xdist(0.0, static_cast<double>(count) * 4.0);
    std::uniform_real_distribution<double> ydist(-1000.0, 1000.0);
    std::uniform_real_distribution<double> len(0.5, 10.0);

    std::vector<Segment2D> segments;
    segments.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const double x = xdist(rng);
        const double y = ydist(rng);
        const double dx = len(rng);
        const double dy = ydist(rng) * 0.01;
        segments.push_back({{x, y}, {x + dx, y + dy}});
    }
    return segments;
}

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

    for (const std::size_t count : {200u, 1000u, 5000u}) {
        const auto segments = generateSegments(count);
        SegmentIntersectionSearchResult brute;
        SegmentIntersectionSearchResult sweep;

        const long long bruteMs = count <= 1000u
            ? elapsedMs([&] { brute = bruteForceSegmentIntersections(segments); })
            : -1;
        const long long sweepMs = elapsedMs([&] { sweep = findSegmentIntersections(segments); });

        std::cout << "segments=" << count
                  << " brute_ms=" << bruteMs
                  << " sweep_ms=" << sweepMs
                  << " sweep_pairs=" << sweep.pairs.size()
                  << " sweep_events=" << sweep.eventCount
                  << " implementation=" << sweep.implementation << '\n';
    }
    return 0;
}
