#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <iomanip>
#include <chrono>
#include <limits>

#include "team_tasks.hpp"

using namespace std;
using namespace std::chrono;

const TasksInfo& TeamTasks::GetPersonTasksInfo(const string& person) const {
    assert(team_tasks_.count(person));
    return team_tasks_.at(person);
}

void TeamTasks::AddNewTask(const string& person) {
    auto& tasks = team_tasks_[person];
    ++tasks[TaskStatus::NEW];
}

tuple<TasksInfo, TasksInfo> TeamTasks::PerformPersonTasks(const string& person, int pop_task_count) {
    if (!team_tasks_.count(person)) return {{}, {}};

    TasksInfo updated_tasks{};
    TasksInfo not_updated_tasks{};

    auto& curr_person_tasks = team_tasks_[person];

    for (int i = static_cast<int>(TaskStatus::NEW); i < static_cast<int>(TaskStatus::DONE); i++) {
        const auto curr = static_cast<TaskStatus>(i);
        const auto next = static_cast<TaskStatus>(i + 1);

        const int initial_count = curr_person_tasks[curr];

        int& curr_count = curr_person_tasks[curr];
        int& next_count = curr_person_tasks[next];
        const int curr_updated_count = updated_tasks[curr];
        int& next_updated_count = updated_tasks[next];
        int& curr_not_updated_count = not_updated_tasks[curr];

        const int available_for_next_count = max(curr_count - curr_updated_count, 0);
        next_updated_count = available_for_next_count >= pop_task_count ? pop_task_count : available_for_next_count;

        next_count += next_updated_count;
        curr_count -= next_updated_count;
        
        curr_not_updated_count = max(initial_count - pop_task_count - curr_updated_count, 0);
        pop_task_count = max(pop_task_count - next_updated_count, 0);
    }

    set<TaskStatus> empty_item{};
    for (auto [status, task_count] : updated_tasks)
        if (!task_count) empty_item.insert(status);
    for (auto item : empty_item) updated_tasks.erase(item);

    empty_item.clear();
    for (auto [status, task_count] : not_updated_tasks)
        if (!task_count) empty_item.insert(status);
    for (auto item : empty_item) not_updated_tasks.erase(item);


    return {updated_tasks, not_updated_tasks};
}


void PrintTasksInfo__(const TasksInfo& tasks_info) {
    if (tasks_info.count(TaskStatus::NEW)) {
        std::cout << "NEW: " << tasks_info.at(TaskStatus::NEW) << " ";
    }
    if (tasks_info.count(TaskStatus::IN_PROGRESS)) {
        std::cout << "IN_PROGRESS: " << tasks_info.at(TaskStatus::IN_PROGRESS) << " ";
    }
    if (tasks_info.count(TaskStatus::TESTING)) {
        std::cout << "TESTING: " << tasks_info.at(TaskStatus::TESTING) << " ";
    }
    if (tasks_info.count(TaskStatus::DONE)) {
        std::cout << "DONE: " << tasks_info.at(TaskStatus::DONE) << " ";
    }
}
 
void PrintTasksInfo__(const TasksInfo& updated_tasks, const TasksInfo& untouched_tasks) {
    std::cout << "Updated: [";
    PrintTasksInfo__(updated_tasks);
    std::cout << "] ";
 
    std::cout << "Untouched: [";
    PrintTasksInfo__(untouched_tasks);
    std::cout << "] ";
 
    std::cout << std::endl;
}

// Принимаем словарь по значению, чтобы иметь возможность
// обращаться к отсутствующим ключам с помощью [] и получать 0,
// не меняя при этом исходный словарь.
void PrintTasksInfo(TasksInfo tasks_info) {
    /*cout << tasks_info[TaskStatus::NEW] << " new tasks"s
         << ", "s << tasks_info[TaskStatus::IN_PROGRESS]
         << " tasks in progress"s
         << ", "s << tasks_info[TaskStatus::TESTING]
         << " tasks are being tested"s
         << ", "s << tasks_info[TaskStatus::DONE] << " tasks are done"s <<
       endl;*/
    cout << "NEW: "s << std::internal << setw(3) << tasks_info[TaskStatus::NEW] << " | "s
         << "IN_PROGRESS: "s << setw(3) << tasks_info[TaskStatus::IN_PROGRESS] << " | "s
         << "TESTING: "s << setw(3) << tasks_info[TaskStatus::TESTING] << " | "s
         << "DONE: "s << setw(3) << tasks_info[TaskStatus::DONE] << endl;
}


int exec_main3() {
    TeamTasks tasks;
    for (int i = 0; i < 3; ++i) {
        tasks.AddNewTask("Ivan"s);
    }

    cout << "Ivan's tasks:\t"s;
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));

    TasksInfo updated_tasks, untouched_tasks;

    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan"s, 4);
    cout << "Total:\t\t"s;
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));
    cout << "Updated:\t"s;
    PrintTasksInfo(updated_tasks);
    cout << "Untouched:\t"s;
    PrintTasksInfo(untouched_tasks);
    cout << endl;

    return 0;
}

int exec_main11() {
    const int cycles_count = 15;//floor(numeric_limits<int>::max() / 2.0) / 5;
    TeamTasks tasks;
    auto start = high_resolution_clock::now();
    for (int i = 0; i < cycles_count; ++i) {
        tasks.AddNewTask("Ivan");
    }
    cout << std::left << setw(14) << "Ivan's tasks: "s;
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout  << endl << "total duration: " << duration.count() << "ms, for " << cycles_count << " cycles." << endl;

    start = high_resolution_clock::now();
    TasksInfo updated_tasks, untouched_tasks;
    cout << endl;
    for (int i = 0; i < cycles_count; ++i) {
        tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan"s, cycles_count);
        cout << std::left << setw(14) << "Total: "s;
        PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));
        cout << std::left << setw(14) << "Updated: "s;
        PrintTasksInfo(updated_tasks);
        cout << std::left << setw(14) << "Untouched: "s;
        PrintTasksInfo(untouched_tasks);
        cout << endl;
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    cout  << endl << "total duration: " << duration.count() << "ms, for " << cycles_count << " cycles." << endl;
    return 0;
}

int exec_main0() {
    TeamTasks tasks;
    for (int i = 0; i < 3; ++i) {
        tasks.AddNewTask("Ivan"s);
    }
    cout << "Ivan's tasks:\t"s;
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));

    TasksInfo updated_tasks, untouched_tasks;
    cout << endl;
    for (int i = 0; i < 5; ++i) {
        tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan"s, 2);
        cout << "Total:\t\t"s;
        PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));
        cout << "Updated:\t"s;
        PrintTasksInfo(updated_tasks);
        cout << "Untouched:\t"s;
        PrintTasksInfo(untouched_tasks);
        cout << endl;
    }
    return 0;
}


int exec_main() {
    TeamTasks tasks;
    TasksInfo updated_tasks;
    TasksInfo untouched_tasks;
 
 
    /* TEST 3 */
    std::cout << "\nLisa" << std::endl;
 
    for (auto i = 0; i < 5; ++i) {
        tasks.AddNewTask("Lisa");
    }
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Lisa", 5);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  // [{"IN_PROGRESS": 5}, {}]
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Lisa", 5);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  // [{"TESTING": 5}, {}]
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Lisa", 1);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  // [{"DONE": 1}, {"TESTING": 4}]
 
    for (auto i = 0; i < 5; ++i) {
        tasks.AddNewTask("Lisa");
    }
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Lisa", 2);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  //! [{"IN_PROGRESS": 2}, {"NEW": 3, "TESTING": 4}]
 
    PrintTasksInfo__(tasks.GetPersonTasksInfo("Lisa"));  // {"NEW": 3, "IN_PROGRESS": 2, "TESTING": 4, "DONE": 1}
    std::cout << std::endl;
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Lisa", 4);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  // [{"IN_PROGRESS": 3, "TESTING": 1}, {"IN_PROGRESS": 1, "TESTING": 4}]
 
    PrintTasksInfo__(tasks.GetPersonTasksInfo("Lisa"));  // {"IN_PROGRESS": 4, "TESTING": 5, "DONE": 1}
    std::cout << std::endl;
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Lisa", 5);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  // [{"TESTING": 4, "DONE": 1}, {"TESTING": 4}]
 
    PrintTasksInfo__(tasks.GetPersonTasksInfo("Lisa"));  // {"TESTING": 8, "DONE": 2}
    std::cout << std::endl;
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Lisa", 10);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  // [{"DONE": 8}, {}]
 
    PrintTasksInfo__(tasks.GetPersonTasksInfo("Lisa"));  // {"DONE": 10}
    std::cout << std::endl;
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Lisa", 10);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  // [{}, {}]
 
    PrintTasksInfo__(tasks.GetPersonTasksInfo("Lisa"));  // {"DONE": 10}
    std::cout << std::endl;
 
    tasks.AddNewTask("Lisa");
 
    PrintTasksInfo__(tasks.GetPersonTasksInfo("Lisa"));  // {"NEW": 1, "DONE": 10}
    std::cout << std::endl;
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Lisa", 2);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  // [{"IN_PROGRESS": 1}, {}]
 
    PrintTasksInfo__(tasks.GetPersonTasksInfo("Lisa"));  // {"IN_PROGRESS": 1, "DONE": 10}
    std::cout << std::endl;
 
    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Bob", 3);
    PrintTasksInfo__(updated_tasks, untouched_tasks);  // [{}, {}]
 
    return 0;
}


/*
Lisa
Updated: [IN_PROGRESS: 5 ] Untouched: [] 
Updated: [TESTING: 5 ] Untouched: [] 
Updated: [DONE: 1 ] Untouched: [TESTING: 4 ] 
Updated: [IN_PROGRESS: 2 ] Untouched: [NEW: 3 TESTING: 4 ] 
NEW: 3 IN_PROGRESS: 2 TESTING: 4 DONE: 1 
Updated: [IN_PROGRESS: 3 TESTING: 1 ] Untouched: [IN_PROGRESS: 1 TESTING: 4 ] 
IN_PROGRESS: 4 TESTING: 5 DONE: 1 
Updated: [TESTING: 4 DONE: 1 ] Untouched: [TESTING: 4 ] 
TESTING: 8 DONE: 2 
Updated: [DONE: 8 ] Untouched: [] 
DONE: 10 
Updated: [] Untouched: [] 
DONE: 10 
NEW: 1 DONE: 10 
Updated: [IN_PROGRESS: 1 ] Untouched: [] 
IN_PROGRESS: 1 DONE: 10 
Updated: [] Untouched: [] 

Updated: [IN_PROGRESS: 5 ] Untouched: [] 
Updated: [TESTING: 5 ] Untouched: [] 
Updated: [DONE: 1 ] Untouched: [TESTING: 4 ] 
Updated: [IN_PROGRESS: 2 ] Untouched: [NEW: 3 TESTING: 4 ] 
NEW: 3 IN_PROGRESS: 2 TESTING: 4 DONE: 1 
Updated: [IN_PROGRESS: 3 TESTING: 1 ] Untouched: [IN_PROGRESS: 1 TESTING: 4 ] 
NEW: 0 IN_PROGRESS: 4 TESTING: 5 DONE: 1 
Updated: [TESTING: 4 DONE: 1 ] Untouched: [TESTING: 4 ] 
NEW: 0 IN_PROGRESS: 0 TESTING: 8 DONE: 2 
Updated: [DONE: 8 ] Untouched: [] 
NEW: 0 IN_PROGRESS: 0 TESTING: 0 DONE: 10 
Updated: [] Untouched: [] 
NEW: 0 IN_PROGRESS: 0 TESTING: 0 DONE: 10 
NEW: 1 IN_PROGRESS: 0 TESTING: 0 DONE: 10 
Updated: [IN_PROGRESS: 1 ] Untouched: [] 
NEW: 0 IN_PROGRESS: 1 TESTING: 0 DONE: 10 
Updated: [] Untouched: [] 


*/

int exec_main000() {
    TeamTasks tasks;
    for (int i = 0; i < 5; ++i) {
        tasks.AddNewTask("Alice");
    }
 
    TasksInfo updated_tasks, untouched_tasks;
 
    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Alice", 5);
 
    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Alice", 5);
 
    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Alice", 1);
 
    for (int i = 0; i < 5; ++i) {
        tasks.AddNewTask("Alice");
    }
 
    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Alice", 2);
    cout << "AlisaTask\n";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Alice"));
    PrintTasksInfo(updated_tasks);
    PrintTasksInfo(untouched_tasks);
    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Alice", 4);
    cout << "AlisaTask too\n";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Alice"));
    PrintTasksInfo(updated_tasks);
    PrintTasksInfo(untouched_tasks);
 
 
    return 0;
}

/*
Correct Output:
AlisaTask
3 new tasks, 2 tasks in progress, 4 tasks are being tested, 1 tasks are done
0 new tasks, 2 tasks in progress, 0 tasks are being tested, 0 tasks are done
3 new tasks, 0 tasks in progress, 4 tasks are being tested, 0 tasks are done
AlisaTask too
0 new tasks, 4 tasks in progress, 5 tasks are being tested, 1 tasks are done
0 new tasks, 3 tasks in progress, 1 tasks are being tested, 0 tasks are done
0 new tasks, 1 tasks in progress, 4 tasks are being tested, 0 tasks are done

-----------------------------------------------------------------------------

NEW:   3 | IN_PROGRESS:   2 | TESTING:   4 | DONE:   1
NEW:   0 | IN_PROGRESS:   2 | TESTING:   0 | DONE:   0
NEW:   3 | IN_PROGRESS:   0 | TESTING:   0 | DONE:   0
AlisaTask too
NEW:   0 | IN_PROGRESS:   4 | TESTING:   5 | DONE:   1
NEW:   0 | IN_PROGRESS:   3 | TESTING:   1 | DONE:   0
NEW:   0 | IN_PROGRESS:   1 | TESTING:   4 | DONE:   0

AlisaTask
NEW:   3 | IN_PROGRESS:   2 | TESTING:   4 | DONE:   1
NEW:   0 | IN_PROGRESS:   2 | TESTING:   0 | DONE:   0
NEW:   3 | IN_PROGRESS:   0 | TESTING:   4 | DONE:   0
AlisaTask too
NEW:   0 | IN_PROGRESS:   4 | TESTING:   5 | DONE:   1
NEW:   0 | IN_PROGRESS:   3 | TESTING:   1 | DONE:   0
NEW:   0 | IN_PROGRESS:   1 | TESTING:   4 | DONE:   0


*/