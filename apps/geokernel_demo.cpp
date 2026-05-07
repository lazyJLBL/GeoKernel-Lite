#include <CLI/CLI.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "geokernel/geokernel.hpp"

using json = nlohmann::json;
using namespace geokernel;

namespace {

Point2D parsePoint(const json& value) {
    if (!value.is_array() || value.size() != 2) {
        throw std::runtime_error("point must be [x, y]");
    }
    return {value.at(0).get<double>(), value.at(1).get<double>()};
}

std::vector<Point2D> parsePoints(const json& value) {
    std::vector<Point2D> points;
    for (const auto& item : value) {
        points.push_back(parsePoint(item));
    }
    return points;
}

Segment2D parseSegment(const json& value) {
    if (!value.is_array() || value.size() != 2) {
        throw std::runtime_error("segment must be [[x1, y1], [x2, y2]]");
    }
    return {parsePoint(value.at(0)), parsePoint(value.at(1))};
}

std::vector<Segment2D> parseSegments(const json& value) {
    std::vector<Segment2D> segments;
    for (const auto& item : value) {
        segments.push_back(parseSegment(item));
    }
    return segments;
}

HalfPlane2D parseHalfPlane(const json& value) {
    return {parsePoint(value.at("p")), parsePoint(value.at("dir"))};
}

std::vector<HalfPlane2D> parseHalfPlanes(const json& value) {
    std::vector<HalfPlane2D> halfPlanes;
    for (const auto& item : value) {
        halfPlanes.push_back(parseHalfPlane(item));
    }
    return halfPlanes;
}

Ring2D parseRing(const json& value) {
    return Ring2D{parsePoints(value)};
}

PolygonWithHoles2D parsePolygonWithHoles(const json& value) {
    if (!value.is_object()) {
        throw std::runtime_error("polygon must be an object with outer and holes");
    }
    PolygonWithHoles2D polygon;
    polygon.outer = parseRing(value.at("outer"));
    for (const auto& hole : value.value("holes", json::array())) {
        polygon.holes.push_back(parseRing(hole));
    }
    return polygon;
}

MultiPolygon2D parseMultiPolygon(const json& value) {
    MultiPolygon2D multipolygon;
    if (value.contains("polygons")) {
        for (const auto& polygon : value.at("polygons")) {
            multipolygon.polygons.push_back(parsePolygonWithHoles(polygon));
        }
        return multipolygon;
    }
    if (value.contains("outer")) {
        multipolygon.polygons.push_back(parsePolygonWithHoles(value));
        return multipolygon;
    }
    throw std::runtime_error("multipolygon must contain polygons or a polygon object with outer");
}

json pointJson(const Point2D& p) {
    return json::array({p.x, p.y});
}

json segmentJson(const Segment2D& s) {
    return json::array({pointJson(s.a), pointJson(s.b)});
}

json polygonJson(const Polygon2D& p) {
    json vertices = json::array();
    for (const auto& point : p.vertices) {
        vertices.push_back(pointJson(point));
    }
    return vertices;
}

json ringJson(const Ring2D& ring) {
    json vertices = json::array();
    for (const auto& point : ring.vertices) vertices.push_back(pointJson(point));
    return vertices;
}

json polygonWithHolesJson(const PolygonWithHoles2D& polygon) {
    json holes = json::array();
    for (const auto& hole : polygon.holes) holes.push_back(ringJson(hole));
    return {{"outer", ringJson(polygon.outer)}, {"holes", holes}};
}

json multiPolygonJson(const MultiPolygon2D& multipolygon) {
    json polygons = json::array();
    for (const auto& polygon : multipolygon.polygons) polygons.push_back(polygonWithHolesJson(polygon));
    return {{"polygons", polygons}};
}

json triangleJson(const Triangle2D& t) {
    return json::array({pointJson(t.a), pointJson(t.b), pointJson(t.c)});
}

std::string intersectionTypeName(IntersectionType type) {
    switch (type) {
        case IntersectionType::None: return "none";
        case IntersectionType::Point: return "point";
        case IntersectionType::Overlap: return "overlap";
    }
    return "unknown";
}

std::string clipStatusName(PolygonClipStatus status) {
    switch (status) {
        case PolygonClipStatus::Empty: return "empty";
        case PolygonClipStatus::Polygon: return "polygon";
        case PolygonClipStatus::Degenerate: return "degenerate";
    }
    return "unknown";
}

std::string halfPlaneStatusName(HalfPlaneIntersectionStatus status) {
    switch (status) {
        case HalfPlaneIntersectionStatus::Empty: return "empty";
        case HalfPlaneIntersectionStatus::BoundedPolygon: return "bounded_polygon";
        case HalfPlaneIntersectionStatus::Unbounded: return "unbounded";
        case HalfPlaneIntersectionStatus::Degenerate: return "degenerate";
    }
    return "unknown";
}

json traceJson(const std::vector<TraceStep>& trace) {
    json out = json::array();
    for (const auto& step : trace) {
        json item;
        item["step_index"] = step.step_index;
        item["phase"] = step.phase;
        item["message"] = step.message;
        item["geometry"]["points"] = json::array();
        item["geometry"]["segments"] = json::array();
        item["geometry"]["polygons"] = json::array();
        for (const auto& p : step.points) item["geometry"]["points"].push_back(pointJson(p));
        for (const auto& s : step.segments) item["geometry"]["segments"].push_back(segmentJson(s));
        for (const auto& p : step.polygons) item["geometry"]["polygons"].push_back(polygonJson(p));
        item["metrics"] = step.metrics;
        out.push_back(item);
    }
    return out;
}

json warningsJson(const std::vector<std::string>& warnings) {
    json out = json::array();
    for (const auto& warning : warnings) out.push_back(warning);
    return out;
}

std::vector<std::string> withWarning(std::vector<std::string> warnings, const std::string& warning) {
    warnings.push_back(warning);
    return warnings;
}

json validationReportJson(const GeometryValidationReport& report) {
    return {
        {"valid", report.valid},
        {"errors", warningsJson(report.errors)},
        {"warnings", warningsJson(report.warnings)}
    };
}

json delaunayValidationJson(const DelaunayValidationReport& report) {
    return {
        {"no_duplicate_triangles", report.noDuplicateTriangles},
        {"all_triangles_ccw", report.allTrianglesCcw},
        {"no_super_triangle_vertex", report.noSuperTriangleVertex},
        {"edges_consistent", report.edgesConsistent},
        {"empty_circle_property", report.emptyCircleProperty},
        {"covers_convex_hull_area", report.coversConvexHullArea},
        {"triangle_area_sum", report.triangleAreaSum},
        {"hull_area", report.hullArea},
        {"area_error", report.areaError},
        {"triangle_count", report.triangleCount},
        {"edge_count", report.edgeCount},
        {"failed_empty_circle_checks", report.failedEmptyCircleChecks},
        {"failed_checks", warningsJson(report.failedChecks)}
    };
}

json segmentIntersectionJson(const SegmentIntersectionResult& result) {
    json out;
    out["type"] = intersectionTypeName(result.type);
    if (result.point.has_value()) out["point"] = pointJson(*result.point);
    if (result.overlap.has_value()) out["overlap"] = segmentJson(*result.overlap);
    return out;
}

json intersectionRecordJson(const SegmentIntersectionRecord& record) {
    return {
        {"first", record.first},
        {"second", record.second},
        {"intersection", segmentIntersectionJson(record.intersection)}
    };
}

json splitSegmentJson(const SplitSegment& split) {
    json splitPoints = json::array();
    json atomicSegments = json::array();
    for (const auto& point : split.splitPoints) splitPoints.push_back(pointJson(point));
    for (const auto& segment : split.atomicSegments) atomicSegments.push_back(segmentJson(segment));
    return {
        {"original", segmentJson(split.original)},
        {"split_points", splitPoints},
        {"atomic_segments", atomicSegments}
    };
}

json arrangementNodeJson(const ArrangementNode& node) {
    json outgoing = json::array();
    for (const auto edge : node.outgoingEdges) outgoing.push_back(edge);
    return {{"point", pointJson(node.point)}, {"outgoing_edges", outgoing}};
}

json arrangementEdgeJson(const ArrangementEdge& edge) {
    return {
        {"from", edge.from},
        {"to", edge.to},
        {"segment", segmentJson(edge.segment)},
        {"source_segment_id", edge.sourceSegmentId}
    };
}

json predicateComparisonJson(const PredicateComparisonResult& result) {
    return {
        {"predicate", result.predicate},
        {"eps", {{"sign", result.epsSign}, {"estimate", result.epsEstimate}}},
        {"filtered", {{"sign", result.filteredSign}, {"estimate", result.filteredEstimate}}},
        {"exact", {{"sign", result.exactSign}}},
        {"eps_differs_from_exact", result.epsDiffersFromExact},
        {"filtered_matches_exact", result.filteredMatchesExact},
        {"disagreement", result.disagreement}
    };
}

json envelopeWithStatus(std::string status, json result, json summary, json trace, json warnings) {
    return {
        {"status", std::move(status)},
        {"summary", std::move(summary)},
        {"result", std::move(result)},
        {"trace", std::move(trace)},
        {"warnings", std::move(warnings)}
    };
}

json envelope(json result, json summary, json trace, json warnings) {
    return envelopeWithStatus("ok", std::move(result), std::move(summary), std::move(trace), std::move(warnings));
}

json predicateSummaryJson(const PredicateContext& predicates) {
    return {
        {"predicate_mode", predicateModeName(predicates.mode)},
        {"predicate_eps", predicates.eps}
    };
}

const json& payloadOf(const json& root) {
    return root.contains("input") ? root.at("input") : root;
}

json runAlgorithm(const std::string& algorithm, const json& root, bool traceEnabled) {
    const json& input = payloadOf(root);
    const double predicateEps = input.value("predicate_eps", input.value("eps", EPS));
    const PredicateContext predicates = predicateContextFromString(input.value("predicate_mode", "filtered_exact"), predicateEps);
    AlgorithmOptions options;
    options.trace = traceEnabled;
    options.predicates = predicates;

    if (algorithm == "convex_hull" || algorithm == "convex_hull_andrew") {
        ConvexHullOptions hullOptions;
        hullOptions.trace = traceEnabled;
        hullOptions.predicates = predicates;
        hullOptions.keepCollinear = input.value("keep_collinear", false);
        auto result = convexHullAndrew(parsePoints(input.at("points")), hullOptions);
        json body;
        body["hull"] = polygonJson(Polygon2D{result.hull});
        body["area"] = result.area;
        body["perimeter"] = result.perimeter;
        return envelope(body, {{"algorithm", "convex_hull"}, {"hull_vertices", result.hull.size()}, {"predicate_mode", predicateModeName(predicates.mode)}, {"predicate_eps", predicates.eps}}, traceJson(result.trace), warningsJson(withWarning(result.warnings, "area_and_perimeter_use_double_construction")));
    }

    if (algorithm == "rotating_calipers") {
        ConvexHullOptions hullOptions;
        hullOptions.predicates = predicates;
        auto hull = convexHullAndrew(parsePoints(input.at("points")), hullOptions).hull;
        auto diameter = convexDiameter(hull, options);
        auto rectangle = minimumAreaBoundingRectangle(hull, options);
        json body;
        body["hull"] = polygonJson(Polygon2D{hull});
        body["diameter"] = {{"p1", pointJson(diameter.p1)}, {"p2", pointJson(diameter.p2)}, {"distance", diameter.distance}};
        body["minimum_area_rectangle"] = {
            {"corners", polygonJson(Polygon2D{std::vector<Point2D>{rectangle.corners.begin(), rectangle.corners.end()}})},
            {"area", rectangle.area},
            {"width", rectangle.width},
            {"height", rectangle.height},
            {"angle", rectangle.angle}
        };
        json trace = traceJson(diameter.trace);
        for (const auto& item : traceJson(rectangle.trace)) trace.push_back(item);
        return envelope(
            body,
            {
                {"algorithm", "rotating_calipers"},
                {"hull_vertices", hull.size()},
                {"diameter_implementation", "rotating_calipers_o_h"},
                {"minimum_area_rectangle_implementation", "edge_direction_scan_o_h2"},
                {"predicate_mode", predicateModeName(predicates.mode)},
                {"predicate_eps", predicates.eps}
            },
            trace,
            warningsJson(std::vector<std::string>{"distance_and_rectangle_constructions_use_double"}));
    }

    if (algorithm == "segment_intersection" || algorithm == "sweep_line") {
        auto result = findSegmentIntersections(parseSegments(input.at("segments")), options, predicates);
        if (result.hasIntersection) result.warnings.push_back("intersection_coordinates_use_double_construction");
        json pairs = json::array();
        for (const auto& pair : result.pairs) {
            pairs.push_back({{"first", pair.first}, {"second", pair.second}, {"intersection", segmentIntersectionJson(pair.intersection)}});
        }
        json body = {{"has_intersection", result.hasIntersection}, {"pairs", pairs}};
        return envelope(
            body,
            {
                {"algorithm", "segment_intersection"},
                {"pair_count", result.pairs.size()},
                {"reported_pair_count", result.pairs.size()},
                {"event_count", result.eventCount},
                {"implementation", result.implementation},
                {"predicate_mode", predicateModeName(result.predicateMode)},
                {"predicate_eps", predicates.eps}
            },
            traceJson(result.trace),
            warningsJson(result.warnings));
    }

    if (algorithm == "segment_arrangement") {
        SegmentArrangementOptions arrangementOptions;
        arrangementOptions.trace = traceEnabled;
        arrangementOptions.predicates = predicates;
        arrangementOptions.validate = input.value("validate", true);
        auto result = buildSegmentArrangement(parseSegments(input.at("segments")), arrangementOptions);
        result.warnings.push_back("intersection_coordinates_use_double_construction");
        json nodes = json::array();
        json edges = json::array();
        json splitSegments = json::array();
        json intersections = json::array();
        for (const auto& node : result.nodes) nodes.push_back(arrangementNodeJson(node));
        for (const auto& edge : result.edges) edges.push_back(arrangementEdgeJson(edge));
        for (const auto& split : result.splitSegments) splitSegments.push_back(splitSegmentJson(split));
        for (const auto& record : result.intersections) intersections.push_back(intersectionRecordJson(record));
        json body = {
            {"valid", result.valid},
            {"nodes", nodes},
            {"edges", edges},
            {"split_segments", splitSegments},
            {"intersections", intersections}
        };
        return envelope(
            body,
            {
                {"algorithm", "segment_arrangement"},
                {"segment_count", input.at("segments").size()},
                {"node_count", result.nodes.size()},
                {"edge_count", result.edges.size()},
                {"intersection_count", result.intersections.size()},
                {"valid", result.valid},
                {"predicate_mode", predicateModeName(predicates.mode)},
                {"predicate_eps", predicates.eps}
            },
            traceJson(result.trace),
            warningsJson(result.warnings));
    }

    if (algorithm == "predicate_compare") {
        const std::string predicate = input.value("predicate", "orient2d");
        const double eps = input.value("eps", EPS);
        const auto points = parsePoints(input.at("points"));
        PredicateComparisonResult comparison;
        if (predicate == "orient2d") {
            if (points.size() != 3) throw std::runtime_error("orient2d requires exactly 3 points");
            comparison = compareOrient2d(points[0], points[1], points[2], eps);
        } else if (predicate == "incircle") {
            if (points.size() != 4) throw std::runtime_error("incircle requires exactly 4 points");
            comparison = compareIncircle(points[0], points[1], points[2], points[3], eps);
        } else {
            throw std::runtime_error("unknown predicate: " + predicate);
        }
        json body = predicateComparisonJson(comparison);
        body["points"] = polygonJson(Polygon2D{points});
        return envelope(
            body,
            {
                {"algorithm", "predicate_compare"},
                {"predicate", comparison.predicate},
                {"predicate_eps", eps},
                {"eps_differs_from_exact", comparison.epsDiffersFromExact},
                {"filtered_matches_exact", comparison.filteredMatchesExact}
            },
            json::array(),
            json::array());
    }

    if (algorithm == "half_plane_intersection") {
        HalfPlaneIntersectionOptions hpOptions;
        hpOptions.trace = traceEnabled;
        hpOptions.predicates = predicates;
        hpOptions.boundingLimit = input.value("bounding_limit", 1000000.0);
        auto result = halfPlaneIntersection(parseHalfPlanes(input.at("halfplanes")), hpOptions);
        result.warnings.push_back("clipping_vertices_and_area_use_double_construction");
        json body = {
            {"status", halfPlaneStatusName(result.status)},
            {"polygon", polygonJson(result.polygon)},
            {"area", result.polygon.area()},
            {"clipped_by_bounding_box", result.clippedByBoundingBox}
        };
        return envelope(body, {{"algorithm", "half_plane_intersection"}, {"vertices", result.polygon.vertices.size()}, {"predicate_mode", predicateModeName(predicates.mode)}, {"predicate_eps", predicates.eps}}, traceJson(result.trace), warningsJson(result.warnings));
    }

    if (algorithm == "closest_pair") {
        auto result = closestPair(parsePoints(input.at("points")), options);
        result.warnings.push_back("distance_comparisons_use_double_construction");
        json body = {
            {"valid", result.valid},
            {"p1", pointJson(result.p1)},
            {"p2", pointJson(result.p2)},
            {"distance", result.distance}
        };
        return envelope(body, {{"algorithm", "closest_pair"}, {"valid", result.valid}, {"predicate_mode", predicateModeName(predicates.mode)}, {"predicate_eps", predicates.eps}}, traceJson(result.trace), warningsJson(result.warnings));
    }

    if (algorithm == "polygon_clipping") {
        auto result = sutherlandHodgmanClip(Polygon2D{parsePoints(input.at("subject"))}, Polygon2D{parsePoints(input.at("clipper"))}, options);
        result.warnings.push_back("clipping_vertices_and_area_use_double_construction");
        json body = {
            {"status", clipStatusName(result.status)},
            {"polygon", polygonJson(result.polygon)},
            {"area", result.polygon.area()}
        };
        return envelope(body, {{"algorithm", "polygon_clipping"}, {"vertices", result.polygon.vertices.size()}, {"predicate_mode", predicateModeName(predicates.mode)}, {"predicate_eps", predicates.eps}}, traceJson(result.trace), warningsJson(result.warnings));
    }

    if (algorithm == "polygon_boolean") {
        const BooleanOp operation = booleanOpFromString(input.value("operation", "intersection"));
        PolygonBooleanOptions boolOptions;
        boolOptions.predicates = predicates;
        boolOptions.fillRule = fillRuleFromString(input.value("fill_rule", "even_odd"));
        boolOptions.normalizeInput = input.value("normalize_input", true);
        boolOptions.validateInput = input.value("validate_input", true);
        boolOptions.validateOutput = input.value("validate_output", true);
        boolOptions.trace = traceEnabled;
        const MultiPolygon2D subject = parseMultiPolygon(input.at("subject"));
        const MultiPolygon2D clip = parseMultiPolygon(input.at("clip"));
        const MultiPolygon2D normalizedSubject = boolOptions.normalizeInput ? normalizeMultiPolygon(subject, predicates) : subject;
        const MultiPolygon2D normalizedClip = boolOptions.normalizeInput ? normalizeMultiPolygon(clip, predicates) : clip;
        auto result = polygonBoolean(subject, clip, operation, boolOptions);
        result.warnings.push_back("polygon_validation_areas_use_double_construction");
        json body = {
            {"multipolygon", multiPolygonJson(result.result)},
            {"normalized_subject", multiPolygonJson(normalizedSubject)},
            {"normalized_clip", multiPolygonJson(normalizedClip)},
            {"input_validation", validationReportJson(result.inputValidation)},
            {"output_validation", validationReportJson(result.outputValidation)}
        };
        json summary = {
            {"algorithm", "polygon_boolean"},
            {"operation", booleanOpName(operation)},
            {"fill_rule", fillRuleName(boolOptions.fillRule)},
            {"predicate_mode", predicateModeName(predicates.mode)},
            {"predicate_eps", predicates.eps},
            {"input_subject_polygons", normalizedSubject.polygons.size()},
            {"input_clip_polygons", normalizedClip.polygons.size()},
            {"output_polygons", result.result.polygons.size()},
            {"area_subject", multiPolygonArea(normalizedSubject)},
            {"area_clip", multiPolygonArea(normalizedClip)},
            {"area_output", multiPolygonArea(result.result)},
            {"valid_input", result.inputValidation.valid},
            {"valid_output", result.valid}
        };
        return envelopeWithStatus(result.inputValidation.valid ? "ok" : "error", body, summary, traceJson(result.trace), warningsJson(result.warnings));
    }

    if (algorithm == "triangulation") {
        auto result = triangulateEarClipping(Polygon2D{parsePoints(input.at("polygon"))}, options);
        result.warnings.push_back("triangle_area_validation_uses_double_construction");
        json triangles = json::array();
        for (const auto& tri : result.triangles) triangles.push_back(triangleJson(tri));
        json body = {
            {"valid", result.valid},
            {"triangles", triangles},
            {"polygon_area", result.polygonArea},
            {"triangles_area", result.trianglesArea},
            {"area_error", result.areaError}
        };
        return envelope(body, {{"algorithm", "triangulation"}, {"triangle_count", result.triangles.size()}, {"predicate_mode", predicateModeName(predicates.mode)}, {"predicate_eps", predicates.eps}}, traceJson(result.trace), warningsJson(result.warnings));
    }

    if (algorithm == "delaunay") {
        const auto points = parsePoints(input.at("points"));
        auto result = delaunayTriangulation(points, options);
        result.warnings.push_back("delaunay_is_experimental_and_constructions_use_double");
        json triangles = json::array();
        json edges = json::array();
        for (const auto& tri : result.triangles) triangles.push_back(triangleJson(tri));
        for (const auto& edge : result.edges) edges.push_back(segmentJson(edge));
        json body = {
            {"experimental", true},
            {"valid", result.valid},
            {"triangles", triangles},
            {"edges", edges},
            {"validation", delaunayValidationJson(result.validation)}
        };
        return envelope(
            body,
            {
                {"algorithm", "delaunay"},
                {"valid", result.valid},
                {"point_count", points.size()},
                {"triangle_count", result.triangles.size()},
                {"edge_count", result.edges.size()},
                {"empty_circle_property", result.validation.emptyCircleProperty},
                {"covers_convex_hull_area", result.validation.coversConvexHullArea},
                {"predicate_mode", predicateModeName(predicates.mode)},
                {"predicate_eps", predicates.eps}
            },
            traceJson(result.trace),
            warningsJson(result.warnings));
    }

    throw std::runtime_error("unknown algorithm: " + algorithm);
}

}  // namespace

int main(int argc, char** argv) {
    CLI::App app{"GeoKernel-Lite command-line demo"};
    std::string algorithm;
    std::string inputPath;
    std::string outputPath;
    bool trace = false;
    bool pretty = false;

    app.add_option("--algorithm", algorithm, "Algorithm name")->required();
    app.add_option("--input", inputPath, "Input JSON path")->required();
    app.add_option("--output", outputPath, "Output JSON path");
    app.add_flag("--trace", trace, "Include algorithm trace");
    app.add_flag("--pretty", pretty, "Pretty-print JSON output");
    CLI11_PARSE(app, argc, argv);

    try {
        std::ifstream inputFile(inputPath);
        if (!inputFile) {
            throw std::runtime_error("failed to open input file: " + inputPath);
        }
        json input = json::parse(inputFile);
        json output = runAlgorithm(algorithm, input, trace);

        if (outputPath.empty()) {
            std::cout << (pretty ? output.dump(2) : output.dump()) << '\n';
        } else {
            std::ofstream outputFile(outputPath);
            if (!outputFile) {
                throw std::runtime_error("failed to open output file: " + outputPath);
            }
            outputFile << (pretty ? output.dump(2) : output.dump()) << '\n';
        }
    } catch (const std::exception& ex) {
        json error = {
            {"status", "error"},
            {"summary", {{"algorithm", algorithm}}},
            {"result", json::object()},
            {"trace", json::array()},
            {"warnings", json::array({ex.what()})}
        };
        std::cerr << error.dump(2) << '\n';
        return 1;
    }
    return 0;
}
