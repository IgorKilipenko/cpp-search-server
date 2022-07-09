#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const string& GetMedianString(vector<string> v) {
    nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
    const string& mid = v[v.size() / 2];

    return mid;
}

int main() {
    vector<string> v = {"cat"s, "dog"s, "elephant"s, "monkey"s, "llama"s};
    const string& res = GetMedianString(v);
    cout << res << endl;
    return 0;
} 