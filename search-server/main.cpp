#include "simple_vector_test.h"

#include <cassert>
#include <iostream>
#include <numeric>

using namespace std;


int main() {
    Test1();
    Test2();
    TestTemporaryObjConstructor();
    TestTemporaryObjOperator();
    TestNamedMoveConstructor();
    TestNamedMoveOperator();
    TestNoncopiableMoveConstructor();
    TestNoncopiablePushBack();
    TestNoncopiableInsert();
    TestNoncopiableErase();
    return 0;
}
