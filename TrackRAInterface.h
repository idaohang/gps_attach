#ifndef TRACK_RA_INTERFACE_H
#define TRACK_RA_INTERFACE_H

#include <fstream>
#include <vector>
#include <utility>
#include <boost/assert.hpp>
#include <string>
#include <iostream>

//#define DEBUG

#define LONG_SIZE 8
#define INT_SIZE 4
#define CHAR_SIZE 1
#define DOUBLE_SIZE 8
#define COORD_MULT 10000000

#define SUB_PROVIDER_BLOCK_HEADER_SIZE (LONG_SIZE + INT_SIZE)
#define DEVICE_BLOCK_HEADER_SIZE (LONG_SIZE + LONG_SIZE)
#define POINT_BLOCK_SIZE (INT_SIZE * 2 + CHAR_SIZE + INT_SIZE * 2)

typedef struct {
    int sub_provider_id;
    unsigned int device_count; 
} SubProviderBlockHeader;

typedef struct {
    long device_id;
    unsigned int point_count;
} DeviceBlockHeader;

typedef struct {
    double lon, lat;
    unsigned char speed;
    int gps_time, sys_time;
} PointBlock;



class TrackRAInterface {
private:
    std::ifstream points_file;
    std::string file_name;
    std::vector<int> points_offsets;

    SubProviderBlockHeader get_prov_header();
    DeviceBlockHeader get_dev_header();
    PointBlock get_point_block();

    int get_int_from_le(unsigned char const (&buf)[INT_SIZE]);
    int get_long_from_le(unsigned char const (&buf)[LONG_SIZE]);
    double get_double_from_le(unsigned char const (&buf)[DOUBLE_SIZE]);
public:
    TrackRAInterface(std::string file_name);
    ~TrackRAInterface();
    std::pair<std::string, std::vector<int>> read();
};


#endif // TRACK_RA_INTERFACE_H

