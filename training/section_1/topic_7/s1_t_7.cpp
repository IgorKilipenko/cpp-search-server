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

    for (int i = static_cast<int>(TaskStatus::NEW) + 1;
         i <= static_cast<int>(TaskStatus::DONE); i++) {
        const auto status = static_cast<TaskStatus>(i);
        if (!tasks.count(status)) {
            tasks[status] = 0;
        }
    }
}

tuple<TasksInfo, TasksInfo> TeamTasks::PerformPersonTasks(
    const string& person, int pop_task_count /* value for pop current task */) {
    assert(team_tasks_.count(person));
    TasksInfo updated_tasks{};
    TasksInfo not_updated_tasks{};

    updated_tasks[TaskStatus::NEW] = 0;

    auto& curr_person_tasks = team_tasks_[person];

    for (int i = static_cast<int>(TaskStatus::NEW);
         i < static_cast<int>(TaskStatus::DONE); i++) {
        const auto curr_status = static_cast<TaskStatus>(i);
        const auto next_status = static_cast<TaskStatus>(i + 1);

        const int curr_task_count = curr_person_tasks.count(curr_status)
                                        ? curr_person_tasks.at(curr_status)
                                        : 0;
        curr_person_tasks[curr_status] = max(
            curr_task_count +
                updated_tasks[curr_status] /* is new task from prev step */ -
                pop_task_count,
            0);
        updated_tasks[next_status] = max(
            curr_task_count -
                (curr_person_tasks[curr_status] - updated_tasks[curr_status]),
            0);
        not_updated_tasks[curr_status] =
            max(curr_task_count - pop_task_count, 0);
        pop_task_count = max(pop_task_count - curr_task_count, 0);
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

// Принимаем словарь по значению, чтобы иметь возможность
// обращаться к отсутствующим ключам с помощью [] и получать 0,
// не меняя при этом исходный словарь.
void PrintTasksInfo(TasksInfo tasks_info) {
    cout << tasks_info[TaskStatus::NEW] << " new tasks"s
         << ", "s << tasks_info[TaskStatus::IN_PROGRESS]
         << " tasks in progress"s
         << ", "s << tasks_info[TaskStatus::TESTING]
         << " tasks are being tested"s
         << ", "s << tasks_info[TaskStatus::DONE] << " tasks are done"s << endl;
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

    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Ivan"s, 4);  //!
    cout << "Updated Ivan's tasks: "s;
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: "s;
    PrintTasksInfo(untouched_tasks);

    return 0;
}

int exec_main() {
    TeamTasks tasks;
    tasks.AddNewTask("Ilia");
    for (int i = 0; i < 3; ++i) {
        tasks.AddNewTask("Ivan");
    }
    cout << "Ilia's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"));
    cout << "Ivan's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));

    TasksInfo updated_tasks, untouched_tasks;

    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    return 0;
}