#ifndef SHAPE_READER_H
#define SHAPE_READER_H

#include <boost/assert.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <fstream>
#include <string>
#include <utility>
#include <iostream>

#define DOUBLE_SIZE 8
#define INT_SIZE 4
#define CHAR_SIZE 1
#define FILE_HEADER_SIZE 100
#define RECORD_HEADER_SIZE 8
#define BYTES_IN_WORD 2

// #define DEBUG

namespace bg = boost::geometry;

typedef bg::model::point<double, 2, bg::cs::cartesian> Point;
typedef bg::model::box<Point> Box;

/* ShapeType == 3 */
class PolyLine { 
    // friend class Util; // needed for testing purposes
    friend class ShapeWriter; // ToDo: refactor this using interfaces
private:
    int record_number, content_length;
    int num_parts, num_points;
    Box bounding_box; // local (per-record)
    std::vector<std::vector<Point>> points; // vector of parts 
    // ToDo: refactor to std::lists and iterators 
    // instead of vectors and ints, respectively
    int curr_part;
    bool is_valid;

    int tracks_count, speeds_sum;
public:
    PolyLine();
    PolyLine(Box const bounding_box, 
                std::vector<std::vector<Point>> const points);
    void set_record_number(int record_number);
    void set_content_length(int content_length);
    void set_num_parts(int const num_parts);
    void set_num_points(int const num_points);

    std::vector<Point> pop_part(); // ToDo: refactor to boost::optional?
    // boost::optional<std::vector<Point>> pop_part(); 

    explicit operator bool() const;
    void push_speed(int speeds_sum);
};

class Shape {
    // friend class Util;
    friend class ShapeWriter;
private:
    Box bounding_box; // global; copied from file header
    std::vector<PolyLine> data;
    int curr_record;
public:
    Shape();
    // Shape(Shape const &that);
    Shape(Box const bounding_box, std::vector<PolyLine> const data);
    std::pair<PolyLine, int> pop_record(); 
    // boost::optional<PolyLine> pop_record(); 
    void push_speed(int record_number, int speeds_sum);
};

class ShapeReader {
    // friend class Util;
    friend class ShapeWriter;
private:
    struct FileHeader {
        int file_code, file_length, version, shape_type;
        Box bounding_box;
    };

    double get_double_from_be(unsigned char const (&buf)[DOUBLE_SIZE]);
    double get_double_from_le(unsigned char const (&buf)[DOUBLE_SIZE]);
    int get_int_from_be(unsigned char const (&buf)[INT_SIZE]);
    int get_int_from_le(unsigned char const (&buf)[INT_SIZE]);
    // refactor to returning a struct of two ints
    int get_record_header(int &record_number, int &content_length);
    PolyLine get_record(int const record_number, int const content_length);

    FileHeader get_file_header();
    
    std::ifstream shape_file;
    FileHeader file_header;

public:
    ShapeReader(std::string const fname); // ToDo: switch to boost::fs
    ~ShapeReader();
    Shape read();
};

#endif /* SHAPE_READER_H */