#include "ShapeWriter.h"
//#define DEBUG

ShapeWriter::ShapeWriter(std::string file_name)
{
    shape_file.open(file_name, std::ios::out | std::ios::binary);
    if (!shape_file) {
        std::cout << "Unable to open output file " << file_name << std::endl;
        exit(1);
    }

    std::string idx_file_name = file_name;
    idx_file_name.erase(idx_file_name.end() - 3, idx_file_name.end());
    idx_file_name.append("shx");

    index_file.open(idx_file_name, std::ios::out | std::ios::binary);
    if (!shape_file) {
        std::cout << "Unable to open output file " << file_name << std::endl;
        exit(1);
    }
}

ShapeWriter::~ShapeWriter()
{
    shape_file.close();
    index_file.close();
}

void
ShapeWriter::write(Shape &shape)
{

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": shape.data.back().points.size()" 
                << shape.data.back().points.size() << std::endl;
    #endif
            
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": *(shape.data.back().points.begin()).size(): " 
                << (shape.data.back().points.begin())->size() << std::endl;
    #endif
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": *(shape.data.back().points.begin() + 1).size(): " 
                << (shape.data.back().points.begin() + 1)->size() << std::endl;
    #endif
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": *(shape.data.back().points.begin() + 2).size(): " 
                << (shape.data.back().points.begin() + 2)->size() << std::endl;
    #endif
    
    char double_bytes[DOUBLE_SIZE];
    char int_bytes[INT_SIZE];
    int content_length = 0;

    make_be_from_int(int_bytes, 9994);
    shape_file.write(int_bytes, sizeof int_bytes);


    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ << ": (header) wrote file code: " 
                << shape_file.tellp() << std::endl;
    #endif
    

    for (int i = 0; i < 5; i++) {
        make_be_from_int(int_bytes, 0);
        shape_file.write(int_bytes, sizeof int_bytes);
    }

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote unused bytes 4:24: " 
                << shape_file.tellp() << std::endl;
    #endif
    
    make_be_from_int(int_bytes, 42);
    shape_file.write(int_bytes, sizeof int_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote file length placeholder: " 
                << shape_file.tellp() << std::endl;
    #endif

    make_le_from_int(int_bytes, 1000);
    shape_file.write(int_bytes, sizeof int_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote file version: " 
                << shape_file.tellp() << std::endl;
    #endif

    make_le_from_int(int_bytes, 23);
    shape_file.write(int_bytes, sizeof int_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote shape type: " 
                << shape_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 
        shape.bounding_box.min_corner().get<0>());
    shape_file.write(double_bytes, sizeof double_bytes);
    
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote BB X min: " 
                << shape_file.tellp() << std::endl;
    #endif
    
    make_le_from_double(double_bytes, 
        shape.bounding_box.min_corner().get<1>());
    shape_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote BB Y min: " 
                << shape_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 
        shape.bounding_box.max_corner().get<0>());
    shape_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote BB X max: " 
                << shape_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 
        shape.bounding_box.max_corner().get<1>());
    shape_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote BB Y max: " 
                << shape_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 0);
    shape_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote BB Z min: " 
                << shape_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 0);
    shape_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote BB Z max: " 
                << shape_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 0);
    shape_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote BB M min: " 
                << shape_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 0);
    shape_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (header) wrote BB M max placeholder: " 
                << shape_file.tellp() << std::endl;
    #endif

    content_length += 100;
    int curr_content_length = 0;
    double max_speed = 0;
    int record_count = shape.data.size();

    std::vector<int> rec_offsets, rec_lengths;

    for (auto rec = shape.data.begin(); rec != shape.data.end(); rec++) {
        // Byte 0 | Record Number | Record Number | Integer | Big
        
        rec_offsets.push_back(shape_file.tellp() / 2);

        make_be_from_int(int_bytes, (*rec).record_number);
        shape_file.write(int_bytes, sizeof int_bytes);

        #ifdef DEBUG
        std::cout << std::endl;
        std::cout << __FILE__ << ":" << __LINE__ 
                    << ": writing record header #" 
                    << (*rec).record_number << std::endl;
        #endif
        
        // Byte 4 | Content Length | Content Length | Integer | Big
        curr_content_length = 4 + 8 * 4 + 4 + 4 + (*rec).num_parts * 4 + 
            (*rec).num_points * 2 * 8 + 8 + 8 + (*rec).num_points * 8;
        content_length += 8 + curr_content_length;

        make_be_from_int(int_bytes, curr_content_length / 2);
        shape_file.write(int_bytes, sizeof int_bytes);

        rec_lengths.push_back(curr_content_length / 2);

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ 
                    << ": wrote record header #" 
                    << (*rec).record_number << ": "
                    << shape_file.tellp() << std::endl;
        #endif

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": writing record #" 
                    << (*rec).record_number<< " body" 
                    << std::endl;
        #endif
        
        // Byte | 0 Shape Type | 23 | Integer | 1 | Little
        make_le_from_int(int_bytes, 23);
        shape_file.write(int_bytes, sizeof int_bytes);
                
        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": wrote shape type: " 
                    << shape_file.tellp() << std::endl;
        #endif
        
        // Byte | 4 | Box | Box | Double | 4 | Little
        make_le_from_double(double_bytes, 
                    (*rec).bounding_box.min_corner().get<0>());
        shape_file.write(double_bytes, sizeof double_bytes);
        make_le_from_double(double_bytes,
                    (*rec).bounding_box.min_corner().get<1>());
        shape_file.write(double_bytes, sizeof double_bytes);
        make_le_from_double(double_bytes,
                    (*rec).bounding_box.max_corner().get<0>());
        shape_file.write(double_bytes, sizeof double_bytes);
        make_le_from_double(double_bytes,
                    (*rec).bounding_box.max_corner().get<1>());
        shape_file.write(double_bytes, sizeof double_bytes);
        
        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": wrote BB: " 
                    << shape_file.tellp() << std::endl;
        #endif

        // Byte | 36 | NumParts | NumParts | Integer | 1 | Little
        make_le_from_int(int_bytes, (*rec).num_parts);
        shape_file.write(int_bytes, sizeof int_bytes);
        
        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": wrote num_parts: " 
                    << shape_file.tellp() << std::endl;
        #endif
        
        // Byte | 40 | NumPoints | NumPoints | Integer | 1 | Little
        make_le_from_int(int_bytes, (*rec).num_points);
        shape_file.write(int_bytes, sizeof int_bytes);

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": wrote num_points: " 
                    << shape_file.tellp() << std::endl;
        #endif

        // Byte | 44 | Parts | Parts | Integer | NumParts | Little

        int curr_offset = 0;
        for (auto part = (*rec).points.begin(); 
                            part != (*rec).points.end(); part++) {
            make_le_from_int(int_bytes, curr_offset);
            shape_file.write(int_bytes, sizeof int_bytes);
            curr_offset += (*part).size();
        }

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": wrote parts array: " 
                    << shape_file.tellp() << std::endl;
        #endif
    
        // Byte | X | Points | Points | Point | NumPoints | Little

        for (int part_num = 0; part_num < (*rec).points.size(); part_num++) {
            for (int point_num = 0; 
                    point_num < (*rec).points[part_num].size(); point_num++) {
                        
                Point curr_point = (*rec).points[part_num][point_num];
                #ifdef DEBUG
                std::cout << __FILE__ << ":" << __LINE__ << ": " 
                          << bg::wkt<Point>(curr_point) 
                          << std::endl;
                #endif
                make_le_from_double(double_bytes, (curr_point).get<0>());
                shape_file.write(double_bytes, sizeof double_bytes);

                make_le_from_double(double_bytes, (curr_point).get<1>());
                shape_file.write(double_bytes, sizeof double_bytes);
            }
        }

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": wrote points array: " 
                    << shape_file.tellp() << std::endl;
        #endif

        // Byte | Y* | Mmin | Mmin | Double | 1 | Little
        make_le_from_double(double_bytes, 0);
        shape_file.write(double_bytes, sizeof double_bytes);

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": wrote BB M min: " 
                    << shape_file.tellp() << std::endl;
        #endif

        // Byte | Y + 8* | Mmax | Mmax | Double | 1 | Little
        double avg_speed = (*rec).tracks_count != 0 ?
                (double)(*rec).speeds_sum / (double)(*rec).tracks_count : 0;
        make_le_from_double(double_bytes, avg_speed);
        shape_file.write(double_bytes, sizeof double_bytes);


        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": (*rec).speeds_sum: " 
                    << (*rec).speeds_sum << ", (*rec).tracks_count: "
                    << (*rec).tracks_count << std::endl; 
        #endif
        

        if (avg_speed > max_speed) {
            max_speed = avg_speed;
        }

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": wrote BB M max: " 
                    << shape_file.tellp() << std::endl;
        #endif

        // Byte | Y + 16* | Marray | Marray | Double | NumPoints | Little
        
        for (int i = 0; i < (*rec).num_points; i++) {
            make_le_from_double(double_bytes, avg_speed);
            shape_file.write(double_bytes, sizeof double_bytes);
        }

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": wrote M values array: " 
                    << shape_file.tellp() << std::endl;
        #endif
    }

    shape_file.seekp(24);
    make_be_from_int(int_bytes, content_length / 2);
    shape_file.write(int_bytes, sizeof int_bytes);

    shape_file.seekp(92);
    make_le_from_double(double_bytes, max_speed);
    shape_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ << ": max_speed = " << max_speed
        << std::endl;
    #endif

    /* Finished with the .shp file, proceeding to .shx */

    make_be_from_int(int_bytes, 9994);
    index_file.write(int_bytes, sizeof int_bytes);


    #ifdef DEBUG
    std::cout << std::endl;
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote file code: " 
                << index_file.tellp() << std::endl;
    #endif
    

    for (int i = 0; i < 5; i++) {
        make_be_from_int(int_bytes, 0);
        index_file.write(int_bytes, sizeof int_bytes);
    }

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote unused bytes 4:24: " 
                << index_file.tellp() << std::endl;
    #endif
    
    make_be_from_int(int_bytes, 50 + record_count * 4);
    index_file.write(int_bytes, sizeof int_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote file length: " 
                << index_file.tellp() << std::endl;
    #endif

    make_le_from_int(int_bytes, 1000);
    index_file.write(int_bytes, sizeof int_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote file version: " 
                << index_file.tellp() << std::endl;
    #endif

    make_le_from_int(int_bytes, 23);
    index_file.write(int_bytes, sizeof int_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote shape type: " 
                << index_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 
        shape.bounding_box.min_corner().get<0>());
    index_file.write(double_bytes, sizeof double_bytes);
    
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote BB X min: " 
                << index_file.tellp() << std::endl;
    #endif
    
    make_le_from_double(double_bytes, 
        shape.bounding_box.min_corner().get<1>());
    index_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote BB Y min: " 
                << index_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 
        shape.bounding_box.max_corner().get<0>());
    index_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote BB X max: " 
                << index_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 
        shape.bounding_box.max_corner().get<1>());
    index_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote BB Y max: " 
                << index_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 0);
    index_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote BB Z min: " 
                << index_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 0);
    index_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote BB Z max: " 
                << index_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, 0);
    index_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote BB M min: " 
                << index_file.tellp() << std::endl;
    #endif

    make_le_from_double(double_bytes, max_speed);
    index_file.write(double_bytes, sizeof double_bytes);

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index header) wrote BB M max: " 
                << index_file.tellp() << std::endl;
    #endif

    for (int i = 0; i < record_count; i++) {
        make_be_from_int(int_bytes, rec_offsets[i]);
        index_file.write(int_bytes, sizeof int_bytes);
        
        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ 
                << ": (index) wrote record #" << i << " offset == " 
                << rec_offsets[i] << ": " << index_file.tellp() << std::endl;
        #endif

        make_be_from_int(int_bytes, rec_lengths[i]);
        index_file.write(int_bytes, sizeof int_bytes);

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ 
                    << ": (index) wrote record #" << i << " length == "
                    << rec_lengths[i] << ": "<< index_file.tellp() << std::endl;
        #endif
    }
}

void
ShapeWriter::make_be_from_double(char (&buf)[DOUBLE_SIZE], double value)
{
    union {
        double value;
        unsigned char bytes[DOUBLE_SIZE];
    } data;

    data.value = value;
    
    for (int i = 0; i < DOUBLE_SIZE; i++) {
        buf[i] = data.bytes[DOUBLE_SIZE - i - 1];
    }
}

void
ShapeWriter::make_le_from_double(char (&buf)[DOUBLE_SIZE], double value)
{
    union {
        double value;
        unsigned char bytes[DOUBLE_SIZE];
    } data;

    data.value = value;

    for (int i = 0; i < DOUBLE_SIZE; i++) {
        buf[i] = data.bytes[i];
    }
}

void
ShapeWriter::make_be_from_int(char (&buf)[INT_SIZE], int value)
{
    union {
        int value;
        unsigned char bytes[INT_SIZE];
    } data;

    data.value = value;

    for (int i = 0; i < INT_SIZE; i++) {
        buf[i] = data.bytes[INT_SIZE - i - 1];
    }
}

void
ShapeWriter::make_le_from_int(char (&buf)[INT_SIZE], int value)
{
    union {
        int value;
        unsigned char bytes[INT_SIZE];
    } data;

    data.value = value;

    for (int i = 0; i < INT_SIZE; i++) {
        buf[i] = data.bytes[i];
    }
}