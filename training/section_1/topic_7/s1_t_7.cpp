#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "team_tasks.hpp"

using namespace std;

const TasksInfo& TeamTasks::GetPersonTasksInfo(const string& person) const {
    assert(team_tasks_.count(person));
    return team_tasks_.at(person);
}

void TeamTasks::AddNewTask(const string& person) {
    auto& tasks = team_tasks_[person];
    ++tasks[TaskStatus::NEW];

    for (int i = static_cast<int>(TaskStatus::NEW) + 1; i <= static_cast<int>(TaskStatus::DONE); i++) {
        const auto status = static_cast<TaskStatus>(i);
        if (!tasks.count(status)) {
            tasks[status] = 0;
        }
    }
}

tuple<TasksInfo, TasksInfo> TeamTasks::PerformPersonTasks(const string& person, int pop_task_count /* value for pop current task */) {
    assert(team_tasks_.count(person));
    TasksInfo updated_tasks{};
    TasksInfo not_updated_tasks{};

    updated_tasks[TaskStatus::NEW] = 0;
    const int initial_pop_task_count = pop_task_count;

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
        //curr_not_updated_count = max(initial_count - initial_pop_task_count, 0);

        if (next_updated_count == 0 && pop_task_count == 0) {
            //curr_not_updated_count = 0;
            break;
        }
    }

    /*
    set<TaskStatus> empty_item{};
    for (auto [status, task_count] : updated_tasks)
        if (!task_count) empty_item.insert(status);
    for (auto item : empty_item) updated_tasks.erase(item);

    empty_item.clear();
    for (auto [status, task_count] : not_updated_tasks)
        if (!task_count) empty_item.insert(status);
    for (auto item : empty_item) not_updated_tasks.erase(item);
    */

    if (not_updated_tasks.count(TaskStatus::DONE)) not_updated_tasks.erase(TaskStatus::DONE);

    return {updated_tasks, not_updated_tasks};
    //return make_tuple(updated_tasks, not_updated_tasks);
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
    cout << "NEW: "s << tasks_info[TaskStatus::NEW] << " | "s
         << "IN_PROGRESS: "s << tasks_info[TaskStatus::IN_PROGRESS] << " | "s
         << "TESTING: "s << tasks_info[TaskStatus::TESTING] << " | "s
         << "DONE: "s << tasks_info[TaskStatus::DONE] << endl;
}

int exec_main0() {
    TeamTasks tasks;
    tasks.AddNewTask("Ilia"s);
    for (int i = 0; i < 3; ++i) {
        tasks.AddNewTask("Ivan"s);
    }
    cout << "Ilia's tasks: "s;
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"s));
    cout << "Ivan's tasks: "s;
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));

    TasksInfo updated_tasks, untouched_tasks;

    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan"s, 4);  //!
    cout << "Updated Ivan's tasks: "s;
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: "s;
    PrintTasksInfo(untouched_tasks);

    return 0;
}

int exec_main2() {
    TeamTasks tasks;
    tasks.AddNewTask("Ilia");
    for (int i = 0; i < 30; ++i) {
        tasks.AddNewTask("Ivan");
    }
    cout << "Ilia's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"));
    cout << "Ivan's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));

    TasksInfo updated_tasks, untouched_tasks;

    for (int i = 0; i < 5; ++i) {
        tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan", 10);
        cout << "Updated Ivan's tasks: ";
        PrintTasksInfo(updated_tasks);
        cout << "Untouched Ivan's tasks: ";
        PrintTasksInfo(untouched_tasks);
        cout << endl;
    }

    return 0;
}

int exec_main() {
    TeamTasks tasks;
    for (int i = 0; i < 3; ++i) {
        tasks.AddNewTask("Ivan"s);
    }
    cout << "Ivan's tasks:\t"s;
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));

    TasksInfo updated_tasks, untouched_tasks;
    cout << endl;
    for (int i = 0; i < 2; ++i) {
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