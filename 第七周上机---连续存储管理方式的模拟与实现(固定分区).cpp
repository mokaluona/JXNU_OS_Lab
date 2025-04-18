#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Partition
{
public:
    Partition(int id, int size, int address) : id(id), size(size), address(address) {}
    Partition() : id(-1), size(-1), address(-1) {} // 默认构造函数
    int getID() { return id; }
    int getSize() { return size; }
    int getAddress() { return address; }
    bool getStatus() { return status; }
    void allocate(int i)
    {
        status = 1;
        job = "JOB" + to_string(i);
    } // 设置分区状态为占用
    void deallocate() { status = 0; } // 设置分区状态为未占用
    void print();                     // 打印分区信息
private:
    int id;
    int size;
    int address;
    bool status = 0; // 默认未占用，占用为1
    string job = ""; // 如被使用，记录作业名
};

void Partition::print()
{
    cout << "  " << setw(9) << left << id
         << setw(11) << left << size
         << setw(11) << left << address
         << setw(13) << left << (status ? job : "未占用") << endl;
}

class Work
{
public:
    Work(int id, int size) : id(id), size(size) {}
    Work() : id(0), size(0) {}
    int getID() { return id; }
    int getSize() { return size; }

private:
    int id;
    int size;
};

class Manager
{
public:
    vector<Partition> partitions;
    Manager(int n);
    bool allocated_resources(Work job); // 分配资源
    bool deallocate_resources(int id);  // 回收资源
    void print_partitions();            // 打印分区信息
    bool done();                        // 判断是否所有作业都完成
};

Manager::Manager(int n) : partitions(n)
{
    cout << "请依次输入：\n分区号 大小 起始" << endl;
    for (int _ = 0; _ < n; _++)
    {
        int id, size, address;
        cin >> id >> size >> address;
        partitions[_] = Partition(id, size, address);
    }
}

bool Manager::allocated_resources(Work job)
{
    for (auto &part : partitions)
    {
        if (job.getSize() <= part.getSize() && part.getStatus() == 0)
        {
            part.allocate(job.getID());
            return true;
        }
    }
    return false;
}

bool Manager::deallocate_resources(int id)
{
    if (partitions[id].getStatus() == 0)
    {
        cout << "分区" << id << "未被占用" << endl;
        return false;
    }
    partitions[id].deallocate();
    return true;
}

void print_area_begin()
{
    cout << "\n-----打印区分区信息-----" << endl;
}

void Manager::print_partitions()
{
    print_area_begin();
    cout << "分区号    大小(KB)    起始(KB)    状态" << endl;
    for (auto &partition : partitions)
    {
        partition.print();
    }
}

bool Manager::done()
{
    for (auto &part : partitions)
    {
        if (part.getStatus() == 0)
            return false;
    }
    return true;
}

void getIn(vector<Work> &a)
{
    int x, n = a.size();
    cout << "请输入这" << n << "个作业的信息";
    for (int i = 0; i < n; i++)
    {
        cout << "请输入第" << i + 1 << "个分区的大小:";
        cin >> x;
        a[i] = Work(i + 1, x);
    }
}

void print_job(vector<Work> a)
{
    cout << "\n打印各作业信息:" << endl;
    cout << "作业名 作业大小" << endl;
    for (auto i : a)
        cout << "JOB" << i.getID() << "   " << i.getSize() << "KB" << endl;
}

void workout(vector<Work> &jobs, Manager &memory)
{
    bool flag;
    for (auto &i : jobs)
    {
        flag = memory.allocated_resources(i);
        if (!flag)
        {
            cout << "作业" << i.getID() << "无法分配" << endl;
            return;
        }
    }
}

int main()
{
    int n, m;
    cout << "请输入系统的分区块数：" << endl;
    cin >> n;
    Manager memory(n);
    memory.print_partitions();
    cout << "请输入作业的个数：" << endl;
    cin >> m;
    vector<Work> jobs(m);
    getIn(jobs);
    print_job(jobs);
    workout(jobs, memory);
    memory.print_partitions();
    int x;
    char c;
    while (!memory.done())
    {
        cout << "是否需要回收(y/n)?";
        cin >> c;
        if (c == 'n')
            break;
        else
        {
            cout << "请输入回收的作业名：";
            cin >> x;
            memory.deallocate_resources(x - 1);
            memory.print_partitions();
        }
    }
    return 0;
}
