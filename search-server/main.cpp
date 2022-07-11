#include <map>
#include <set>
#include <string>
#include <string_view>

#include "my_assert.h"

using namespace std;

class Translator {
   public:
    Translator() : words_{}, translated_words_{}, forward_translations_{}, backward_translations_{} {}

    void Add(std::string_view source, std::string_view target) {
        auto [src_ptr, _] = words_.emplace(source);
        auto [target_ptr, __] = translated_words_.emplace(target);
        forward_translations_.insert({*src_ptr, *target_ptr});
        backward_translations_.insert({*target_ptr, *src_ptr});
    }

    std::string_view TranslateForward(std::string_view source) const {
        if (forward_translations_.empty() || !forward_translations_.count(source)) {
            return Translator::INVALID_RESULT;
        }

        auto result = forward_translations_.at(source);
        return result;
    }

    std::string_view TranslateBackward(std::string_view target) const {
        if (backward_translations_.empty() || !backward_translations_.count(target)) {
            return Translator::INVALID_RESULT;
        }

        auto result = backward_translations_.at(target);
        return result;
    }

   private:
    inline static const std::string INVALID_RESULT = ""s;
    std::set<std::string> words_;
    std::set<std::string> translated_words_;
    std::map<std::string_view, std::string_view> forward_translations_;
    std::map<std::string_view, std::string_view> backward_translations_;
};

void TestSimple() {
    Translator translator;
    translator.Add(string("okno"s), string("window"s));
    translator.Add(string("stol"s), string("table"s));

    assert(translator.TranslateForward("okno"s) == "window"s);
    assert(translator.TranslateBackward("table"s) == "stol"s);
    assert(translator.TranslateForward("table"s) == ""s);
}

int main() {
    TestSimple();
    return 0;
}
