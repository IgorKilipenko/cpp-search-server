#include <iostream>

using namespace std;

extern string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

extern int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}
