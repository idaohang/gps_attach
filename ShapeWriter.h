#ifndef SHAPE_WRITER_H
#define SHAPE_WRITER_H

#include <fstream>
#include <string>
#include "ShapeReader.h"
#include "RTreeLoader.h"
#include "TrackRAInterface.h"

#define DOUBLE_SIZE 8
#define INT_SIZE 4
#define CHAR_SIZE 1

//#define DEBUG

class ShapeWriter {
private:
    std::ofstream shape_file;
    std::ofstream index_file;
    void make_be_from_double(char (&buf)[DOUBLE_SIZE], double value);
    void make_le_from_double(char (&buf)[DOUBLE_SIZE], double value);
    void make_be_from_int(char (&buf)[INT_SIZE], int value);
    void make_le_from_int(char (&buf)[INT_SIZE], int value);
public:
    ShapeWriter(std::string file_name);
    ~ShapeWriter();
    void write(Shape &shape);
};

#endif // SHAPE_WRITER_H
