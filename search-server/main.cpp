#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

int EffectiveCount(const vector<int>& v, int n, int i)  {
    // место для вашего решения
    const int expected_result_count = (v.size())*(i + 1)/(n + 1);
    const bool use_find_method = expected_result_count <= log2(v.size());

    int result = 0;
    if (use_find_method) {
        auto ptr = find_if(v.begin(), v.end(), [i](int val) {
            return val > i;
        });
        cout << "Using find_if"s << endl;
        if (ptr == v.end()) {
            return v.size();
        }
        result = ptr - v.begin();
    } else {
        auto ptr = upper_bound(v.begin(), v.end(), i);
        cout << "Using upper_bound"s << endl;
        if (ptr == v.end()) {
            return v.size();
        }
        result = ptr - v.begin();
    }
    return result;
}

int main() {
    static const int NUMBERS = 1'000'000;
    static const int MAX = 1'000'000'000;

    mt19937 r;
    uniform_int_distribution<int> uniform_dist(0, MAX);

    vector<int> nums;
    for (int i = 0; i < NUMBERS; ++i) {
        int random_number = uniform_dist(r);
        nums.push_back(random_number);
    }
    sort(nums.begin(), nums.end());

    int i;
    cin >> i;
    int result = EffectiveCount(nums, MAX, i);
    cout << "Total numbers before "s << i << ": "s << result << endl;
}