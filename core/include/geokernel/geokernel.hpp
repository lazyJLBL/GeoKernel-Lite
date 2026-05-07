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

// TODO(architecture): The implementation still lives in this umbrella header for
// source compatibility. New module headers under geokernel/core, geokernel/trace,
// geokernel/io, and geokernel/algorithm expose stable include paths while the
// implementation is gradually split in future milestones.

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

struct PredicateContext {
    PredicateMode mode = PredicateMode::FilteredExact;
    double eps = EPS;

    int orient(const Point2D& a, const Point2D& b, const Point2D& c) const {
        switch (mode) {
            case PredicateMode::Eps: return orient2dEps(a, b, c, eps);
            case PredicateMode::FilteredExact: return orient2dFiltered(a, b, c);
            case PredicateMode::Exact: return orient2dExact(a, b, c);
        }
        return orient2dFiltered(a, b, c);
    }

    int incircle(const Point2D& a, const Point2D& b, const Point2D& c, const Point2D& d) const {
        switch (mode) {
            case PredicateMode::Eps: return incircleEps(a, b, c, d, eps);
            case PredicateMode::FilteredExact: return incircleFiltered(a, b, c, d);
            case PredicateMode::Exact: return incircleExact(a, b, c, d);
        }
        return incircleFiltered(a, b, c, d);
    }

    bool equals(const Point2D& a, const Point2D& b) const {
        if (mode == PredicateMode::Eps) return geokernel::equals(a, b, eps);
        return a.x == b.x && a.y == b.y;
    }

    int compareLexicographic(const Point2D& a, const Point2D& b) const {
        if (equals(a, b)) return 0;
        if (a.x < b.x || (a.x == b.x && a.y < b.y)) return -1;
        return 1;
    }
};

inline PredicateContext epsPredicateContext(double eps = EPS) {
    return {PredicateMode::Eps, eps};
}

inline PredicateContext filteredExactPredicateContext(double eps = EPS) {
    return {PredicateMode::FilteredExact, eps};
}

inline PredicateContext exactPredicateContext(double eps = EPS) {
    return {PredicateMode::Exact, eps};
}

inline PredicateContext predicateContextFromMode(PredicateMode mode, double eps = EPS) {
    return {mode, eps};
}

inline PredicateContext predicateContextFromString(const std::string& value, double eps = EPS) {
    return predicateContextFromMode(predicateModeFromString(value), eps);
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

inline bool pointEqualsByContext(const Point2D& a, const Point2D& b, const PredicateContext& predicates) {
    return predicates.equals(a, b);
}

inline bool segmentIsPointByMode(const Segment2D& s, PredicateMode mode) {
    return pointEqualsByMode(s.a, s.b, mode);
}

inline bool segmentIsPointByContext(const Segment2D& s, const PredicateContext& predicates) {
    return predicates.equals(s.a, s.b);
}

inline bool pointLessByMode(const Point2D& a, const Point2D& b, PredicateMode mode) {
    if (mode == PredicateMode::Eps && equals(a, b)) return false;
    if (a.x != b.x) return a.x < b.x;
    return a.y < b.y;
}

inline bool pointLessByContext(const Point2D& a, const Point2D& b, const PredicateContext& predicates) {
    return predicates.compareLexicographic(a, b) < 0;
}

inline bool coordinateBetween(double lo, double value, double hi, PredicateMode mode) {
    const double minValue = std::min(lo, hi);
    const double maxValue = std::max(lo, hi);
    if (mode == PredicateMode::Eps) {
        return lessOrEqual(minValue, value) && lessOrEqual(value, maxValue);
    }
    return minValue <= value && value <= maxValue;
}

inline bool coordinateBetween(double lo, double value, double hi, const PredicateContext& predicates) {
    const double minValue = std::min(lo, hi);
    const double maxValue = std::max(lo, hi);
    if (predicates.mode == PredicateMode::Eps) {
        return lessOrEqual(minValue, value, predicates.eps) && lessOrEqual(value, maxValue, predicates.eps);
    }
    return minValue <= value && value <= maxValue;
}

inline bool segmentContainsPoint(const Segment2D& s, const Point2D& p, PredicateMode mode) {
    if (mode == PredicateMode::Eps) return s.contains(p);
    return orient2d(s.a, s.b, p, mode) == 0 &&
           coordinateBetween(s.a.x, p.x, s.b.x, mode) &&
           coordinateBetween(s.a.y, p.y, s.b.y, mode);
}

inline bool segmentContainsPoint(const Segment2D& s, const Point2D& p, const PredicateContext& predicates) {
    return predicates.orient(s.a, s.b, p) == 0 &&
           coordinateBetween(s.a.x, p.x, s.b.x, predicates) &&
           coordinateBetween(s.a.y, p.y, s.b.y, predicates);
}

inline std::optional<Point2D> lineIntersectionRaw(const Line2D& a, const Line2D& b) {
    const double denom = cross(a.dir, b.dir);
    if (denom == 0.0) return std::nullopt;
    return a.p + a.dir * (cross(b.p - a.p, b.dir) / denom);
}

inline SegmentIntersectionResult segmentIntersection(const Segment2D& s1, const Segment2D& s2, const PredicateContext& predicates) {
    SegmentIntersectionResult result;
    const bool s1Point = segmentIsPointByContext(s1, predicates);
    const bool s2Point = segmentIsPointByContext(s2, predicates);
    if (s1Point && s2Point) {
        if (pointEqualsByContext(s1.a, s2.a, predicates)) {
            result.type = IntersectionType::Point;
            result.point = s1.a;
        }
        return result;
    }
    if (s1Point) {
        if (segmentContainsPoint(s2, s1.a, predicates)) {
            result.type = IntersectionType::Point;
            result.point = s1.a;
        }
        return result;
    }
    if (s2Point) {
        if (segmentContainsPoint(s1, s2.a, predicates)) {
            result.type = IntersectionType::Point;
            result.point = s2.a;
        }
        return result;
    }

    const int o1 = predicates.orient(s1.a, s1.b, s2.a);
    const int o2 = predicates.orient(s1.a, s1.b, s2.b);
    const int o3 = predicates.orient(s2.a, s2.b, s1.a);
    const int o4 = predicates.orient(s2.a, s2.b, s1.b);

    if (o1 == 0 && o2 == 0 && o3 == 0 && o4 == 0) {
        std::array<Point2D, 2> p1{s1.a, s1.b};
        std::array<Point2D, 2> p2{s2.a, s2.b};
        const auto lessPoint = [&predicates](const Point2D& a, const Point2D& b) {
            return pointLessByContext(a, b, predicates);
        };
        std::sort(p1.begin(), p1.end(), lessPoint);
        std::sort(p2.begin(), p2.end(), lessPoint);
        const Point2D start = lessPoint(p1[0], p2[0]) ? p2[0] : p1[0];
        const Point2D end = lessPoint(p1[1], p2[1]) ? p1[1] : p2[1];
        if (lessPoint(end, start) && !pointEqualsByContext(start, end, predicates)) return result;
        if (pointEqualsByContext(start, end, predicates)) {
            result.type = IntersectionType::Point;
            result.point = start;
        } else {
            result.type = IntersectionType::Overlap;
            result.overlap = Segment2D{start, end};
        }
        return result;
    }

    if ((o1 == 0 && segmentContainsPoint(s1, s2.a, predicates)) || (o2 == 0 && segmentContainsPoint(s1, s2.b, predicates)) ||
        (o3 == 0 && segmentContainsPoint(s2, s1.a, predicates)) || (o4 == 0 && segmentContainsPoint(s2, s1.b, predicates)) ||
        (o1 * o2 < 0 && o3 * o4 < 0)) {
        result.type = IntersectionType::Point;
        const auto p = predicates.mode == PredicateMode::Eps
            ? lineIntersection(Line2D::fromPoints(s1.a, s1.b), Line2D::fromPoints(s2.a, s2.b))
            : lineIntersectionRaw(Line2D::fromPoints(s1.a, s1.b), Line2D::fromPoints(s2.a, s2.b));
        if (p.has_value()) {
            result.point = *p;
        } else if (segmentContainsPoint(s1, s2.a, predicates)) {
            result.point = s2.a;
        } else if (segmentContainsPoint(s1, s2.b, predicates)) {
            result.point = s2.b;
        } else if (segmentContainsPoint(s2, s1.a, predicates)) {
            result.point = s1.a;
        } else if (segmentContainsPoint(s2, s1.b, predicates)) {
            result.point = s1.b;
        }
    }
    return result;
}

inline SegmentIntersectionResult segmentIntersection(const Segment2D& s1, const Segment2D& s2, PredicateMode mode = PredicateMode::Eps) {
    return segmentIntersection(s1, s2, predicateContextFromMode(mode));
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
        return containsPoint(p, epsPredicateContext());
    }

    PointInPolygonResult containsPoint(const Point2D& p, const PredicateContext& predicates) const {
        if (vertices.empty()) return PointInPolygonResult::Outside;
        bool inside = false;
        for (std::size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
            const Point2D& a = vertices[j];
            const Point2D& b = vertices[i];
            if (segmentContainsPoint(Segment2D{a, b}, p, predicates)) return PointInPolygonResult::OnBoundary;
            if ((a.y > p.y) != (b.y > p.y)) {
                const double xAtY = a.x + (p.y - a.y) * (b.x - a.x) / (b.y - a.y);
                if (xAtY > p.x + predicates.eps) inside = !inside;
            }
        }
        return inside ? PointInPolygonResult::Inside : PointInPolygonResult::Outside;
    }

    bool isConvex() const {
        return isConvex(epsPredicateContext());
    }

    bool isConvex(const PredicateContext& predicates) const {
        if (vertices.size() < 3) return false;
        int direction = 0;
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            const int turn = predicates.orient(vertices[i], vertices[(i + 1) % vertices.size()], vertices[(i + 2) % vertices.size()]);
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
    bool inside(const Point2D& q, const PredicateContext& predicates) const { return predicates.orient(p, p + dir, q) >= 0; }
    std::optional<Point2D> intersection(const Line2D& line) const { return lineIntersection(Line2D{p, dir}, line); }
    double angle() const { return geokernel::angle(dir); }
};

inline std::vector<Point2D> removeDuplicatePoints(std::vector<Point2D> points, const PredicateContext& predicates);
inline std::vector<Point2D> removeConsecutiveDuplicatePoints(const std::vector<Point2D>& points, const PredicateContext& predicates);
inline std::vector<Point2D> removeCollinearVertices(const std::vector<Point2D>& points, const PredicateContext& predicates);
inline Polygon2D normalizePolygon(Polygon2D polygon, const PredicateContext& predicates);

inline std::vector<Point2D> sortPointsLexicographically(std::vector<Point2D> points) {
    std::sort(points.begin(), points.end());
    return points;
}

inline std::vector<Point2D> removeDuplicatePoints(std::vector<Point2D> points) {
    return removeDuplicatePoints(std::move(points), epsPredicateContext());
}

inline std::vector<Point2D> removeDuplicatePoints(std::vector<Point2D> points, const PredicateContext& predicates) {
    std::sort(points.begin(), points.end());
    std::vector<Point2D> unique;
    for (const auto& p : points) {
        if (unique.empty() || !predicates.equals(unique.back(), p)) unique.push_back(p);
    }
    return unique;
}

inline std::vector<Point2D> removeConsecutiveDuplicatePoints(const std::vector<Point2D>& points) {
    return removeConsecutiveDuplicatePoints(points, epsPredicateContext());
}

inline std::vector<Point2D> removeConsecutiveDuplicatePoints(const std::vector<Point2D>& points, const PredicateContext& predicates) {
    std::vector<Point2D> result;
    for (const auto& p : points) {
        if (result.empty() || !predicates.equals(result.back(), p)) result.push_back(p);
    }
    if (result.size() > 1 && predicates.equals(result.front(), result.back())) result.pop_back();
    return result;
}

inline std::vector<Point2D> removeCollinearVertices(const std::vector<Point2D>& points) {
    return removeCollinearVertices(points, epsPredicateContext());
}

inline std::vector<Point2D> removeCollinearVertices(const std::vector<Point2D>& points, const PredicateContext& predicates) {
    std::vector<Point2D> cleaned = removeConsecutiveDuplicatePoints(points, predicates);
    bool changed = true;
    while (changed && cleaned.size() >= 3) {
        changed = false;
        std::vector<Point2D> next;
        for (std::size_t i = 0; i < cleaned.size(); ++i) {
            const Point2D& prev = cleaned[(i + cleaned.size() - 1) % cleaned.size()];
            const Point2D& curr = cleaned[i];
            const Point2D& nxt = cleaned[(i + 1) % cleaned.size()];
            if (predicates.orient(prev, curr, nxt) == 0 && segmentContainsPoint(Segment2D{prev, nxt}, curr, predicates)) {
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
    return normalizePolygon(std::move(polygon), epsPredicateContext());
}

inline Polygon2D normalizePolygon(Polygon2D polygon, const PredicateContext& predicates) {
    polygon.vertices = removeCollinearVertices(polygon.vertices, predicates);
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

using Trace = std::vector<TraceStep>;

struct AlgorithmOptions {
    bool trace = false;
    PredicateContext predicates = filteredExactPredicateContext();
};

struct Ring2D {
    std::vector<Point2D> vertices;
};

struct PolygonWithHoles2D {
    Ring2D outer;
    std::vector<Ring2D> holes;
};

struct MultiPolygon2D {
    std::vector<PolygonWithHoles2D> polygons;
};

enum class FillRule {
    EvenOdd,
    NonZero
};

enum class BooleanOp {
    Union,
    Intersection,
    Difference,
    Xor
};

enum class RingOrientation {
    Clockwise,
    CounterClockwise,
    Degenerate
};

enum class BoundaryLocation {
    Outside,
    Inside,
    OnBoundary
};

struct GeometryValidationReport {
    bool valid = true;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

struct PolygonBooleanOptions {
    PredicateContext predicates = filteredExactPredicateContext();
    FillRule fillRule = FillRule::EvenOdd;
    bool normalizeInput = true;
    bool validateInput = true;
    bool validateOutput = true;
    bool trace = false;
};

struct PolygonBooleanResult {
    MultiPolygon2D result;
    bool valid = false;
    std::vector<std::string> warnings;
    Trace trace;
    GeometryValidationReport inputValidation;
    GeometryValidationReport outputValidation;
};

inline const char* fillRuleName(FillRule rule) {
    switch (rule) {
        case FillRule::EvenOdd: return "even_odd";
        case FillRule::NonZero: return "non_zero";
    }
    return "unknown";
}

inline FillRule fillRuleFromString(const std::string& value) {
    if (value == "even_odd" || value == "evenodd") return FillRule::EvenOdd;
    if (value == "non_zero" || value == "nonzero") return FillRule::NonZero;
    throw std::invalid_argument("unknown fill rule: " + value);
}

inline const char* booleanOpName(BooleanOp op) {
    switch (op) {
        case BooleanOp::Union: return "union";
        case BooleanOp::Intersection: return "intersection";
        case BooleanOp::Difference: return "difference";
        case BooleanOp::Xor: return "xor";
    }
    return "unknown";
}

inline BooleanOp booleanOpFromString(const std::string& value) {
    if (value == "union") return BooleanOp::Union;
    if (value == "intersection" || value == "intersect") return BooleanOp::Intersection;
    if (value == "difference" || value == "diff") return BooleanOp::Difference;
    if (value == "xor" || value == "symmetric_difference") return BooleanOp::Xor;
    throw std::invalid_argument("unknown polygon boolean operation: " + value);
}

inline const char* ringOrientationName(RingOrientation orientation) {
    switch (orientation) {
        case RingOrientation::Clockwise: return "clockwise";
        case RingOrientation::CounterClockwise: return "counter_clockwise";
        case RingOrientation::Degenerate: return "degenerate";
    }
    return "unknown";
}

inline const char* boundaryLocationName(BoundaryLocation location) {
    switch (location) {
        case BoundaryLocation::Outside: return "outside";
        case BoundaryLocation::Inside: return "inside";
        case BoundaryLocation::OnBoundary: return "on_boundary";
    }
    return "unknown";
}

inline void addValidationError(GeometryValidationReport& report, const std::string& error) {
    report.valid = false;
    report.errors.push_back(error);
}

inline void appendValidationReport(
    GeometryValidationReport& target,
    const GeometryValidationReport& source,
    const std::string& prefix) {
    if (!source.valid) target.valid = false;
    for (const auto& error : source.errors) target.errors.push_back(prefix + error);
    for (const auto& warning : source.warnings) target.warnings.push_back(prefix + warning);
}

inline Ring2D removeClosingDuplicate(Ring2D ring, const PredicateContext& predicates = filteredExactPredicateContext()) {
    if (ring.vertices.size() > 1 && predicates.equals(ring.vertices.front(), ring.vertices.back())) {
        ring.vertices.pop_back();
    }
    return ring;
}

inline Ring2D removeConsecutiveDuplicates(Ring2D ring, const PredicateContext& predicates = filteredExactPredicateContext()) {
    ring.vertices = removeConsecutiveDuplicatePoints(ring.vertices, predicates);
    return ring;
}

inline Ring2D removeCollinearVertices(Ring2D ring, const PredicateContext& predicates = filteredExactPredicateContext()) {
    ring.vertices = geokernel::removeCollinearVertices(ring.vertices, predicates);
    return ring;
}

inline double signedRingArea(const Ring2D& ring) {
    return Polygon2D{ring.vertices}.signedArea();
}

inline double ringArea(const Ring2D& ring) {
    return std::fabs(signedRingArea(ring));
}

inline RingOrientation ringOrientation(const Ring2D& ring, const PredicateContext& predicates = filteredExactPredicateContext()) {
    const int areaSign = sign(signedRingArea(ring), predicates.eps);
    if (areaSign > 0) return RingOrientation::CounterClockwise;
    if (areaSign < 0) return RingOrientation::Clockwise;
    return RingOrientation::Degenerate;
}

inline std::vector<Segment2D> ringSegments(const Ring2D& ring) {
    return Polygon2D{ring.vertices}.edges();
}

inline Box2D ringBoundingBox(const Ring2D& ring) {
    Box2D box;
    if (ring.vertices.empty()) return box;
    box.min = ring.vertices.front();
    box.max = ring.vertices.front();
    for (const auto& p : ring.vertices) {
        box.min.x = std::min(box.min.x, p.x);
        box.min.y = std::min(box.min.y, p.y);
        box.max.x = std::max(box.max.x, p.x);
        box.max.y = std::max(box.max.y, p.y);
    }
    return box;
}

inline bool boxesOverlap(const Box2D& a, const Box2D& b, const PredicateContext& predicates = filteredExactPredicateContext()) {
    if (predicates.mode == PredicateMode::Eps) {
        return lessOrEqual(a.min.x, b.max.x, predicates.eps) &&
               lessOrEqual(b.min.x, a.max.x, predicates.eps) &&
               lessOrEqual(a.min.y, b.max.y, predicates.eps) &&
               lessOrEqual(b.min.y, a.max.y, predicates.eps);
    }
    return a.min.x <= b.max.x && b.min.x <= a.max.x && a.min.y <= b.max.y && b.min.y <= a.max.y;
}

inline Ring2D normalizeRing(
    Ring2D ring,
    const PredicateContext& predicates = filteredExactPredicateContext(),
    RingOrientation desired = RingOrientation::CounterClockwise) {
    ring = removeClosingDuplicate(std::move(ring), predicates);
    ring = removeConsecutiveDuplicates(std::move(ring), predicates);
    ring = removeCollinearVertices(std::move(ring), predicates);
    const RingOrientation orientation = ringOrientation(ring, predicates);
    if (desired == RingOrientation::CounterClockwise && orientation == RingOrientation::Clockwise) {
        std::reverse(ring.vertices.begin(), ring.vertices.end());
    } else if (desired == RingOrientation::Clockwise && orientation == RingOrientation::CounterClockwise) {
        std::reverse(ring.vertices.begin(), ring.vertices.end());
    }
    return ring;
}

inline PolygonWithHoles2D normalizePolygonWithHoles(
    PolygonWithHoles2D polygon,
    const PredicateContext& predicates = filteredExactPredicateContext()) {
    polygon.outer = normalizeRing(std::move(polygon.outer), predicates, RingOrientation::CounterClockwise);
    for (auto& hole : polygon.holes) {
        hole = normalizeRing(std::move(hole), predicates, RingOrientation::Clockwise);
    }
    return polygon;
}

inline MultiPolygon2D normalizeMultiPolygon(
    MultiPolygon2D multipolygon,
    const PredicateContext& predicates = filteredExactPredicateContext()) {
    for (auto& polygon : multipolygon.polygons) {
        polygon = normalizePolygonWithHoles(std::move(polygon), predicates);
    }
    return multipolygon;
}

inline BoundaryLocation pointInRing(
    const Ring2D& ring,
    const Point2D& point,
    const PredicateContext& predicates = filteredExactPredicateContext()) {
    const auto location = Polygon2D{ring.vertices}.containsPoint(point, predicates);
    switch (location) {
        case PointInPolygonResult::Inside: return BoundaryLocation::Inside;
        case PointInPolygonResult::OnBoundary: return BoundaryLocation::OnBoundary;
        case PointInPolygonResult::Outside: return BoundaryLocation::Outside;
    }
    return BoundaryLocation::Outside;
}

inline bool ringSelfIntersects(const Ring2D& ring, const PredicateContext& predicates = filteredExactPredicateContext()) {
    const auto edges = ringSegments(ring);
    if (edges.size() < 4) return false;
    for (std::size_t i = 0; i < edges.size(); ++i) {
        for (std::size_t j = i + 1; j < edges.size(); ++j) {
            const bool adjacent = j == i + 1 || (i == 0 && j + 1 == edges.size());
            if (adjacent) continue;
            if (segmentIntersection(edges[i], edges[j], predicates).type != IntersectionType::None) {
                return true;
            }
        }
    }
    return false;
}

inline bool ringsIntersect(
    const Ring2D& a,
    const Ring2D& b,
    const PredicateContext& predicates = filteredExactPredicateContext()) {
    if (!boxesOverlap(ringBoundingBox(a), ringBoundingBox(b), predicates)) return false;
    const auto aEdges = ringSegments(a);
    const auto bEdges = ringSegments(b);
    for (const auto& ae : aEdges) {
        for (const auto& be : bEdges) {
            if (segmentIntersection(ae, be, predicates).type != IntersectionType::None) return true;
        }
    }
    return false;
}

inline BoundaryLocation pointInPolygonWithHoles(
    const PolygonWithHoles2D& polygon,
    const Point2D& point,
    const PredicateContext& predicates = filteredExactPredicateContext()) {
    const BoundaryLocation outer = pointInRing(polygon.outer, point, predicates);
    if (outer != BoundaryLocation::Inside) return outer;
    for (const auto& hole : polygon.holes) {
        const BoundaryLocation inHole = pointInRing(hole, point, predicates);
        if (inHole == BoundaryLocation::OnBoundary) return BoundaryLocation::OnBoundary;
        if (inHole == BoundaryLocation::Inside) return BoundaryLocation::Outside;
    }
    return BoundaryLocation::Inside;
}

inline BoundaryLocation pointInMultiPolygon(
    const MultiPolygon2D& multipolygon,
    const Point2D& point,
    const PredicateContext& predicates = filteredExactPredicateContext()) {
    bool insideAny = false;
    for (const auto& polygon : multipolygon.polygons) {
        const BoundaryLocation location = pointInPolygonWithHoles(polygon, point, predicates);
        if (location == BoundaryLocation::OnBoundary) return BoundaryLocation::OnBoundary;
        if (location == BoundaryLocation::Inside) insideAny = true;
    }
    return insideAny ? BoundaryLocation::Inside : BoundaryLocation::Outside;
}

inline GeometryValidationReport validateRing(
    const Ring2D& ring,
    const PredicateContext& predicates = filteredExactPredicateContext()) {
    GeometryValidationReport report;
    Ring2D cleaned = removeClosingDuplicate(ring, predicates);
    cleaned = removeConsecutiveDuplicates(std::move(cleaned), predicates);
    cleaned = removeCollinearVertices(std::move(cleaned), predicates);
    if (cleaned.vertices.size() < 3) {
        addValidationError(report, "ring_has_fewer_than_three_vertices");
        return report;
    }
    if (ringOrientation(cleaned, predicates) == RingOrientation::Degenerate) {
        addValidationError(report, "ring_area_is_zero");
    }
    if (ringSelfIntersects(cleaned, predicates)) {
        addValidationError(report, "ring_self_intersects");
    }
    return report;
}

inline GeometryValidationReport validatePolygonWithHoles(
    const PolygonWithHoles2D& polygon,
    const PredicateContext& predicates = filteredExactPredicateContext()) {
    GeometryValidationReport report;
    appendValidationReport(report, validateRing(polygon.outer, predicates), "outer.");
    if (!report.valid) return report;

    for (std::size_t i = 0; i < polygon.holes.size(); ++i) {
        const std::string holePrefix = "hole_" + std::to_string(i) + ".";
        const auto holeReport = validateRing(polygon.holes[i], predicates);
        appendValidationReport(report, holeReport, holePrefix);
        if (!holeReport.valid) continue;
        if (ringsIntersect(polygon.outer, polygon.holes[i], predicates)) {
            addValidationError(report, holePrefix + "intersects_or_touches_outer_boundary");
        } else if (polygon.holes[i].vertices.empty() ||
                   pointInRing(polygon.outer, polygon.holes[i].vertices.front(), predicates) != BoundaryLocation::Inside) {
            addValidationError(report, holePrefix + "not_strictly_inside_outer");
        }
        for (std::size_t j = i + 1; j < polygon.holes.size(); ++j) {
            if (ringsIntersect(polygon.holes[i], polygon.holes[j], predicates)) {
                addValidationError(report, "holes_intersect_or_touch");
            } else if (!polygon.holes[i].vertices.empty() && !polygon.holes[j].vertices.empty()) {
                const bool iInsideJ = pointInRing(polygon.holes[j], polygon.holes[i].vertices.front(), predicates) == BoundaryLocation::Inside;
                const bool jInsideI = pointInRing(polygon.holes[i], polygon.holes[j].vertices.front(), predicates) == BoundaryLocation::Inside;
                if (iInsideJ || jInsideI) addValidationError(report, "holes_overlap_or_nested");
            }
        }
    }
    return report;
}

inline GeometryValidationReport validateMultiPolygon(
    const MultiPolygon2D& multipolygon,
    const PredicateContext& predicates = filteredExactPredicateContext()) {
    GeometryValidationReport report;
    if (multipolygon.polygons.empty()) {
        report.warnings.push_back("multipolygon_is_empty");
        return report;
    }
    for (std::size_t i = 0; i < multipolygon.polygons.size(); ++i) {
        appendValidationReport(report, validatePolygonWithHoles(multipolygon.polygons[i], predicates), "polygon_" + std::to_string(i) + ".");
    }
    for (std::size_t i = 0; i < multipolygon.polygons.size(); ++i) {
        for (std::size_t j = i + 1; j < multipolygon.polygons.size(); ++j) {
            if (ringsIntersect(multipolygon.polygons[i].outer, multipolygon.polygons[j].outer, predicates)) {
                addValidationError(report, "multipolygon_components_intersect_or_touch");
                continue;
            }
            if (!multipolygon.polygons[i].outer.vertices.empty() &&
                pointInPolygonWithHoles(multipolygon.polygons[j], multipolygon.polygons[i].outer.vertices.front(), predicates) == BoundaryLocation::Inside) {
                addValidationError(report, "multipolygon_components_overlap");
            }
            if (!multipolygon.polygons[j].outer.vertices.empty() &&
                pointInPolygonWithHoles(multipolygon.polygons[i], multipolygon.polygons[j].outer.vertices.front(), predicates) == BoundaryLocation::Inside) {
                addValidationError(report, "multipolygon_components_overlap");
            }
        }
    }
    return report;
}

inline double polygonWithHolesArea(const PolygonWithHoles2D& polygon) {
    double area = ringArea(polygon.outer);
    for (const auto& hole : polygon.holes) area -= ringArea(hole);
    return area;
}

inline double multiPolygonArea(const MultiPolygon2D& multipolygon) {
    double area = 0.0;
    for (const auto& polygon : multipolygon.polygons) area += polygonWithHolesArea(polygon);
    return area;
}

inline PolygonBooleanResult polygonBoolean(
    const MultiPolygon2D& subjectInput,
    const MultiPolygon2D& clipInput,
    BooleanOp operation,
    const PolygonBooleanOptions& options = {}) {
    PolygonBooleanResult result;
    MultiPolygon2D subject = options.normalizeInput ? normalizeMultiPolygon(subjectInput, options.predicates) : subjectInput;
    MultiPolygon2D clip = options.normalizeInput ? normalizeMultiPolygon(clipInput, options.predicates) : clipInput;
    int step = 0;
    if (options.trace) {
        std::vector<Polygon2D> polygons;
        for (const auto& polygon : subject.polygons) polygons.push_back(Polygon2D{polygon.outer.vertices});
        for (const auto& polygon : clip.polygons) polygons.push_back(Polygon2D{polygon.outer.vertices});
        result.trace.push_back({step++, "normalization", "Normalized polygon boolean inputs.", {}, {}, polygons, {{"subject_polygons", static_cast<double>(subject.polygons.size())}, {"clip_polygons", static_cast<double>(clip.polygons.size())}}});
    }

    if (options.validateInput) {
        appendValidationReport(result.inputValidation, validateMultiPolygon(subject, options.predicates), "subject.");
        appendValidationReport(result.inputValidation, validateMultiPolygon(clip, options.predicates), "clip.");
        if (options.trace) {
            result.trace.push_back({step++, "validation", "Validated polygon boolean inputs.", {}, {}, {}, {{"valid", result.inputValidation.valid ? 1.0 : 0.0}}});
        }
        if (!result.inputValidation.valid) {
            result.warnings.insert(result.warnings.end(), result.inputValidation.errors.begin(), result.inputValidation.errors.end());
            return result;
        }
    }

    (void)operation;
    result.valid = false;
    result.warnings.push_back("polygon_boolean_operation_not_implemented");
    if (options.trace) {
        result.trace.push_back({step++, "operation_not_implemented", "Polygon boolean overlay is not implemented in this stage.", {}, {}, {}, {}});
    }
    if (options.validateOutput) {
        result.outputValidation = validateMultiPolygon(result.result, options.predicates);
    }
    return result;
}

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

inline bool allCollinear(const std::vector<Point2D>& points, const PredicateContext& predicates);

inline bool allCollinear(const std::vector<Point2D>& points) {
    return allCollinear(points, epsPredicateContext());
}

inline bool allCollinear(const std::vector<Point2D>& points, const PredicateContext& predicates) {
    if (points.size() < 3) return true;
    for (std::size_t i = 2; i < points.size(); ++i) {
        if (predicates.orient(points[0], points[1], points[i]) != 0) return false;
    }
    return true;
}

inline ConvexHullResult convexHullAndrew(const std::vector<Point2D>& input, const ConvexHullOptions& options = {}) {
    ConvexHullResult result;
    const PredicateContext& predicates = options.predicates;
    std::vector<Point2D> points = removeDuplicatePoints(input, predicates);
    int step = 0;
    if (options.trace) {
        result.trace.push_back({step++, "sort_unique", "Sorted input points and removed duplicates.", points, {}, {}, {{"input_count", static_cast<double>(input.size())}, {"unique_count", static_cast<double>(points.size())}}});
    }
    if (points.size() <= 2) {
        result.hull = points;
        result.perimeter = points.size() == 2 ? 2.0 * distance(points[0], points[1]) : 0.0;
        return result;
    }
    if (allCollinear(points, predicates)) {
        result.warnings.push_back("all_points_collinear");
        result.hull = options.keepCollinear ? points : std::vector<Point2D>{points.front(), points.back()};
        result.perimeter = result.hull.size() == 2 ? 2.0 * distance(result.hull[0], result.hull[1]) : polygonPerimeter(result.hull);
        if (options.trace) result.trace.push_back({step++, "degenerate_collinear", "All unique points are collinear.", result.hull, {}, {}, {}});
        return result;
    }

    std::vector<Point2D> lower;
    for (const auto& p : points) {
        while (lower.size() >= 2) {
            const int turn = predicates.orient(lower[lower.size() - 2], lower.back(), p);
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
            const int turn = predicates.orient(upper[upper.size() - 2], upper.back(), *it);
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

inline DiameterResult bruteForceConvexDiameter(const std::vector<Point2D>& hull, const AlgorithmOptions& options = {}) {
    DiameterResult result;
    int step = 0;
    if (hull.empty()) return result;
    if (hull.size() == 1) {
        result.p1 = hull.front();
        result.p2 = hull.front();
        if (options.trace) result.trace.push_back({step++, "diameter_degenerate", "Single-point hull has zero diameter.", {hull.front()}, {}, {}, {{"distance", 0.0}}});
        return result;
    }
    for (std::size_t i = 0; i < hull.size(); ++i) {
        for (std::size_t j = i + 1; j < hull.size(); ++j) {
            const double d = distance(hull[i], hull[j]);
            if (d > result.distance) {
                result.p1 = hull[i];
                result.p2 = hull[j];
                result.distance = d;
                if (options.trace) result.trace.push_back({step++, "bruteforce_diameter_update", "Updated brute-force farthest pair.", {hull[i], hull[j]}, {Segment2D{hull[i], hull[j]}}, {}, {{"distance", d}}});
            }
        }
    }
    return result;
}

inline DiameterResult convexDiameter(const std::vector<Point2D>& hull, const AlgorithmOptions& options = {}) {
    DiameterResult result;
    int step = 0;
    if (hull.empty()) return result;
    if (hull.size() == 1) {
        result.p1 = hull.front();
        result.p2 = hull.front();
        if (options.trace) result.trace.push_back({step++, "diameter_degenerate", "Single-point hull has zero diameter.", {hull.front()}, {}, {}, {{"distance", 0.0}}});
        return result;
    }
    if (hull.size() == 2) {
        result.p1 = hull[0];
        result.p2 = hull[1];
        result.distance = distance(hull[0], hull[1]);
        if (options.trace) result.trace.push_back({step++, "diameter_segment", "Two-point hull diameter is the input segment.", hull, {Segment2D{hull[0], hull[1]}}, {}, {{"distance", result.distance}}});
        return result;
    }

    const PredicateContext& predicates = options.predicates;
    auto consider = [&](std::size_t i, std::size_t j, const std::string& phase) {
        const double squared = squaredNorm(hull[i] - hull[j]);
        if (squared > result.distance * result.distance) {
            result.p1 = hull[i];
            result.p2 = hull[j];
            result.distance = std::sqrt(squared);
            if (options.trace) {
                result.trace.push_back({step++, phase, "Updated rotating-calipers farthest pair.", {hull[i], hull[j]}, {Segment2D{hull[i], hull[j]}}, {}, {{"distance", result.distance}, {"i", static_cast<double>(i)}, {"j", static_cast<double>(j)}}});
            }
        }
    };

    if (allCollinear(hull, predicates)) {
        std::size_t first = 0;
        std::size_t last = 0;
        for (std::size_t i = 1; i < hull.size(); ++i) {
            if (pointLessByContext(hull[i], hull[first], predicates)) first = i;
            if (pointLessByContext(hull[last], hull[i], predicates)) last = i;
        }
        result.p1 = hull[first];
        result.p2 = hull[last];
        result.distance = distance(result.p1, result.p2);
        if (options.trace) result.trace.push_back({step++, "diameter_collinear", "Collinear hull diameter uses lexicographic endpoints.", {result.p1, result.p2}, {Segment2D{result.p1, result.p2}}, {}, {{"distance", result.distance}}});
        return result;
    }

    const std::size_t n = hull.size();
    std::size_t j = 1;
    auto twiceTriangleAreaAbs = [&](std::size_t i, std::size_t antipodal) {
        const std::size_t next = (i + 1) % n;
        return std::fabs(cross(hull[next] - hull[i], hull[antipodal] - hull[i]));
    };

    if (options.trace) result.trace.push_back({step++, "calipers_start", "Started O(h) rotating-calipers diameter scan.", hull, {}, {Polygon2D{hull}}, {{"hull_vertices", static_cast<double>(n)}}});
    for (std::size_t i = 0; i < n; ++i) {
        std::size_t guard = 0;
        while (guard < n) {
            const std::size_t nextJ = (j + 1) % n;
            const double currentArea = twiceTriangleAreaAbs(i, j);
            const double nextArea = twiceTriangleAreaAbs(i, nextJ);
            if (nextArea > currentArea + predicates.eps) {
                j = nextJ;
                ++guard;
            } else {
                break;
            }
        }
        consider(i, j, "diameter_update");
        consider((i + 1) % n, j, "diameter_update");
    }
    if (options.trace) result.trace.push_back({step++, "calipers_complete", "Completed rotating-calipers diameter scan.", {result.p1, result.p2}, {Segment2D{result.p1, result.p2}}, {}, {{"distance", result.distance}}});
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

struct SegmentIntersectionRecord {
    std::size_t first = 0;
    std::size_t second = 0;
    SegmentIntersectionResult intersection;
};

struct SplitSegment {
    Segment2D original;
    std::vector<Point2D> splitPoints;
    std::vector<Segment2D> atomicSegments;
};

struct ArrangementNode {
    Point2D point;
    std::vector<std::size_t> outgoingEdges;
};

struct ArrangementEdge {
    std::size_t from = 0;
    std::size_t to = 0;
    Segment2D segment;
    std::size_t sourceSegmentId = 0;
};

struct SegmentArrangementOptions : AlgorithmOptions {
    bool validate = true;
};

struct SegmentArrangementResult {
    std::vector<ArrangementNode> nodes;
    std::vector<ArrangementEdge> edges;
    std::vector<SplitSegment> splitSegments;
    std::vector<SegmentIntersectionRecord> intersections;
    bool valid = false;
    std::vector<std::string> warnings;
    Trace trace;
};

inline void sortSegmentIntersectionPairs(std::vector<SegmentIntersectionPair>& pairs) {
    std::sort(pairs.begin(), pairs.end(), [](const SegmentIntersectionPair& a, const SegmentIntersectionPair& b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });
}

inline SegmentIntersectionSearchResult bruteForceSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options,
    const PredicateContext& predicates);

inline SegmentIntersectionSearchResult sweepLineSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options,
    const PredicateContext& predicates);

inline SegmentIntersectionSearchResult bruteForceSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options = {}) {
    return bruteForceSegmentIntersections(segments, options, options.predicates);
}

inline SegmentIntersectionSearchResult bruteForceSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options,
    PredicateMode mode) {
    AlgorithmOptions adjusted = options;
    adjusted.predicates = predicateContextFromMode(mode, options.predicates.eps);
    return bruteForceSegmentIntersections(segments, adjusted, adjusted.predicates);
}

inline SegmentIntersectionSearchResult bruteForceSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options,
    const PredicateContext& predicates) {
    SegmentIntersectionSearchResult result;
    result.implementation = "brute_force";
    result.predicateMode = predicates.mode;
    int step = 0;
    result.eventCount = segments.size() * (segments.size() > 0 ? segments.size() - 1 : 0) / 2;
    for (std::size_t i = 0; i < segments.size(); ++i) {
        if (segmentIsPointByContext(segments[i], predicates)) result.warnings.push_back("zero_length_segment");
        for (std::size_t j = i + 1; j < segments.size(); ++j) {
            const auto inter = segmentIntersection(segments[i], segments[j], predicates);
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
    const AlgorithmOptions& options = {}) {
    return sweepLineSegmentIntersections(segments, options, options.predicates);
}

inline SegmentIntersectionSearchResult sweepLineSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options,
    PredicateMode mode) {
    AlgorithmOptions adjusted = options;
    adjusted.predicates = predicateContextFromMode(mode, options.predicates.eps);
    return sweepLineSegmentIntersections(segments, adjusted, adjusted.predicates);
}

inline SegmentIntersectionSearchResult sweepLineSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options,
    const PredicateContext& predicates) {
    SegmentIntersectionSearchResult result;
    result.implementation = "sweep_line_ordered_active_with_oracle_completion";
    result.predicateMode = predicates.mode;
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
        if (segmentIsPointByContext(segments[i], predicates)) result.warnings.push_back("zero_length_segment");
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

    auto segmentYAtX = [&](int index, double x) {
        const Segment2D& segment = segments[static_cast<std::size_t>(index)];
        if (segmentIsPointByContext(segment, predicates)) return segment.a.y;
        if (equals(segment.a.x, segment.b.x, predicates.eps)) return std::min(segment.a.y, segment.b.y);
        const double t = (x - segment.a.x) / (segment.b.x - segment.a.x);
        return segment.a.y + (segment.b.y - segment.a.y) * t;
    };

    auto segmentSlopeKey = [&](int index) {
        const Segment2D& segment = segments[static_cast<std::size_t>(index)];
        const double dx = segment.b.x - segment.a.x;
        if (equals(dx, 0.0, predicates.eps)) {
            return segment.b.y >= segment.a.y ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity();
        }
        return (segment.b.y - segment.a.y) / dx;
    };

    std::vector<int> active;
    auto sortActive = [&](double sweepX) {
        std::sort(active.begin(), active.end(), [&](int a, int b) {
            const double ya = segmentYAtX(a, sweepX);
            const double yb = segmentYAtX(b, sweepX);
            if (!equals(ya, yb, predicates.eps)) return ya < yb;
            const double sa = segmentSlopeKey(a);
            const double sb = segmentSlopeKey(b);
            if (!equals(sa, sb, predicates.eps)) return sa < sb;
            return a < b;
        });
    };

    if (options.trace) {
        std::vector<Point2D> eventPoints;
        eventPoints.reserve(events.size());
        for (const auto& event : events) {
            eventPoints.push_back({event.x, event.y});
        }
        result.trace.push_back({step++, "events_sorted", "Prepared sweep-line endpoint events.", eventPoints, segments, {}, {{"event_count", static_cast<double>(events.size())}}});
    }

    std::set<std::pair<int, int>> checked;
    std::set<std::pair<int, int>> reported;

    auto checkPair = [&](int a, int b) {
        if (a == b) return;
        if (a > b) std::swap(a, b);
        const auto key = std::make_pair(a, b);
        if (!checked.insert(key).second) return;
        const auto inter = segmentIntersection(segments[a], segments[b], predicates);
        if (options.trace) result.trace.push_back({step++, "candidate_check", "Checked active sweep-line segment pair.", {}, {segments[a], segments[b]}, {}, {{"first", static_cast<double>(a)}, {"second", static_cast<double>(b)}, {"intersects", inter.type == IntersectionType::None ? 0.0 : 1.0}}});
        if (inter.type != IntersectionType::None && reported.insert(key).second) {
            result.hasIntersection = true;
            result.pairs.push_back({a, b, inter});
        }
    };

    auto insertActive = [&](int segment) {
        if (std::find(active.begin(), active.end(), segment) == active.end()) {
            active.push_back(segment);
        }
    };

    auto checkNeighborsAround = [&](int segment, double sweepX) {
        sortActive(sweepX);
        auto it = std::find(active.begin(), active.end(), segment);
        if (it == active.end()) return;
        const std::size_t pos = static_cast<std::size_t>(std::distance(active.begin(), it));
        if (pos > 0) checkPair(active[pos - 1], segment);
        if (pos + 1 < active.size()) checkPair(segment, active[pos + 1]);
    };

    auto segmentIsVerticalAtX = [&](int segment, double sweepX) {
        const Segment2D& s = segments[static_cast<std::size_t>(segment)];
        return equals(s.a.x, s.b.x, predicates.eps) && equals(s.a.x, sweepX, predicates.eps);
    };

    auto verticalCoversY = [&](int segment, double y) {
        const Segment2D& s = segments[static_cast<std::size_t>(segment)];
        return coordinateBetween(s.a.y, y, s.b.y, predicates);
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

        std::vector<int> eventSegments = starting;
        eventSegments.insert(eventSegments.end(), ending.begin(), ending.end());
        std::sort(eventSegments.begin(), eventSegments.end());
        eventSegments.erase(std::unique(eventSegments.begin(), eventSegments.end()), eventSegments.end());
        for (std::size_t a = 0; a < eventSegments.size(); ++a) {
            for (std::size_t b = a + 1; b < eventSegments.size(); ++b) {
                checkPair(eventSegments[a], eventSegments[b]);
            }
        }

        for (int segment : starting) {
            insertActive(segment);
            checkNeighborsAround(segment, sweepX);
        }

        sortActive(sweepX);
        for (int segment : eventSegments) {
            if (!segmentIsVerticalAtX(segment, sweepX)) continue;
            for (int activeSegment : active) {
                if (activeSegment == segment) continue;
                const double y = segmentYAtX(activeSegment, sweepX);
                if (verticalCoversY(segment, y)) {
                    checkPair(segment, activeSegment);
                }
            }
        }

        for (int segment : ending) {
            checkNeighborsAround(segment, sweepX);
            active.erase(std::remove(active.begin(), active.end(), segment), active.end());
        }

        if (options.trace) {
            result.trace.push_back({step++, "sweep_x_complete", "Processed all endpoint events at one x-coordinate.", {}, {}, {}, {{"x", sweepX}, {"active_count", static_cast<double>(active.size())}}});
        }
    }

    AlgorithmOptions oracleOptions = options;
    oracleOptions.trace = false;
    const auto oracle = bruteForceSegmentIntersections(segments, oracleOptions, predicates);
    std::size_t oracleCompletionCount = 0;
    for (const auto& pair : oracle.pairs) {
        const auto key = std::make_pair(pair.first, pair.second);
        if (reported.insert(key).second) {
            result.hasIntersection = true;
            result.pairs.push_back(pair);
            ++oracleCompletionCount;
        }
    }
    if (oracleCompletionCount > 0) {
        result.warnings.push_back("sweep_line_oracle_completion_added_pairs");
        if (options.trace) {
            result.trace.push_back({step++, "oracle_completion", "Added pairs missed by endpoint-neighbor sweep to preserve the all-intersections contract.", {}, {}, {}, {{"added_pairs", static_cast<double>(oracleCompletionCount)}}});
        }
    }

    sortSegmentIntersectionPairs(result.pairs);
    return result;
}

inline SegmentIntersectionSearchResult findSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options = {}) {
    return sweepLineSegmentIntersections(segments, options, options.predicates);
}

inline SegmentIntersectionSearchResult findSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options,
    PredicateMode mode) {
    return sweepLineSegmentIntersections(segments, options, mode);
}

inline SegmentIntersectionSearchResult findSegmentIntersections(
    const std::vector<Segment2D>& segments,
    const AlgorithmOptions& options,
    const PredicateContext& predicates) {
    return sweepLineSegmentIntersections(segments, options, predicates);
}

inline bool hasAnySegmentIntersection(const std::vector<Segment2D>& segments) {
    return findSegmentIntersections(segments).hasIntersection;
}

inline double segmentParameter(const Segment2D& segment, const Point2D& point) {
    const Vector2D ab = segment.b - segment.a;
    const double denom = squaredNorm(ab);
    if (denom == 0.0) return 0.0;
    return dot(point - segment.a, ab) / denom;
}

inline void addSplitPoint(
    std::vector<Point2D>& points,
    const Point2D& point,
    const PredicateContext& predicates) {
    for (const auto& existing : points) {
        if (predicates.equals(existing, point)) return;
    }
    points.push_back(point);
}

inline bool segmentEndpointsEqualUndirected(
    const Segment2D& a,
    const Segment2D& b,
    const PredicateContext& predicates) {
    return (predicates.equals(a.a, b.a) && predicates.equals(a.b, b.b)) ||
           (predicates.equals(a.a, b.b) && predicates.equals(a.b, b.a));
}

inline bool pointIsSegmentEndpoint(
    const Segment2D& segment,
    const Point2D& point,
    const PredicateContext& predicates) {
    return predicates.equals(segment.a, point) || predicates.equals(segment.b, point);
}

inline std::size_t arrangementNodeId(
    std::vector<ArrangementNode>& nodes,
    const Point2D& point,
    const PredicateContext& predicates) {
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (predicates.equals(nodes[i].point, point)) return i;
    }
    nodes.push_back({point, {}});
    return nodes.size() - 1;
}

inline bool validateArrangementAtomicSegments(
    const std::vector<ArrangementEdge>& edges,
    const PredicateContext& predicates,
    std::vector<std::string>& warnings) {
    bool valid = true;
    for (std::size_t i = 0; i < edges.size(); ++i) {
        for (std::size_t j = i + 1; j < edges.size(); ++j) {
            const auto intersection = segmentIntersection(edges[i].segment, edges[j].segment, predicates);
            if (intersection.type == IntersectionType::None) continue;
            if (intersection.type == IntersectionType::Point && intersection.point.has_value()) {
                const bool endpointOnFirst = pointIsSegmentEndpoint(edges[i].segment, *intersection.point, predicates);
                const bool endpointOnSecond = pointIsSegmentEndpoint(edges[j].segment, *intersection.point, predicates);
                if (!endpointOnFirst || !endpointOnSecond) {
                    warnings.push_back("atomic_segments_have_internal_point_intersection");
                    valid = false;
                }
            } else if (intersection.type == IntersectionType::Overlap && intersection.overlap.has_value()) {
                if (!segmentEndpointsEqualUndirected(edges[i].segment, edges[j].segment, predicates)) {
                    warnings.push_back("atomic_segments_have_internal_overlap");
                    valid = false;
                }
            }
        }
    }
    return valid;
}

inline SegmentArrangementResult buildSegmentArrangement(
    const std::vector<Segment2D>& segments,
    const SegmentArrangementOptions& options = {}) {
    SegmentArrangementResult result;
    const PredicateContext& predicates = options.predicates;
    result.splitSegments.reserve(segments.size());
    std::vector<std::vector<Point2D>> splitPoints(segments.size());
    int step = 0;

    for (std::size_t i = 0; i < segments.size(); ++i) {
        result.splitSegments.push_back({segments[i], {}, {}});
        if (segmentIsPointByContext(segments[i], predicates)) {
            result.warnings.push_back("zero_length_segment");
            addSplitPoint(splitPoints[i], segments[i].a, predicates);
            continue;
        }
        addSplitPoint(splitPoints[i], segments[i].a, predicates);
        addSplitPoint(splitPoints[i], segments[i].b, predicates);
    }

    AlgorithmOptions intersectionOptions;
    intersectionOptions.predicates = predicates;
    const auto intersections = bruteForceSegmentIntersections(segments, intersectionOptions, predicates);
    for (const auto& pair : intersections.pairs) {
        result.intersections.push_back({static_cast<std::size_t>(pair.first), static_cast<std::size_t>(pair.second), pair.intersection});
        const std::size_t first = static_cast<std::size_t>(pair.first);
        const std::size_t second = static_cast<std::size_t>(pair.second);
        if (pair.intersection.type == IntersectionType::Point && pair.intersection.point.has_value()) {
            addSplitPoint(splitPoints[first], *pair.intersection.point, predicates);
            addSplitPoint(splitPoints[second], *pair.intersection.point, predicates);
        } else if (pair.intersection.type == IntersectionType::Overlap && pair.intersection.overlap.has_value()) {
            const std::array<Point2D, 6> candidates{
                segments[first].a,
                segments[first].b,
                segments[second].a,
                segments[second].b,
                pair.intersection.overlap->a,
                pair.intersection.overlap->b
            };
            for (const auto& candidate : candidates) {
                if (segmentContainsPoint(segments[first], candidate, predicates)) addSplitPoint(splitPoints[first], candidate, predicates);
                if (segmentContainsPoint(segments[second], candidate, predicates)) addSplitPoint(splitPoints[second], candidate, predicates);
            }
        }
    }
    if (options.trace) {
        result.trace.push_back({step++, "intersection_detection", "Detected pairwise segment intersections for arrangement splitting.", {}, segments, {}, {{"intersection_count", static_cast<double>(result.intersections.size())}}});
    }

    if (options.trace) {
        std::vector<Point2D> allSplitPoints;
        for (const auto& points : splitPoints) {
            allSplitPoints.insert(allSplitPoints.end(), points.begin(), points.end());
        }
        result.trace.push_back({step++, "split_point_collection", "Collected segment endpoints, crossing points, and overlap boundaries.", allSplitPoints, segments, {}, {{"split_point_count", static_cast<double>(allSplitPoints.size())}}});
    }

    for (std::size_t i = 0; i < segments.size(); ++i) {
        auto& points = splitPoints[i];
        std::sort(points.begin(), points.end(), [&](const Point2D& a, const Point2D& b) {
            const double ta = segmentParameter(segments[i], a);
            const double tb = segmentParameter(segments[i], b);
            if (ta != tb) return ta < tb;
            return predicates.compareLexicographic(a, b) < 0;
        });
        std::vector<Point2D> unique;
        for (const auto& point : points) {
            if (unique.empty() || !predicates.equals(unique.back(), point)) unique.push_back(point);
        }
        result.splitSegments[i].splitPoints = unique;
        for (std::size_t j = 1; j < unique.size(); ++j) {
            if (!predicates.equals(unique[j - 1], unique[j])) {
                result.splitSegments[i].atomicSegments.push_back({unique[j - 1], unique[j]});
            }
        }
    }
    if (options.trace) {
        std::vector<Segment2D> atomic;
        for (const auto& split : result.splitSegments) {
            atomic.insert(atomic.end(), split.atomicSegments.begin(), split.atomicSegments.end());
        }
        result.trace.push_back({step++, "segment_splitting", "Split input segments into atomic subsegments.", {}, atomic, {}, {{"atomic_segment_count", static_cast<double>(atomic.size())}}});
    }

    for (std::size_t source = 0; source < result.splitSegments.size(); ++source) {
        for (const auto& atomic : result.splitSegments[source].atomicSegments) {
            const std::size_t from = arrangementNodeId(result.nodes, atomic.a, predicates);
            const std::size_t to = arrangementNodeId(result.nodes, atomic.b, predicates);
            if (from == to) continue;
            const std::size_t edgeId = result.edges.size();
            result.edges.push_back({from, to, atomic, source});
            result.nodes[from].outgoingEdges.push_back(edgeId);
        }
    }
    if (options.trace) {
        std::vector<Point2D> nodes;
        nodes.reserve(result.nodes.size());
        for (const auto& node : result.nodes) nodes.push_back(node.point);
        result.trace.push_back({step++, "node_dedup", "Deduplicated arrangement nodes with PredicateContext.", nodes, {}, {}, {{"node_count", static_cast<double>(result.nodes.size())}}});
        result.trace.push_back({step++, "edge_creation", "Created directed arrangement edges for atomic subsegments.", {}, {}, {}, {{"edge_count", static_cast<double>(result.edges.size())}}});
    }

    result.valid = true;
    if (options.validate) {
        result.valid = validateArrangementAtomicSegments(result.edges, predicates, result.warnings);
    }
    if (options.trace) {
        result.trace.push_back({step++, "validation", "Validated that atomic segments do not contain unsplit interior intersections.", {}, {}, {}, {{"valid", result.valid ? 1.0 : 0.0}}});
    }
    return result;
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

inline std::vector<Point2D> clipPointsByHalfPlane(const std::vector<Point2D>& subject, const HalfPlane2D& hp, const PredicateContext& predicates);

inline std::vector<Point2D> clipPointsByHalfPlane(const std::vector<Point2D>& subject, const HalfPlane2D& hp) {
    return clipPointsByHalfPlane(subject, hp, epsPredicateContext());
}

inline std::vector<Point2D> clipPointsByHalfPlane(const std::vector<Point2D>& subject, const HalfPlane2D& hp, const PredicateContext& predicates) {
    std::vector<Point2D> output;
    if (subject.empty()) return output;
    for (std::size_t i = 0; i < subject.size(); ++i) {
        const Point2D current = subject[i];
        const Point2D previous = subject[(i + subject.size() - 1) % subject.size()];
        const bool currentInside = hp.inside(current, predicates);
        const bool previousInside = hp.inside(previous, predicates);
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
    return removeConsecutiveDuplicatePoints(output, predicates);
}

inline PolygonClipResult sutherlandHodgmanClip(const Polygon2D& subject, const Polygon2D& clipper, const AlgorithmOptions& options = {}) {
    PolygonClipResult result;
    const PredicateContext& predicates = options.predicates;
    Polygon2D normalizedClipper = normalizePolygon(clipper, predicates);
    if (!normalizedClipper.isConvex(predicates)) result.warnings.push_back("clipper_is_not_convex");
    std::vector<Point2D> output = removeConsecutiveDuplicatePoints(subject.vertices, predicates);
    int step = 0;
    for (const auto& edge : normalizedClipper.edges()) {
        output = clipPointsByHalfPlane(output, HalfPlane2D{edge.a, edge.b - edge.a}, predicates);
        if (options.trace) result.trace.push_back({step++, "clip_edge", "Clipped subject polygon by one clipper edge.", {}, {edge}, {Polygon2D{output}}, {{"vertex_count", static_cast<double>(output.size())}}});
        if (output.empty()) break;
    }
    result.polygon = Polygon2D{removeConsecutiveDuplicatePoints(output, predicates)};
    if (result.polygon.vertices.size() < 3 || sign(result.polygon.area(), predicates.eps) == 0) {
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
    const PredicateContext& predicates = options.predicates;
    const double b = options.boundingLimit;
    std::vector<Point2D> poly{{-b, -b}, {b, -b}, {b, b}, {-b, b}};
    int step = 0;
    result.clippedByBoundingBox = options.useBoundingBox;
    for (const auto& hp : halfPlanes) {
        poly = clipPointsByHalfPlane(poly, hp, predicates);
        if (options.trace) result.trace.push_back({step++, "halfplane_clip", "Clipped current region by half-plane.", {}, {Segment2D{hp.p, hp.p + hp.dir}}, {Polygon2D{poly}}, {{"vertex_count", static_cast<double>(poly.size())}}});
        if (poly.empty()) break;
    }
    result.polygon = Polygon2D{removeConsecutiveDuplicatePoints(poly, predicates)};
    if (result.polygon.vertices.empty()) {
        result.status = HalfPlaneIntersectionStatus::Empty;
        return result;
    }
    if (result.polygon.vertices.size() < 3 || sign(result.polygon.area(), predicates.eps) == 0) {
        result.status = HalfPlaneIntersectionStatus::Degenerate;
        return result;
    }
    bool touchesBox = false;
    for (const auto& p : result.polygon.vertices) {
        if (equals(std::fabs(p.x), b, std::max(1e-6, predicates.eps)) || equals(std::fabs(p.y), b, std::max(1e-6, predicates.eps))) {
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
    const PredicateContext& predicates = options.predicates;
    if (input.size() < 2) {
        result.warnings.push_back("fewer_than_two_points");
        return result;
    }
    std::vector<Point2D> points = input;
    std::sort(points.begin(), points.end(), [&predicates](const Point2D& a, const Point2D& b) {
        return predicates.compareLexicographic(a, b) < 0;
    });
    for (std::size_t i = 1; i < points.size(); ++i) {
        if (predicates.equals(points[i], points[i - 1])) {
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
        std::sort(strip.begin(), strip.end(), [&predicates](const Point2D& a, const Point2D& b) {
            if (!equals(a.y, b.y, predicates.eps)) return a.y < b.y;
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

inline bool pointInTriangle(const Point2D& p, const Triangle2D& t, const PredicateContext& predicates);
inline bool isSimplePolygon(const Polygon2D& polygon, const PredicateContext& predicates);

inline bool pointInTriangle(const Point2D& p, const Triangle2D& t) {
    return pointInTriangle(p, t, epsPredicateContext());
}

inline bool pointInTriangle(const Point2D& p, const Triangle2D& t, const PredicateContext& predicates) {
    const int o1 = predicates.orient(t.a, t.b, p);
    const int o2 = predicates.orient(t.b, t.c, p);
    const int o3 = predicates.orient(t.c, t.a, p);
    return (o1 >= 0 && o2 >= 0 && o3 >= 0) || (o1 <= 0 && o2 <= 0 && o3 <= 0);
}

inline bool isSimplePolygon(const Polygon2D& polygon) {
    return isSimplePolygon(polygon, epsPredicateContext());
}

inline bool isSimplePolygon(const Polygon2D& polygon, const PredicateContext& predicates) {
    const auto edges = polygon.edges();
    for (std::size_t i = 0; i < edges.size(); ++i) {
        for (std::size_t j = i + 1; j < edges.size(); ++j) {
            if (j == i + 1 || (i == 0 && j + 1 == edges.size())) continue;
            if (segmentIntersection(edges[i], edges[j], predicates).type != IntersectionType::None) return false;
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
    const PredicateContext& predicates = options.predicates;
    Polygon2D poly = normalizePolygon(polygon, predicates);
    result.polygonArea = poly.area();
    int step = 0;
    if (poly.vertices.size() < 3) {
        result.warnings.push_back("fewer_than_three_vertices");
        return result;
    }
    if (!isSimplePolygon(poly, predicates)) {
        result.warnings.push_back("polygon_is_not_simple");
        return result;
    }
    std::vector<int> idx(poly.vertices.size());
    std::iota(idx.begin(), idx.end(), 0);
    auto isEar = [&](std::size_t pos) {
        const int prev = idx[(pos + idx.size() - 1) % idx.size()];
        const int curr = idx[pos];
        const int next = idx[(pos + 1) % idx.size()];
        if (predicates.orient(poly.vertices[prev], poly.vertices[curr], poly.vertices[next]) <= 0) return false;
        Triangle2D tri{poly.vertices[prev], poly.vertices[curr], poly.vertices[next]};
        for (const int id : idx) {
            if (id == prev || id == curr || id == next) continue;
            if (pointInTriangle(poly.vertices[id], tri, predicates)) return false;
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

struct DelaunayValidationReport {
    bool noDuplicateTriangles = true;
    bool allTrianglesCcw = true;
    bool noSuperTriangleVertex = true;
    bool edgesConsistent = true;
    bool emptyCircleProperty = true;
    bool coversConvexHullArea = true;
    double triangleAreaSum = 0.0;
    double hullArea = 0.0;
    double areaError = 0.0;
    std::size_t triangleCount = 0;
    std::size_t edgeCount = 0;
    std::size_t failedEmptyCircleChecks = 0;
    std::vector<std::string> failedChecks;
};

struct DelaunayResult {
    std::vector<Triangle2D> triangles;
    std::vector<Segment2D> edges;
    bool valid = false;
    bool experimental = true;
    DelaunayValidationReport validation;
    std::vector<TraceStep> trace;
    std::vector<std::string> warnings;
};

inline bool circumcircleContains(const Triangle2D& t, const Point2D& p, const PredicateContext& predicates);

inline std::optional<std::size_t> findPointIndex(
    const std::vector<Point2D>& points,
    const Point2D& point,
    const PredicateContext& predicates) {
    for (std::size_t i = 0; i < points.size(); ++i) {
        if (predicates.equals(points[i], point)) return i;
    }
    return std::nullopt;
}

inline std::array<std::size_t, 3> sortedTriangleIndices(std::array<std::size_t, 3> ids) {
    std::sort(ids.begin(), ids.end());
    return ids;
}

inline std::pair<std::size_t, std::size_t> sortedEdgeKey(std::size_t a, std::size_t b) {
    return a < b ? std::make_pair(a, b) : std::make_pair(b, a);
}

inline DelaunayValidationReport validateDelaunayTriangulation(
    const std::vector<Point2D>& inputPoints,
    const std::vector<Triangle2D>& triangles,
    const PredicateContext& predicates,
    std::vector<Segment2D>* edgesOut = nullptr) {
    DelaunayValidationReport report;
    const std::vector<Point2D> points = removeDuplicatePoints(inputPoints, predicates);
    report.triangleCount = triangles.size();

    std::set<std::array<std::size_t, 3>> triangleKeys;
    std::map<std::pair<std::size_t, std::size_t>, std::size_t> edgeCounts;
    for (const auto& tri : triangles) {
        report.triangleAreaSum += tri.area();
        if (predicates.orient(tri.a, tri.b, tri.c) <= 0) {
            report.allTrianglesCcw = false;
            report.failedChecks.push_back("triangle_not_ccw");
        }
        const auto ia = findPointIndex(points, tri.a, predicates);
        const auto ib = findPointIndex(points, tri.b, predicates);
        const auto ic = findPointIndex(points, tri.c, predicates);
        if (!ia.has_value() || !ib.has_value() || !ic.has_value()) {
            report.noSuperTriangleVertex = false;
            report.failedChecks.push_back("triangle_contains_non_input_vertex");
            continue;
        }
        const auto key = sortedTriangleIndices({*ia, *ib, *ic});
        if (!triangleKeys.insert(key).second) {
            report.noDuplicateTriangles = false;
            report.failedChecks.push_back("duplicate_triangle");
        }
        ++edgeCounts[sortedEdgeKey(*ia, *ib)];
        ++edgeCounts[sortedEdgeKey(*ib, *ic)];
        ++edgeCounts[sortedEdgeKey(*ic, *ia)];

        for (std::size_t i = 0; i < points.size(); ++i) {
            if (i == *ia || i == *ib || i == *ic) continue;
            const int incircleSign = predicates.incircle(tri.a, tri.b, tri.c, points[i]);
            if (incircleSign > 0) {
                report.emptyCircleProperty = false;
                ++report.failedEmptyCircleChecks;
            }
        }
    }
    if (!report.emptyCircleProperty) report.failedChecks.push_back("empty_circle_property_failed");

    std::set<std::pair<std::size_t, std::size_t>> hullEdges;
    ConvexHullOptions hullOptions;
    hullOptions.predicates = predicates;
    const auto hull = convexHullAndrew(points, hullOptions);
    report.hullArea = hull.area;
    if (hull.hull.size() >= 3) {
        for (std::size_t i = 0; i < hull.hull.size(); ++i) {
            const auto a = findPointIndex(points, hull.hull[i], predicates);
            const auto b = findPointIndex(points, hull.hull[(i + 1) % hull.hull.size()], predicates);
            if (a.has_value() && b.has_value()) hullEdges.insert(sortedEdgeKey(*a, *b));
        }
    }

    for (const auto& [edge, count] : edgeCounts) {
        if (count == 0 || count > 2) report.edgesConsistent = false;
        const bool isHullEdge = hullEdges.find(edge) != hullEdges.end();
        if ((isHullEdge && count != 1) || (!isHullEdge && count != 2)) {
            report.edgesConsistent = false;
        }
    }
    if (!report.edgesConsistent) report.failedChecks.push_back("edges_not_consistent");

    report.edgeCount = edgeCounts.size();
    if (edgesOut != nullptr) {
        edgesOut->clear();
        edgesOut->reserve(edgeCounts.size());
        for (const auto& [edge, count] : edgeCounts) {
            (void)count;
            edgesOut->push_back({points[edge.first], points[edge.second]});
        }
    }

    report.areaError = std::fabs(report.triangleAreaSum - report.hullArea);
    const double areaTolerance = 1e-7 * std::max(1.0, report.hullArea);
    report.coversConvexHullArea = report.areaError <= areaTolerance;
    if (!report.coversConvexHullArea) report.failedChecks.push_back("triangles_do_not_cover_convex_hull_area");
    return report;
}

inline bool circumcircleContains(const Triangle2D& t, const Point2D& p) {
    return circumcircleContains(t, p, epsPredicateContext());
}

inline bool circumcircleContains(const Triangle2D& t, const Point2D& p, const PredicateContext& predicates) {
    if (predicates.mode != PredicateMode::Eps) {
        const int triOrientation = predicates.orient(t.a, t.b, t.c);
        const int circle = predicates.incircle(t.a, t.b, t.c, p);
        return triOrientation >= 0 ? circle > 0 : circle < 0;
    }
    const double ax = t.a.x - p.x;
    const double ay = t.a.y - p.y;
    const double bx = t.b.x - p.x;
    const double by = t.b.y - p.y;
    const double cx = t.c.x - p.x;
    const double cy = t.c.y - p.y;
    const double det = (ax * ax + ay * ay) * (bx * cy - cx * by) -
                       (bx * bx + by * by) * (ax * cy - cx * ay) +
                       (cx * cx + cy * cy) * (ax * by - bx * ay);
    return cross(t.a, t.b, t.c) > 0 ? det > predicates.eps : det < -predicates.eps;
}

inline DelaunayResult delaunayTriangulation(const std::vector<Point2D>& input, const AlgorithmOptions& options = {}) {
    DelaunayResult result;
    const PredicateContext& predicates = options.predicates;
    std::vector<Point2D> points = removeDuplicatePoints(input, predicates);
    if (points.size() != input.size()) {
        result.warnings.push_back("duplicate_points_removed");
    }
    if (points.size() < 3) {
        result.warnings.push_back("fewer_than_three_unique_points");
        result.validation = validateDelaunayTriangulation(points, result.triangles, predicates, &result.edges);
        return result;
    }
    if (allCollinear(points, predicates)) {
        result.warnings.push_back("all_points_collinear");
        result.validation = validateDelaunayTriangulation(points, result.triangles, predicates, &result.edges);
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
    auto sameEdge = [&predicates](const Segment2D& a, const Segment2D& b) {
        return (predicates.equals(a.a, b.a) && predicates.equals(a.b, b.b)) || (predicates.equals(a.a, b.b) && predicates.equals(a.b, b.a));
    };
    for (const auto& p : points) {
        std::vector<Triangle2D> bad;
        std::vector<Triangle2D> kept;
        for (const auto& tri : triangles) {
            if (circumcircleContains(tri, p, predicates)) bad.push_back(tri);
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
            if (predicates.orient(tri.a, tri.b, tri.c) < 0) std::swap(tri.a, tri.b);
            triangles.push_back(tri);
        }
        if (options.trace) result.trace.push_back({step++, "insert_point", "Inserted point and rebuilt Delaunay cavity.", {p}, boundary, {}, {{"bad_triangles", static_cast<double>(bad.size())}, {"boundary_edges", static_cast<double>(boundary.size())}}});
    }
    for (const auto& tri : triangles) {
        if (predicates.equals(tri.a, s1) || predicates.equals(tri.a, s2) || predicates.equals(tri.a, s3) ||
            predicates.equals(tri.b, s1) || predicates.equals(tri.b, s2) || predicates.equals(tri.b, s3) ||
            predicates.equals(tri.c, s1) || predicates.equals(tri.c, s2) || predicates.equals(tri.c, s3)) {
            continue;
        }
        result.triangles.push_back(tri);
    }
    result.validation = validateDelaunayTriangulation(points, result.triangles, predicates, &result.edges);
    result.valid = result.validation.noDuplicateTriangles &&
                   result.validation.allTrianglesCcw &&
                   result.validation.noSuperTriangleVertex &&
                   result.validation.edgesConsistent &&
                   result.validation.emptyCircleProperty &&
                   result.validation.coversConvexHullArea;
    if (!result.valid) {
        result.warnings.push_back("delaunay_validation_failed");
    }
    return result;
}

}  // namespace geokernel

#include "geokernel/core/constants.hpp"
#include "geokernel/core/types.hpp"
#include "geokernel/core/predicates.hpp"
#include "geokernel/core/predicate_context.hpp"
#include "geokernel/core/geometry_utils.hpp"
#include "geokernel/core/intersections.hpp"
#include "geokernel/core/polygon.hpp"
#include "geokernel/trace/trace.hpp"
#include "geokernel/io/json_io.hpp"
#include "geokernel/algorithm/convex_hull.hpp"
#include "geokernel/algorithm/rotating_calipers.hpp"
#include "geokernel/algorithm/segment_intersection.hpp"
#include "geokernel/algorithm/sweep_line.hpp"
#include "geokernel/algorithm/half_plane_intersection.hpp"
#include "geokernel/algorithm/closest_pair.hpp"
#include "geokernel/algorithm/polygon_clipping.hpp"
#include "geokernel/algorithm/triangulation.hpp"
#include "geokernel/algorithm/delaunay.hpp"
#include "geokernel/experimental/delaunay.hpp"
#include "geokernel/algorithm/constrained_delaunay.hpp"
#include "geokernel/algorithm/polygon_boolean.hpp"
#include "geokernel/algorithm/arrangement.hpp"
