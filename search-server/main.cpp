#include <iostream>
#include <string>
#include <set>

// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь: 271

// Закомитьте изменения и отправьте их в свой репозиторий.

int main()
{
    const char findNumber = '3';
    int result = 0;
    for (int i = 1; i <= 1000; i++)
    {
        std::string str = std::to_string(i);
        const std::set<char> buff(str.begin(), str.end());
        if (buff.count(findNumber))
        {
            ++result;
            std::cout << "number: " << i << " contains " << findNumber << std::endl;
        }
    }
    std::cout << std::endl;
    std::cout << result << " numbers contains " << findNumber << std::endl
              << std::endl;
    return 0;
}