#include <memory>
#include <queue>
#include <string>
#include <functional>
#include <iostream>
#include <vector>

enum TaskType {
    CREATE_PROMPT_TASK,
    USE_LLM_TASK,
    PARSE_JSON_TASK,
};

struct Task {
    TaskType type;
    std::string name;
    std::string id;
    std::function<void()> execute;
    std::vector<Task*> inputs;
    std::vector<Task*> outputs;
};

class ExecutionEnv {
private:
    std::queue<Task*> execution_queue;
public:
    void register_task(Task* task) {
        std::cout << "Pushing task: " << task->name << std::endl;
        execution_queue.push(task);
    }
    
    void execute() {
        while (!execution_queue.empty()) {
            Task* task = execution_queue.front();
            execution_queue.pop();
            task->execute();
        }
    }
};

int main() {
    std::unique_ptr<ExecutionEnv> env = std::make_unique<ExecutionEnv>();
    
    // Create tasks
    Task prompt_task;
    Task llm_task;
    
    // Initialize prompt task
    prompt_task.id = "2";
    prompt_task.name = "Craft Prompt";
    prompt_task.type = TaskType::CREATE_PROMPT_TASK;
    prompt_task.outputs.push_back(&llm_task);
    prompt_task.execute = [] {
        std::cout << "Executing Prompt Task!" << std::endl;
    };
    
    // Initialize llm task
    llm_task.id = "1";
    llm_task.name = "Use LLM";
    llm_task.type = TaskType::USE_LLM_TASK;
    llm_task.inputs.push_back(&prompt_task);
    llm_task.execute = [] {
        std::cout << "Executing LLM task!" << std::endl;
    };
    
    env->register_task(&prompt_task);
    env->register_task(&llm_task);
    env->execute();
    
    return 0;
}
