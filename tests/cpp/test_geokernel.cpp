#include <gtest/gtest.h>

#include <limits>
#include <random>
#include <set>

#include "geokernel/core/constants.hpp"
#include "geokernel/core/types.hpp"
#include "geokernel/core/predicates.hpp"
#include "geokernel/core/predicate_context.hpp"
#include "geokernel/core/geometry_utils.hpp"
#include "geokernel/core/intersections.hpp"
#include "geokernel/core/polygon.hpp"
#include "geokernel/trace/trace.hpp"
#include "geokernel/io/json_io.hpp"
#include "geokernel/algorithm/segment_intersection.hpp"
#include "geokernel/algorithm/delaunay.hpp"
#include "geokernel/algorithm/constrained_delaunay.hpp"
#include "geokernel/algorithm/polygon_boolean.hpp"
#include "geokernel/algorithm/arrangement.hpp"
#include "geokernel/geokernel.hpp"

using namespace geokernel;

TEST(Point2DTest, UsesEpsEquality) {
    EXPECT_TRUE(equals(Point2D{1.0, 2.0}, Point2D{1.0 + 1e-10, 2.0 - 1e-10}));
    EXPECT_EQ(sign(cross(Point2D{1, 0}, Point2D{0, 1})), 1);
}

TEST(PredicateTest, ExactOrientationDistinguishesEpsCollapse) {
    Point2D a{0, 0};
    Point2D b{1, 0};
    Point2D c{0.5, 1e-12};

    EXPECT_EQ(orient2dEps(a, b, c), 0);
    EXPECT_EQ(orient2dExact(a, b, c), 1);
    EXPECT_EQ(orient2dFiltered(a, b, c), orient2dExact(a, b, c));

    const auto comparison = compareOrient2d(a, b, c);
    EXPECT_TRUE(comparison.epsDiffersFromExact);
    EXPECT_TRUE(comparison.filteredMatchesExact);
}

TEST(PredicateTest, ExactIncircleDistinguishesEpsCollapse) {
    Point2D a{0, 0};
    Point2D b{1, 0};
    Point2D c{0, 1};
    Point2D d{1, 0.999999999999};

    EXPECT_EQ(incircleEps(a, b, c, d), 0);
    EXPECT_EQ(incircleExact(a, b, c, d), 1);
    EXPECT_EQ(incircleFiltered(a, b, c, d), incircleExact(a, b, c, d));

    const auto comparison = compareIncircle(a, b, c, d);
    EXPECT_TRUE(comparison.epsDiffersFromExact);
    EXPECT_TRUE(comparison.filteredMatchesExact);
}

TEST(PredicateTest, RejectsNonFiniteInputs) {
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    EXPECT_THROW(orient2dExact(0, 0, inf, 0, 1, 1), std::invalid_argument);
    EXPECT_THROW(incircleFiltered(0, 0, 1, 0, 0, 1, nan, 1), std::invalid_argument);
}

TEST(PredicateContextTest, RoutesPredicateModesAndComparisons) {
    const Point2D a{0, 0};
    const Point2D b{1, 0};
    const Point2D c{0.5, 1e-12};

    PredicateContext eps = epsPredicateContext();
    PredicateContext exact = exactPredicateContext();

    EXPECT_EQ(eps.orient(a, b, c), 0);
    EXPECT_EQ(exact.orient(a, b, c), 1);
    EXPECT_TRUE(eps.equals({0, 0}, {5e-10, 0}));
    EXPECT_FALSE(exact.equals({0, 0}, {5e-10, 0}));
    EXPECT_LT(exact.compareLexicographic({0, 0}, {1, 0}), 0);
}

TEST(SegmentIntersectionTest, ClassifiesOverlapAndEndpoint) {
    auto overlap = segmentIntersection({{0, 0}, {4, 0}}, {{2, 0}, {6, 0}});
    ASSERT_EQ(overlap.type, IntersectionType::Overlap);
    ASSERT_TRUE(overlap.overlap.has_value());
    EXPECT_TRUE(equals(overlap.overlap->a, Point2D{2, 0}));
    EXPECT_TRUE(equals(overlap.overlap->b, Point2D{4, 0}));

    auto endpoint = segmentIntersection({{0, 0}, {1, 1}}, {{1, 1}, {2, 0}});
    ASSERT_EQ(endpoint.type, IntersectionType::Point);
    ASSERT_TRUE(endpoint.point.has_value());
    EXPECT_TRUE(equals(*endpoint.point, Point2D{1, 1}));
}

TEST(SegmentIntersectionTest, ExactModeDoesNotTreatNearParallelSegmentsAsCollinear) {
    Segment2D a{{0, 0}, {2, 0}};
    Segment2D b{{1, 1e-10}, {3, 1e-10}};

    EXPECT_EQ(segmentIntersection(a, b, PredicateMode::Eps).type, IntersectionType::Overlap);
    EXPECT_EQ(segmentIntersection(a, b, PredicateMode::FilteredExact).type, IntersectionType::None);
}

TEST(PredicateModeCoverageTest, ConvexHullNearCollinearDiffersByMode) {
    std::vector<Point2D> points{{0, 0}, {1, 0}, {0.5, 1e-12}, {0.5, -1e-12}};
    ConvexHullOptions epsOptions;
    epsOptions.predicates = epsPredicateContext();
    ConvexHullOptions exactOptions;
    exactOptions.predicates = exactPredicateContext();

    const auto epsHull = convexHullAndrew(points, epsOptions);
    const auto exactHull = convexHullAndrew(points, exactOptions);
    EXPECT_NE(epsHull.hull.size(), exactHull.hull.size());
    EXPECT_FALSE(epsHull.warnings.empty());
}

TEST(PredicateModeCoverageTest, TriangulationNearCollinearEarDiffersByMode) {
    Polygon2D polygon{{{0, 0}, {1, 0}, {1 + 1e-12, 1e-12}, {1, 1}, {0, 1}}};
    AlgorithmOptions epsOptions;
    epsOptions.predicates = epsPredicateContext();
    AlgorithmOptions exactOptions;
    exactOptions.predicates = exactPredicateContext();

    const auto epsResult = triangulateEarClipping(polygon, epsOptions);
    const auto exactResult = triangulateEarClipping(polygon, exactOptions);
    EXPECT_TRUE(epsResult.valid);
    EXPECT_TRUE(exactResult.valid);
    EXPECT_NE(epsResult.triangles.size(), exactResult.triangles.size());
}

TEST(PredicateModeCoverageTest, DelaunayIncircleDiffersByMode) {
    Triangle2D tri{{0, 0}, {1, 0}, {0, 1}};
    Point2D nearCocircular{1, 0.999999999999};
    EXPECT_FALSE(circumcircleContains(tri, nearCocircular, epsPredicateContext()));
    EXPECT_TRUE(circumcircleContains(tri, nearCocircular, exactPredicateContext()));
}

TEST(PredicateModeCoverageTest, HalfPlaneBoundaryDiffersByMode) {
    HalfPlane2D upper{{0, 0}, {1, 0}};
    Point2D barelyOutside{0.5, -1e-12};
    EXPECT_TRUE(upper.inside(barelyOutside, epsPredicateContext()));
    EXPECT_FALSE(upper.inside(barelyOutside, exactPredicateContext()));
}

TEST(PolygonTest, ClassifiesInsideBoundaryOutside) {
    Polygon2D square{{{0, 0}, {2, 0}, {2, 2}, {0, 2}}};
    EXPECT_EQ(square.containsPoint({1, 1}), PointInPolygonResult::Inside);
    EXPECT_EQ(square.containsPoint({0, 1}), PointInPolygonResult::OnBoundary);
    EXPECT_EQ(square.containsPoint({3, 1}), PointInPolygonResult::Outside);
    EXPECT_TRUE(square.isConvex());
}

TEST(PolygonBooleanValidationTest, NormalizesOuterHoleAndDuplicates) {
    PredicateContext predicates = filteredExactPredicateContext();
    PolygonWithHoles2D polygon{
        Ring2D{{{0, 0}, {0, 4}, {4, 4}, {4, 0}, {0, 0}}},
        {Ring2D{{{1, 1}, {3, 1}, {3, 3}, {1, 3}, {1, 1}}}}
    };

    polygon = normalizePolygonWithHoles(std::move(polygon), predicates);
    EXPECT_EQ(polygon.outer.vertices.size(), 4);
    EXPECT_EQ(polygon.holes.front().vertices.size(), 4);
    EXPECT_EQ(ringOrientation(polygon.outer, predicates), RingOrientation::CounterClockwise);
    EXPECT_EQ(ringOrientation(polygon.holes.front(), predicates), RingOrientation::Clockwise);
    EXPECT_TRUE(validatePolygonWithHoles(polygon, predicates).valid);
}

TEST(PolygonBooleanValidationTest, RejectsSelfIntersectionAndInvalidHoles) {
    PredicateContext predicates = filteredExactPredicateContext();
    Ring2D bowTie{{{0, 0}, {2, 2}, {0, 2}, {2, 0}}};
    EXPECT_FALSE(validateRing(bowTie, predicates).valid);

    PolygonWithHoles2D outsideHole{
        Ring2D{{{0, 0}, {4, 0}, {4, 4}, {0, 4}}},
        {Ring2D{{{5, 5}, {6, 5}, {6, 6}, {5, 6}}}}
    };
    outsideHole = normalizePolygonWithHoles(std::move(outsideHole), predicates);
    EXPECT_FALSE(validatePolygonWithHoles(outsideHole, predicates).valid);

    PolygonWithHoles2D touchingHole{
        Ring2D{{{0, 0}, {4, 0}, {4, 4}, {0, 4}}},
        {Ring2D{{{0, 1}, {2, 1}, {2, 2}, {0, 2}}}}
    };
    touchingHole = normalizePolygonWithHoles(std::move(touchingHole), predicates);
    EXPECT_FALSE(validatePolygonWithHoles(touchingHole, predicates).valid);
}

TEST(PolygonBooleanValidationTest, ClassifiesPointWithHoleBoundary) {
    PredicateContext predicates = filteredExactPredicateContext();
    PolygonWithHoles2D polygon{
        Ring2D{{{0, 0}, {4, 0}, {4, 4}, {0, 4}}},
        {Ring2D{{{1, 1}, {3, 1}, {3, 3}, {1, 3}}}}
    };
    polygon = normalizePolygonWithHoles(std::move(polygon), predicates);
    EXPECT_EQ(pointInPolygonWithHoles(polygon, {0.5, 0.5}, predicates), BoundaryLocation::Inside);
    EXPECT_EQ(pointInPolygonWithHoles(polygon, {2, 2}, predicates), BoundaryLocation::Outside);
    EXPECT_EQ(pointInPolygonWithHoles(polygon, {1, 2}, predicates), BoundaryLocation::OnBoundary);
    EXPECT_EQ(pointInPolygonWithHoles(polygon, {5, 5}, predicates), BoundaryLocation::Outside);
}

TEST(PolygonBooleanValidationTest, SkeletonReportsNotImplementedAfterValidation) {
    PolygonBooleanOptions options;
    options.predicates = filteredExactPredicateContext();
    options.trace = true;
    MultiPolygon2D subject{{PolygonWithHoles2D{Ring2D{{{0, 0}, {2, 0}, {2, 2}, {0, 2}}}, {}}}};
    MultiPolygon2D clip{{PolygonWithHoles2D{Ring2D{{{1, 1}, {3, 1}, {3, 3}, {1, 3}}}, {}}}};

    auto result = polygonBoolean(subject, clip, BooleanOp::Union, options);
    EXPECT_TRUE(result.inputValidation.valid);
    EXPECT_FALSE(result.valid);
    EXPECT_FALSE(result.warnings.empty());
    EXPECT_FALSE(result.trace.empty());
}

TEST(ConvexHullTest, HandlesDuplicatesAndCollinearPolicy) {
    std::vector<Point2D> points{{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {0.5, 0}};
    ConvexHullOptions options;
    options.trace = true;
    auto hull = convexHullAndrew(points, options);
    EXPECT_EQ(hull.hull.size(), 4);
    EXPECT_NEAR(hull.area, 1.0, 1e-9);
    EXPECT_FALSE(hull.trace.empty());
}

TEST(RotatingCalipersTest, ComputesDiameterAndRectangle) {
    std::vector<Point2D> hull{{0, 0}, {2, 0}, {2, 1}, {0, 1}};
    auto diameter = convexDiameter(hull);
    EXPECT_NEAR(diameter.distance, std::sqrt(5.0), 1e-9);
    auto rect = minimumAreaBoundingRectangle(hull);
    EXPECT_NEAR(rect.area, 2.0, 1e-9);
}

TEST(RotatingCalipersTest, HandlesDegenerateAndCollinearDiameter) {
    EXPECT_NEAR(convexDiameter({}).distance, 0.0, 1e-12);
    auto single = convexDiameter({{2, 3}});
    EXPECT_NEAR(single.distance, 0.0, 1e-12);
    EXPECT_TRUE(equals(single.p1, Point2D{2, 3}));
    EXPECT_TRUE(equals(single.p2, Point2D{2, 3}));

    auto segment = convexDiameter({{0, 0}, {3, 4}});
    EXPECT_NEAR(segment.distance, 5.0, 1e-12);

    std::vector<Point2D> collinear{{0, 0}, {1, 1}, {4, 4}, {2, 2}};
    auto diameter = convexDiameter(collinear);
    EXPECT_NEAR(diameter.distance, std::sqrt(32.0), 1e-12);
}

TEST(RotatingCalipersTest, MatchesBruteForceDiameterOracleOnConvexHulls) {
    std::mt19937 rng(987);
    std::uniform_real_distribution<double> dist(-25.0, 25.0);

    for (int round = 0; round < 30; ++round) {
        std::vector<Point2D> points;
        for (int i = 0; i < 50; ++i) points.push_back({dist(rng), dist(rng)});
        const auto hullResult = convexHullAndrew(points);
        ASSERT_GE(hullResult.hull.size(), 2);
        const auto calipers = convexDiameter(hullResult.hull);
        const auto oracle = bruteForceConvexDiameter(hullResult.hull);
        EXPECT_NEAR(calipers.distance, oracle.distance, 1e-9);
    }
}

TEST(SweepLineTest, FindsAllPairs) {
    std::vector<Segment2D> segments{{{0, 0}, {2, 2}}, {{0, 2}, {2, 0}}, {{3, 0}, {4, 0}}};
    AlgorithmOptions options;
    options.trace = true;
    auto result = findSegmentIntersections(segments, options);
    EXPECT_TRUE(result.hasIntersection);
    EXPECT_EQ(result.pairs.size(), 1);
    EXPECT_EQ(result.implementation, "sweep_line_ordered_active_with_oracle_completion");
    EXPECT_EQ(result.predicateMode, PredicateMode::FilteredExact);
    EXPECT_FALSE(result.trace.empty());
}

TEST(SweepLineTest, MatchesBruteForceOracleOnDegenerateSet) {
    std::vector<Segment2D> segments{
        {{0, 0}, {3, 3}},
        {{0, 3}, {3, 0}},
        {{1, -1}, {1, 4}},
        {{0, 1}, {2, 1}},
        {{1, 1}, {1, 1}},
        {{0, 0}, {2, 0}},
        {{1, 0}, {3, 0}},
        {{4, 4}, {5, 5}}
    };

    const auto sweep = findSegmentIntersections(segments);
    const auto brute = bruteForceSegmentIntersections(segments);

    auto keys = [](const SegmentIntersectionSearchResult& result) {
        std::set<std::pair<int, int>> out;
        for (const auto& pair : result.pairs) out.insert({pair.first, pair.second});
        return out;
    };

    EXPECT_EQ(keys(sweep), keys(brute));
    EXPECT_EQ(sweep.eventCount, segments.size() * 2);
}

TEST(SegmentArrangementTest, SplitsSimpleCross) {
    SegmentArrangementOptions options;
    options.predicates = filteredExactPredicateContext();
    options.trace = true;
    const auto result = buildSegmentArrangement({{{0, 0}, {2, 2}}, {{0, 2}, {2, 0}}}, options);

    EXPECT_TRUE(result.valid);
    ASSERT_EQ(result.splitSegments.size(), 2);
    EXPECT_EQ(result.intersections.size(), 1);
    EXPECT_EQ(result.nodes.size(), 5);
    EXPECT_EQ(result.edges.size(), 4);
    EXPECT_EQ(result.splitSegments[0].atomicSegments.size(), 2);
    EXPECT_EQ(result.splitSegments[1].atomicSegments.size(), 2);
    EXPECT_FALSE(result.trace.empty());
}

TEST(SegmentArrangementTest, SplitsOverlappingHorizontalSegmentsConsistently) {
    SegmentArrangementOptions options;
    options.predicates = filteredExactPredicateContext();
    const auto result = buildSegmentArrangement({{{0, 0}, {4, 0}}, {{2, 0}, {6, 0}}}, options);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.intersections.size(), 1);
    EXPECT_EQ(result.nodes.size(), 4);
    EXPECT_EQ(result.edges.size(), 4);
    EXPECT_EQ(result.splitSegments[0].splitPoints.size(), 3);
    EXPECT_EQ(result.splitSegments[1].splitPoints.size(), 3);
}

TEST(SegmentArrangementTest, HandlesSharedEndpointDuplicateAndZeroLength) {
    SegmentArrangementOptions options;
    options.predicates = filteredExactPredicateContext();
    std::vector<Segment2D> segments{
        {{0, 0}, {1, 0}},
        {{1, 0}, {2, 0}},
        {{0, 0}, {1, 0}},
        {{3, 3}, {3, 3}}
    };
    const auto result = buildSegmentArrangement(segments, options);

    EXPECT_TRUE(result.valid);
    EXPECT_GE(result.nodes.size(), 3);
    EXPECT_EQ(result.splitSegments[3].atomicSegments.size(), 0);
    EXPECT_FALSE(result.warnings.empty());
}

TEST(SegmentArrangementTest, RandomSmallArrangementHasNoInteriorAtomicIntersections) {
    SegmentArrangementOptions options;
    options.predicates = filteredExactPredicateContext();
    std::vector<Segment2D> segments{
        {{0, 0}, {3, 3}},
        {{0, 3}, {3, 0}},
        {{1, -1}, {1, 4}},
        {{0, 1}, {3, 1}},
        {{2, -1}, {2, 4}}
    };

    const auto result = buildSegmentArrangement(segments, options);
    EXPECT_TRUE(result.valid);
    EXPECT_FALSE(result.nodes.empty());
    EXPECT_FALSE(result.edges.empty());
}

TEST(FuzzTest, SweepMatchesBruteForceOnDeterministicRandomSegments) {
    std::mt19937 rng(123);
    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    for (int round = 0; round < 20; ++round) {
        std::vector<Segment2D> segments;
        for (int i = 0; i < 12; ++i) {
            segments.push_back({{dist(rng), dist(rng)}, {dist(rng), dist(rng)}});
        }
        const auto sweep = findSegmentIntersections(segments);
        const auto brute = bruteForceSegmentIntersections(segments);
        std::set<std::pair<int, int>> sweepPairs;
        std::set<std::pair<int, int>> brutePairs;
        for (const auto& pair : sweep.pairs) sweepPairs.insert({pair.first, pair.second});
        for (const auto& pair : brute.pairs) brutePairs.insert({pair.first, pair.second});
        EXPECT_EQ(sweepPairs, brutePairs);
    }
}

TEST(FuzzTest, SweepMatchesBruteForceOnDeterministicDegenerateIntegerSegments) {
    std::mt19937 rng(654);
    std::uniform_int_distribution<int> coord(-3, 3);

    for (int round = 0; round < 25; ++round) {
        std::vector<Segment2D> segments;
        for (int i = 0; i < 14; ++i) {
            Point2D a{static_cast<double>(coord(rng)), static_cast<double>(coord(rng))};
            Point2D b{static_cast<double>(coord(rng)), static_cast<double>(coord(rng))};
            if (i % 5 == 0) b.x = a.x;
            if (i % 7 == 0) b = a;
            segments.push_back({a, b});
        }
        const auto sweep = findSegmentIntersections(segments);
        const auto brute = bruteForceSegmentIntersections(segments);
        std::set<std::pair<int, int>> sweepPairs;
        std::set<std::pair<int, int>> brutePairs;
        for (const auto& pair : sweep.pairs) sweepPairs.insert({pair.first, pair.second});
        for (const auto& pair : brute.pairs) brutePairs.insert({pair.first, pair.second});
        EXPECT_EQ(sweepPairs, brutePairs);
    }
}

TEST(FuzzTest, ConvexHullContainsDeterministicRandomPoints) {
    std::mt19937 rng(456);
    std::uniform_real_distribution<double> dist(-50.0, 50.0);

    for (int round = 0; round < 10; ++round) {
        std::vector<Point2D> points;
        for (int i = 0; i < 40; ++i) points.push_back({dist(rng), dist(rng)});
        ConvexHullOptions options;
        options.predicates = filteredExactPredicateContext();
        const auto hull = convexHullAndrew(points, options);
        ASSERT_GE(hull.hull.size(), 3);
        Polygon2D hullPolygon{hull.hull};
        for (const auto& p : points) {
            const auto location = hullPolygon.containsPoint(p, options.predicates);
            EXPECT_NE(location, PointInPolygonResult::Outside);
        }
    }
}

TEST(ClippingTest, ClipsConvexWindow) {
    Polygon2D subject{{{-1, -1}, {3, -1}, {3, 3}, {-1, 3}}};
    Polygon2D clipper{{{0, 0}, {2, 0}, {2, 2}, {0, 2}}};
    auto result = sutherlandHodgmanClip(subject, clipper);
    EXPECT_EQ(result.status, PolygonClipStatus::Polygon);
    EXPECT_NEAR(result.polygon.area(), 4.0, 1e-9);
}

TEST(HalfPlaneTest, ClipsByBoundingBox) {
    std::vector<HalfPlane2D> halfplanes{
        {{0, 0}, {1, 0}},
        {{1, 0}, {0, 1}},
        {{1, 1}, {-1, 0}},
        {{0, 1}, {0, -1}}
    };
    HalfPlaneIntersectionOptions options;
    options.boundingLimit = 10.0;
    auto result = halfPlaneIntersection(halfplanes, options);
    EXPECT_EQ(result.status, HalfPlaneIntersectionStatus::BoundedPolygon);
    EXPECT_NEAR(result.polygon.area(), 1.0, 1e-9);
}

TEST(ClosestPairTest, MatchesDuplicateAndNonDuplicateCases) {
    auto duplicate = closestPair({{0, 0}, {1, 1}, {0, 0}});
    ASSERT_TRUE(duplicate.valid);
    EXPECT_NEAR(duplicate.distance, 0.0, 1e-9);

    auto normal = closestPair({{0, 0}, {5, 0}, {2, 2}, {2.1, 2}});
    ASSERT_TRUE(normal.valid);
    EXPECT_NEAR(normal.distance, 0.1, 1e-9);
}

TEST(TriangulationTest, PreservesArea) {
    Polygon2D poly{{{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 3}, {0, 3}}};
    AlgorithmOptions options;
    options.trace = true;
    auto result = triangulateEarClipping(poly, options);
    EXPECT_TRUE(result.valid);
    EXPECT_NEAR(result.areaError, 0.0, 1e-9);
    EXPECT_FALSE(result.trace.empty());
}

TEST(DelaunayTest, ProducesExperimentalTriangles) {
    auto result = delaunayTriangulation({{0, 0}, {1, 0}, {0, 1}, {1, 1}});
    EXPECT_TRUE(result.experimental);
    EXPECT_FALSE(result.triangles.empty());
}

TEST(DelaunayValidationTest, TriangleValidates) {
    auto result = delaunayTriangulation({{0, 0}, {1, 0}, {0, 1}});
    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.triangles.size(), 1);
    EXPECT_TRUE(result.validation.allTrianglesCcw);
    EXPECT_TRUE(result.validation.emptyCircleProperty);
    EXPECT_TRUE(result.validation.coversConvexHullArea);
    EXPECT_NEAR(result.validation.areaError, 0.0, 1e-9);
}

TEST(DelaunayValidationTest, SquareValidationAllowsCocircularBoundary) {
    auto result = delaunayTriangulation({{0, 0}, {1, 0}, {1, 1}, {0, 1}});
    EXPECT_TRUE(result.valid);
    EXPECT_TRUE(result.validation.emptyCircleProperty);
    EXPECT_EQ(result.validation.triangleCount, 2);
    EXPECT_EQ(result.edges.size(), result.validation.edgeCount);
}

TEST(DelaunayValidationTest, DegenerateInputsWarnClearly) {
    auto duplicates = delaunayTriangulation({{0, 0}, {1, 0}, {0, 1}, {0, 1}});
    EXPECT_TRUE(duplicates.valid);
    EXPECT_FALSE(duplicates.warnings.empty());

    auto collinear = delaunayTriangulation({{0, 0}, {1, 0}, {2, 0}, {3, 0}});
    EXPECT_FALSE(collinear.valid);
    EXPECT_TRUE(collinear.triangles.empty());
    EXPECT_FALSE(collinear.warnings.empty());
}
