#include <map>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

// Перечислимый тип для статуса задачи
enum class TaskStatus {
    NEW /*=1*/,                 // новая
    IN_PROGRESS /*=1<<(2^0)*/,  // в разработке
    TESTING /*=1<<(2^1)*/,      // на тестировании
    DONE /*=1<<(2^2)*/          // завершена
};

// Объявляем тип-синоним для map<TaskStatus, int>,
// позволяющего хранить количество задач каждого статуса
using TasksInfo = map<TaskStatus, int>;

class TeamTasks {
   public:
    // Получить статистику по статусам задач конкретного разработчика
    const TasksInfo& GetPersonTasksInfo(const string& person) const;

    // Добавить новую задачу (в статусе NEW) для конкретного разработчитка
    void AddNewTask(const string& person);

    // Обновить статусы по данному количеству задач конкретного разработчика,
    // подробности см. ниже
    tuple<TasksInfo, TasksInfo> PerformPersonTasks(const string& person,
                                                   int task_count);

   private:
    map<string, TasksInfo> team_tasks_;
};

int exec_main(void);