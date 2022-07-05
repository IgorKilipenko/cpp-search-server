#include <algorithm>
#include <deque>
#include <iostream>
#include <string>

using namespace std;

struct Ticket {
    int id = 0;
    string name;
};

class TicketOffice {
   public:
    // добавить билет в систему
    void PushTicket(const string& name) {
        // реализуйте метод
        if (first_id_ < 0) first_id_ = 0;
        ++last_id_;
        ++size_;
        tickets_.push_back({last_id_, name});
    }

    // получить количество доступных билетов
    int GetAvailable() const {
        // реализуйте метод
        return size_;
    }

    // получить количество доступных билетов определённого типа
    int GetAvailable(const string& name) const {
        // реализуйте метод
        int result = count_if(tickets_.begin(), tickets_.end(), [&name](const Ticket& t) {
            return t.name == name;
        });
        return result;
    }

    // отозвать старые билеты (до определённого id)
    void Invalidate(int minimum) {
        // реализуйте метод
        int size = min(minimum, last_id_ + 1);
        for (int i = first_id_; i < size; ++i) {
            tickets_.pop_front();
            --size_;
            first_id_++;
        }
    }

   private:
    int last_id_ = -1;
    int first_id_ = -1;
    int size_ = 0;
    deque<Ticket> tickets_;
};

int main() {
    TicketOffice tickets;

    tickets.PushTicket("Swan Lake");      // id - 0
    tickets.PushTicket("Swan Lake");      // id - 1
    tickets.PushTicket("Boris Godunov");  // id - 2
    tickets.PushTicket("Boris Godunov");  // id - 3
    tickets.PushTicket("Swan Lake");      // id - 4
    tickets.PushTicket("Boris Godunov");  // id - 5
    tickets.PushTicket("Boris Godunov");  // id - 6

    cout << tickets.GetAvailable() << endl;                 // Вывод: 7
    cout << tickets.GetAvailable("Swan Lake") << endl;      // Вывод: 3
    cout << tickets.GetAvailable("Boris Godunov") << endl;  // Вывод: 4

    // Invalidate удалит билеты с номерами 0, 1, 2:
    tickets.Invalidate(3);

    cout << tickets.GetAvailable() << endl;                 // Вывод: 4
    cout << tickets.GetAvailable("Swan Lake") << endl;      // Вывод: 1
    cout << tickets.GetAvailable("Boris Godunov") << endl;  // Вывод: 3

    tickets.PushTicket("Swan Lake");  // id - 7
    tickets.PushTicket("Swan Lake");  // id - 8

    cout << tickets.GetAvailable("Swan Lake") << endl;  // Вывод: 3
}