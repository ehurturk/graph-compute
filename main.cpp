#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

/*
 * TODO:
 *  - Improve terminate
 *  - Do phase analysis / multithreaded execution.
 */

enum TaskType {
    CREATE_PROMPT_TASK,
    USE_LLM_TASK,
    PARSE_JSON_TASK,
    OUTPUT_DATA_TASK,
};

struct Task {
    TaskType type;
    string name;
    string id;
    function<void()> execute;
    vector<Task *> inputs;
    vector<Task *> outputs;
    bool entry_point;
};

class ExecutionEnv {
  private:
    queue<Task *> q;
    vector<Task *> adj_list;

    vector<Task *> get_entry_points();

  public:
    void add_task(Task *t);
    void execute();
    void terminate();
};

void ExecutionEnv::add_task(Task *t) { adj_list.push_back(t); }

vector<Task *> ExecutionEnv::get_entry_points() {
    vector<Task *> entry_points;
    for (const auto &task : adj_list) {
        if (task->entry_point)
            entry_points.push_back(task);
    }
    return entry_points;
}

void ExecutionEnv::execute() {
    cout << "[ExecutionEnv]:START EXECUTION" << endl;
    // 1st approach: BFS
    vector<Task *> inbounds = get_entry_points();
    unordered_map<string, bool> visited;
    Task *t = new Task;
    t->outputs = inbounds;
    t->execute = [] {};
    t->id = "START_NODE";
    t->name = "START_NODE";
    t->inputs = {};
    q.push(t);

    while (!q.empty()) {
        Task *ft = q.front();
        ft->execute();
        q.pop();

        for (const auto &adj : ft->outputs) {
            // for every neighbour
            if (!visited[adj->id]) {
                visited[adj->id] = true;
                q.push(adj);
            }
        }
    }

    cout << "[ExecutionEnv]:END EXECUTION" << endl;
    delete t;
}

void ExecutionEnv::terminate() {
    for (int i = 0; i < adj_list.size(); i++) {
        delete adj_list[i];
        adj_list.erase(adj_list.begin() + i);
        i--; // TODO: Maybe erase this??
    }
}

uint32_t id = 0;
Task *drag_drop_node(TaskType type, function<void()> func, ExecutionEnv *env) {
    Task *nt = new Task;
    nt->type = type;
    nt->id = ++id;
    nt->name = "node" + to_string(id);
    nt->execute = func;
    nt->inputs = {};
    nt->outputs = {};
    switch (type) {
    case CREATE_PROMPT_TASK:
        nt->entry_point = true;
        break;
    default:
        nt->entry_point = false;
        break;
    }
    env->add_task(nt);
    return nt;
}

void connect_task_to(Task *src, Task *dest) { src->outputs.push_back(dest); }

int main() {
    unique_ptr<ExecutionEnv> env = make_unique<ExecutionEnv>();

    Task *t1 = drag_drop_node(
        TaskType::CREATE_PROMPT_TASK,
        [] { cout << "Creating node with blah blah..." << endl; }, env.get());

    Task *t2 = drag_drop_node(
        TaskType::PARSE_JSON_TASK, [] { cout << "Parsing json..." << endl; },
        env.get());

    Task *t3 = drag_drop_node(
        TaskType::USE_LLM_TASK, [] { cout << "Using llm..." << endl; },
        env.get());

    Task *t4 = drag_drop_node(
        TaskType::OUTPUT_DATA_TASK,
        [] { cout << "Outputting data..." << endl; }, env.get());

    env->add_task(t1);
    env->add_task(t4);
    env->add_task(t3);
    env->add_task(t2);

    connect_task_to(t1, t2);
    connect_task_to(t1, t3);
    connect_task_to(t2, t4);
    connect_task_to(t3, t4);

    // Now, env array is unordered.
    // We need to create a phasing of the tasks based on their adjacency
    // list.

    env->execute();
    env->terminate();

    return 0;
}
