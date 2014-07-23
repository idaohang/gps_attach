#include "RTreeLoader.h"

RTreeLoader::RTreeLoader(Shape &shape)
{
    std::pair<PolyLine, int> curr_record;
    while (true)
    {
        curr_record = shape.pop_record();
        if (curr_record.second == -1) {
            break;
        }
        while (true) {
            std::vector<Point> part = curr_record.first.pop_part();
            if (part.empty()) {
                break;
            }

            Point curr_point = part[0];
            for (int i = 1; i < part.size(); i++ /* i += 2 ???*/) {
                Box curr_box = make_box(part[i - 1], part[i]);

                #ifdef DEBUG
                std::cout << __FILE__ << ":" << __LINE__ << ": " 
                            << bg::wkt<Box>(curr_box) << std::endl;
                #endif
                
                Segment curr_segment = make_segment(part[i - 1], part[i]);

                #ifdef DEBUG
                std::cout << __FILE__ << ":" << __LINE__ << ": " 
                            << bg::wkt<Segment>(curr_segment) << std::endl;
                #endif
                
                Node curr_value = std::make_tuple(curr_box, curr_segment,
                                                    curr_record.second);
                
                rtree.insert(curr_value);
            }
        }
    }
}

Box
RTreeLoader::make_box(Point first, Point second)
{
    return Box(Point(std::min(first.get<0>(), second.get<0>()),
                     std::min(first.get<1>(), second.get<1>())), 
                Point(std::max(first.get<0>(), second.get<0>()),
                      std::max(first.get<1>(), second.get<1>())));

}

Segment
RTreeLoader::make_segment(Point first, Point second)
{
    return Segment(first, second);

}

Tree
RTreeLoader::copy_tree()
{
    return rtree;
}
