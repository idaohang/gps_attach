#ifndef JOB_RUNNER_H
#define JOB_RUNNER_H

#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <iomanip>
#include <thread>
#include <functional>
#include <mutex>
#include "RTreeLoader.h"
#include "TrackRAInterface.h"

//#define DEBUG

#define COORD_MULT 10000000

class JobRunner {
private:
    std::ifstream points_file;
    PointBlock get_point_block(int offset);
    inline int get_int_from_le(unsigned char const (&buf)[INT_SIZE]);
    std::vector<int> points_offsets;
    Tree rtree;
    double epsilon;
    int jobs;
    void thread_worker(Shape &shape, Tree rtree, int begin, int end);
    std::vector<std::thread> threads;
    std::recursive_mutex lock;
public:
    JobRunner(Tree rtree, std::pair<std::string, std::vector<int>> iface);
    ~JobRunner();
    void process(Shape &shape);
    void set_epsilon(double epsilon);
    void set_jobs(int jobs);
};

#endif // JOB_RUNNER_H
