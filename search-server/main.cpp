#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

// не меняйте файлы xml.h и xml.cpp
#include "xml.h"

using namespace std;

struct Spending {
    string category;
    int amount;
};

int CalculateTotalSpendings(const vector<Spending>& spendings) {
    return accumulate(spendings.begin(), spendings.end(), 0, [](int current, const Spending& spending) {
        return current + spending.amount;
    });
}

string FindMostExpensiveCategory(const vector<Spending>& spendings) {
    assert(!spendings.empty());
    auto compare_by_amount = [](const Spending& lhs, const Spending& rhs) {
        return lhs.amount < rhs.amount;
    };
    return max_element(begin(spendings), end(spendings), compare_by_amount)->category;
}

vector<Spending> LoadFromXml(istream& input) {
    // место для вашей реализации
    // пример корректного XML-документа в условии
    Document json = Load(input);
    const Node& root = json.GetRoot();
    const std::vector<Node>& data = root.Children();
    vector<Spending> result{data.size()};
    std::transform(data.begin(), data.end(), result.begin(), [](const Node& node) -> Spending {
        return {node.AttributeValue<std::string>("category"), node.AttributeValue<int>("amount")};
    });
    return result;
}

int main() {
    const vector<Spending> spendings = LoadFromXml(cin);
    cout << "Total "sv << CalculateTotalSpendings(spendings) << '\n';
    cout << "Most expensive is "sv << FindMostExpensiveCategory(spendings) << '\n';
}