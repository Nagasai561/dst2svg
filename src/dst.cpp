#include "dst.hpp"

void DSTHeader::print_header() {
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
    
void DSTHeader::from_stream(std::ifstream &file) {
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
std::string DSTHeader::get_clean_str_(std::string s, int start, int length) {
    auto substr = s.substr(start, length);
    // Remove PAD char
    std::erase(substr, PAD_CHAR);
    return substr;
}

void DSTBody::from_stream(std::ifstream &file) {
    char buffer[3];
    int idx = 0;
    Stitch decoded_command{0, 0, false, false};
    
    file.seekg(BODY_START);
    while(file.peek() != EOF) {
        file.read(buffer, 3);
        decoded_command = decode_stitch_command_(buffer);
        stitches.push_back(decoded_command);
        idx++;
    }
}

// Decodes a 3-byte stitch command starting from the given pointer
Stitch DSTBody::decode_stitch_command_(char *start) {
    int delta[2] = {0, 0};
    bool is_color_change = false;
    bool is_jump = false;

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

    if((start[2] & (1 << 7)) && (start[2] & (1 << 6))) {
        // 6-th and 7-th bits are set, indicating a color change
        is_color_change = true;
    }
    
    if((start[2] & (1 << 7)) && !(start[2] & (1 << 6))) {
       is_jump = true; 
    }

    return Stitch{delta[0], delta[1], is_jump, is_color_change};
}
    
void DSTBody::print_body() {
    std:: cout << "# BODY\n";
    int color_idx = 0;
    for (int i = 0; i < stitches.size(); i++) {
        std::cout << "Stitch " << i << ": (" 
                  << stitches[i].delta_x << ", " 
                  << stitches[i].delta_y << ")";
        
        if (stitches[i].is_color_change) {
            std::cout << " [Color Change]";
            color_idx++;
        }
        
        if (stitches[i].is_jump) {
            std::cout << " [Jump]";
        }
        std::cout << "\n";
    }
}


DSTFile::DSTFile(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    this->header.from_stream(file);
    this->body.from_stream(file);
    file.close();
}
