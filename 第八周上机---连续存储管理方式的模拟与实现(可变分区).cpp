#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <string>
#include <vector>
using namespace std;

class Partition
{
public:
    Partition(int size, int address, int number) : id(number), size(size), address(address) {}
    Partition(int size, int address, string name, int number) : id(number), size(size), address(address), job(name)
    {
        status = 1;
    } // 添加新的已占用分区的构造函数
    int getID() { return id; }
    int getSize() { return size; }
    int getAddress() { return address; }
    bool getStatus() { return status; }
    string getJOB() { return job; }
    void apart(int s)
    {
        size -= s;
        address += s;
    }
    void conbin(int s)
    {
        size += s;
    }
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
         << setw(14) << left << (status ? job : "空闲") << endl;
}

class MemoryManager
{
public:
    MemoryManager(int total_memory, int start) { free.emplace_back(total_memory, start, min_id(cnt)); } // 初始化内存，整个空间作为空闲分区
    bool first_fit(string name, int size);                                                              // 首次分配
    bool best_fit(string name, int size);                                                               // 最佳分配
    bool worst_fit(string name, int size);                                                              // 最坏分配
    void allocate_resourse(list<Partition>::iterator it, int size, string name);
    bool deallocate();       // 回收资源
    void print_partitions(); // 打印分区信息
    void combain();          // 合并相邻空闲分区
    //     bool done();                        // 判断是否所有作业都完成
private:
    list<Partition> allocated; // 已分配分区表
    list<Partition> free;      // 未分配分区表（按地址排序）
    int min_id(int cnt);
    int cnt = 1;
};

void MemoryManager::allocate_resourse(list<Partition>::iterator it, int size, string name)
{
    allocated.emplace_back(size, it->getAddress(), name, min_id(cnt));
    cnt++;
    it->apart(size);
    if (it->getSize() == 0)
        free.erase(it);
}

bool MemoryManager::first_fit(string name, int size)
{
    for (auto it = free.begin(); it != free.end(); ++it)
    {
        if (it->getSize() >= size)
        {
            allocate_resourse(it, size, name);
            return true;
        }
    }
    return false;
}

bool MemoryManager::best_fit(string name, int size)
{
    auto best_it = free.end();
    for (auto it = free.begin(); it != free.end(); ++it)
    {
        if (it->getSize() >= size && (best_it == free.end() || it->getSize() < best_it->getSize()))
            best_it = it;
    }
    if (best_it != free.end())
    {
        allocate_resourse(best_it, size, name);
        return true;
    }
    else
    {
        return false;
    }
}

bool MemoryManager::worst_fit(string name, int size)
{
    auto worst_it = free.end();
    for (auto it = free.begin(); it != free.end(); ++it)
    {
        if (it->getSize() >= size && (worst_it == free.end() || it->getSize() > worst_it->getSize()))
            worst_it = it;
    }
    if (worst_it != free.end())
    {
        allocate_resourse(worst_it, size, name);
        return true;
    }
    else
    {
        return false;
    }
}

int MemoryManager::min_id(int cnt)
{
    vector<int> ids(cnt + 1, 0);
    for (auto it : free)
    {
        ids[it.getID()] = 1;
    }
    for (auto it : allocated)
    {
        ids[it.getID()] = 1;
    }
    for (int i = 1; i <= cnt; i++)
    {
        if (!ids[i])
            return i;
    }
    return cnt + 1;
}

bool MemoryManager::deallocate()
{
    string name;
    cout << "输入要回收的作业名称：";
    cin >> name;
    for (auto it = allocated.begin(); it != allocated.end(); ++it)
    {
        if (it->getJOB() == name)
        {
            int start = it->getAddress();
            int size = it->getSize();
            int id = it->getID();
            allocated.erase(it);
            auto insert_pos = free.begin();
            while (insert_pos != free.end() && insert_pos->getAddress() < start)
            {
                ++insert_pos;
            }
            free.emplace(insert_pos, size, start, id); // 按地址升序插入
            combain();
            return true;
        }
    }
    return false;
}

void MemoryManager::combain()
{
    auto it = free.begin();
    while (it != free.end())
    {
        auto next = it;
        ++next;
        if (next != free.end() && it->getAddress() + it->getSize() == next->getAddress())
        {
            it->conbin(next->getSize());
            free.erase(next);
        }
        else
        {
            ++it;
        }
    }
}

void MemoryManager::print_partitions()
{
    cout << "------主存分配情况------" << endl;
    if (allocated.size() != 0)
    {
        cout << "已分配:" << endl;
        cout << "分区号    大小(KB)    起始(KB)    状态" << endl;
        for (auto it : allocated)
        {
            it.print();
        }
    }
    if (free.size() != 0)
    {
        cout << "未分配：" << endl;
        cout << "分区号    大小(KB)    起始(KB)    状态" << endl;
        for (auto it : free)
        {
            it.print();
        }
    }
}

class ManageUI
{
public:
    MemoryManager manager; // 组合内存管理核心对象
    ManageUI(int total_memory, int start) : manager(total_memory, start) {}
    bool show_main_menu(); // 主管理菜单

private:
    void select_allocation_algorithm(); // 选择分配算法子菜单
};

bool ManageUI::show_main_menu()
{
    cout << "------可变分区管理------" << endl;
    cout << "1. 分配内存" << endl;
    cout << "2. 回收内存" << endl;
    cout << "0. 退出" << endl;
    cout << "请选择操作：";
    int choice;
    bool flag = 0;
    cin >> choice;
    switch (choice)
    {
    case 1:
        select_allocation_algorithm();
        break;
    case 2:
        flag = manager.deallocate();
        if (!flag)
            cout << "该作业不存在，请重新输入";
        break;
    case 0:
        return 0;
    default:
        cout << "输入错误，请重新输入" << endl;
        break;
    }
    return 1;
}

void ManageUI::select_allocation_algorithm()
{
    cout << "1. 首次分配算法" << endl;
    cout << "2. 最优分配算法" << endl;
    cout << "3. 最坏分配算法" << endl;
    cout << "0. 退出" << endl;
    int choice, size;
    string name;
    bool flag;
    cout << "请选择算法：";
    if (!(cin >> choice))
    { // 输入非数字时处理
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "输入无效，请重新选择！" << endl;
        return;
    }
    cout << "请输入作业名：";
    cin >> name;
    cout << name << "需要分配的主存大小为(单位:KB):";
    cin >> size;
    switch (choice)
    {
    case 1:
        flag = manager.first_fit(name, size);
        break;
    case 2:
        flag = manager.best_fit(name, size);
        break;
    case 3:
        flag = manager.worst_fit(name, size);
        break;
    }
    if (!flag)
    {
        cout << "分配失败！";
    }
}

int main()
{
    int memorySize, startAddress;
    cout << "请输入内存大小：" << endl;
    cin >> memorySize;
    cout << "请输入起始地址：" << endl;
    cin >> startAddress;
    ManageUI manager(memorySize, startAddress);
    while (manager.show_main_menu())
    {
        manager.manager.print_partitions();
    }
    return 0;
}
