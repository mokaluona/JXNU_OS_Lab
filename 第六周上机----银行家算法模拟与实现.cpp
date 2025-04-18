#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

class Process
{
public:
    string name;                                  // 进程名字
    vector<int> max_resources;                    // 进程对每类资源的最大需求
    vector<int> allocated_resources;              // 进程当前已分配的资源。
    vector<int> needed_resources;                 // 进程剩余需求的资源（动态计算：max_resources - allocated_resources）。
    bool finished = 0;                            // 进程状态 1完成
    bool check_finished();                        // 检查进程是否完成
    bool is_enough(vector<int> available, int n); // 判断是否满足需求
    Process(int n);
};

Process::Process(int n) : max_resources(n), allocated_resources(n, 0), needed_resources(n)
{
    cin >> name;
    for (int i = 0; i < n; i++)
    {
        cin >> max_resources[i];
        needed_resources[i] = max_resources[i];
    }
}

bool Process::is_enough(vector<int> available, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (available[i] < needed_resources[i])
        {
            return false;
        }
    }
    return true;
}

bool Process::check_finished()
{
    for (auto i : needed_resources)
    {
        if (i != 0)
        {
            return false;
        }
    }
    finished = 1;
    return true;
}

class Bank
{
public:
    int num_resourse;           // 资源数量
    vector<int> available;      // 可用资源
    int num_process;            // 进程数
    vector<Process> work;       // 所有进程
    Bank(int num_r, int num_p); // 构造函数
    void req();
    bool is_finish();                               // 判断是否所有进程都已完成
    int find_name(string name);                     // 找到进程的索引
    bool is_available(vector<int> reqs, int index); // 判断资源是否足够
    bool safity();                                  // 安全性检查算法
    void display();

private:
    vector<string> safe_que; // 安全序列
    void print_safe_que();
};
Bank::Bank(int num_r, int num_p) : num_resourse(num_r), num_process(num_p), available(num_r)
{
    for (int i = 0; i < num_resourse; i++)
    {
        cin >> available[i];
    }
    cout << "请输入进程的相关信息:\n进程名   最大需求量" << endl;
    for (int i = 0; i < num_process; i++)
    {
        Process p(num_resourse);
        work.push_back(p);
    }
}

bool Bank::is_finish()
{
    for (auto p : work)
    {
        if (!p.finished)
            return false;
    }
    return true;
}

int Bank::find_name(string name)
{
    for (int i = 0; i < num_process; i++)
    {
        if (work[i].name == name)
        {
            return i;
        }
    }
    return -1;
}
void Bank::print_safe_que()
{
    for (auto i : safe_que)
    {
        cout << i;
        if (i != safe_que.back())
        {
            cout << "->";
        }
    }
    cout << endl;
}

void print(string s)
{
    cout << s;
}
void print(vector<int> a)
{
    for (auto i : a)
    {
        cout << i << " ";
    }
}
void print(bool state)
{
    if (state)
    {
        cout << "Finished";
    }
    else
    {
        cout << "Working";
    }
}
void print_s(int i)
{
    for (int j = 0; j < i; j++)
    {
        cout << " ";
    }
}

void Bank::display()
{
    cout << "进程名\t最大需求量\t剩余需求量\t已分配资源\t执行状态" << endl;
    for (auto i : work)
    {
        print(i.name);
        print_s(8);
        print(i.max_resources);
        print_s(10);
        print(i.needed_resources);
        print_s(10);
        print(i.allocated_resources);
        print_s(9);
        print(i.finished);
        cout << endl;
    }
    cout << "系统剩余资源数：" << endl;
    print(available);
    cout << "\n\n";
}

bool Bank::is_available(vector<int> reqs, int index)
{
    for (int i = 0; i < num_resourse; i++)
    {
        if (reqs[i] > available[i])
        {
            return false;
        }
        else if (reqs[i] > work[index].needed_resources[i])
        {
            return false;
        }
    }
    return true;
}

bool Bank::safity()
{
    bool flag = 1;
    while (!work.empty() && flag)
    {
        flag = 0;
        work.erase(
            remove_if(work.begin(), work.end(), [](const Process &p)
                      { return p.finished; }),
            work.end());
        for (auto &i : work)
        {
            if (i.is_enough(available, num_resourse))
            {
                flag = 1;
                for (int j = 0; j < num_resourse; j++)
                {
                    available[j] += i.allocated_resources[j];
                }
                i.finished = 1;
                safe_que.push_back(i.name);
            }
        }
    }
    if (work.empty())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Bank::req()
{
    string name;
    int index;
    vector<int> reqs(num_resourse);
    cout << "请为进程分配相关资源" << endl;
    while (!is_finish())
    {
        cout << "请输入进程名：" << endl;
        cin >> name;
        while ((index = find_name(name)) == -1)
        {
            cout << "进程不存在或已完成，请重新输入" << endl;
            cin >> name;
        }
        cout << "请输入该进程当前申请的各资源数量" << endl;
        for (int i = 0; i < num_resourse; ++i)
        {
            cin >> reqs[i];
        }
        if (!is_available(reqs, index))
        {
            cout << "\n申请异常，请重新输入\n当前系统可用资源如下:" << endl;
            for (auto i : available)
            {
                cout << i << " ";
            }
            cout << "\n当前进程剩余所需资源如下" << endl;
            for (auto i : work[index].needed_resources)
            {
                cout << i << " ";
            }
            cout << "\n请重新输入";
            continue;
        }
        Bank test = *this;
        test.safe_que.clear();
        for (int i = 0; i < num_resourse; i++)
        {
            test.available[i] -= reqs[i];
            test.work[index].allocated_resources[i] += reqs[i];
            test.work[index].needed_resources[i] -= reqs[i];
        }
        if (test.safity())
        {
            safe_que = test.safe_que;
            safe_que.erase(
                remove_if(safe_que.begin(), safe_que.end(), [&](const string &str)
                          {
                              return str == name; // 如果元素等于 name，则返回 true
                          }),
                safe_que.end());

            for (int i = 0; i < num_resourse; i++)
            { // 分配资源
                available[i] -= reqs[i];
                work[index].allocated_resources[i] += reqs[i];
                work[index].needed_resources[i] -= reqs[i];
            }
            if (work[index].check_finished())
            { // 若完成释放资源
                for (int i = 0; i < num_resourse; i++)
                {
                    available[i] += work[index].allocated_resources[i];
                    work[index].allocated_resources[i] = 0;
                }
            }
            cout << "申请成功,安全序列为:";
            print_safe_que();
        }
        else
        {
            cout << "无安全序列,请重新输入" << endl;
        }
        cout << "是否需要申请系统资源？（Y/N）：" << endl;
        char c;
        cin >> c;
        display();
        if (c == 'N' || c == 'n')
        {
            break;
        }
    }
}

int main()
{
    int n1, n2;
    cout << "请输入进程数" << endl;
    cin >> n1;
    cout << "请输入资源种类的数量" << endl;
    cin >> n2;
    cout << "请输入每种资源初始数量" << endl;
    Bank bank(n2, n1);
    cout << "进程开始执行！" << endl;
    bank.req();
}

/*

5
3
10 5 7

p1 7  5  3
p2 3  2  2
p3 9  0  2
p4 2  2  2
p5 4  3  3

p1
0  1  1
Y
p2
2  0  0
Y
p3
3  0  2
Y
p4
2  1  1
Y
p5
0  0  2

y
p1
4 3 1
p1
1 2 1

y

p1
1 2 0

y
p4
0 1 1

y
p2
1 2 2

y
p3
6 0 0

y
p5
1 1 1

y
p1
6 2 2

y
p5
4 3 1

n

*/
