#include "TrackRAInterface.h"

TrackRAInterface::TrackRAInterface(std::string file_name)
{
    BOOST_ASSERT(sizeof(int) == INT_SIZE);
    BOOST_ASSERT(sizeof(int) == LONG_SIZE);
    BOOST_ASSERT(sizeof(char) == CHAR_SIZE);
            
    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ << ": " 
                << file_name << std::endl;
    #endif

    this->file_name = file_name;
    points_file.open(file_name, std::ios::in | std::ios::binary);
    if (!points_file) {
        std::cout << "Unable to open input file " << file_name << std::endl;
        exit(1);
    }
}

TrackRAInterface::~TrackRAInterface()
{
    points_file.close();
}

int
TrackRAInterface::get_int_from_le(unsigned char const (&buf)[INT_SIZE])
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

int
TrackRAInterface::get_long_from_le(unsigned char const (&buf)[LONG_SIZE])
{
    union {
        int result;
        unsigned char bytes[LONG_SIZE];
    } data;

    for (int i = 0; i < LONG_SIZE; i++) {
        data.bytes[i] = buf[i];
    }

    return data.result;
}

double
TrackRAInterface::get_double_from_le(unsigned char const (&buf)[DOUBLE_SIZE])
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

SubProviderBlockHeader
TrackRAInterface::get_prov_header()
{
    SubProviderBlockHeader result;
    char raw_data[SUB_PROVIDER_BLOCK_HEADER_SIZE];
    unsigned char int_bytes[INT_SIZE], long_bytes[LONG_SIZE];

    points_file.read(raw_data, SUB_PROVIDER_BLOCK_HEADER_SIZE);

    for (int i = 0; i < INT_SIZE; i++) {
        int_bytes[i] = raw_data[i];
    }
    result.sub_provider_id = get_int_from_le(int_bytes);

    for (int i = INT_SIZE; i < INT_SIZE + LONG_SIZE; i++) {
        long_bytes[i - INT_SIZE] = raw_data[i];
    }
    result.device_count = get_long_from_le(long_bytes);

    return result;
}

DeviceBlockHeader
TrackRAInterface::get_dev_header()
{
    DeviceBlockHeader result;
    char raw_data[DEVICE_BLOCK_HEADER_SIZE];
    unsigned char int_bytes[INT_SIZE], long_bytes[LONG_SIZE];

    points_file.read(raw_data, DEVICE_BLOCK_HEADER_SIZE);

    for (int i = 0; i < LONG_SIZE; i++) {
        long_bytes[i] = raw_data[i];
    }
    result.device_id = get_long_from_le(long_bytes);

    for (int i = LONG_SIZE; i < LONG_SIZE + LONG_SIZE; i++) {
        long_bytes[i - LONG_SIZE] = raw_data[i];
    }
    result.point_count = get_long_from_le(long_bytes);

    return result;
}

PointBlock
TrackRAInterface::get_point_block()
{
    points_offsets.push_back(points_file.tellg());

    PointBlock result = {0};
    char raw_data[POINT_BLOCK_SIZE];
    unsigned char int_bytes[INT_SIZE];

    points_file.read(raw_data, POINT_BLOCK_SIZE);

    if (points_file.eof()) {
        //std::cout << "Corrupted file " << file_name << ". Skipping." 
        //                << std::endl;
        return result;
    }

    for (int i = 0; i < INT_SIZE; i++) {
        int_bytes[i] = raw_data[i];
    }
    result.lon = (double)get_int_from_le(int_bytes) / COORD_MULT;

    for (int i = INT_SIZE; i < INT_SIZE + INT_SIZE; i++) {
        int_bytes[i - INT_SIZE] = raw_data[i];
    }
    result.lat = (double)get_int_from_le(int_bytes) / COORD_MULT;

    result.speed = raw_data[INT_SIZE * 2];

    int curr_offset = INT_SIZE * 2 + 1;
    for (int i = curr_offset; i < curr_offset + INT_SIZE; i++) {
        int_bytes[i - curr_offset] = raw_data[i];
    }
    result.gps_time = get_int_from_le(int_bytes);
    
    curr_offset += INT_SIZE;
    for (int i = curr_offset; i < curr_offset + INT_SIZE; i++) {
        int_bytes[i - curr_offset] = raw_data[i];
    }
    result.sys_time = get_int_from_le(int_bytes);

    return result;
}

std::pair<std::string, std::vector<int>>
TrackRAInterface::read()
{
    SubProviderBlockHeader curr_prov_header;
    DeviceBlockHeader curr_dev_header;
    PointBlock curr_point_block;

    while(true) {
        curr_prov_header = get_prov_header();
        
        if (points_file.eof()) {
            break;
        }

        for (int j = 0; j < curr_prov_header.device_count; j++) {
            curr_dev_header = get_dev_header();
            for (int i = 0; i < curr_dev_header.point_count; i++) {
                curr_point_block = get_point_block();
                if (points_offsets.size() % 1000000 == 0) {
                    #ifdef DEBUG
                    std::cout << "points_offsets.size() == " 
                            << points_offsets.size() << std::endl;
                    #endif
                }
                if (!curr_point_block.gps_time) {
                    #ifdef DEBUG
                    std::cout << __FILE__ << ":" << __LINE__ << ": " 
                                << file_name << std::endl;
                    #endif
                    return std::make_pair(file_name, points_offsets);
                }
            }
        }
    }

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ << ": " 
                << file_name << std::endl;
    #endif
    
    return std::make_pair(file_name, points_offsets);
}

