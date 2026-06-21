#include <iostream>
#include <fstream>
#include <vector>

// Structure of the .dst file is studied from 
// https://edutechwiki.unige.ch/en/Embroidery_format_DST

class DSTHeader {

private:
    static const int HEADER_SIZE = 125;
    // Fields of headers are padded with spaces (ASCII 32) to fill the fixed field length.
    static inline const char PAD_CHAR = ' '; 
    std::string get_clean_str_(std::string s, int start, int length);

public:
    std::string label;
    std::string stitch_count;
    std::string color_count;
    std::pair<int, int> x_range;
    std::pair<int, int> y_range;
    std::pair<int, int> end_pos_rel_start_pos; // This is in 0.1 mm units
                                           
    void print_header();
    void from_stream(std::ifstream &file);
};

class DSTBody {
private:
    static const int END = 0b00011010;
    static const int BODY_START = 512;

public:
    std::vector<std::pair<int, int>> stitches;
    // List of indices in the stitch list where color changes occur.
    std::vector<int> color_chg_idx;

    void from_stream(std::ifstream &file);
    
    // Decodes a 3-byte stitch command starting from the given pointer
    std::tuple<int, int, bool> decode_stitch_command_(char *start);    
    void print_body();
};

class DSTFile {
public:   
    DSTHeader header;
    DSTBody body;

    DSTFile(const std::string &filepath);
};
