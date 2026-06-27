#include <fstream>
#include <format>
#include "dst.hpp"
#include "xml.cpp"

class SVG : public XML {
    std::ofstream &out;
    
public:
    SVG(std::ofstream &out, int x, int y, int width, int height) : XML(out), out(out) {
        create_tag("svg"); 
        create_attr("viewBox", std::format("{} {} {} {}", x, y, width, height));
        create_attr("xmlns", "http://www.w3.org/2000/svg");
    }
    
    ~SVG() {
        close_tag("svg");
        out << std::endl;
    }
};

void write_as_svg(std::ofstream &out_stream, DSTFile &dst_file) {
    // Calculate width and height to initialize SVG
    const int height = dst_file.header.y_range.second + dst_file.header.y_range.first;
    const int width = dst_file.header.x_range.second + dst_file.header.x_range.first;
    
    auto svg = SVG(out_stream, 0, 0, width, height);
    svg.create_tag("path");
    std::string path_string = "";
    
    // Translating first point in XY coords to SVG coords
    auto stitches = dst_file.body.stitches;
    std::pair<int, int> start_pt_xy = stitches[0];
    std::pair<int, int> start_pt_svg;
    start_pt_svg.first = dst_file.header.x_range.first + start_pt_xy.first;
    start_pt_svg.second = height - (dst_file.header.y_range.first + start_pt_xy.second);
    path_string += std::format("M {} {}", start_pt_svg.first, start_pt_svg.second);

    // Translating rest of lineTo moves to SVG coords
    for (auto stitch: stitches) {
        path_string += std::format(" l {} {}",
            stitch.first,
            -stitch.second
        );
    }
    svg.create_attr("d", path_string);
    svg.create_attr("fill", "none");
    svg.create_attr("stroke", "red");
    svg.create_attr("stroke-width", "3");

    svg.close_tag("path");
}