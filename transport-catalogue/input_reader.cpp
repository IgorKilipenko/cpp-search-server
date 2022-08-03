#include "input_reader.h"

namespace transport_catalogue::io::detail {
    size_t TrimStart(std::string_view& str, const char ch) {
        size_t idx = str.find_first_not_of(ch);
        if (idx != std::string::npos) {
            str.remove_prefix(idx);
            return idx;
        }
        return 0;
    }
    size_t TrimEnd(std::string_view& str, const char ch) {
        size_t idx = str.find_last_not_of(ch);
        if (idx != str.npos) {
            str.remove_suffix(str.size() - idx - 1);
            return idx;
        }
        return 0;
    }
    void Trim(std::string_view& str, const char ch) {
        TrimStart(str, ch);
        TrimEnd(str, ch);
    }
}

namespace transport_catalogue::io {
    std::string Reader::ReadRawLine() {
        std::string line;
        std::getline(in_stream_, line);
        return line;
    }
}