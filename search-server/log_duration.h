#pragma once

#include <chrono>
#include <iostream>
#include <string>

using namespace std;

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)

class LogDuration {
   public:
    LogDuration(string name, ostream& os = cerr) : name_{name}, os_{os}, timer_{chrono::steady_clock::now()} {}
    ~LogDuration() {
        const auto duration = chrono::steady_clock::now() - timer_;
        os_ << name_ << ": "s << chrono::duration_cast<chrono::milliseconds>(duration).count() << " ms"s << endl;
    }

   private:
    string name_;
    ostream& os_;
    chrono::steady_clock::time_point timer_;
};