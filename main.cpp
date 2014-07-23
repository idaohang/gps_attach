#include <iostream>
#include <string>
#include "ShapeReader.h"
#include "TrackRAInterface.h"
#include "RTreeLoader.h"
#include "JobRunner.h"
#include "ShapeWriter.h"

int main(int argc, char **argv)
{
    if (argc != 6) {
        std::cout << "USAGE: " << argv[0] << " " << "input_file.shp " 
                << "points_file.bin " << "output_file.shp " 
                << "jobs " << "epsilon" << std::endl;
        return 0;
    }

    ShapeReader reader(argv[1]);
    std::cout << "Reading shapefile " << argv[1] << "..." << std::endl;
    Shape shape = reader.read();
    std::cout << "Loading shapefile to R*-tree..." << std::endl;
    RTreeLoader loader(shape);
    
    TrackRAInterface iface(argv[2]);

    std::cout << "Creating RA interface to file with track data..." << std::endl;
    JobRunner job(loader.copy_tree(), iface.read());
    job.set_jobs(std::stoi(argv[4]));
    job.set_epsilon(std::stod(argv[5]));
    std::cout << "Running " << std::stoi(argv[4]) << " threads..." << std::endl;
    job.process(shape);
    std::cout << "Writing output data..." << std::endl;
    ShapeWriter writer(argv[3]);
    writer.write(shape);

    return 0;
}