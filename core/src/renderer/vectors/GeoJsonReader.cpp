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
        SDL_Log("FAILED TO OPEN GEOJSON: %s", filename.c_str());
        return false;
    }

    OGRLayer* layer = dataset->GetLayer(0);
    if (!layer) {
        SDL_Log("FAILED TO GET LAYER");
        return false;
    }

    OGRSpatialReference* source = layer->GetSpatialRef();
    OGRSpatialReference target;
    target.importFromEPSG(3857);
    target.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

    if (source) {
        source->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        if (source->IsSame(&target)) {
            SDL_Log("DATA ALREADY EPSG:3857 - NO TRANSFORM");
            transform = nullptr;
        } else {
            transform = OGRCreateCoordinateTransformation(source, &target);
            if (!transform) {
                SDL_Log("FAILED TO CREATE CRS TRANSFORM");
                return false;
            }
            SDL_Log("CREATED CRS TRANSFORM -> EPSG:3857");
        }
    } else {
        SDL_Log("NO CRS FOUND - ASSUMING EPSG:4326");
        OGRSpatialReference assumed;
        assumed.importFromEPSG(4326);
        assumed.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        transform = OGRCreateCoordinateTransformation(&assumed, &target);
        if (!transform) {
            SDL_Log("FAILED TO CREATE DEFAULT TRANSFORM");
            return false;
        }
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
    if (!dataset) return result;

    OGRLayer* layer = dataset->GetLayer(0);

    if (!layer) return result;

    layer->ResetReading();

    OGRFeature* feature;
    while ((feature = layer->GetNextFeature()) != nullptr) {
        OGRGeometry* geometry = feature->GetGeometryRef();

        OGREnvelope env;
        geometry->getEnvelope(&env);
        SDL_Log("Top Left     : %.8f, %.8f", env.MinX, env.MaxY);
        SDL_Log("Top Right    : %.8f, %.8f", env.MaxX, env.MaxY);
        SDL_Log("Bottom Left  : %.8f, %.8f", env.MinX, env.MinY);
        SDL_Log("Bottom Right : %.8f, %.8f", env.MaxX, env.MinY);

        if (geometry) {
            processGeometry(geometry, result);
        }
        OGRFeature::DestroyFeature(feature);
    }

    return result;
}

void GeoJsonReader::processGeometry(OGRGeometry* geometry, std::vector<GeoFeature>& out) {
    if (!geometry)
        return;

    switch (wkbFlatten(geometry->getGeometryType())) {

    case wkbPoint:
    {
        GeoFeature feature;
        feature.type = GeometryType::Point;
        feature.id = nextFeatureID++;

        auto* point = geometry->toPoint();

        double x = point->getX();
        double y = point->getY();

        transformPoint(x, y);
        updateBounds(x, y);

        feature.vertices.push_back({ x, y, (float) feature.id});
        out.push_back(std::move(feature));
        break;
    }

    case wkbLineString:
    {
        GeoFeature feature;
        feature.type = GeometryType::LineString;
        feature.id = nextFeatureID++;

        processLineString(
            geometry->toLineString(),
            feature
        );
        out.push_back(std::move(feature));
        break;
    }

    case wkbPolygon:
    {
        GeoFeature feature;
        feature.type = GeometryType::Polygon;
        feature.id = nextFeatureID++;

        processPolygon(geometry->toPolygon(), feature);
        if (!feature.vertices.empty())
            out.push_back(std::move(feature));
        break;
    }

    case wkbMultiPolygon:
    {
        auto* multi = geometry->toMultiPolygon();
        // SDL_Log("MULTIPOLYGON CONTAINS %d POLYGONS", multi->getNumGeometries());

        for (int i = 0; i < multi->getNumGeometries(); ++i) {
            auto* poly = multi->getGeometryRef(i)->toPolygon();
            if (!poly) continue;
            
            GeoFeature feature;
            feature.type = GeometryType::Polygon;
            feature.id = nextFeatureID++;

            processPolygon(poly, feature);

            if (!feature.vertices.empty())
                out.push_back(std::move(feature));
        }
        break;
    }

    default:
        break;
    }
}

void GeoJsonReader::processLineString(OGRLineString* line,GeoFeature& feature) {
    int count = line->getNumPoints();
    for(int i = 0; i < count; i++) {
        double x = line->getX(i);
        double y = line->getY(i);
        transformPoint(x, y);
        updateBounds(x, y);
        feature.vertices.push_back({x, y, (float) feature.id});
    }
}

void GeoJsonReader::processPolygon(OGRPolygon* polygon, GeoFeature& feature) {
    if (!polygon) return;

    constexpr double MIN_HOLE_AREA = 100000.0;
    constexpr int MAX_HOLES = 32;

    auto processRing = [&](OGRLinearRing* ring) {
        if (!ring) return;

        feature.ringStarts.push_back(feature.vertices.size());

        int count = ring->getNumPoints();
        if (count < 2) return;

        for (int i = 0; i < count - 1; ++i) {
            double x = ring->getX(i);
            double y = ring->getY(i);

            transformPoint(x, y);
            updateBounds(x, y);

            feature.vertices.push_back({x, y, (float) feature.id});
        }
    };


    // -------------------------------
    // Exterior ring normalization
    // Earcut expects outer rings CCW
    // -------------------------------
    OGRLinearRing* exterior = polygon->getExteriorRing();
    if (exterior) {
        if (exterior->isClockwise()) {
            exterior->reverseWindingOrder();
        }
        processRing(exterior);
    }


    struct Hole {
        OGRLinearRing* ring;
        double area;
    };

    int holeCount = polygon->getNumInteriorRings();
    if (holeCount > 1000) {
        // SDL_Log("LARGE POLYGON: %d HOLES, KEEPING EXTERIOR ONLY", holeCount);
        processRing(polygon->getExteriorRing());
        return;
    }

    std::vector<Hole> validHoles;
    validHoles.reserve(holeCount);

    // -------------------------------
    // Hole normalization
    // Earcut expects holes CW
    // -------------------------------
    for (int i = 0; i < holeCount; ++i) {
        OGRLinearRing* ring = polygon->getInteriorRing(i);

        if (!ring) continue;

        double area = std::abs(ring->get_Area());

        if (area < MIN_HOLE_AREA) continue;

        if (!ring->isClockwise()) {
            ring->reverseWindingOrder();
        }
        validHoles.push_back({ ring, area });
    }


    if (validHoles.size() > MAX_HOLES) {
        SDL_Log("POLYGON HAS %zu HOLES, KEEPING LARGEST %d", validHoles.size(), MAX_HOLES);
        std::sort(
            validHoles.begin(),
            validHoles.end(),
            [](const Hole& a, const Hole& b) {
                return a.area > b.area;
            }
        );
    }

    int keep = std::min(static_cast<int>(validHoles.size()), MAX_HOLES);
    for (int i = 0; i < keep; ++i) {
        processRing(validHoles[i].ring);
    }
}