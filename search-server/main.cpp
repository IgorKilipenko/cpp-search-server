#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "log_duration.h"

using namespace std;


class Bruteforce {
   public:
    template <typename F>
    const string& generate(const string& input, F check) {
        result_ = ""s;
        string result(input.size(), input[0]);
        vector<int> index(input.size(), 0);

        // loop over the output lengths.
        for (int length = 1; length <= input.size(); length++) {
            int updateIndex = 0;
            do {
                if (element(result, 0, length, check)) {
                    return result_;
                }
                // update values that need to reset.
                for (updateIndex = length - 1; updateIndex != -1 && ++index[updateIndex] == input.size();
                     result[updateIndex] = input[0], index[updateIndex] = 0, updateIndex--)
                    ;

                // update the character that is not resetting, if valid
                if (updateIndex != -1) result[updateIndex] = input[index[updateIndex]];
            } while (updateIndex != -1);
        }
        return result_;
    }
    template <typename F>
    bool element(const string& result, int offset, int length, F check) {
        string text = result.substr(offset, length);
        //cout << text << endl;
        if (check(text)) {
            result_ = text;
            return true;
        }
        return false;
    }

   private:
    string result_;
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
    Bruteforce bruteforce;
    const string password = bruteforce.generate(chars, check);
    return password;
}

int main() {
    string pass = "ARTUR"s;
    auto check = [pass](const string& s) {
        return s == pass;
    };
    cout << BruteForce(check) << endl;
}