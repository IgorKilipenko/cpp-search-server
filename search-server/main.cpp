#include <algorithm>
#include <cassert>
#include <cstddef>
#include <execution>
#include <iostream>
#include <vector>

using namespace std;

class MoneyBox {
   public:
    explicit MoneyBox(vector<int64_t> nominals) : nominals_(move(nominals)), counts_(nominals_.size()) {}

    const vector<int>& GetCounts() const {
        return counts_;
    }

    void PushCoin(int64_t value) {
        auto idx = GetIndex(value);
        ++counts_[idx];
    }

    void PrintCoins(ostream& out) const {
        for (int i = 0; i < counts_.size(); ++i) {
            if (counts_[i]) {
                out << nominals_[i] << ": " << counts_[i] << std::endl;
            }
        }
    }

   private:
    const vector<int64_t> nominals_;
    vector<int> counts_;

    size_t GetIndex(int64_t nominal) const {
        auto ptr = find(std::execution::par, nominals_.begin(), nominals_.end(), nominal);
        assert(ptr != nominals_.end());
        return ptr - nominals_.begin();
    }
};

ostream& operator<<(ostream& out, const MoneyBox& cash) {
    cash.PrintCoins(out);
    return out;
}

int main() {
    MoneyBox cash({1, 500, 10000});
    cash.PushCoin(500);
    cash.PushCoin(500);
    cash.PushCoin(10000);
    assert((cash.GetCounts() == vector<int>{0, 2, 1}));
    cout << cash << endl;
}