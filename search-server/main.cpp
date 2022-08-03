#include <iostream>

#include "libstat.h"
using namespace std;

int main() {
    using namespace statistics::tests;
    AggregSum();
    AggregMax();
    AggregMean();
    AggregStandardDeviation();
    AggregMode();
    AggregPrinter();

    cout << "Test passed!"sv << endl;
}