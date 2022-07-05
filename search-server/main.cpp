#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "log_duration.h"

using namespace std;

template <typename TestFunc = function<bool(const string&)>>
class Bruteforce {
   public:
    Bruteforce(TestFunc check) : custom_check_function_{check} {}
    string Generate(const string& input) {
        password_ = ""s;
        string result(input.size(), input[0]);
        vector<int> index(input.size(), 0);

        for (int length = 1; length <= input.size(); length++) {
            int updateIndex = 0;
            do {
                if (Test(result, 0, length)) {
                    return password_;
                }

                for (updateIndex = length - 1; updateIndex != -1 && ++index[updateIndex] == input.size();
                     result[updateIndex] = input[0], index[updateIndex] = 0, updateIndex--) {
                }

                if (updateIndex != -1) result[updateIndex] = input[index[updateIndex]];
            } while (updateIndex != -1);
        }
        return password_;
    }
    string GetPassword() const {
        return password_;
    }

   private:
    string password_;
    TestFunc custom_check_function_;
    bool Test(const string& result, int offset, int length) {
        string text = result.substr(offset, length);
        if (custom_check_function_(text)) {
            password_ = text;
            return true;
        }
        return false;
    }
};

template <typename F>
string BruteForce(F check) {
    // верните строку str, для которой check(str) будет true
    int start_ch = static_cast<int>('A');
    int end_ch = static_cast<int>('Z') + 1;
    string chars;
    chars.reserve(end_ch - start_ch);
    for (int i = start_ch; i < end_ch; i++) {
        chars.push_back(static_cast<char>(i));
    }
    Bruteforce bruteforce(check);
    const string password = bruteforce.Generate(chars);
    return password;
}

int main() {
    string pass = "ARTUR"s;
    auto check = [pass](const string& s) {
        return s == pass;
    };
    cout << BruteForce(check) << endl;
}