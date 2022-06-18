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

#include "team_tasks.hpp"

using namespace std;

const TasksInfo& TeamTasks::GetPersonTasksInfo(const string& person) const {
    assert(team_tasks_.count(person));
    return team_tasks_.at(person);
}

void TeamTasks::AddNewTask(const string& person) {
    auto& tasks = team_tasks_[person];
    ++tasks[TaskStatus::NEW];
    
    /*tasks[TaskStatus::NEW] = 3;
    tasks[TaskStatus::IN_PROGRESS] = 2;
    tasks[TaskStatus::TESTING] = 4;
    tasks[TaskStatus::DONE] = 1;*/
    
    for (int i = static_cast<int>(TaskStatus::NEW) + 1; i <= static_cast<int>(TaskStatus::DONE); i++) {
        const auto status = static_cast<TaskStatus>(i);
        if (!tasks.count(status)) {
            tasks[status] = 0;
        }
    }
    
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

        if (next_updated_count == 0 && pop_task_count == 0) break;
    }
/*
    const TaskStatus done_staus = TaskStatus::DONE;
    if (pop_task_count && curr_person_tasks.count(done_staus)) {
        const int updated_count = updated_tasks[done_staus];
        const int available_for_update = max(curr_person_tasks.at(done_staus) - updated_count, 0);
        curr_person_tasks.at(done_staus) = max(available_for_update - pop_task_count,0);
    }
*/
    if (not_updated_tasks.count(TaskStatus::DONE)) not_updated_tasks.erase(TaskStatus::DONE);

    return {updated_tasks, not_updated_tasks};
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

int exec_main() {
    TeamTasks tasks;
    for (int i = 0; i < 10000030; ++i) {
        tasks.AddNewTask("Ivan");
    }
    cout << std::left << setw(14) << "Ivan's tasks: "s;
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));

    TasksInfo updated_tasks, untouched_tasks;
    cout << endl;
    for (int i = 0; i < 10000030 / 5; ++i) {
        tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan"s, 10000030 / 5);
        cout << std::left << setw(14) << "Total: "s;
        PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));
        cout << std::left << setw(14) << "Updated: "s;
        PrintTasksInfo(updated_tasks);
        cout << std::left << setw(14) << "Untouched: "s;
        PrintTasksInfo(untouched_tasks);
        cout << endl;
    }

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