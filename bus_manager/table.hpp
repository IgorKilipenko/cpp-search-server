#ifndef __DATEBASE_H__
#define __DATEBASE_H__

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

class Table : public vector<pair<string, vector<string>>> {
   public:
    int UpdateKeysValues(const vector<string>& keys, const string& value) {
        int update_count = 0;
        for (const auto& key : keys) {
            auto& curr_values = (*this)[key];
            if (!std::count(curr_values.begin(), curr_values.end(), value)) {
                curr_values.push_back(value);
                ++update_count;
            }
        }

        return update_count;
    }

    /**
     * @brief Synchronization of dependent tables
     *
     */
    void SyncTables() {
        throw "Not implemented yet";
    }

    int count(const string& key) const {
        return count_if(this->begin(), this->end(), [&key](const auto& item) {
            return item.first == key;
        });
    }

    vector<string>& operator[](const string& key) {
        if (!this->count(key)) {
            this->push_back({key, {}});
            return this->back().second;
        }

        for (auto& item : *this) {
            if (item.first == key) return item.second;
        }

        throw "Error: Unknown key";
    }

    const vector<string>& at(const string& key) const {
        for (auto& item : *this) {
            if (item.first == key) return item.second;
        }
        throw "Argument not found: "s + key;
    }

    Table GetSortedTable() const {
        Table result = *this;
        sort(result.begin(), result.end(), [](const auto& lv, const auto& rv) {
            return lv.first < rv.first;
        });
        return result;
    }

   private:
    set<shared_ptr<Table>> _ref_tables;
};

#endif  // __DATEBASE_H__
