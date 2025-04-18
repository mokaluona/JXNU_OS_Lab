#include <iomanip>
#include <iostream>
#include <string.h>
#include <vector>
using namespace std;

class Time
{
public:
    int hour = 0;
    int minute = 0;
    Time() = default;
    Time(int hour, int minute) : hour(hour), minute(minute) {}
    Time(int sum) : hour(sum / 60), minute(sum % 60) {}
    bool compare(const Time &other) const;
    int cal_sum();
};

// 函数名：Time::compare()
bool Time::compare(const Time &other) const
{ // t<o(时间没到) false    t >= o true
    // 函数功能：比较进程到达时间和当前时间，若小于则返回false，否则返回true
    if (hour != other.hour)
        return hour >= other.hour;
    return minute >= other.minute;
}

int Time::cal_sum()
{
    return hour * 60 + minute;
}

class Node
{
public:
    int id;      // 进程编号
    string name; // 进程名
    int good;    // 优先级
    Time arrive; // 到达就绪队列的时间
    int zx;      // 执行时间
    Time start;  // 开始执行时间
    Time finish; // 执行完成时间
    int zz;      // 周转时间=执行完成时间-到达就绪队列时间
    float zzxs;  // 带权周转时间=周转时间/执行时间
    Node(int id, string name, int good, Time arrive = Time(), int zx = 0, Time start = Time(), Time finish = Time(), int zz = 0, float zzxs = 0.0f) : id(id), name(name), good(good), arrive(arrive), zx(zx), start(start), finish(finish), zz(zz), zzxs(zzxs) {}
    bool Levelcompare(const Node &other) const;
    void ComputeTime(Time *now);
};

// 函数名：Levelcompare()
bool Node::Levelcompare(const Node &other) const
{
    // 函数功能：比较t,o的优先级，t的优先级高则返回true，低则返回false，否则比较到达时间，t先或同时到达则返回true，反之则false
    if (this->good != other.good)
        return this->good < other.good;
    return this->arrive.compare(other.arrive);
}

// 函数名：ComputeTime()
void Node::ComputeTime(Time *now)
{
    // 函数功能：更新当前时间和进程的各项时间
    if (now->compare(arrive))
    {
        start = *now;
    }
    else
    {
        start = arrive;
    }
    finish = Time(start.cal_sum() + zx);
    zz = finish.cal_sum() - arrive.cal_sum();
    zzxs = static_cast<float>(zz) / zx;
    *now = finish;
}

class Queue
{
public:
    vector<Node> que;
    void addNode(Node p);
    void priority(Queue *d, Time *now);
    void Print();
    Time ScanIn(int n);
    bool reflesh_ready(Queue *ready, const Time &now);
    Time find_first();
};

// 函数名： addNode()
void Queue::addNode(Node p)
{
    auto it = que.begin();
    while (it != que.end() && p.Levelcompare(*it))
    {
        ++it;
    }
    que.insert(it, p); // 插入到该位置前
}

Time Queue::find_first()
{
    Time t(23, 59);
    for (auto &it : que)
    {
        if (t.compare(it.arrive))
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
        bool falg = now.compare(it->arrive);
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

// 函数名：priority()    参数:now当前时间
void Queue::priority(Queue *d, Time *now)
{
    // 函数功能：进行优先级进程调度，并同时更新当前时间。
    Queue ready;
    bool flag = true;
    while (flag)
    {
        flag = reflesh_ready(&ready, *now);
        if (ready.que.empty())
        {
            *now = find_first(); // 就绪队列为空则跳到下一个最近的进程就绪时间
            continue;
        }
        ready.que.begin()->ComputeTime(now);
        d->que.push_back(*ready.que.begin());
        ready.que.erase(ready.que.begin()); // 弹出已经执行好的元素
    }
    while (!ready.que.empty())
    {
        ready.que.begin()->ComputeTime(now);
        d->que.push_back(*ready.que.begin());
        ready.que.erase(ready.que.begin());
    }
}

void print_value(Time a)
{
    cout << "   " << setfill('0') << setw(2) << a.hour << ":"
         << setw(2) << a.minute << "     ";
}
void print_value(int a) { cout << a << "     "; }
void print_value(float a) { cout << a << "    "; }
void print_value(string a) { cout << a << "     "; }

// 函数名：Print()    参数:que进程队列指针, n 进程数
void Queue::Print()
{
    // 函数功能：打印输出进程优先进程调度结果
    cout << "模拟进程有线调度过程输出结果" << endl;
    cout << "id号    名字   优先级   到达时间   执行时间（分钟）   开始时间   完成时间  周转时间（分钟）  带权周转系数" << endl;
    float arrzz = 0, arrzzxs = 0;
    for (auto it = que.begin(); it != que.end(); ++it)
    {
        print_value(it->id);
        print_value(it->name);
        print_value(it->good);
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
Time Queue::ScanIn(int n)
{
    // 函数功能：输入进程信息，返回最早的进程到达时间
    int id, good, a1, a2, zx;
    char c;
    string name;
    Time earlytime;
    while (n--)
    {
        cin >> id >> name >> good >> a1 >> c >> a2 >> zx;
        Time arrive(a1, a2);
        Node p(id, name, good, arrive, zx);
        addNode(p);
        if (earlytime.compare(arrive))
        {
            earlytime = arrive;
        }
    }
    return earlytime;
}

int main()
{

    Queue q, d;
    int flag, n;
    Time earlytime;

    while (1)
    {
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
            earlytime = q.ScanIn(n);
            q.priority(&d, &earlytime);
            d.Print();
        }
        q.que.clear();
        d.que.clear();
    }

    return 0;
}
