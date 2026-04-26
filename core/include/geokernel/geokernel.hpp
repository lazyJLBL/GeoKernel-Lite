#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "geokernel/predicates/predicates.hpp"

namespace geokernel {

constexpr double EPS = 1e-9;

inline int sign(double x, double eps = EPS) {
    if (x > eps) return 1;
    if (x < -eps) return -1;
    return 0;
}

inline bool equals(double a, double b, double eps = EPS) {
    return std::fabs(a - b) <= eps;
}

inline bool lessThan(double a, double b, double eps = EPS) {
    return a < b - eps;
}

inline bool lessOrEqual(double a, double b, double eps = EPS) {
    return a <= b + eps;
}

struct Point2D {
    double x = 0.0;
    double y = 0.0;

    Point2D() = default;
    Point2D(double xValue, double yValue) : x(xValue), y(yValue) {}

    Point2D operator+(const Point2D& other) const { return {x + other.x, y + other.y}; }
    Point2D operator-(const Point2D& other) const { return {x - other.x, y - other.y}; }
    Point2D operator*(double scalar) const { return {x * scalar, y * scalar}; }
    Point2D operator/(double scalar) const { return {x / scalar, y / scalar}; }

    bool operator==(const Point2D& other) const {
        return equals(x, other.x) && equals(y, other.y);
    }

    bool operator<(const Point2D& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

using Vector2D = Point2D;

inline Point2D operator*(double scalar, const Point2D& p) {
    return p * scalar;
}

inline bool equals(const Point2D& a, const Point2D& b, double eps = EPS) {
    return equals(a.x, b.x, eps) && equals(a.y, b.y, eps);
}

inline double dot(const Vector2D& a, const Vector2D& b) {
    return a.x * b.x + a.y * b.y;
}

inline double cross(const Vector2D& a, const Vector2D& b) {
    return a.x * b.y - a.y * b.x;
}

inline double cross(const Point2D& a, const Point2D& b, const Point2D& c) {
    return cross(b - a, c - a);
}

inline int orientation(const Point2D& a, const Point2D& b, const Point2D& c) {
    return sign(cross(a, b, c));
}

inline double squaredNorm(const Vector2D& a) {
    return dot(a, a);
}

inline double norm(const Vector2D& a) {
    return std::sqrt(squaredNorm(a));
}

inline double length(const Vector2D& a) {
    return norm(a);
}

inline double distance(const Point2D& a, const Point2D& b) {
    return norm(a - b);
}

inline Vector2D normalize(const Vector2D& a) {
    const double len = norm(a);
    return sign(len) == 0 ? Vector2D{0.0, 0.0} : a / len;
}

inline double angle(const Vector2D& a) {
    return std::atan2(a.y, a.x);
}

inline Vector2D rotate(const Vector2D& a, double theta) {
    const double c = std::cos(theta);
    const double s = std::sin(theta);
    return {a.x * c - a.y * s, a.x * s + a.y * c};
}

struct Line2D {
    Point2D p;
    Vector2D dir;

    Line2D() = default;
    Line2D(Point2D point, Vector2D direction) : p(point), dir(direction) {}

    static Line2D fromPoints(const Point2D& a, const Point2D& b) {
        return {a, b - a};
    }

    static Line2D fromABC(double a, double b, double c) {
        Point2D point;
        if (sign(b) != 0) {
            point = {0.0, -c / b};
        } else if (sign(a) != 0) {
            point = {-c / a, 0.0};
        }
        return {point, {b, -a}};
    }
};

inline bool parallel(const Line2D& a, const Line2D& b) {
    return sign(cross(a.dir, b.dir)) == 0;
}

inline bool sameLine(const Line2D& a, const Line2D& b) {
    return parallel(a, b) && sign(cross(a.dir, b.p - a.p)) == 0;
}

inline std::optional<Point2D> lineIntersection(const Line2D& a, const Line2D& b) {
    const double denom = cross(a.dir, b.dir);
    if (sign(denom) == 0) return std::nullopt;
    return a.p + a.dir * (cross(b.p - a.p, b.dir) / denom);
}

inline double distancePointToLine(const Point2D& p, const Line2D& line) {
    const double d = norm(line.dir);
    return sign(d) == 0 ? distance(p, line.p) : std::fabs(cross(line.dir, p - line.p)) / d;
}

inline Point2D projectionPointToLine(const Point2D& p, const Line2D& line) {
    const double denom = squaredNorm(line.dir);
    return sign(denom) == 0 ? line.p : line.p + line.dir * (dot(p - line.p, line.dir) / denom);
}

inline Point2D reflectionPointAcrossLine(const Point2D& p, const Line2D& line) {
    const Point2D projected = projectionPointToLine(p, line);
    return projected * 2.0 - p;
}

enum class IntersectionType {
    None,
    Point,
    Overlap
};

struct Segment2D {
    Point2D a;
    Point2D b;

    Segment2D() = default;
    Segment2D(Point2D start, Point2D end) : a(start), b(end) {}

    double length() const { return distance(a, b); }
    bool isPoint() const { return equals(a, b); }

    bool contains(const Point2D& p) const {
        return orientation(a, b, p) == 0 &&
               lessOrEqual(std::min(a.x, b.x), p.x) &&
               lessOrEqual(p.x, std::max(a.x, b.x)) &&
               lessOrEqual(std::min(a.y, b.y), p.y) &&
               lessOrEqual(p.y, std::max(a.y, b.y));
    }

    double distanceToPoint(const Point2D& p) const {
        const Vector2D ab = b - a;
        const double denom = squaredNorm(ab);
        if (sign(denom) == 0) return distance(p, a);
        const double t = std::max(0.0, std::min(1.0, dot(p - a, ab) / denom));
        return distance(p, a + ab * t);
    }
};

struct SegmentIntersectionResult {
    IntersectionType type = IntersectionType::None;
    std::optional<Point2D> point;
    std::optional<Segment2D> overlap;
};

inline bool pointEqualsByMode(const Point2D& a, const Point2D& b, PredicateMode mode) {
    return mode == PredicateMode::Eps ? equals(a, b) : (a.x == b.x && a.y == b.y);
}

inline bool segmentIsPointByMode(const Segment2D& s, PredicateMode mode) {
    return pointEqualsByMode(s.a, s.b, mode);
}

inline bool pointLessByMode(const Point2D& a, const Point2D& b, PredicateMode mode) {
    if (mode == PredicateMode::Eps && equals(a, b)) return false;
    if (a.x != b.x) return a.x < b.x;
    return a.y < b.y;
}

inline bool coordinateBetween(double lo, double value, double hi, PredicateMode mode) {
    const double minValue = std::min(lo, hi);
    const double maxValue = std::max(lo, hi);
    if (mode == PredicateMode::Eps) {
        return lessOrEqual(minValue, value) && lessOrEqual(value, maxValue);
    }
    return minValue <= value && value <= maxValue;
}

inline bool segmentContainsPoint(const Segment2D& s, const Point2D& p, PredicateMode mode) {
    if (mode == PredicateMode::Eps) return s.contains(p);
    return orient2d(s.a, s.b, p, mode) == 0 &&
           coordinateBetween(s.a.x, p.x, s.b.x, mode) &&
           coordinateBetween(s.a.y, p.y, s.b.y, mode);
}

inline std::optional<Point2D> lineIntersectionRaw(const Line2D& a, const Line2D& b) {
    const double denom = cross(a.dir, b.dir);
    if (denom == 0.0) return std::nullopt;
    return a.p + a.dir * (cross(b.p - a.p, b.dir) / denom);
}

inline SegmentIntersectionResult segmentIntersection(const Segment2D& s1, const Segment2D& s2, PredicateMode mode = PredicateMode::Eps) {
    SegmentIntersectionResult result;
    const bool s1Point = segmentIsPointByMode(s1, mode);
    const bool s2Point = segmentIsPointByMode(s2, mode);
    if (s1Point && s2Point) {
        if (pointEqualsByMode(s1.a, s2.a, mode)) {
            result.type = IntersectionType::Point;
            result.point = s1.a;
        }
        return result;
    }
    if (s1Point) {
        if (segmentContainsPoint(s2, s1.a, mode)) {
            result.type = IntersectionType::Point;
            result.point = s1.a;
        }
        return result;
    }
    if (s2Point) {
        if (segmentContainsPoint(s1, s2.a, mode)) {
            result.type = IntersectionType::Point;
            result.point = s2.a;
        }
        return result;
    }

    const int o1 = mode == PredicateMode::Eps ? orientation(s1.a, s1.b, s2.a) : orient2d(s1.a, s1.b, s2.a, mode);
    const int o2 = mode == PredicateMode::Eps ? orientation(s1.a, s1.b, s2.b) : orient2d(s1.a, s1.b, s2.b, mode);
    const int o3 = mode == PredicateMode::Eps ? orientation(s2.a, s2.b, s1.a) : orient2d(s2.a, s2.b, s1.a, mode);
    const int o4 = mode == PredicateMode::Eps ? orientation(s2.a, s2.b, s1.b) : orient2d(s2.a, s2.b, s1.b, mode);

    if (o1 == 0 && o2 == 0 && o3 == 0 && o4 == 0) {
        std::array<Point2D, 2> p1{s1.a, s1.b};
        std::array<Point2D, 2> p2{s2.a, s2.b};
        const auto lessPoint = [mode](const Point2D& a, const Point2D& b) {
            return pointLessByMode(a, b, mode);
        };
        std::sort(p1.begin(), p1.end(), lessPoint);
        std::sort(p2.begin(), p2.end(), lessPoint);
        const Point2D start = lessPoint(p1[0], p2[0]) ? p2[0] : p1[0];
        const Point2D end = lessPoint(p1[1], p2[1]) ? p1[1] : p2[1];
        if (lessPoint(end, start) && !pointEqualsByMode(start, end, mode)) return result;
        if (pointEqualsByMode(start, end, mode)) {
            result.type = IntersectionType::Point;
            result.point = start;
        } else {
            result.type = IntersectionType::Overlap;
            result.overlap = Segment2D{start, end};
        }
        return result;
    }

    if ((o1 == 0 && segmentContainsPoint(s1, s2.a, mode)) || (o2 == 0 && segmentContainsPoint(s1, s2.b, mode)) ||
        (o3 == 0 && segmentContainsPoint(s2, s1.a, mode)) || (o4 == 0 && segmentContainsPoint(s2, s1.b, mode)) ||
        (o1 * o2 < 0 && o3 * o4 < 0)) {
        result.type = IntersectionType::Point;
        const auto p = mode == PredicateMode::Eps
            ? lineIntersection(Line2D::fromPoints(s1.a, s1.b), Line2D::fromPoints(s2.a, s2.b))
            : lineIntersectionRaw(Line2D::fromPoints(s1.a, s1.b), Line2D::fromPoints(s2.a, s2.b));
        if (p.has_value()) {
            result.point = *p;
        } else if (segmentContainsPoint(s1, s2.a, mode)) {
            result.point = s2.a;
        } else if (segmentContainsPoint(s1, s2.b, mode)) {
            result.point = s2.b;
        } else if (segmentContainsPoint(s2, s1.a, mode)) {
            result.point = s1.a;
        } else if (segmentContainsPoint(s2, s1.b, mode)) {
            result.point = s1.b;
        }
    }
    return result;
}

inline bool segmentsIntersect(const Segment2D& a, const Segment2D& b) {
    return segmentIntersection(a, b).type != IntersectionType::None;
}

inline double distanceSegmentToSegment(const Segment2D& a, const Segment2D& b) {
    if (segmentsIntersect(a, b)) return 0.0;
    return std::min({a.distanceToPoint(b.a), a.distanceToPoint(b.b), b.distanceToPoint(a.a), b.distanceToPoint(a.b)});
}

struct Circle2D {
    Point2D center;
    double radius = 0.0;
};

struct Box2D {
    Point2D min;
    Point2D max;
};

struct Triangle2D {
    Point2D a;
    Point2D b;
    Point2D c;

    double signedArea() const { return cross(a, b, c) / 2.0; }
    double area() const { return std::fabs(signedArea()); }
};

enum class PointInPolygonResult {
    Outside,
    Inside,
    OnBoundary
};

struct Polygon2D {
    std::vector<Point2D> vertices;

    Polygon2D() = default;
    explicit Polygon2D(std::vector<Point2D> points) : vertices(std::move(points)) {}

    double signedArea() const {
        if (vertices.size() < 3) return 0.0;
        double sum = 0.0;
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            sum += cross(vertices[i], vertices[(i + 1) % vertices.size()]);
        }
        return sum / 2.0;
    }

    double area() const { return std::fabs(signedArea()); }
    int orientation() const { return sign(signedArea()); }
    bool isClockwise() const { return orientation() < 0; }
    bool isCounterClockwise() const { return orientation() > 0; }

    std::vector<Segment2D> edges() const {
        std::vector<Segment2D> result;
        if (vertices.size() < 2) return result;
        result.reserve(vertices.size());
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            result.push_back({vertices[i], vertices[(i + 1) % vertices.size()]});
        }
        return result;
    }

    PointInPolygonResult containsPoint(const Point2D& p) const {
        if (vertices.empty()) return PointInPolygonResult::Outside;
        bool inside = false;
        for (std::size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
            const Point2D& a = vertices[j];
            const Point2D& b = vertices[i];
            if (Segment2D{a, b}.contains(p)) return PointInPolygonResult::OnBoundary;
            if ((a.y > p.y) != (b.y > p.y)) {
                const double xAtY = a.x + (p.y - a.y) * (b.x - a.x) / (b.y - a.y);
                if (xAtY > p.x + EPS) inside = !inside;
            }
        }
        return inside ? PointInPolygonResult::Inside : PointInPolygonResult::Outside;
    }

    bool isConvex() const {
        if (vertices.size() < 3) return false;
        int direction = 0;
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            const int turn = geokernel::orientation(vertices[i], vertices[(i + 1) % vertices.size()], vertices[(i + 2) % vertices.size()]);
            if (turn == 0) continue;
            if (direction == 0) direction = turn;
            else if (direction != turn) return false;
        }
        return direction != 0;
    }
};

struct HalfPlane2D {
    Point2D p;
    Vector2D dir;

    HalfPlane2D() = default;
    HalfPlane2D(Point2D point, Vector2D direction) : p(point), dir(direction) {}

    bool inside(const Point2D& q) const { return sign(cross(dir, q - p)) >= 0; }
    std::optional<Point2D> intersection(const Line2D& line) const { return lineIntersection(Line2D{p, dir}, line); }
    double angle() const { return geokernel::angle(dir); }
};

inline std::vector<Point2D> sortPointsLexicographically(std::vector<Point2D> points) {
    std::sort(points.begin(), points.end());
    return points;
}

inline std::vector<Point2D> removeDuplicatePoints(std::vector<Point2D> points) {
    std::sort(points.begin(), points.end());
    std::vector<Point2D> unique;
    for (const auto& p : points) {
        if (unique.empty() || !equals(unique.back(), p)) unique.push_back(p);
    }
    return unique;
}

inline std::vector<Point2D> removeConsecutiveDuplicatePoints(const std::vector<Point2D>& points) {
    std::vector<Point2D> result;
    for (const auto& p : points) {
        if (result.empty() || !equals(result.back(), p)) result.push_back(p);
    }
    if (result.size() > 1 && equals(result.front(), result.back())) result.pop_back();
    return result;
}

inline std::vector<Point2D> removeCollinearVertices(const std::vector<Point2D>& points) {
    std::vector<Point2D> cleaned = removeConsecutiveDuplicatePoints(points);
    bool changed = true;
    while (changed && cleaned.size() >= 3) {
        changed = false;
        std::vector<Point2D> next;
        for (std::size_t i = 0; i < cleaned.size(); ++i) {
            const Point2D& prev = cleaned[(i + cleaned.size() - 1) % cleaned.size()];
            const Point2D& curr = cleaned[i];
            const Point2D& nxt = cleaned[(i + 1) % cleaned.size()];
            if (orientation(prev, curr, nxt) == 0 && Segment2D{prev, nxt}.contains(curr)) {
                changed = true;
                continue;
            }
            next.push_back(curr);
        }
        cleaned = next;
    }
    return cleaned;
}

inline Polygon2D ensureCounterClockwise(Polygon2D polygon) {
    if (polygon.isClockwise()) std::reverse(polygon.vertices.begin(), polygon.vertices.end());
    return polygon;
}

inline Polygon2D normalizePolygon(Polygon2D polygon) {
    polygon.vertices = removeCollinearVertices(polygon.vertices);
    return ensureCounterClockwise(std::move(polygon));
}

struct TraceStep {
    int step_index = 0;
    std::string phase;
    std::string message;
    std::vector<Point2D> points;
    std::vector<Segment2D> segments;
    std::vector<Polygon2D> polygons;
    std::map<std::string, double> metrics;
};

struct AlgorithmOptions {
    bool trace = false;
};

struct ConvexHullOptions : AlgorithmOptions {
    bool keepCollinear = false;
};

struct ConvexHullResult {
    std::vector<Point2D> hull;
    double area = 0.0;
    double perimeter = 0.0;
    std::vector<TraceStep> trace;
    std::vector<std::string> warnings;
};

inline double polygonPerimeter(const std::vector<Point2D>& poly) {
    if (poly.size() < 2) return 0.0;
    double sum = 0.0;
    for (std::size_t i = 0; i < poly.size(); ++i) sum += distance(poly[i], poly[(i + 1) % poly.size()]);
    return sum;
}

inline bool allCollinear(const std::vector<Point2D>& points) {
    if (points.size() < 3) return true;
    for (std::size_t i = 2; i < points.size(); ++i) {
        if (orientation(points[0], points[1], points[i]) != 0) return false;
    }
    return true;
}

inline ConvexHullResult convexHullAndrew(const std::vector<Point2D>& input, const ConvexHullOptions& options = {}) {
    ConvexHullResult result;
    std::vector<Point2D> points = removeDuplicatePoints(input);
    int step = 0;
    if (options.trace) {
        result.trace.push_back({step++, "sort_unique", "Sorted input points and removed duplicates.", points, {}, {}, {{"input_count", static_cast<double>(input.size())}, {"unique_count", static_cast<double>(points.size())}}});
    }
    if (points.size() <= 2) {
        result.hull = points;
        result.perimeter = points.size() == 2 ? 2.0 * distance(points[0], points[1]) : 0.0;
        return result;
    }
    if (allCollinear(points)) {
        result.warnings.push_back("all_points_collinear");
        result.hull = options.keepCollinear ? points : std::vector<Point2D>{points.front(), points.back()};
        result.perimeter = result.hull.size() == 2 ? 2.0 * distance(result.hull[0], result.hull[1]) : polygonPerimeter(result.hull);
        if (options.trace) result.trace.push_back({step++, "degenerate_collinear", "All unique points are collinear.", result.hull, {}, {}, {}});
        return result;
    }

    std::vector<Point2D> lower;
    for (const auto& p : points) {
        while (lower.size() >= 2) {
            const int turn = orientation(lower[lower.size() - 2], lower.back(), p);
            if (options.keepCollinear ? turn < 0 : turn <= 0) {
                if (options.trace) result.trace.push_back({step++, "lower_pop", "Removed non-left turn from lower hull.", {lower.back(), p}, {}, {}, {{"turn", static_cast<double>(turn)}}});
                lower.pop_back();
            } else {
                break;
            }
        }
        lower.push_back(p);
        if (options.trace) result.trace.push_back({step++, "lower_push", "Added point to lower hull.", lower, {}, {}, {}});
    }

    std::vector<Point2D> upper;
    for (auto it = points.rbegin(); it != points.rend(); ++it) {
        while (upper.size() >= 2) {
            const int turn = orientation(upper[upper.size() - 2], upper.back(), *it);
            if (options.keepCollinear ? turn < 0 : turn <= 0) {
                if (options.trace) result.trace.push_back({step++, "upper_pop", "Removed non-left turn from upper hull.", {upper.back(), *it}, {}, {}, {{"turn", static_cast<double>(turn)}}});
                upper.pop_back();
            } else {
                break;
            }
        }
        upper.push_back(*it);
        if (options.trace) result.trace.push_back({step++, "upper_push", "Added point to upper hull.", upper, {}, {}, {}});
    }

    lower.pop_back();
    upper.pop_back();
    result.hull = lower;
    result.hull.insert(result.hull.end(), upper.begin(), upper.end());
    result.area = Polygon2D{result.hull}.area();
    result.perimeter = polygonPerimeter(result.hull);
    if (options.trace) result.trace.push_back({step++, "complete", "Convex hull completed.", result.hull, {}, {Polygon2D{result.hull}}, {{"area", result.area}, {"perimeter", result.perimeter}}});
    return result;
}

inline ConvexHullResult convexHullGraham(const std::vector<Point2D>& points, const ConvexHullOptions& options = {}) {
    return convexHullAndrew(points, options);
}

struct DiameterResult {
    Point2D p1;
    Point2D p2;
    double distance = 0.0;
    std::vector<TraceStep> trace;
};

struct MinAreaRectangleResult {
    std::array<Point2D, 4> corners{};
    double area = 0.0;
    double width = 0.0;
    double height = 0.0;
    double angle = 0.0;
    std::vector<TraceStep> trace;
};

inline DiameterResult convexDiameter(const std::vector<Point2D>& hull, const AlgorithmOptions& options = {}) {
    DiameterResult result;
    int step = 0;
    for (std::size_t i = 0; i < hull.size(); ++i) {
        for (std::size_t j = i + 1; j < hull.size(); ++j) {
            const double d = distance(hull[i], hull[j]);
            if (d > result.distance) {
                result.p1 = hull[i];
                result.p2 = hull[j];
                result.distance = d;
                if (options.trace) result.trace.push_back({step++, "diameter_update", "Updated farthest pair.", {hull[i], hull[j]}, {Segment2D{hull[i], hull[j]}}, {}, {{"distance", d}}});
            }
        }
    }
    return result;
}

inline MinAreaRectangleResult minimumAreaBoundingRectangle(const std::vector<Point2D>& hull, const AlgorithmOptions& options = {}) {
    MinAreaRectangleResult best;
    best.area = std::numeric_limits<double>::infinity();
    int step = 0;
    if (hull.empty()) {
        best.area = 0.0;
        return best;
    }
    if (hull.size() == 1) {
        best.corners = {hull[0], hull[0], hull[0], hull[0]};
        best.area = 0.0;
        return best;
    }
    for (std::size_t i = 0; i < hull.size(); ++i) {
        const Vector2D u = normalize(hull[(i + 1) % hull.size()] - hull[i]);
        const Vector2D v{-u.y, u.x};
        double minU = std::numeric_limits<double>::infinity();
        double maxU = -std::numeric_limits<double>::infinity();
        double minV = std::numeric_limits<double>::infinity();
        double maxV = -std::numeric_limits<double>::infinity();
        for (const auto& p : hull) {
            const double pu = dot(p, u);
            const double pv = dot(p, v);
            minU = std::min(minU, pu);
            maxU = std::max(maxU, pu);
            minV = std::min(minV, pv);
            maxV = std::max(maxV, pv);
        }
        const double width = maxU - minU;
        const double height = maxV - minV;
        const double areaValue = width * height;
        if (options.trace) best.trace.push_back({step++, "rectangle_candidate", "Evaluated edge-aligned rectangle.", {}, {Segment2D{hull[i], hull[(i + 1) % hull.size()]}}, {}, {{"area", areaValue}, {"width", width}, {"height", height}}});
        if (areaValue < best.area) {
            best.area = areaValue;
            best.width = width;
            best.height = height;
            best.angle = geokernel::angle(u);
            best.corners = {u * minU + v * minV, u * maxU + v * minV, u * maxU + v * maxV, u * minU + v * maxV};
        }
    }
    if (options.trace) best.trace.push_back({step++, "minimum_rectangle", "Selected minimum-area rectangle.", {}, {}, {Polygon2D{std::vector<Point2D>{best.corners.begin(), best.corners.end()}}}, {{"area", best.area}, {"width", best.width}, {"height", best.height}}});
    return best;
}

struct SegmentIntersectionPair {
    int first = -1;
    int second = -1;
    SegmentIntersectionResult intersection;
};

struct SegmentIntersectionSearchResult {
    bool hasIntersection = false;
    std::vector<SegmentIntersectionPair> pairs;
    std::vector<TraceStep> trace;
    std::vector<std::string> warnings;
    std::size_t eventCount = 0;
    std::string implementation = "unknown";
    PredicateMode predicateMode = PredicateMode::FilteredExact;
};

inline void sortSegmentIntersectionPairs(std::vector<SegmentIntersectionPair>& pairs) {
    std::sort(pairs.begin(), pairs.end(), [](const SegmentIntersectionPair& a, const SegmentIntersectionPair& b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });
}

inline SegmentIntersectionSearchResult bruteForceSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options = {},
    PredicateMode mode = PredicateMode::FilteredExact) {
    SegmentIntersectionSearchResult result;
    result.implementation = "brute_force";
    result.predicateMode = mode;
    int step = 0;
    result.eventCount = segments.size() * (segments.size() > 0 ? segments.size() - 1 : 0) / 2;
    for (std::size_t i = 0; i < segments.size(); ++i) {
        if (segmentIsPointByMode(segments[i], mode)) result.warnings.push_back("zero_length_segment");
        for (std::size_t j = i + 1; j < segments.size(); ++j) {
            const auto inter = segmentIntersection(segments[i], segments[j], mode);
            if (options.trace) result.trace.push_back({step++, "candidate_check", "Checked brute-force segment pair.", {}, {segments[i], segments[j]}, {}, {{"first", static_cast<double>(i)}, {"second", static_cast<double>(j)}, {"intersects", inter.type == IntersectionType::None ? 0.0 : 1.0}}});
            if (inter.type != IntersectionType::None) {
                result.hasIntersection = true;
                result.pairs.push_back({static_cast<int>(i), static_cast<int>(j), inter});
            }
        }
    }
    sortSegmentIntersectionPairs(result.pairs);
    return result;
}

inline SegmentIntersectionSearchResult sweepLineSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options = {},
    PredicateMode mode = PredicateMode::FilteredExact) {
    SegmentIntersectionSearchResult result;
    result.implementation = "sweep_line_active_set";
    result.predicateMode = mode;
    int step = 0;

    struct Event {
        double x = 0.0;
        double y = 0.0;
        int segment = -1;
        bool start = true;
    };

    std::vector<Event> events;
    events.reserve(segments.size() * 2);
    auto endpointLess = [](const Point2D& a, const Point2D& b) {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    };
    for (std::size_t i = 0; i < segments.size(); ++i) {
        if (segmentIsPointByMode(segments[i], mode)) result.warnings.push_back("zero_length_segment");
        Point2D left = segments[i].a;
        Point2D right = segments[i].b;
        if (endpointLess(right, left)) std::swap(left, right);
        events.push_back({left.x, left.y, static_cast<int>(i), true});
        events.push_back({right.x, right.y, static_cast<int>(i), false});
    }
    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        if (a.x != b.x) return a.x < b.x;
        if (a.start != b.start) return a.start && !b.start;
        if (a.y != b.y) return a.y < b.y;
        return a.segment < b.segment;
    });
    result.eventCount = events.size();

    if (options.trace) {
        std::vector<Point2D> eventPoints;
        eventPoints.reserve(events.size());
        for (const auto& event : events) {
            eventPoints.push_back({event.x, event.y});
        }
        result.trace.push_back({step++, "events_sorted", "Prepared sweep-line endpoint events.", eventPoints, segments, {}, {{"event_count", static_cast<double>(events.size())}}});
    }

    std::vector<int> active;
    std::set<std::pair<int, int>> checked;
    std::set<std::pair<int, int>> reported;

    auto checkPair = [&](int a, int b) {
        if (a == b) return;
        if (a > b) std::swap(a, b);
        const auto key = std::make_pair(a, b);
        if (!checked.insert(key).second) return;
        const auto inter = segmentIntersection(segments[a], segments[b], mode);
        if (options.trace) result.trace.push_back({step++, "candidate_check", "Checked active sweep-line segment pair.", {}, {segments[a], segments[b]}, {}, {{"first", static_cast<double>(a)}, {"second", static_cast<double>(b)}, {"intersects", inter.type == IntersectionType::None ? 0.0 : 1.0}}});
        if (inter.type != IntersectionType::None && reported.insert(key).second) {
            result.hasIntersection = true;
            result.pairs.push_back({a, b, inter});
        }
    };

    for (std::size_t i = 0; i < events.size();) {
        const double sweepX = events[i].x;
        std::vector<int> starting;
        std::vector<int> ending;
        while (i < events.size() && events[i].x == sweepX) {
            if (events[i].start) starting.push_back(events[i].segment);
            else ending.push_back(events[i].segment);
            ++i;
        }

        for (std::size_t a = 0; a < starting.size(); ++a) {
            for (int activeSegment : active) {
                checkPair(starting[a], activeSegment);
            }
            for (std::size_t b = 0; b < a; ++b) {
                checkPair(starting[a], starting[b]);
            }
        }

        for (int segment : starting) {
            if (std::find(active.begin(), active.end(), segment) == active.end()) {
                active.push_back(segment);
            }
        }

        for (int segment : ending) {
            active.erase(std::remove(active.begin(), active.end(), segment), active.end());
        }

        if (options.trace) {
            result.trace.push_back({step++, "sweep_x_complete", "Processed all endpoint events at one x-coordinate.", {}, {}, {}, {{"x", sweepX}, {"active_count", static_cast<double>(active.size())}}});
        }
    }

    sortSegmentIntersectionPairs(result.pairs);
    return result;
}

inline SegmentIntersectionSearchResult findSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options = {},
    PredicateMode mode = PredicateMode::FilteredExact) {
    return sweepLineSegmentIntersections(segments, options, mode);
}

inline bool hasAnySegmentIntersection(const std::vector<Segment2D>& segments) {
    return findSegmentIntersections(segments).hasIntersection;
}

enum class PolygonClipStatus {
    Empty,
    Polygon,
    Degenerate
};

struct PolygonClipResult {
    PolygonClipStatus status = PolygonClipStatus::Empty;
    Polygon2D polygon;
    std::vector<TraceStep> trace;
    std::vector<std::string> warnings;
};

inline std::vector<Point2D> clipPointsByHalfPlane(const std::vector<Point2D>& subject, const HalfPlane2D& hp) {
    std::vector<Point2D> output;
    if (subject.empty()) return output;
    for (std::size_t i = 0; i < subject.size(); ++i) {
        const Point2D current = subject[i];
        const Point2D previous = subject[(i + subject.size() - 1) % subject.size()];
        const bool currentInside = hp.inside(current);
        const bool previousInside = hp.inside(previous);
        if (currentInside) {
            if (!previousInside) {
                auto p = lineIntersection(Line2D::fromPoints(previous, current), Line2D{hp.p, hp.dir});
                if (p.has_value()) output.push_back(*p);
            }
            output.push_back(current);
        } else if (previousInside) {
            auto p = lineIntersection(Line2D::fromPoints(previous, current), Line2D{hp.p, hp.dir});
            if (p.has_value()) output.push_back(*p);
        }
    }
    return removeConsecutiveDuplicatePoints(output);
}

inline PolygonClipResult sutherlandHodgmanClip(const Polygon2D& subject, const Polygon2D& clipper, const AlgorithmOptions& options = {}) {
    PolygonClipResult result;
    Polygon2D normalizedClipper = normalizePolygon(clipper);
    if (!normalizedClipper.isConvex()) result.warnings.push_back("clipper_is_not_convex");
    std::vector<Point2D> output = removeConsecutiveDuplicatePoints(subject.vertices);
    int step = 0;
    for (const auto& edge : normalizedClipper.edges()) {
        output = clipPointsByHalfPlane(output, HalfPlane2D{edge.a, edge.b - edge.a});
        if (options.trace) result.trace.push_back({step++, "clip_edge", "Clipped subject polygon by one clipper edge.", {}, {edge}, {Polygon2D{output}}, {{"vertex_count", static_cast<double>(output.size())}}});
        if (output.empty()) break;
    }
    result.polygon = Polygon2D{removeConsecutiveDuplicatePoints(output)};
    if (result.polygon.vertices.size() < 3 || sign(result.polygon.area()) == 0) {
        result.status = result.polygon.vertices.empty() ? PolygonClipStatus::Empty : PolygonClipStatus::Degenerate;
    } else {
        result.status = PolygonClipStatus::Polygon;
    }
    return result;
}

enum class HalfPlaneIntersectionStatus {
    Empty,
    BoundedPolygon,
    Unbounded,
    Degenerate
};

struct HalfPlaneIntersectionOptions : AlgorithmOptions {
    double boundingLimit = 1000000.0;
    bool useBoundingBox = true;
};

struct HalfPlaneIntersectionResult {
    HalfPlaneIntersectionStatus status = HalfPlaneIntersectionStatus::Empty;
    Polygon2D polygon;
    bool clippedByBoundingBox = false;
    std::vector<TraceStep> trace;
    std::vector<std::string> warnings;
};

inline HalfPlaneIntersectionResult halfPlaneIntersection(const std::vector<HalfPlane2D>& halfPlanes, const HalfPlaneIntersectionOptions& options = {}) {
    HalfPlaneIntersectionResult result;
    const double b = options.boundingLimit;
    std::vector<Point2D> poly{{-b, -b}, {b, -b}, {b, b}, {-b, b}};
    int step = 0;
    result.clippedByBoundingBox = options.useBoundingBox;
    for (const auto& hp : halfPlanes) {
        poly = clipPointsByHalfPlane(poly, hp);
        if (options.trace) result.trace.push_back({step++, "halfplane_clip", "Clipped current region by half-plane.", {}, {Segment2D{hp.p, hp.p + hp.dir}}, {Polygon2D{poly}}, {{"vertex_count", static_cast<double>(poly.size())}}});
        if (poly.empty()) break;
    }
    result.polygon = Polygon2D{removeConsecutiveDuplicatePoints(poly)};
    if (result.polygon.vertices.empty()) {
        result.status = HalfPlaneIntersectionStatus::Empty;
        return result;
    }
    if (result.polygon.vertices.size() < 3 || sign(result.polygon.area()) == 0) {
        result.status = HalfPlaneIntersectionStatus::Degenerate;
        return result;
    }
    bool touchesBox = false;
    for (const auto& p : result.polygon.vertices) {
        if (equals(std::fabs(p.x), b, 1e-6) || equals(std::fabs(p.y), b, 1e-6)) {
            touchesBox = true;
            break;
        }
    }
    result.status = touchesBox ? HalfPlaneIntersectionStatus::Unbounded : HalfPlaneIntersectionStatus::BoundedPolygon;
    if (touchesBox) result.warnings.push_back("intersection_clipped_by_bounding_box");
    return result;
}

struct ClosestPairResult {
    bool valid = false;
    Point2D p1;
    Point2D p2;
    double distance = std::numeric_limits<double>::infinity();
    std::vector<TraceStep> trace;
    std::vector<std::string> warnings;
};

inline ClosestPairResult closestPair(const std::vector<Point2D>& input, const AlgorithmOptions& options = {}) {
    ClosestPairResult result;
    if (input.size() < 2) {
        result.warnings.push_back("fewer_than_two_points");
        return result;
    }
    std::vector<Point2D> points = input;
    std::sort(points.begin(), points.end());
    for (std::size_t i = 1; i < points.size(); ++i) {
        if (equals(points[i], points[i - 1])) {
            result.valid = true;
            result.p1 = points[i - 1];
            result.p2 = points[i];
            result.distance = 0.0;
            result.warnings.push_back("duplicate_points");
            if (options.trace) result.trace.push_back({0, "duplicate_detected", "Duplicate points give zero closest distance.", {points[i - 1], points[i]}, {}, {}, {{"distance", 0.0}}});
            return result;
        }
    }

    std::vector<TraceStep> trace;
    int step = 0;
    auto chooseBetter = [](const ClosestPairResult& a, const ClosestPairResult& b) {
        return a.distance <= b.distance ? a : b;
    };
    std::function<ClosestPairResult(std::vector<Point2D>&, int)> solve = [&](std::vector<Point2D>& pts, int depth) -> ClosestPairResult {
        ClosestPairResult local;
        if (pts.size() <= 3) {
            for (std::size_t i = 0; i < pts.size(); ++i) {
                for (std::size_t j = i + 1; j < pts.size(); ++j) {
                    const double d = distance(pts[i], pts[j]);
                    if (d < local.distance) {
                        local.valid = true;
                        local.p1 = pts[i];
                        local.p2 = pts[j];
                        local.distance = d;
                    }
                }
            }
            return local;
        }
        const std::size_t mid = pts.size() / 2;
        const double midX = pts[mid].x;
        std::vector<Point2D> left(pts.begin(), pts.begin() + static_cast<std::ptrdiff_t>(mid));
        std::vector<Point2D> right(pts.begin() + static_cast<std::ptrdiff_t>(mid), pts.end());
        if (options.trace) trace.push_back({step++, "divide", "Split point set for closest-pair recursion.", pts, {}, {}, {{"depth", static_cast<double>(depth)}, {"mid_x", midX}}});
        ClosestPairResult best = chooseBetter(solve(left, depth + 1), solve(right, depth + 1));
        std::vector<Point2D> strip;
        for (const auto& p : pts) {
            if (std::fabs(p.x - midX) < best.distance) strip.push_back(p);
        }
        std::sort(strip.begin(), strip.end(), [](const Point2D& a, const Point2D& b) {
            if (!equals(a.y, b.y)) return a.y < b.y;
            return a.x < b.x;
        });
        for (std::size_t i = 0; i < strip.size(); ++i) {
            for (std::size_t j = i + 1; j < strip.size() && strip[j].y - strip[i].y < best.distance; ++j) {
                const double d = distance(strip[i], strip[j]);
                if (d < best.distance) {
                    best.valid = true;
                    best.p1 = strip[i];
                    best.p2 = strip[j];
                    best.distance = d;
                }
            }
        }
        if (options.trace) trace.push_back({step++, "merge_strip", "Checked cross-boundary strip.", strip, {}, {}, {{"depth", static_cast<double>(depth)}, {"best_distance", best.distance}}});
        return best;
    };

    result = solve(points, 0);
    result.trace = trace;
    return result;
}

inline bool pointInTriangle(const Point2D& p, const Triangle2D& t) {
    const int o1 = orientation(t.a, t.b, p);
    const int o2 = orientation(t.b, t.c, p);
    const int o3 = orientation(t.c, t.a, p);
    return (o1 >= 0 && o2 >= 0 && o3 >= 0) || (o1 <= 0 && o2 <= 0 && o3 <= 0);
}

inline bool isSimplePolygon(const Polygon2D& polygon) {
    const auto edges = polygon.edges();
    for (std::size_t i = 0; i < edges.size(); ++i) {
        for (std::size_t j = i + 1; j < edges.size(); ++j) {
            if (j == i + 1 || (i == 0 && j + 1 == edges.size())) continue;
            if (segmentsIntersect(edges[i], edges[j])) return false;
        }
    }
    return true;
}

struct TriangulationResult {
    std::vector<Triangle2D> triangles;
    bool valid = false;
    double polygonArea = 0.0;
    double trianglesArea = 0.0;
    double areaError = 0.0;
    std::vector<TraceStep> trace;
    std::vector<std::string> warnings;
};

inline TriangulationResult triangulateEarClipping(const Polygon2D& polygon, const AlgorithmOptions& options = {}) {
    TriangulationResult result;
    Polygon2D poly = normalizePolygon(polygon);
    result.polygonArea = poly.area();
    int step = 0;
    if (poly.vertices.size() < 3) {
        result.warnings.push_back("fewer_than_three_vertices");
        return result;
    }
    if (!isSimplePolygon(poly)) {
        result.warnings.push_back("polygon_is_not_simple");
        return result;
    }
    std::vector<int> idx(poly.vertices.size());
    std::iota(idx.begin(), idx.end(), 0);
    auto isEar = [&](std::size_t pos) {
        const int prev = idx[(pos + idx.size() - 1) % idx.size()];
        const int curr = idx[pos];
        const int next = idx[(pos + 1) % idx.size()];
        if (orientation(poly.vertices[prev], poly.vertices[curr], poly.vertices[next]) <= 0) return false;
        Triangle2D tri{poly.vertices[prev], poly.vertices[curr], poly.vertices[next]};
        for (const int id : idx) {
            if (id == prev || id == curr || id == next) continue;
            if (pointInTriangle(poly.vertices[id], tri)) return false;
        }
        return true;
    };
    while (idx.size() > 3) {
        bool clipped = false;
        for (std::size_t pos = 0; pos < idx.size(); ++pos) {
            if (isEar(pos)) {
                const int prev = idx[(pos + idx.size() - 1) % idx.size()];
                const int curr = idx[pos];
                const int next = idx[(pos + 1) % idx.size()];
                Triangle2D tri{poly.vertices[prev], poly.vertices[curr], poly.vertices[next]};
                result.triangles.push_back(tri);
                result.trianglesArea += tri.area();
                if (options.trace) result.trace.push_back({step++, "clip_ear", "Clipped one ear from polygon.", {tri.a, tri.b, tri.c}, {}, {Polygon2D{{tri.a, tri.b, tri.c}}}, {{"remaining_vertices", static_cast<double>(idx.size() - 1)}}});
                idx.erase(idx.begin() + static_cast<std::ptrdiff_t>(pos));
                clipped = true;
                break;
            }
        }
        if (!clipped) {
            result.warnings.push_back("ear_detection_failed");
            break;
        }
    }
    if (idx.size() == 3) {
        Triangle2D tri{poly.vertices[idx[0]], poly.vertices[idx[1]], poly.vertices[idx[2]]};
        result.triangles.push_back(tri);
        result.trianglesArea += tri.area();
        if (options.trace) result.trace.push_back({step++, "final_triangle", "Added final triangle.", {tri.a, tri.b, tri.c}, {}, {Polygon2D{{tri.a, tri.b, tri.c}}}, {}});
    }
    result.areaError = std::fabs(result.polygonArea - result.trianglesArea);
    result.valid = result.triangles.size() + 2 == poly.vertices.size() && result.areaError <= 1e-7;
    return result;
}

struct DelaunayResult {
    std::vector<Triangle2D> triangles;
    bool experimental = true;
    std::vector<TraceStep> trace;
    std::vector<std::string> warnings;
};

inline bool circumcircleContains(const Triangle2D& t, const Point2D& p) {
    const double ax = t.a.x - p.x;
    const double ay = t.a.y - p.y;
    const double bx = t.b.x - p.x;
    const double by = t.b.y - p.y;
    const double cx = t.c.x - p.x;
    const double cy = t.c.y - p.y;
    const double det = (ax * ax + ay * ay) * (bx * cy - cx * by) -
                       (bx * bx + by * by) * (ax * cy - cx * ay) +
                       (cx * cx + cy * cy) * (ax * by - bx * ay);
    return cross(t.a, t.b, t.c) > 0 ? det > EPS : det < -EPS;
}

inline DelaunayResult delaunayTriangulation(const std::vector<Point2D>& input, const AlgorithmOptions& options = {}) {
    DelaunayResult result;
    std::vector<Point2D> points = removeDuplicatePoints(input);
    if (points.size() < 3) {
        result.warnings.push_back("fewer_than_three_unique_points");
        return result;
    }
    double minX = points[0].x, maxX = points[0].x, minY = points[0].y, maxY = points[0].y;
    for (const auto& p : points) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }
    const double delta = std::max(maxX - minX, maxY - minY) + 1.0;
    const double midX = (minX + maxX) / 2.0;
    const double midY = (minY + maxY) / 2.0;
    const Point2D s1{midX - 20.0 * delta, midY - delta};
    const Point2D s2{midX, midY + 20.0 * delta};
    const Point2D s3{midX + 20.0 * delta, midY - delta};
    std::vector<Triangle2D> triangles{{s1, s2, s3}};
    int step = 0;
    auto sameEdge = [](const Segment2D& a, const Segment2D& b) {
        return (equals(a.a, b.a) && equals(a.b, b.b)) || (equals(a.a, b.b) && equals(a.b, b.a));
    };
    for (const auto& p : points) {
        std::vector<Triangle2D> bad;
        std::vector<Triangle2D> kept;
        for (const auto& tri : triangles) {
            if (circumcircleContains(tri, p)) bad.push_back(tri);
            else kept.push_back(tri);
        }
        std::vector<Segment2D> edges;
        for (const auto& tri : bad) {
            edges.push_back({tri.a, tri.b});
            edges.push_back({tri.b, tri.c});
            edges.push_back({tri.c, tri.a});
        }
        std::vector<Segment2D> boundary;
        for (std::size_t i = 0; i < edges.size(); ++i) {
            bool shared = false;
            for (std::size_t j = 0; j < edges.size(); ++j) {
                if (i != j && sameEdge(edges[i], edges[j])) {
                    shared = true;
                    break;
                }
            }
            if (!shared) boundary.push_back(edges[i]);
        }
        triangles = kept;
        for (const auto& e : boundary) {
            Triangle2D tri{e.a, e.b, p};
            if (tri.signedArea() < 0) std::swap(tri.a, tri.b);
            triangles.push_back(tri);
        }
        if (options.trace) result.trace.push_back({step++, "insert_point", "Inserted point and rebuilt Delaunay cavity.", {p}, boundary, {}, {{"bad_triangles", static_cast<double>(bad.size())}, {"boundary_edges", static_cast<double>(boundary.size())}}});
    }
    for (const auto& tri : triangles) {
        if (equals(tri.a, s1) || equals(tri.a, s2) || equals(tri.a, s3) ||
            equals(tri.b, s1) || equals(tri.b, s2) || equals(tri.b, s3) ||
            equals(tri.c, s1) || equals(tri.c, s2) || equals(tri.c, s3)) {
            continue;
        }
        result.triangles.push_back(tri);
    }
    return result;
}

}  // namespace geokernel
