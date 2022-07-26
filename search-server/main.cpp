#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <execution>
#include <future>
#include <map>
#include <mutex>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "log_duration.h"
#include "test_framework.h"

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
   public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");
    struct Bucket {
        std::mutex mutex;
        std::map<Key, Value> map;
        Value& operator[](const Key& key) {
            return map[key];
        }
    };
    class Access {
       private:
        std::lock_guard<std::mutex> guard_;

       public:
        Value& ref_to_value;
        Access(const Key& key, Bucket& bucket) : guard_{bucket.mutex}, ref_to_value{bucket[key]} {}
    };

    explicit ConcurrentMap(size_t bucket_count) : buckets_{bucket_count} {}

    Access operator[](const Key& key) {
        return Access{key, buckets_[static_cast<size_t>(key) % buckets_.size()]};
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for (auto& [mutex, container] : buckets_) {
            std::lock_guard guard(mutex);
            result.insert(container.begin(), container.end());
        }
        return result;
    }

   private:
    std::vector<Bucket> buckets_;
};

using namespace std;

void RunConcurrentUpdates(ConcurrentMap<int, int>& cm, size_t thread_count, int key_count) {
    auto kernel = [&cm, key_count](int seed) {
        vector<int> updates(key_count);
        iota(begin(updates), end(updates), -key_count / 2);
        shuffle(begin(updates), end(updates), mt19937(seed));

        for (int i = 0; i < 2; ++i) {
            for (auto key : updates) {
                ++cm[key].ref_to_value;
            }
        }
    };

    vector<future<void>> futures;
    for (size_t i = 0; i < thread_count; ++i) {
        futures.push_back(async(kernel, i));
    }
}

void RunConcurrentUpdatesSync(ConcurrentMap<int, int>& cm, size_t thread_count, int key_count) {
    for (size_t i = 0; i < thread_count; ++i) {
        vector<int> updates(key_count);
        iota(begin(updates), end(updates), -key_count / 2);
        shuffle(begin(updates), end(updates), mt19937(i));

        for (int i = 0; i < 2; ++i) {
            for (auto key : updates) {
                ++cm[key].ref_to_value;
            }
        }
    }
}

void TestConcurrentUpdate() {
    constexpr size_t THREAD_COUNT = 3;
    constexpr size_t KEY_COUNT = 50000;

    ConcurrentMap<int, int> cm(THREAD_COUNT);
    RunConcurrentUpdates(cm, THREAD_COUNT, KEY_COUNT);

    const auto result = cm.BuildOrdinaryMap();
    ASSERT_EQUAL(result.size(), KEY_COUNT);
    for (auto& [k, v] : result) {
        AssertEqual(v, 6, "Key = " + to_string(k));
    }
}

void TestConcurrentUpdateSync() {
    constexpr size_t THREAD_COUNT = 3;
    constexpr size_t KEY_COUNT = 50000;

    ConcurrentMap<int, int> cm(THREAD_COUNT);
    RunConcurrentUpdates(cm, THREAD_COUNT, KEY_COUNT);

    const auto result = cm.BuildOrdinaryMap();
    ASSERT_EQUAL(result.size(), KEY_COUNT);
    for (auto& [k, v] : result) {
        AssertEqual(v, 6, "Key = " + to_string(k));
    }
}

void TestReadAndWrite() {
    ConcurrentMap<size_t, string> cm(5);

    auto updater = [&cm] {
        for (size_t i = 0; i < 50000; ++i) {
            cm[i].ref_to_value.push_back('a');
        }
    };
    auto reader = [&cm] {
        vector<string> result(50000);
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = cm[i].ref_to_value;
        }
        return result;
    };

    auto u1 = async(updater);
    auto r1 = async(reader);
    auto u2 = async(updater);
    auto r2 = async(reader);

    u1.get();
    u2.get();

    for (auto f : {&r1, &r2}) {
        auto result = f->get();
        ASSERT(all_of(result.begin(), result.end(), [](const string& s) {
            return s.empty() || s == "a" || s == "aa";
        }));
    }
}

void TestSpeedup() {
    {
        ConcurrentMap<int, int> single_lock(1);

        LOG_DURATION("Single lock");
        RunConcurrentUpdates(single_lock, 4, 50000);
    }
    {
        ConcurrentMap<int, int> many_locks(100);

        LOG_DURATION("100 locks");
        RunConcurrentUpdates(many_locks, 4, 50000);
    }
}

int main() {
    TestRunner tr;
    // RUN_TEST(tr, TestConcurrentUpdateSync);
    RUN_TEST(tr, TestConcurrentUpdate);
    RUN_TEST(tr, TestReadAndWrite);
    RUN_TEST(tr, TestSpeedup);
    /*
        ConcurrentMap<int, int> cm{4};
        cm[0].ref_to_value = 1;
        cm[1].ref_to_value = 2;
        cm[2].ref_to_value = 3;
        cm[3].ref_to_value = 4;*/
}