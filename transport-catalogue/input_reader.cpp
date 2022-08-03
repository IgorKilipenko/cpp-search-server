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
    std::vector<std::string_view> SplitIntoWords(std::string_view str, const char ch) {
        if (str.empty()) {
            return {};
        }
        std::vector<std::string_view> result;
        str.remove_prefix(std::min(str.find_first_not_of(ch), str.size()));

        do {
            int64_t space = str.find(ch, 0);
            result.push_back(space == static_cast<int64_t>(str.npos) ? str.substr(0) : str.substr(0, space));
            str.remove_prefix(std::min(str.find_first_not_of(ch, space), str.size()));
        } while (!str.empty());

        return result;
    }
}

namespace transport_catalogue::io {
    std::string Reader::ReadRawLine() {
        std::string line;
        std::getline(in_stream_, line);
        return line;
    }
}