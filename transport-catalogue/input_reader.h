#pragma once

#include <iostream>
#include <vector>

namespace transport_catalogue::io::detail {
    size_t TrimStart(std::string_view& str, const char ch = ' ');
    size_t TrimEnd(std::string_view& str, const char ch = ' ');
    void Trim(std::string_view& str, const char ch = ' ');
}

namespace transport_catalogue::io {

    class Reader {
    public:
        Reader(std::istream& in_stream = std::cin) : in_stream_{in_stream} {}
        template <typename TOut = std::string>
        TOut Read();
        std::string ReadRawLine();
        std::vector<std::string> ReadAllInputLines();

    private:
        std::istream& in_stream_;
    };
}