#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Process
{
public:
    string name;                     // 进程名字
    vector<int> max_resources;       // 最大需求
    vector<int> allocated_resources; // 已分配资源
    vector<int> needed_resources;    // 剩余需求
    bool finished;                   // 是否完成

    Process(int n) : max_resources(n), allocated_resources(n, 0), needed_resources(n), finished(false) {}

    // 初始化进程信息
    void initialize(int res_type)
    {
        cout << "输入进程名: ";
        cin >> name;
        cout << "输入" << res_type << "类资源的最大需求（空格分隔）: ";
        for (int i = 0; i < res_type; i++)
        {
            cin >> max_resources[i];
            needed_resources[i] = max_resources[i] - allocated_resources[i];
        }
    }

    // 更新资源分配
    void update_allocation(const vector<int> &request)
    {
        for (size_t j = 0; j < request.size(); j++)
        {
            allocated_resources[j] += request[j];
            needed_resources[j] = max_resources[j] - allocated_resources[j];
        }
    }
};

class Bank
{
private:
    int num_resourse;          // 资源种类数
    int num_process;           // 进程数
    vector<int> available;     // 可用资源
    vector<Process> processes; // 所有进程
    vector<int> safe_sequence; // 安全序列

public:
    Bank()
    {
        cout << "请输入资源种类数: ";
        cin >> num_resourse;
        cout << "请输入进程数: ";
        cin >> num_process;

        // 初始化可用资源
        available.resize(num_resourse);
        cout << "请输入每类资源的初始可用量（空格分隔）: ";
        for (int i = 0; i < num_resourse; i++)
        {
            cin >> available[i];
        }

        // 初始化进程信息
        for (int i = 0; i < num_process; i++)
        {
            Process p(num_resourse);
            p.initialize(num_resourse);
            processes.push_back(p);
        }
    }

    // 处理资源请求
    bool request_resources(int pid, const vector<int> &request)
    {
        // 检查请求合法性
        for (int j = 0; j < num_resourse; j++)
        {
            if (request[j] > processes[pid].needed_resources[j] ||
                request[j] > available[j])
            {
                cout << "非法请求：超过需求或资源不足！" << endl;
                return false;
            }
        }

        // 预分配资源（保存旧状态以便回滚）
        vector<int> old_available = available;
        vector<Process> old_processes = processes;

        // 更新资源状态
        for (int j = 0; j < num_resourse; j++)
        {
            available[j] -= request[j];
            processes[pid].update_allocation(request);
        }

        // 执行安全性检查
        if (!is_safe())
        {
            cout << "分配后系统不安全，回滚操作！" << endl;
            available = old_available;
            processes = old_processes;
            return false;
        }

        cout << "分配成功！安全序列：";
        for (int p : safe_sequence)
            cout << processes[p].name << " ";
        cout << endl;
        return true;
    }

    // 安全性检查算法
    bool is_safe()
    {
        vector<int> work = available;
        vector<bool> finish(num_process, false);
        safe_sequence.clear();

        bool found;
        do
        {
            found = false;
            for (int i = 0; i < num_process; i++)
            {
                if (!finish[i] && can_allocate(processes[i].needed_resources, work))
                {
                    // 模拟资源释放
                    for (int j = 0; j < num_resourse; j++)
                    {
                        work[j] += processes[i].allocated_resources[j];
                    }
                    finish[i] = true;
                    safe_sequence.push_back(i);
                    found = true;
                }
            }
        } while (found);

        // 检查所有进程是否完成
        for (bool fin : finish)
        {
            if (!fin)
                return false;
        }
        return true;
    }

    // 判断当前资源能否满足需求
    bool can_allocate(const vector<int> &need, const vector<int> &work)
    {
        for (int j = 0; j < num_resourse; j++)
        {
            if (need[j] > work[j])
                return false;
        }
        return true;
    }
};

int main()
{
    Bank bank;

    // 示例：进程0请求资源
    int pid;
    vector<int> request;
    cout << "\n输入要请求资源的进程号（0~" << bank.num_process - 1 << "）: ";
    cin >> pid;
    request.resize(bank.num_resourse);
    cout << "输入请求的资源量（" << bank.num_resourse << "类，空格分隔）: ";
    for (int i = 0; i < bank.num_resourse; i++)
        cin >> request[i];

    if (bank.request_resources(pid, request))
    {
        cout << "资源分配成功！" << endl;
    }
    else
    {
        cout << "资源分配失败！" << endl;
    }

    return 0;
}