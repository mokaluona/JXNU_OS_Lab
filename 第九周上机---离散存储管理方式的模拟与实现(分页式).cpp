#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>
using namespace std;

class Random
{
public:
    Random() : gen(rd())
    {
        distrib = uniform_int_distribution<>(0, 1);
    }
    int randomBit()
    {
        return distrib(gen);
    }

private:
    random_device rd;                   // 随机设备，用于生成种子
    mt19937 gen;                        // 随机数生成器
    uniform_int_distribution<> distrib; // 随机数分布
};

class Memory
{
public:
    Memory(int size, int ts) : size(size), type_length(ts), freeBlocks(size), blockstatus(size, 0) { init_block(); }
    void allocate(int i);
    void dellocate(int i);
    void printStatus();
    int getFreeBlocks() { return freeBlocks; }
    int getSize() { return size; }
    bool isFree(int i) { return blockstatus[i] == 0; }

private:
    int size;
    int type_length;
    int freeBlocks;
    vector<int> blockstatus;
    Random rd;
    void init_block();
};

void Memory::init_block()
{
    for (auto &i : blockstatus)
    {
        if (rd.randomBit())
        {
            i = 1;
            freeBlocks--;
        }
    }
}

void Memory::printStatus()
{
    cout << "\n主存位视图如下所示范：" << endl;
    int x = 0, y = 0;
    cout << "     ";
    for (int i = 0; i < type_length; i++)
    {
        cout << setw(2) << left << i << " ";
    }
    cout << "\n"
         << setw(2) << right << y << " | ";
    for (auto i : blockstatus)
    {
        cout << setw(2) << left << i << " ";
        x++;
        if (x == type_length)
        {
            cout << "\n"
                 << setw(2) << right
                 << ++y;
            cout << " | ";
            x = 0;
        }
    }
    cout << "\n剩余物理块数:" << freeBlocks << endl;
    cout << endl;
}

void Memory::allocate(int i)
{
    blockstatus[i] = 1;
    freeBlocks--;
}

void Memory::dellocate(int i)
{
    blockstatus[i] = 0;
    freeBlocks++;
}

class Job
{
public:
    Job(int size, string name) : size(size), name(name) {}
    int getSize() { return size; }
    void addPage(int num, int page);
    void printPageTable();
    string getName() { return name; }
    int returnPage(int num) { return pageTable[num]; }

private:
    int size;
    string name;
    map<int, int> pageTable;
};

void Job::addPage(int num, int page)
{
    pageTable[num] = page;
}

void Job::printPageTable()
{
    cout << "------作业" << name << "的页表如下------" << endl;
    for (auto i : pageTable)
    {
        cout << "页号：" << i.first << " 物理块号：" << i.second << endl;
    }
    cout << endl;
}

class JobManager
{
public:
    void addJob(Job j);
    int findJob(string name);
    void delJob(int i);
    vector<Job> JobList;
    void show_list(); // 显示作业列表
};

void JobManager::addJob(Job j)
{
    JobList.push_back(j);
}

int JobManager::findJob(string name)
{
    for (auto it = JobList.begin(); it != JobList.end(); ++it)
    {
        if (it->getName() == name)
        {
            return distance(JobList.begin(), it);
        }
    }
    return -1;
}

void JobManager::delJob(int i)
{
    JobList.erase(JobList.begin() + i);
}

void JobManager::show_list()
{
    cout << "------作业列表如下------" << endl;
    for (auto i : JobList)
    {
        cout << i.getName() << endl;
    }
    cout << endl;
}

class pageTable
{
public:
    Memory memory;
    JobManager jobManager;
    int memory_size;
    int type_length;
    int block_size;
    pageTable(int ms, int tl, int bs) : memory(ms / bs, tl), memory_size(ms), type_length(tl), block_size(bs) { memory.printStatus(); }
    void run();
    bool handleAllocation();
    void handleRecycle();
};

bool pageTable::handleAllocation()
{
    cout << "请输入作业名字：";
    string name;
    cin >> name;
    cout << "请输入作业所需分配大小：";
    int size;
    cin >> size;
    Job j((size + block_size - 1) / block_size, name);
    if (memory.getFreeBlocks() < j.getSize())
    {
        cout << "内存不足" << endl;
        return 0;
    }
    int k = 0;
    for (int i = 0; i < memory.getSize(); i++)
    {
        if (memory.isFree(i))
        {
            k++;
            memory.allocate(i);
            j.addPage(k, i);
            if (k == j.getSize())
            {
                jobManager.addJob(j);
                cout << "分配成功!分配了" << j.getSize() << "个内存块" << endl;
                return 1;
            }
        }
    }
    return 1;
}

void pageTable::handleRecycle()
{
    cout << "请输入作业名字：";
    string name;
    cin >> name;
    int index = jobManager.findJob(name);
    if (index == -1)
    {
        cout << "作业不存在" << endl;
        return;
    }
    for (int i = 0; i < jobManager.JobList[index].getSize(); i++)
    {
        memory.dellocate(jobManager.JobList[index].returnPage(i));
    }
    jobManager.delJob(index);
    cout << "回收成功" << endl;
}

void pageTable::run()
{
    while (true)
    {
        cout << "------分页式管理------" << endl;
        cout << "   1.内存分配\n   2.内存回收\n   0.退出\n";
        int choice;
        bool flag;
        cout << "请输入选项：";
        cin >> choice;
        switch (choice)
        {
        case 1:
        {
            flag = handleAllocation();
            if (flag)
                memory.printStatus();
            jobManager.JobList.back().printPageTable();
            break;
        }
        case 2:
        {
            jobManager.show_list();
            handleRecycle();
            memory.printStatus();
            break;
        }
        case 0:
        {
            return;
        }
        }
    }
}

int main()
{
    int memory_size, type_length, block_size;
    cout << "请输入系统内存空间的大小:";
    cin >> memory_size;
    cout << "请输入字长（16/32/64）：";
    cin >> type_length;
    cout << "请输入物理块大小:";
    cin >> block_size;
    pageTable pt(memory_size, type_length, block_size);
    pt.run();
    return 0;
}