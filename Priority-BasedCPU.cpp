//Priority-BasedCPU.cpp 
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

struct Task {
    std::string name;
    int arrivalTime;
    int priority;
    int burstTime;
    int startTime = -1;
    int endTime = -1;
    int remainingTime;
};

class Scheduler {
private:
    std::vector<Task> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    int currentTime = 0;

public:
    void loadTasks(const std::string& filename);
    void schedule();
    void generateGanttChart();
    void reset();  // Added reset function
};

void Scheduler::loadTasks(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        std::istringstream iss(line);
        Task task;
        char comma;

        if (std::getline(iss, task.name, ',') &&
            (iss >> task.arrivalTime >> comma) && comma == ',' &&
            (iss >> task.priority >> comma) && comma == ',' &&
            (iss >> task.burstTime)) {
            task.remainingTime = task.burstTime;
            tasks.push_back(task);
        } else {
            std::cerr << "Warning: Skipping malformed line: " << line << std::endl;
            std::cerr << "Expected format: TaskName, arrivalTime, priority, burstTime (with commas and spaces as shown)" << std::endl;
        }
    }
    file.close();

    std::cout << "Tasks loaded successfully:\n";
    for (const auto& task : tasks) {
        std::cout << "Task: " << task.name << ", Arrival Time: " << task.arrivalTime 
                  << ", Priority: " << task.priority << ", Burst Time: " << task.burstTime << std::endl;
    }
}

void Scheduler::schedule() {
    std::sort(tasks.begin(), tasks.end(), [](const Task &a, const Task &b) {
        return (a.arrivalTime < b.arrivalTime) || 
               (a.arrivalTime == b.arrivalTime && a.priority > b.priority);
    });

    auto compare = [](Task* a, Task* b) {
        if (a->priority == b->priority) {
            return a->arrivalTime > b->arrivalTime;
        }
        return a->priority < b->priority;
    };
    std::priority_queue<Task*, std::vector<Task*>, decltype(compare)> readyQueue(compare);

    int numTasks = tasks.size();
    int completedTasks = 0;
    Task* currentTask = nullptr;

    while (completedTasks < numTasks) {
        for (Task& task : tasks) {
            if (task.arrivalTime <= currentTime && task.remainingTime > 0 && task.startTime == -1) {
                readyQueue.push(&task);
                task.startTime = currentTime;
            }
        }

        // Check if we need to preempt the current task
        if (!readyQueue.empty() && (currentTask == nullptr || readyQueue.top()->priority > currentTask->priority)) {
            if (currentTask != nullptr && currentTask->remainingTime > 0) {
                readyQueue.push(currentTask);
            }
            currentTask = readyQueue.top();
            readyQueue.pop();
        }

        // Process the current task
        if (currentTask != nullptr) {
            currentTask->remainingTime--;
            currentTime++;
            if (currentTask->remainingTime == 0) {
                currentTask->endTime = currentTime;
                completedTasks++;
                currentTask = nullptr;
            }
        } else {
            currentTime++;
        }
    }
}

// Generate Gantt Chart from scheduled tasks
void Scheduler::generateGanttChart() {
    std::cout << "\nGantt Chart:\n";
    for (const Task& task : tasks) {
        std::cout << "Task: " << task.name 
                  << ", Start: " << task.startTime 
                  << ", End: " << task.endTime 
                  << std::endl;
    }
}

// Reset the scheduler to clear tasks and reset time
void Scheduler::reset() {
    tasks.clear();
    currentTime = 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <task_file>" << std::endl;
        return 1;
    }

    Scheduler scheduler;
    scheduler.loadTasks(argv[1]);
    scheduler.schedule();
    scheduler.generateGanttChart();

    // Additional Test Cases
    std::cout << "\nRunning additional test cases...\n";

    // Test Case 1
    std::cout << "\nTest Case 1:\n";
    scheduler.reset();  // Reset scheduler to clear tasks and currentTime
    scheduler.loadTasks("tests1.txt");
    scheduler.schedule();
    scheduler.generateGanttChart();

    // Test Case 2
    std::cout << "\nTest Case 2:\n";
    scheduler.reset();  // Reset scheduler to clear tasks and currentTime
    scheduler.loadTasks("tests2.txt");
    scheduler.schedule();
    scheduler.generateGanttChart();

    // Test Case 3
    std::cout << "\nTest Case 3:\n";
    scheduler.reset();  // Reset scheduler to clear tasks and currentTime
    scheduler.loadTasks("tests3.txt");
    scheduler.schedule();
    scheduler.generateGanttChart();

    return 0;
}
