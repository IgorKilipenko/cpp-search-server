#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Circle {
    double x;
    double y;
    double r;
};

struct Dumbbell {
    Circle circle1;
    Circle circle2;
    string text;
};

struct DumbbellHash {
    size_t operator()(const Dumbbell& val) const {
        vector<double> vals = {val.circle1.x, val.circle1.y, val.circle1.r, val.circle2.x, val.circle2.y, val.circle2.r};
        size_t result = 0;
        std::for_each(vals.begin(), vals.end(), [&](const double val) {
            result = result * 37 + static_cast<size_t>(val*100);
        }); 
        result += std::hash<std::string>{}(val.text);
        return result;
    }
};

int main() {
    DumbbellHash hash;
    Dumbbell dumbbell{{10, 11.5, 2.3}, {3.14, 15, -8}, "abc"s};
    cout << "Dumbbell hash "s << hash(dumbbell);
}