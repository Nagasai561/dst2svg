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

// This is the default color palette used in the matplotlib library.
// i-th color corresponds to `Ci` in the matplotlib color map.
const std::vector<std::string> color_pallete{
    "#1f77b4",
    "#ff7f0e",	
    "#2ca02c",	
    "#d62728",	
    "#9467bd",	
    "#8c564b",	
    "#e377c2",	
    "#7f7f7f",	
    "#bcbd22",	
    "#17becf"	
};

const std::string SVG_STROKE_WIDTH = "3";

void write_as_svg(std::ofstream &out_stream, DSTFile &dst_file) {
    // Calculate width and height to initialize SVG
    const int height = dst_file.header.y_range.second + dst_file.header.y_range.first;
    const int width = dst_file.header.x_range.second + dst_file.header.x_range.first;
    
    auto svg = SVG(out_stream, 0, 0, width, height);
    int color_idx = 0;
    svg.create_tag("path");
    svg.create_attr("stroke", color_pallete[color_idx]);
    svg.create_attr("stroke-width", SVG_STROKE_WIDTH);
    svg.create_attr("fill", "none");

    std::string path_string = "";
    
    // Translating first point in XY coords to SVG coords
    auto stitches = dst_file.body.stitches;
    std::pair<int, int> start_pt_xy{stitches[0].delta_x, stitches[0].delta_y};
    std::pair<int, int> curr_pt_svg;
    curr_pt_svg.first = dst_file.header.x_range.first + start_pt_xy.first;
    curr_pt_svg.second = height - (dst_file.header.y_range.first + start_pt_xy.second);
    path_string += std::format("M {} {}", curr_pt_svg.first, curr_pt_svg.second);

    // Translating rest of lineTo moves to SVG coords
    for (int i = 1; i < stitches.size(); i++) {
        auto stitch = stitches[i];
    }
    
    for (auto stitch: stitches) {
        curr_pt_svg.first += stitch.delta_x;
        curr_pt_svg.second += -stitch.delta_y;

        if (stitch.is_color_change) {
            color_idx = (color_idx + 1) % color_pallete.size();
        }
        
        if (stitch.is_jump) {
            svg.create_attr("d", path_string);
            path_string.clear();
            svg.close_tag("path");

            svg.create_tag("path");
            svg.create_attr("stroke", color_pallete[color_idx]);
            svg.create_attr("stroke-width", SVG_STROKE_WIDTH);
            svg.create_attr("fill", "none");
            
            path_string += std::format("M {} {}", 
                curr_pt_svg.first, curr_pt_svg.second
            );
        } else {
            path_string += std::format(" l {} {}",
                stitch.delta_x,
                -stitch.delta_y
            );
        }
    }
    svg.create_attr("d", path_string);
    svg.close_tag("path");
}