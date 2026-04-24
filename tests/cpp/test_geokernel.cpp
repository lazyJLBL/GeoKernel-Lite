#include <gtest/gtest.h>

#include "geokernel/geokernel.hpp"

using namespace geokernel;

TEST(Point2DTest, UsesEpsEquality) {
    EXPECT_TRUE(equals(Point2D{1.0, 2.0}, Point2D{1.0 + 1e-10, 2.0 - 1e-10}));
    EXPECT_EQ(sign(cross(Point2D{1, 0}, Point2D{0, 1})), 1);
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

TEST(PolygonTest, ClassifiesInsideBoundaryOutside) {
    Polygon2D square{{{0, 0}, {2, 0}, {2, 2}, {0, 2}}};
    EXPECT_EQ(square.containsPoint({1, 1}), PointInPolygonResult::Inside);
    EXPECT_EQ(square.containsPoint({0, 1}), PointInPolygonResult::OnBoundary);
    EXPECT_EQ(square.containsPoint({3, 1}), PointInPolygonResult::Outside);
    EXPECT_TRUE(square.isConvex());
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

TEST(SweepLineTest, FindsAllPairs) {
    std::vector<Segment2D> segments{{{0, 0}, {2, 2}}, {{0, 2}, {2, 0}}, {{3, 0}, {4, 0}}};
    AlgorithmOptions options;
    options.trace = true;
    auto result = findSegmentIntersections(segments, options);
    EXPECT_TRUE(result.hasIntersection);
    EXPECT_EQ(result.pairs.size(), 1);
    EXPECT_FALSE(result.trace.empty());
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
