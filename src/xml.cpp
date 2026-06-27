#include <fstream>
#include <string>
#include <vector>
#include <iostream>

class XML {
public:
    std::ofstream &out;
    std::vector<std::string> curr_tags;
    bool last_tag_open = false;

    XML(std::ofstream &out_stream) : out(out_stream) {
    }
    
    ~XML() {
        if (!curr_tags.empty()) {
            std::cerr << "Warning: Unclosed tags remain at destruction: ";
        }
    }
    
    void create_tag(std::string tag){
        if (last_tag_open) {
            out << ">\n";
        }
        out << "<" + tag;
        curr_tags.push_back(tag);
        last_tag_open = true;
    }
    
    void close_tag(std::string tag) {
        if(curr_tags.empty() || curr_tags.back() != tag) {
            throw std::runtime_error("Mismatched tag closure: " + tag);
        }
        if(last_tag_open) out << ">";
        last_tag_open = false;
        out << "\n</" + curr_tags.back() + ">";
        curr_tags.pop_back();
    }
    
    void create_attr(std::string attr, std::string value) {
        if (curr_tags.empty()) {
            throw std::runtime_error("Cannot create an attribute outside of a tag");
        }
        out << " " + attr + "=\"" + value + "\"";
    }
   
};