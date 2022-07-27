#include <atomic>
#include <cstddef>
#include <execution>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

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
        size_t Size() const {
            return map.size();
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
        if (buckets_.empty()) {
            return {};
        }
        std::map<Key, Value> result;
        for (auto& [mutex, container] : buckets_) {
            std::lock_guard<std::mutex> guard(mutex);
            result.insert(container.begin(), container.end());
        }
        return result;
    }

    std::vector<std::pair<Key, Value>> BuildOrdinaryVector() {
        if (buckets_.empty()) {
            return {};
        }
        std::vector<std::pair<Key, Value>> result;
        result.reserve(buckets_.size() * buckets_.front().Size());
        for (auto& [mutex, container] : buckets_) {
            std::lock_guard<std::mutex> guard(mutex);
            result.insert(result.end(), container.begin(), container.end());
        }
        return result;
    }

    void Erase(const Key& key) {
        auto& bucket = buckets_[static_cast<size_t>(key) % buckets_.size()];
        std::lock_guard<std::mutex> lock(bucket.mutex);
        bucket.map.erase(key);
    }

    size_t Size() const {
        std::atomic_size_t result = 0;
        for (auto& [_, container] : buckets_) {
            result += container.size();
        }
        return result;
    }

   private:
    std::vector<Bucket> buckets_;
};

template <typename Key>
class ConcurrentSet {
   public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");
    struct Bucket {
        std::mutex mutex;
        std::set<Key> set;
        template <typename ValueType>
        Key& operator[](ValueType&& key) {
            auto result = set.insert(key);
            return const_cast<Key&>(*result.first);
        }
        size_t Size() const {
            return set.size();
        }
    };
    class Access {
       private:
        std::lock_guard<std::mutex> guard_;

       public:
        Key& ref_to_value;
        Access(const Key& key, Bucket& bucket) : guard_{bucket.mutex}, ref_to_value{bucket[key]} {}
    };

    explicit ConcurrentSet(size_t bucket_count) : buckets_{bucket_count} {}

    Access operator[](const Key& key) {
        return Access{key, buckets_[static_cast<size_t>(key) % buckets_.size()]};
    }

    std::set<Key> BuildOrdinarySet() {
        if (buckets_.empty()) {
            return {};
        }
        std::set<Key> result;
        for (auto& [mutex, container] : buckets_) {
            std::lock_guard guard(mutex);
            result.insert(container.begin(), container.end());
        }
        return result;
    }

    std::vector<Key> BuildOrdinaryVector() {
        if (buckets_.empty()) {
            return {};
        }
        std::vector<Key> result;
        result.reserve(buckets_.size() * buckets_.front().Size());
        for (auto& [mutex, container] : buckets_) {
            std::lock_guard guard(mutex);
            result.insert(result.end(), container.begin(), container.end());
        }
        return result;
    }

    void Erase(const Key& key) {
        auto& bucket = buckets_[static_cast<size_t>(key) % buckets_.size()];
        std::lock_guard<std::mutex> lock(bucket.mutex);
        bucket.set.erase(key);
    }

    size_t Size() const {
        std::atomic_size_t result = 0;
        for (auto& [_, container] : buckets_) {
            result += container.size();
        }
        return result;
    }

   private:
    std::vector<Bucket> buckets_;
};