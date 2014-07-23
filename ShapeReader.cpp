#include "ShapeReader.h"

//#define DEBUG

// ToDo: refactor the mess of for cycles (change them with function calls)

/* PolyLine class implementation */
PolyLine::PolyLine()
{
    this->record_number = 0;
    this->content_length = 0;

    num_parts = 0;
    num_points = 0;
    bounding_box = Box(Point(0, 0), Point(0, 0));
    curr_part = 0;
    is_valid = false;

    this->tracks_count = 0;
    this->speeds_sum = 0;
}


PolyLine::PolyLine(Box const bounding_box, 
                std::vector<std::vector<Point>> const points)
{
    this->record_number = 0;
    this->content_length = 0;

    this->bounding_box = bounding_box;
    this->points = points;
    this->curr_part = 0;        
    this->is_valid = true;

    this->tracks_count = 0;
    this->speeds_sum = 0;
}

void
PolyLine::set_record_number(int record_number)
{
    this->record_number = record_number;
}

void
PolyLine::set_content_length(int content_length)
{
    this->content_length = content_length;
}


void 
PolyLine::set_num_parts(int const num_parts)
{
    this->num_parts = num_parts;
}

void 
PolyLine::set_num_points(int const num_points)
{   
    this->num_points = num_points;
}

std::vector<Point> 
PolyLine::pop_part()
{

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ << ": points[curr_part].size(): " 
                << points[curr_part].size() << std::endl;
    #endif
    
    if (curr_part < points.size()) {
        return points[curr_part++];
    }

    return std::vector<Point>(0);
}

PolyLine::operator bool() const
{
    return is_valid;      
}

void 
PolyLine::push_speed(int speeds_sum)
{
    this->tracks_count++;
    this->speeds_sum += speeds_sum;
}

/* Shape class implementation */
Shape::Shape()
{

    this->bounding_box = Box(Point(0, 0), Point(0, 0));
    this->curr_record = 0;    
}

// Shape::Shape(Shape const &that)
// {
//     this->bounding_box = that.bounding_box;
//     this->data = that.data;
//     this->curr_record = 0; 
// }

Shape::Shape(Box const bounding_box,
        std::vector<PolyLine> const data)
{
    this->bounding_box = bounding_box;
    this->data = data;
    this->curr_record = 0;
}

std::pair<PolyLine, int>
Shape::pop_record()
{
    if (curr_record < data.size()) {
        auto result = std::make_pair(data[curr_record], curr_record);
        curr_record++;
        return result;
    }

    return std::make_pair(PolyLine(), -1); // ToDo: refactor!
}

void 
Shape::push_speed(int record_number, int speeds_sum)
{
    data[record_number].push_speed(speeds_sum);
}

/* ShapeReader class implementation */
// ToDo: refactor to std::string instead of char *
ShapeReader::ShapeReader(std::string const file_name)
{
    BOOST_ASSERT(sizeof(unsigned char) == CHAR_SIZE);
    BOOST_ASSERT(sizeof(int) == INT_SIZE);
    BOOST_ASSERT(sizeof(double) == DOUBLE_SIZE);

    shape_file.open(file_name, std::ios::in | std::ios::binary);
    if (!shape_file) {
        std::cout << "Unable to open input file " << file_name << std::endl;
        exit(1);
    }
}

ShapeReader::~ShapeReader()
{
    shape_file.close();
}

double
ShapeReader::get_double_from_be(unsigned char const (&buf)[DOUBLE_SIZE])
{
    union {
        double result;
        unsigned char bytes[DOUBLE_SIZE];
    } data;

    for (int i = 0; i < DOUBLE_SIZE; i++) {
        data.bytes[i] = buf[DOUBLE_SIZE - i - 1];
    }

    return data.result;
}

double
ShapeReader::get_double_from_le(unsigned char const (&buf)[DOUBLE_SIZE])
{
    union {
        double result;
        unsigned char bytes[DOUBLE_SIZE];
    } data;

    for (int i = 0; i < DOUBLE_SIZE; i++) {
        data.bytes[i] = buf[i];
    }

    return data.result;
}

int
ShapeReader::get_int_from_be(unsigned char const (&buf)[INT_SIZE])
{
    union {
        int result;
        unsigned char bytes[INT_SIZE];
    } data;

    for (int i = 0; i < INT_SIZE; i++) {
        data.bytes[i] = buf[INT_SIZE - i - 1];
    }

    return data.result;
}

int
ShapeReader::get_int_from_le(unsigned char const (&buf)[INT_SIZE])
{
    union {
        int result;
        unsigned char bytes[INT_SIZE];
    } data;

    for (int i = 0; i < INT_SIZE; i++) {
        data.bytes[i] = buf[i];
    }

    return data.result;
}

ShapeReader::FileHeader 
ShapeReader::get_file_header()
{
    FileHeader result;
    char raw_data[FILE_HEADER_SIZE];
    unsigned char int_bytes[INT_SIZE], double_bytes[DOUBLE_SIZE];

    shape_file.read(raw_data, FILE_HEADER_SIZE);
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__
        << ": shape_file.tellg(): "
        << shape_file.tellg() << std::endl;
    #endif

    // Position | Field | Value | Type | Byte Order

    // Byte 0 | File Code | 9994 | Integer | Big

    for (int i = 0; i < INT_SIZE; i++) {
        int_bytes[i] = raw_data[i];
    }
    result.file_code = get_int_from_be(int_bytes);

    // Byte 24 | File Length | File Length | Integer | Big

    for (int i = 24; i < 24 + INT_SIZE; i++) {
        int_bytes[i - 24] = raw_data[i];
    }
    result.file_length = get_int_from_be(int_bytes);

    // Byte 28 | Version | 1000 | Integer | Little

    for (int i = 28; i < 28 + INT_SIZE; i++) {
        int_bytes[i - 28] = raw_data[i];
    }
    result.version = get_int_from_le(int_bytes);

    // Byte 32 | Shape Type | Shape Type | Integer | Little

    for (int i = 32; i < 32 + INT_SIZE; i++) {
        int_bytes[i - 32] = raw_data[i];
    }
    result.shape_type = get_int_from_le(int_bytes);

    double x_min, y_min, x_max, y_max;
    // Byte 36 | Bounding Box | Xmin | Double | Little

    for (int i = 36; i < 36 + DOUBLE_SIZE; i++) {
        double_bytes[i - 36] = raw_data[i];
    }
    x_min = get_double_from_le(double_bytes);

    // Byte 44 | Bounding Box | Ymin | Double | Little

    for (int i = 44; i < 44 + DOUBLE_SIZE; i++) {
        double_bytes[i - 44] = raw_data[i];
    }
    y_min = get_double_from_le(double_bytes);

    // Byte 52 | Bounding Box | Xmax | Double | Little

    for (int i = 52; i < 52 + DOUBLE_SIZE; i++) {
        double_bytes[i - 52] = raw_data[i];
    }
    x_max = get_double_from_le(double_bytes);

    // Byte 60 | Bounding Box | Ymax | Double | Little

    for (int i = 60; i < 60 + DOUBLE_SIZE; i++) {
        double_bytes[i - 60] = raw_data[i];
    }
    y_max = get_double_from_le(double_bytes);

    Point min_point(x_min, y_min);
    Point max_point(x_max, y_max);

    result.bounding_box = Box(min_point, max_point);

    return result;
}

int
ShapeReader::get_record_header(int &record_number, int &content_length)
{
    char raw_data[RECORD_HEADER_SIZE];
    unsigned char int_bytes[INT_SIZE];
    shape_file.read(raw_data, RECORD_HEADER_SIZE);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__
        << ": shape_file.tellg(): "
        << shape_file.tellg() << std::endl;
    #endif
    
    if (shape_file.eof()) {
        return 0;
    }

    // Byte 0 | Record Number | Record Number | Integer | Big
    for (int i = 0; i < INT_SIZE; i++) {
        int_bytes[i] = raw_data[i];
    }
    record_number = get_int_from_be(int_bytes);    

    // Byte 4 | Content Length | Content Length | Integer | Big
    for (int i = 4; i < 4 + INT_SIZE; i++) {
        int_bytes[i - 4] = raw_data[i];
    }
    content_length = get_int_from_be(int_bytes);    

    return RECORD_HEADER_SIZE;
}

PolyLine
ShapeReader::get_record(int const record_number, int const content_length)
{
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ << ": content_length: "
                                << content_length << std::endl;
    #endif 

    char *raw_data = new char[content_length * BYTES_IN_WORD];
    unsigned char int_bytes[INT_SIZE], double_bytes[DOUBLE_SIZE];

    shape_file.read(raw_data, content_length * BYTES_IN_WORD);
    
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__
        << ": shape_file.tellg(): "
        << shape_file.tellg() << std::endl;
    #endif

    double x_min, y_min, x_max, y_max;
    // Byte 4 | Xmin | Box | Double | Little


    for (int i = 4; i < 4 + DOUBLE_SIZE; i++) {
        double_bytes[i - 4] = raw_data[i];
    }
    x_min = get_double_from_le(double_bytes);

    // Byte 12 | Ymin | Box | Double | Little
    
     for (int i = 12; i < 12 + DOUBLE_SIZE; i++) {
        double_bytes[i - 12] = raw_data[i];
    }
    y_min = get_double_from_le(double_bytes);

    // Byte 20 | Xmax | Box | Double | Little
    
     for (int i = 20; i < 20 + DOUBLE_SIZE; i++) {
        double_bytes[i - 20] = raw_data[i];
    }
    x_max = get_double_from_le(double_bytes);

    // Byte 28 | Ymax | Box | Double | Little

    for (int i = 28; i < 28 + DOUBLE_SIZE; i++) {
        double_bytes[i - 28] = raw_data[i];
    }
    y_max = get_double_from_le(double_bytes);

    Point min_point(x_min, y_min);
    Point max_point(x_max, y_max);

    Box bounding_box(min_point, max_point);

    // Byte 36 | NumParts | NumParts | Integer | Little

    int num_parts, num_points;
    for (int i = 36; i < 36 + INT_SIZE; i++) {
        int_bytes[i - 36] = raw_data[i];
    }
    num_parts = get_int_from_le(int_bytes);

    // Byte 40 | NumPoints | NumPoints | Integer | Little
    
    for (int i = 40; i < 40 + INT_SIZE; i++) {
        int_bytes[i - 40] = raw_data[i];
    }
    num_points = get_int_from_le(int_bytes);

    // Byte 44 | Parts | Parts | Integer | Little

    std::vector<int> parts;

    for (int j = 0; j < num_parts; j++) { // iterating over parts
        int curr_offset = j * INT_SIZE; 
        // we read 4 bytes of integer value in the inner cycle
        for (int i = 44 + curr_offset; i < 44 + curr_offset + INT_SIZE; i++) { 
            int_bytes[i - 44 - curr_offset] = raw_data[i];
        }
        parts.push_back(get_int_from_le(int_bytes));
    }

    // we add fake last record to simplify parts counting while reading
    parts.push_back(num_points); 

    // Byte X = 44 + 4 * NumParts | Points | Points | Point | Little

    std::vector<std::vector<Point>> points;

    int start_from = 44 + 4 * num_parts;
    double curr_x, curr_y;

    for (int k = 1; k < parts.size(); k++) {
        points.push_back(std::vector<Point>(0));

        for (int j = parts[k - 1]; j < parts[k]; j++) {
            // read double curr_x
            
            for (int i = start_from; i < start_from + DOUBLE_SIZE; i++) {
                double_bytes[i - start_from] = raw_data[i];
            }
            curr_x = get_double_from_le(double_bytes);
            start_from += DOUBLE_SIZE;

            // read double curr_y
            for (int i = start_from; i < start_from + DOUBLE_SIZE; i++) {
                double_bytes[i - start_from] = raw_data[i];
            }
            curr_y = get_double_from_le(double_bytes);
            start_from += DOUBLE_SIZE;

            points.back().emplace_back(curr_x, curr_y);
        }
    }
    
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": points.size(): " 
                << points.size() << std::endl;
    #endif
            
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": points.begin()->size(): " 
                << points.begin()->size() << std::endl;
    #endif

    PolyLine result(bounding_box, points);
    result.set_record_number(record_number);
    result.set_num_parts(num_parts);
    result.set_num_points(num_points);

    delete[] raw_data;

    return result;
}

Shape
ShapeReader::read()
{
    file_header = get_file_header();
    int record_number = 0, content_length = 0;
    std::vector<PolyLine> data;

    while (get_record_header(record_number, content_length)) {
        data.push_back(get_record(record_number, content_length));
    }

    Shape result(file_header.bounding_box, data);

    return result;
}
