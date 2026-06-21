#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include <vector>

class DSTHeader {

private:
    static const int HEADER_SIZE = 125;
    // Fields of headers are padded with spaces (ASCII 32) to fill the fixed field length.
    static inline const char PAD_CHAR = ' '; 

public:
    std::string label;
    std::string stitch_count;
    std::string color_count;
    std::pair<int, int> x_range;
    std::pair<int, int> y_range;
    std::pair<int, int> end_pos_rel_start_pos; // This is in 0.1 mm units
                                           
    void print_header() {
        std::cout << "# HEADER\n";
        std::cout << "Label: " << label << "\n";
        std::cout << "Stitch Count: " << stitch_count << "\n";
        std::cout << "Color Count: " << color_count << "\n";
        std::cout << "X Range: (" << x_range.first << ", " << x_range.second << ")\n";
        std::cout << "Y Range: (" << y_range.first << ", " << y_range.second << ")\n"; 
        std::cout << "End Position Relative to Start Position: (" 
                  << end_pos_rel_start_pos.first / 10.0 << " mm, " 
                  << end_pos_rel_start_pos.second / 10.0 << " mm)" 
                  << std::endl;
    }
    
    void from_stream(std::ifstream &file) {
        std::string header_data(HEADER_SIZE, '\0');
        file.read(header_data.data(), HEADER_SIZE);

        this->label = get_clean_str_(header_data, 3, 16);
        this->stitch_count = get_clean_str_(header_data, 23, 7);
        this->color_count = get_clean_str_(header_data, 34, 3);
        this->x_range = std::make_pair(
            std::stoi(get_clean_str_(header_data, 51, 5)),
            std::stoi(get_clean_str_(header_data, 41, 5))
        );
        this->y_range = std::make_pair(
            std::stoi(get_clean_str_(header_data, 68, 5)),
            std::stoi(get_clean_str_(header_data, 59, 5))
        );
        this->end_pos_rel_start_pos = std::make_pair(
            std::stoi(get_clean_str_(header_data, 77, 6)),
            std::stoi(get_clean_str_(header_data, 87, 6))
        );
    }

    // Helper function to removing PAD_CHAR from the extracted substring
    std::string get_clean_str_(std::string s, int start, int length) {
        auto substr = s.substr(start, length);
        // Remove PAD char
        std::erase(substr, PAD_CHAR);
        return substr;
    }
};

class DSTBody {
private:
    static const int END = 0b00011010;
    static const int BODY_START = 512;

public:
    std::vector<std::pair<int, int>> stitches;
    // List of indices in the stitch list where color changes occur.
    std::vector<int> color_chg_idx;

    void from_stream(std::ifstream &file) {
        char buffer[3];
        int idx = 0;
        std::tuple<int, int, bool> decoded_command;
        
        file.seekg(BODY_START);
        while(file.peek() != EOF && file.peek() != END) {
            file.read(buffer, 3);
            decoded_command = decode_stitch_command_((uint8_t*) buffer);
            stitches.push_back(std::make_pair(
                std::get<0>(decoded_command),
                std::get<1>(decoded_command)
            ));
            if(std::get<2>(decoded_command)) {
                color_chg_idx.push_back(idx);
            }
            idx++;
        }
    }
    
    // Decodes a 3-byte stitch command starting from the given pointer
    std::tuple<int, int, bool> decode_stitch_command_(uint8_t *start) {
        int delta[2] = {0, 0};
        bool is_color_change = false;

        // byte-1
        for(int i=0; i<8; i++) {
            int distance = std::min(i, 7-i);
            int &direction = delta[i / 4];

            if (start[0] & (1 << i)) {
                // i-th bit is set
                switch (distance) {
                    case 0: direction += 1; break;
                    case 1: direction -= 1; break;
                    case 2: direction += 9; break;
                    case 3: direction -= 9; break;
                }
            }
        }
        
        // byte-2
        for(int i=0; i<8; i++) {
            int distance = std::min(i, 7-i);
            int &direction = delta[i / 4];
            
            if(start[1] & (1 << i)) {
                // i-th bit is set
                switch (distance) {
                    case 0: direction += 3; break;
                    case 1: direction -= 3; break;
                    case 2: direction += 27; break;
                    case 3: direction -= 27; break;
                }
            }
        }
        
        // byte-3
        for(int i=0; i<8; i++) {
            int distance = std::min(i, 7-i);
            int &direction = delta[i / 4];

            if(start[2] & (1 << i)) {
                // i-th bit is set
                switch (distance) {
                    case 2: direction += 81; break;
                    case 3: direction -= 81; break;
                }
            }
        }

        if(start[2] >= (1 << 7) + (1 << 6)) {
            // 6-th and 7-th bits are set, indicating a color change
            is_color_change = true;
        }

        return std::make_tuple(delta[0], delta[1], is_color_change);
    }
    
    void print_body() {
        std:: cout << "# BODY\n";
        int color_idx = 0;
        for (int i = 0; i < stitches.size(); ++i) {
            std::cout << "Stitch " << i << ": (" 
                      << stitches[i].first << ", " 
                      << stitches[i].second << ")";
            
            if (i == color_chg_idx[color_idx]) {
                std::cout << " [Color Change]";
                color_idx++;
            }
            std::cout << "\n";
        }
    }
};

class DSTFile {
public:   
    DSTHeader header;
    DSTBody body;

    DSTFile(const std::string &filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filepath);
        }
        this->header.from_stream(file);
        this->body.from_stream(file);
        file.close();
    }
};
