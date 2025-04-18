#include <iomanip>
#include <iostream>
#include <string.h>
#include <vector>
using namespace std;
int timepiece = 8;

class Time
{
public:
    Time() = default;
    Time(int hour, int minute) : hour(hour), minute(minute) {}
    explicit Time(int sum) : hour(sum / 60), minute(sum % 60) {}
    bool operator>=(const Time &other) const;
    bool operator<(const Time &other) const;
    Time &operator+=(const int &addtime);
    Time operator-(const Time &other) const;
    int cal_sum() const;
    void print_time() const;

private:
    int hour = 0;
    int minute = 0;
};

bool Time::operator>=(const Time &other) const
{
    if (hour != other.hour)
        return hour >= other.hour;
    return minute >= other.minute;
}

bool Time::operator<(const Time &other) const
{
    if (hour != other.hour)
        return hour < other.hour;
    return minute < other.minute;
}
Time &Time::operator+=(const int &addtime)
{
    *this = Time(cal_sum() + addtime);
    return *this;
}

Time Time::operator-(const Time &other) const
{
    Time x = Time(cal_sum() - other.cal_sum());
    return x;
}

int Time::cal_sum() const
{
    return hour * 60 + minute;
}

void Time::print_time() const
{
    cout << "   " << setfill('0') << setw(2) << hour << ":"
         << setw(2) << minute << "     ";
}

class Node
{
public:
    int id;       // 进程编号
    string name;  // 进程名
    Time arrive;  // 到达就绪队列的时间
    int zx;       // 执行时间
    Time start;   // 开始执行时间
    Time finish;  // 执行完成时间
    int zz;       // 周转时间=执行完成时间-到达就绪队列时间
    float zzxs;   // 带权周转时间=周转时间/执行时间
    Time current; // 当前进程开始执行的时间
    int done = 0; // 进程已经完成的时间
    int remain;   // 当前进程的剩余时间
    // explicit Node(int id, string name, Time arrive = Time(), int zx = 0, Time start = Time(), Time finish = Time(), int zz = 0, float zzxs = 0.0f) : id(id), name(name), arrive(arrive), zx(zx), start(start), finish(finish), zz(zz), zzxs(zzxs) {}
    explicit Node(int id,
                  std::string name,
                  Time arrive = {},
                  int execute_time = 0)
        : id(id),
          name(std::move(name)),
          arrive(arrive),
          zx(execute_time) { remain = zx; }
    bool ComputeTime(Time &now);
    bool not_started() { return started; }
    void check_started(Time now);

private:
    bool started = 1;
};

void Node::check_started(Time now)
{
    if (not_started())
    {
        start = now;
        started = 0;
    }
}

// 函数名：ComputeTime()
bool Node::ComputeTime(Time &now)
{
    // 函数功能：更新当前时间和进程的各项时间
    Time new_time = max(now, arrive);
    current = new_time;
    now = current;
    check_started(now);
    int during = min(remain, timepiece);
    now += during;
    done += during;
    remain -= during;
    if (during == timepiece)
    {
        return 0;
    }
    else
    {
        finish = now;
        zz = finish.cal_sum() - arrive.cal_sum();
        zzxs = static_cast<float>(zz) / zx;
    }
    return 1;
}

class Queue
{
public:
    vector<Node> que;
    void addNode(Node p);
    void workout(Queue *d, Time &now);
    void Print();
    void ScanIn(int n);
    bool reflesh_ready(Queue *ready, const Time &now);
    Time find_first();
    void turn(int &now);
    bool Apiece(Queue &ready, Time &now, int &n, Queue &d);
};

// 函数名： addNode()
void Queue::addNode(Node p)
{
    que.push_back(p);
}

Time Queue::find_first()
{
    Time t(23, 59);
    for (auto &it : que)
    {
        if (it.arrive < t)
        {
            t = it.arrive;
        }
    }
    return t;
}

bool Queue::reflesh_ready(Queue *ready, const Time &now)
{
    if (que.empty())
    {
        return 0;
    }
    for (auto it = que.begin(); it != que.end(); ++it)
    {
        bool falg = now >= it->arrive;
        if (falg) // 将已到达进程加入就绪队列，并在等待队列中删除
        {
            ready->addNode(*it);
            it = que.erase(it);
            if (it == que.end())
                return 1;
        }
    }
    return 1;
}

void print_value(Time a)
{
    a.print_time();
}
void print_value(int a) { cout << a << "     "; }
void print_value(float a) { cout << a << "    "; }
void print_value(string a) { cout << a << "     "; }

void Queue::turn(int &n)
{
    cout << "第" << n << "轮就绪和调度的结果:" << endl;
    cout << "id号    名字   到达时间   总执行时间（分钟）   当前开始时间   已完成时间（分钟）  剩余完成时间（分钟）" << endl;
    for (auto it = que.begin(); it != que.end(); ++it)
    {
        print_value(it->id);
        print_value(it->name);
        print_value(it->arrive);
        print_value(it->zx);
        cout << "         ";
        print_value(it->current);
        cout << "       ";
        print_value(it->done);
        cout << "            ";
        print_value(it->remain);
        cout << endl;
    }
    n++;
}

bool Queue::Apiece(Queue &ready, Time &now, int &n, Queue &d)
{
    bool dele, flag;
    Node a = *ready.que.begin();
    dele = a.ComputeTime(now);         // 执行时间片
    flag = reflesh_ready(&ready, now); // 刷新队列
    *ready.que.begin() = a;            // 更新第一个
    ready.turn(n);                     // 输出
    now.print_time();
    cout << endl;
    ready.que.erase(ready.que.begin()); // 弹出已经执行好的节点
    if (dele)
    {
        d.que.push_back(a);
    }
    else
    {
        ready.que.push_back(a);
    }
    return flag;
}

// 函数名：workout()    参数:now当前时间
void Queue::workout(Queue *d, Time &now)
{
    // 函数功能：进行优先级进程调度，并同时更新当前时间。
    Queue ready;
    bool flag = true; // 0表示等待队列为空
    int n = 1;
    flag = reflesh_ready(&ready, now);
    while (flag)
    {
        if (ready.que.empty())
        {
            now = find_first(); // 就绪队列为空则跳到下一个最近的进程就绪时间
            flag = reflesh_ready(&ready, now);
            continue;
        }
        flag = Apiece(ready, now, n, *d);
    }
    while (!ready.que.empty())
    {
        flag = Apiece(ready, now, n, *d);
    }
}

// 函数名：Print()    参数:que进程队列指针, n 进程数
void Queue::Print()
{
    // 函数功能：打印输出进程优先进程调度结果
    cout << "模拟进程有线调度过程输出结果" << endl;
    cout << "id号    名字   到达时间   执行时间（分钟）   开始时间   完成时间  周转时间（分钟）  带权周转系数" << endl;
    float arrzz = 0, arrzzxs = 0;
    for (auto it = que.begin(); it != que.end(); ++it)
    {
        print_value(it->id);
        print_value(it->name);
        print_value(it->arrive);
        print_value(it->zx);
        cout << "         ";
        print_value(it->start);
        print_value(it->finish);
        print_value(it->zz);
        cout << "          ";
        print_value(it->zzxs);
        arrzz += it->zz;
        arrzzxs += it->zzxs;
        cout << endl;
    }
    int n = que.size();
    cout << "系统平均周转周期为： " << static_cast<float>(arrzz / n) << endl;
    cout << "系统平均带权周转系数为： " << static_cast<float>(arrzzxs / n) << endl;
    cout << endl;
}

// 函数名：ScanIn()    参数:n 进程数
void Queue::ScanIn(int n)
{
    // 函数功能：输入进程信息，返回最早的进程到达时间
    int id, a1, a2, zx;
    char c;
    string name;
    while (n--)
    {
        cin >> id >> name >> a1 >> c >> a2 >> zx;
        Time arrive(a1, a2);
        Node p(id, name, arrive, zx);
        addNode(p);
    }
    return;
}

int main()
{

    Queue q, d;
    int flag, n;
    Time earlytime;
    while (1)
    {
        earlytime = Time(0, 0);
        cout << "请输入操作:(1:开始进程;0:结束进程):" << endl;
        cin >> flag;
        if (flag == 0)
        {
            cout << "\n操作结束!" << endl;
            break;
        }
        else
        {
            cout << "请输入进程数量:" << endl;
            cin >> n;
            cout << "请输入进程的参数:" << endl;
            q.ScanIn(n);
            q.workout(&d, earlytime);
            d.Print();
        }
        q.que.clear();
        d.que.clear();
    }

    return 0;
}