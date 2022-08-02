#include "ini.h"

#include <cstddef>
#include <memory>
#include <string_view>

using namespace std;

// место для реализаций методов и функций библиотеки ini
// не забудьте, что они должны быть помещены в namespace ini

namespace ini {
    namespace detail {
        size_t TrimStart(std::string_view& str, const char ch = ' ') {
            size_t idx = str.find_first_not_of(ch);
            if (idx != std::string::npos) {
                str.remove_prefix(idx);
                return idx;
            }
            return 0;
        }
        size_t TrimEnd(std::string_view& str, const char ch = ' ') {
            size_t idx = str.find_last_not_of(ch);
            if (idx != str.npos) {
                str.remove_suffix(str.size() - idx - 1);
                return idx;
            }
            return 0;
        }
        void Trim(std::string_view& str, const char ch = ' ') {
            TrimStart(str, ch);
            TrimEnd(str, ch);
        }
        bool ParseSection(std::string_view& str) {
            size_t start_idx = str.find('[');
            if (start_idx == std::string::npos) {
                return false;
            }
            size_t end_idx = str.find(']');
            if (end_idx == std::string::npos || end_idx - start_idx < 2) {
                return false;
            }
            str = str.substr(start_idx + 1, end_idx - start_idx - 1);
            return true;
        }
    }

    Section& Document::AddSection(std::string name) {
        return sections_[name];
    }

    const Section& Document::GetSection(const std::string& name) const {
        static const Section empty_section = {};
        return sections_.count(name) ? sections_.at(name) : empty_section;
    }

    std::size_t Document::GetSectionCount() const {
        return sections_.size();
    }

    std::shared_ptr<std::pair<std::string_view, std::string_view>> ParseKeyValueLine(std::string_view str) {
        size_t equal_idx = str.find('=');
        if (equal_idx == std::string::npos) {
            return nullptr;
        }
        std::string_view key = str.substr(0, equal_idx);
        detail::TrimEnd(key);
        std::string_view value = str.substr(equal_idx + 1);
        detail::TrimStart(value);
        std::pair<std::string_view, std::string_view> result{key, value};
        return std::make_shared<std::pair<std::string_view, std::string_view>>(result);
    }

    Document Load(std::istream& input) {
        Document result;
        Section* cur_section = nullptr;
        for (std::string line; std::getline(input, line);) {
            std::string_view str_ptr = line;
            detail::TrimStart(str_ptr);
            if (str_ptr.empty()) {
                continue;
            }
            detail::TrimEnd(str_ptr);
            if (detail::ParseSection(str_ptr)) {
                cur_section = &result.AddSection(static_cast<std::string>(str_ptr));
                continue;
            }
            if (cur_section) {
                detail::Trim(str_ptr);
                auto value = ParseKeyValueLine(str_ptr);
                if (value) {
                    cur_section->operator[](static_cast<std::string>(value->first)) = value->second;
                }
            }
        }
        return result;
    }
}