#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
using namespace std;

class DiskSol
{
protected:
    int curp;
    vector<int> que;
    vector<int> result;
    int moved = 0;

public:
    virtual ~DiskSol() = default;
    explicit DiskSol(int pos, vector<int> req) : curp(pos), que(move(req)) {}
    virtual void solve() = 0;
    void cul_moved();
    void print_result() const;
    virtual void print() const = 0;
};

void DiskSol::cul_moved()
{
    moved = 0;
    int prev = curp;
    for (auto i : result)
    {
        moved += abs(prev - i);
        prev = i;
    }
}

void DiskSol::print_result() const
{
    cout << "顺序为：";
    for (auto i : result)
        cout << i << " ";
    cout << "\n移动距离为：" << moved << endl;
}

// 1. FCFS：按照磁道访问顺序来对磁臂进行驱动读取数据
class FCFS : public DiskSol
{
public:
    using DiskSol::DiskSol;
    void print() const override
    {
        cout << "顺序移动调度的";
        print_result();
    }
    void solve() override
    {
        result = que;
        cul_moved();
        print();
    }
};

// 2. SSTF：每次将磁臂驱动到与当前磁臂位置最近的一个磁道
class SSTF : public DiskSol
{
public:
    using DiskSol::DiskSol;
    void print() const override
    {
        cout << "最短寻道时间优先调度的";
        print_result();
    }
    void solve() override
    {
        vector<int> temp = que;
        int current = curp;
        result.clear();
        while (!temp.empty())
        {
            auto closest = min_element(temp.begin(), temp.end(),
                                       [current](int a, int b)
                                       {
                                           return abs(a - current) < abs(b - current);
                                       });
            result.push_back(*closest);
            current = *closest;
            temp.erase(closest);
        }
        cul_moved();
        print();
    }
};

/*3.电梯调度算法(Elevator_Scheduler),SCAN：
    每次向一个方向移动，直至该方向上没有需要访问的磁道，
    然后如果反方向有需要访问的磁道则反方向移动，否则停止，以此往复*/
class SCAN : public DiskSol
{
public:
    SCAN(int pos, vector<int> req, bool dir = true)
        : DiskSol(pos, req) {}

    void print() const override
    {
        cout << "梯调度(由里向外)的";
        print_result();
    }
    void solve() override
    {
        vector<int> temp = que;
        sort(temp.begin(), temp.end());
        auto it = partition(temp.begin(), temp.end(),
                            [this](int x)
                            { return x >= curp; });
        auto t = vector<int>(it, temp.end());
        result = vector<int>(temp.begin(), it);
        sort(result.begin(), result.end(), [](int a, int b)
             { return a < b; });
        sort(t.begin(), t.end(), [](int a, int b)
             { return a < b; });
        result.insert(result.begin(), t.rbegin(), t.rend());
        cul_moved();
        print();
    }
};

// 4. 单向扫描（C-SCAN）
class C_SCAN : public DiskSol
{
public:
    using DiskSol::DiskSol;
    void print() const override
    {
        cout << "单向扫描调度的";
        print_result();
    }
    void solve() override
    {
        sort(que.begin(), que.end());
        auto it = partition(que.begin(), que.end(), [this](int x)
                            { return x >= curp; });
        result = vector<int>(que.begin(), it);
        sort(result.begin(), result.end());
        auto temp = vector<int>(it, que.end());
        sort(temp.begin(), temp.end());
        result.insert(result.end(), temp.begin(), temp.end());
        cul_moved();
        print_result();
    }
};

/*5. 双向扫描：（LOOK）
每次向一个方向移动，不管该方向上有没有需要访问的磁道，
都继续移动，直至最后反向，以此往复。*/
class Look : public DiskSol
{
public:
    using DiskSol::DiskSol;
    void print() const override
    {
        cout << "双向扫描调度的";
        print_result();
    }
    void solve() override
    {
        sort(que.begin(), que.end());
        auto it = partition(que.begin(), que.end(), [this](int x)
                            { return x >= curp; });
        result = vector<int>(que.begin(), it);
        sort(result.begin(), result.end(), [](int a, int b)
             { return a < b; });
        auto temp = vector<int>(it, que.end());
        sort(temp.begin(), temp.end());
        result.insert(result.end(), temp.rbegin(), temp.rend());
        cul_moved();
        print_result();
    }
};

class memu
{
public:
    void mains();
    void show();
    unique_ptr<DiskSol> create_scheduler(int type, int pos, vector<int> req);
    tuple<int, vector<int>> get_input();
};

void memu::show()
{
    cout << "\n------ 磁盘调度算法 ------\n"
         << "1. 顺序移动FCFS\n"
         << "2. 最短寻道时间SSTF\n"
         << "3. 电梯调度SCAN\n"
         << "4. 单向扫描C-SCAN\n"
         << "5. 双向扫描LOOK\n"
         << "0. 退出\n"
         << "请输入选择：";
}

tuple<int, vector<int>> memu::get_input()
{
    int len, pos;
    cout << "输入请求数量：";
    cin >> len;
    vector<int> req(len);
    cout << "输入磁道序列（空格分隔）：";
    for (auto &r : req)
        cin >> r;
    cout << "输入初始磁头位置：";
    cin >> pos;
    return {pos, req};
}

unique_ptr<DiskSol> memu::create_scheduler(int type, int pos, vector<int> req)
{
    switch (type)
    {
    case 1:
        return make_unique<FCFS>(pos, req);
    case 2:
        return make_unique<SSTF>(pos, req);
    case 3:
        return make_unique<SCAN>(pos, req);
    case 4:
        return make_unique<C_SCAN>(pos, req);
    case 5:
        return make_unique<Look>(pos, req);
    default:
        throw invalid_argument("无效选择");
    }
}

void memu::mains()
{
    while (true)
    {
        show();
        int choice;
        cin >> choice;
        if (choice == 0)
            break;
        auto [pos, req] = get_input();
        try
        {
            auto scheduler = create_scheduler(choice, pos, req);
            scheduler->solve();
        }
        catch (const exception &e)
        {
            cerr << "输入错误：" << e.what() << endl;
        }
    }
}

int main()
{
    memu m;
    m.mains();
    return 0;
}