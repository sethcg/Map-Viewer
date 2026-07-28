#include <algorithm>
#include <iostream>

#include <SDL3/SDL.h>

#include <GeoJsonReader.hpp>

GeoJsonReader::GeoJsonReader() {
    dataset = nullptr;
    transform = nullptr;

    CPLSetConfigOption("PROJ_LIB", "../assets/proj/");

    // USING THE DEFAULT: "EPSG:3857" (WEB MERCATOR)
    GDALAllRegister();
}

GeoJsonReader::~GeoJsonReader() {
    if (transform) {
        delete transform;
        transform = nullptr;
    }

    if (dataset) {
        GDALClose(dataset);
        dataset = nullptr;
    }
}

bool GeoJsonReader::open(const std::string& filename) {
    dataset =
        (GDALDataset*)GDALOpenEx(
            filename.c_str(),
            GDAL_OF_VECTOR,
            nullptr,
            nullptr,
            nullptr
        );

    if (!dataset) {
        SDL_Log("FAILED TO OPEN GEOJSON: %s\n", filename);
        return false;
    }
    return true;
}

void GeoJsonReader::transformPoint(double& x, double& y) {
    if (!transform)
        return;

    transform->Transform(1, &x, &y);
}

void GeoJsonReader::updateBounds(double x, double y) {
    bounds.minX = std::min(bounds.minX, x);
    bounds.minY = std::min(bounds.minY, y);
    bounds.maxX = std::max(bounds.maxX, x);
    bounds.maxY = std::max(bounds.maxY, y);
}

std::vector<GeoFeature> GeoJsonReader::readAll() {
    std::vector<GeoFeature> result;
    if (!dataset)
        return result;

    OGRLayer* layer = dataset->GetLayer(0);
    layer->ResetReading();

    OGRFeature* feature;
    while ((feature = layer->GetNextFeature()) != nullptr) {
        OGRGeometry* geometry = feature->GetGeometryRef();
        if (geometry) {
            OGRGeometry* simplified = geometry->SimplifyPreserveTopology(5.0);
            if (simplified) {
                result.push_back(processGeometry(simplified));
                OGRGeometryFactory::destroyGeometry(simplified);
            }
        }
        OGRFeature::DestroyFeature(feature);
    }

    return result;
}

GeoFeature GeoJsonReader::processGeometry(OGRGeometry* geometry) {
    GeoFeature feature;
    auto type = wkbFlatten(geometry->getGeometryType());

    switch(type) {
        case wkbPoint:
        {
            feature.type = GeometryType::Point;
            auto* point = geometry->toPoint();
            double x = point->getX();
            double y = point->getY();
            transformPoint(x, y);
            updateBounds(x, y);
            feature.vertices.push_back({ x, y });
            break;
        }
        case wkbLineString:
        {
            feature.type = GeometryType::LineString;
            processLineString(geometry->toLineString(), feature);
            break;
        }
        case wkbPolygon:
        {
            feature.type = GeometryType::Polygon;
            processPolygon(geometry->toPolygon(), feature);
            break;
        }
        case wkbMultiPolygon:
        {
            feature.type = GeometryType::Polygon;
            auto* multi = geometry->toMultiPolygon();
            for(int i = 0; i < multi->getNumGeometries(); i++) {
                processPolygon(multi->getGeometryRef(i)->toPolygon(), feature);
            }
            break;
        }
        default:
            break;
    }
    return feature;
}

void GeoJsonReader::processLineString(OGRLineString* line,GeoFeature& feature) {
    int count = line->getNumPoints();
    for(int i = 0; i < count; i++) {
        double x = line->getX(i);
        double y = line->getY(i);
        transformPoint(x, y);
        updateBounds(x, y);
        feature.vertices.push_back({x, y});
    }
}

// void GeoJsonReader::processPolygon(OGRPolygon* polygon, GeoFeature& feature) {
//     auto processRing =
//     [&](OGRLinearRing* ring) {
//         feature.ringStarts.push_back(feature.vertices.size());
//         int count = ring->getNumPoints();

//         for(int i = 0; i < count - 1; i++) {
//             double x = ring->getX(i);
//             double y = ring->getY(i);

//             transformPoint(x, y);
//             updateBounds(x, y);

//             feature.vertices.push_back({x, y});
//         }
//     };

//     // EXTERIOR BOUNDARY
//     processRing(polygon->getExteriorRing());

//     // HOLES
//     for(int i = 0; i < polygon->getNumInteriorRings(); i++) {
//         processRing( polygon->getInteriorRing(i));
//     }
// }

void GeoJsonReader::processPolygon(OGRPolygon* polygon, GeoFeature& feature) {
    constexpr double MIN_HOLE_AREA = 100000.0; // ~316m x 316m

    auto processRing =
    [&](OGRLinearRing* ring) {
        feature.ringStarts.push_back(feature.vertices.size());

        int count = ring->getNumPoints();
        for (int i = 0; i < count - 1; i++) {
            double x = ring->getX(i);
            double y = ring->getY(i);

            transformPoint(x, y);
            updateBounds(x, y);

            feature.vertices.push_back({x, y});
        }
    };

    // EXTERIOR BOUNDARY
    processRing(polygon->getExteriorRing());

    // HOLES
    for (int i = 0; i < polygon->getNumInteriorRings(); i++) {
        OGRLinearRing* ring = polygon->getInteriorRing(i);

        // MEASURE HOLE AREA
        OGRPolygon holePolygon;
        OGRLinearRing* ringCopy = ring->clone();
        holePolygon.addRingDirectly(ringCopy);

        // SKIP OVER TINY HOLES
        double area = holePolygon.get_Area();
        if (area < MIN_HOLE_AREA)
            continue;
        
        processRing(ring);
    }
}