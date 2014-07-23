#include "JobRunner.h"

JobRunner::JobRunner(Tree rtree, std::pair<std::string, 
                                            std::vector<int>> iface)
{
    points_file.open(iface.first, std::ios::in | std::ios::binary);
    if (!points_file) {
        std::cout << "Unable to open input file " << iface.first << std::endl;
        exit(1);
    }

    this->rtree = rtree;
    this->points_offsets = iface.second;
    this->jobs = 0;
    this->epsilon = 0;
}

JobRunner::~JobRunner()
{
    points_file.close();
}

void
JobRunner::process(Shape &shape)
{
    // assert jobs > points_offsets.size()
    int curr_begin = 0, curr_end = points_offsets.size() / jobs;
    for (int i = 0; i < jobs - 1; i++) {
        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ 
                    << ": points_offsets.size() == " << points_offsets.size()
                    << " begin = " << curr_begin 
                    << " end = " << curr_end << std::endl;
        #endif

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": curr_begin: " 
                    << curr_begin << std::endl;
        #endif

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": curr_end: " 
                    << curr_end << std::endl;
        #endif        
        
        threads.emplace_back(&JobRunner::thread_worker, this,
                                 std::ref(shape), rtree, curr_begin, curr_end);
        curr_begin += points_offsets.size() / jobs;
        curr_end += points_offsets.size() / jobs;
    }

    curr_end += points_offsets.size() % jobs;

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ 
                << ": points_offsets.size() == " << points_offsets.size()
                << " begin = " << curr_begin 
                << " end = " << curr_end << std::endl;
    #endif

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ << ": curr_begin: " 
                << curr_begin << std::endl;
    #endif

    #ifdef DEBUG
    std::cout << __FILE__ << ":" << __LINE__ << ": curr_end: " 
                << curr_end << std::endl;
    #endif        

    threads.emplace_back(&JobRunner::thread_worker, this,
                             std::ref(shape), rtree, curr_begin, curr_end);

    for (int i = 0; i < jobs; i++) {
        threads[i].join();
    }
}

void
JobRunner::thread_worker(Shape &shape, Tree rtree, int begin, int end)
{
    PointBlock curr_point_block;
    // ToDo: refactor to smth else instead of std::vector to avoid
    // wasting resources for dealing with the vectors of length 1
    std::vector<Node> result_n;  

    #ifdef DEBUG
    std::cout.precision(9);
    #endif
    

    // ToDo: switch to using arbitrary precision arithmetics, e.g. GMP
    // to prevent precision loss while substracting and integer overflow
    for (int i = begin; i < end; i++) {
            {
                std::lock_guard<std::recursive_mutex> locker(lock);

                curr_point_block = get_point_block(points_offsets[i]);

                result_n.clear();
            }
        rtree.query(bgi::nearest(Point(curr_point_block.lon, 
            curr_point_block.lat), 1), std::back_inserter(result_n));

        // A fancy way to determine if a point R = (x_0, y_0) lies on the line 
        // connecting points P = (x_1, y_1) and Q = (x_2, y_2) is to check
        // whether the determinant of the matrix
        // {{x_2 - x_1, y_2 - y_1}, {x_0 - x_1, y_0 - y_1}},
        // namely 
        // (x_2 - x_1) * (y_0 - y_1) - (y_2 - y_1) * (x_0 - x_1) 
        // is close to 0.

        double x_0 = curr_point_block.lon; 
        double y_0 = curr_point_block.lat; 
        double x_1 = std::get<1>(result_n.back()).first.get<0>();
        double y_1 = std::get<1>(result_n.back()).first.get<1>();
        double x_2 = std::get<1>(result_n.back()).second.get<0>();
        double y_2 = std::get<1>(result_n.back()).second.get<1>();


        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": Point: " 
                    << bg::wkt<Point>(Point(x_0, y_0)) << std::endl;
        #endif
        
        #ifdef DEBUG
        std::cout << std::fixed << __FILE__ << ":" << __LINE__ << ": "  
                    << bg::wkt<Segment>(std::get<1>(result_n.back()))
                                 << std::endl;
        #endif

        auto determinant = (x_2 - x_1) * (y_0 - y_1) - (y_2 - y_1) *(x_0 - x_1);

        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": determinant:" 
                    << determinant << std::endl;
        #endif
                
        #ifdef DEBUG
        std::cout << __FILE__ << ":" << __LINE__ << ": " 
                    << std::min(x_1, x_2) << " " 
                        << std::max(x_1, x_2) << std::endl;
        std::cout << __FILE__ << ":" << __LINE__ << ": " 
                            << std::min(y_1, y_2)  << " " 
                              << std::max(y_1, y_2) << std::endl;

        #endif
        
        if (abs(determinant) < epsilon) { //&&
            // // ToDo: think if we do really need the following two lines
            // std::min(x_1, x_2) <= x_0 && x_0 <= std::max(x_1, x_2) &&
            // std::min(y_1, y_2) <= y_0 && y_0 <= std::max(y_1, y_2)) {

            auto curr_segment = std::get<1>(result_n.back());

            #ifdef DEBUG
            std::cout << __FILE__ << ":" << __LINE__ 
                    << ": Segment found successfully" << std::endl;
            #endif
            
            // std::lock_guard<std::recursive_mutex> locker(lock);

            shape.push_speed(std::get<2>(result_n.back()),
                                 curr_point_block.speed);


            #ifdef DEBUG
            std::cout << __FILE__ << ":" << __LINE__ << ": Speed: " 
                        << (int)curr_point_block.speed << std::endl;
            #endif
            
        }         
        #ifdef DEBUG
        else {
            std::cout << __FILE__ << ":" << __LINE__ 
                    << ": Segment not found" << std::endl;
        }
        #endif
        

        #ifdef DEBUG
        std::cout << std::endl;
        #endif
    }
}


PointBlock
JobRunner::get_point_block(int offset)
{
    PointBlock result;
    char raw_data[POINT_BLOCK_SIZE];
    unsigned char int_bytes[INT_SIZE];

    points_file.seekg(offset);
    points_file.read(raw_data, POINT_BLOCK_SIZE);

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

inline int
JobRunner::get_int_from_le(unsigned char const (&buf)[INT_SIZE])
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

void
JobRunner::set_epsilon(double epsilon)
{
    this->epsilon = epsilon;
}

void
JobRunner::set_jobs(int jobs)
{
    this->jobs = jobs;
}
