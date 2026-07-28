#pragma once

#include <string>
#include <vector>
#include <limits>

#include <gdal.h>
#include <ogrsf_frmts.h>

struct GeoVertex {
    double x;
    double y;
};

struct GeoBounds {
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
};

enum class GeometryType {
    Point,
    LineString,
    Polygon
};

struct GeoFeature {
    GeometryType type;
    std::vector<GeoVertex> vertices;
    std::vector<size_t> ringStarts;
};

class GeoJsonReader {
    public:
        GeoJsonReader();
        ~GeoJsonReader();

        bool open(const std::string& filename);

        std::vector<GeoFeature> readAll();

        GeoBounds GetBounds() const {
            return bounds;
        }

    private:
        GDALDataset* dataset = nullptr;
        OGRCoordinateTransformation* transform = nullptr;

        GeoBounds bounds;

        GeoFeature processGeometry(OGRGeometry* geometry);

        void transformPoint(double& x, double& y);

        void updateBounds(double x, double y);

        void processLineString(OGRLineString* line, GeoFeature& feature);

        void processPolygon(OGRPolygon* polygon, GeoFeature& feature);
};