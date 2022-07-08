#include <cassert>
#include <numeric>
#include <vector>

template <typename T>
void ReverseArray(T* start, size_t size) {
    // Напишите тело функции самостоятельно
    if (size == 0 || start == nullptr) return;
    T* end = start + size - 1;
    for (int i = 0; i < size / 2; ++i) {
        T tmp = *(end - i);
        *(end - i) = *(start + i);
        *(start + i) = tmp;
    }
}

int main() {
    using namespace std;

    vector<int> v(0);
    iota(v.begin(), v.end(), 1);
    vector<int> expected_v(v.rbegin(), v.rend());
    ReverseArray(v.data(), v.size());
    assert(v == (expected_v));
}
