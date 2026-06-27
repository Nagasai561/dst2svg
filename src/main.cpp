#include "dst.hpp"
#include "svg.cpp"

#include <iostream>
#include <memory>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <dst_file> <out_file>" << std::endl;
        return 1;
    }
    
    auto out_file_stream = std::ofstream(argv[2]); 
    if (!out_file_stream.is_open()) {
        std::cerr << "Error: Could not open output file " << argv[2] << std::endl;
        return 1;
    }

    DSTFile dst_file(argv[1]);
    write_as_svg(out_file_stream, dst_file);

    return 0;
}