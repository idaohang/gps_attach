#ifndef R_TREE_LOADER_H
#define R_TREE_LOADER_H

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <vector>
#include <iostream>
#include <boost/foreach.hpp>
#include <tuple>

#include "ShapeReader.h"

// #define DEBUG

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::cartesian> Point;
typedef bg::model::box<Point> Box;
typedef bg::model::segment<Point> Segment;
typedef std::tuple<Box, Segment, int> Node;
typedef bgi::rtree<Node, bgi::rstar<16>> Tree;

class RTreeLoader {
private:
    Tree rtree;
    Box make_box(Point first, Point second);
    Segment make_segment(Point first, Point second);
public:
    RTreeLoader(Shape &shape);
    Tree copy_tree();
};

#endif // R_TREE_LOADER_H
