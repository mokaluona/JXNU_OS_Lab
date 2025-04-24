#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class MemoryBlock
{
public:
    MemoryBlock(int id, int size, int addr) : id(id), size(size), address(addr), status(false), name("") {}
    int getid() const { return id; }
    int getsize() const { return size; }
    bool getflag() const { return status; }
    string getname() const { return name; }
    int getaddress() const { return address; }
    void changeSize(int newSize) { size = newSize; }
    void allocate(string name)
    {
        this->name = name;
        status = 1;
    }
    void deallocate() { status = 0; }
    void print() const;

private:
    int id;
    int size;
    int address;
    bool status;
    string name;
};

void MemoryBlock::print() const
{
    cout << setw(9) << left << id
         << setw(16) << left << size
         << setw(15) << left << address
         << setw(10) << left << (status ? name : "空闲") << endl;
}

class SegmentTable
{
private:
    unordered_map<string, list<pair<int, int>>> segments; // 作业名->[段号,分区号]
public:
    void addSegment(const string &job, int segID, int blockId)
    {
        segments[job].emplace_back(segID, blockId);
    }
    void removeJob(const string &job)
    {
        segments.erase(job);
    }
    const auto &getSegments(const string &job) const
    {
        return segments.at(job);
    }
};

class MemoryManager
{
private:
    list<MemoryBlock> freeList;      // 空闲分区表
    list<MemoryBlock> allocatedList; // 已分配分区表
    SegmentTable segmentTable;
    int nextBId = 1; // 分区号计数器
    int totalSize;   // 总内存大小
    static bool compareByAddr(const MemoryBlock &a, const MemoryBlock &b)
    {
        return a.getaddress() < b.getaddress(); // 升序排列
    }

public:
    explicit MemoryManager(int total, int start) : totalSize(total), freeSize(total)
    {
        freeList.emplace_back(nextBId++, total, start);
    }
    int freeSize; // 剩余内存大小
    bool allocate(const string &job, int i, const int &size, int segID);
    bool deallocate(const string &job);
    void printMemory() const;
    void printSegmentTable(string name) const;
    const MemoryBlock &getTheBlock(int id) const; // 获取指定ID的分区
    void combine(list<MemoryBlock>::iterator it);
};

bool MemoryManager::allocate(const string &job, int i, const int &size, int segID)
{
    for (auto it = freeList.begin(); it != freeList.end(); ++it)
    {
        if (it->getsize() >= size)
        {
            freeSize -= size;
            if (it->getsize() != size)
            {
                freeList.emplace(it, nextBId++, it->getsize() - size, it->getaddress() + size);
                it->changeSize(size);
            }
            it->allocate(job + to_string(i));
            allocatedList.splice(allocatedList.end(), freeList, it);
            segmentTable.addSegment(job, segID, it->getid());
            return true;
        }
    }
    return false;
}

bool MemoryManager::deallocate(const string &job)
{
    auto it = segmentTable.getSegments(job);
    for (const auto &[segID, blockId] : it)
    {
        auto it2 = find_if(allocatedList.begin(), allocatedList.end(), [blockId](const MemoryBlock &blk)
                           { return blk.getid() == blockId; });
        if (it2 == allocatedList.end())
        {
            throw std::runtime_error("分区号 " + to_string(blockId) + " 不存在！");
        }
        freeSize += it2->getsize();
        segmentTable.removeJob(job);
        it2->deallocate();
        freeList.splice(freeList.end(), allocatedList, it2);
        cout << "作业" << job << "的段" << job << segID << "回收成功" << endl;
        freeList.sort(compareByAddr);
        combine(it2);
    }
    return 1;
}

void MemoryManager::combine(list<MemoryBlock>::iterator currentIt)
{
    bool merged = false;

    // 前向合并
    if (currentIt != freeList.begin())
    {
        auto prevIt = prev(currentIt);
        if (prevIt->getaddress() + prevIt->getsize() == currentIt->getaddress())
        {
            prevIt->changeSize(prevIt->getsize() + currentIt->getsize());
            freeList.erase(currentIt);
            currentIt = prevIt;
            merged = true;
        }
    }

    // 后向合并
    auto nextIt = next(currentIt);
    if (nextIt != freeList.end() &&
        currentIt->getaddress() + currentIt->getsize() == nextIt->getaddress())
    {
        currentIt->changeSize(currentIt->getsize() + nextIt->getsize());
        freeList.erase(nextIt);
        merged = true;
    }

    // 若发生合并，递归检查新的相邻块
    if (merged)
    {
        combine(currentIt);
    }
}

void MemoryManager::printMemory() const
{
    cout << "\n-------主存分配表-------" << endl;
    cout << "已分配：" << endl;
    ;
    cout << "ID\t大小(KB)\t起始(KB)\t状态" << endl;
    for (const auto &blk : allocatedList)
    {
        blk.print();
    }
    cout << "未分配:" << endl;
    for (const auto &blk : freeList)
    {
        blk.print();
    }
}

void MemoryManager::printSegmentTable(string name) const
{
    cout << "-------打印" << name << "的段表-------" << endl;
    cout << "段号\t段长\t基址" << endl;
    try
    {
        const auto it = segmentTable.getSegments(name);
        for (const auto &[segID, blockId] : it)
        {
            const MemoryBlock &bl = getTheBlock(blockId);
            cout << setw(9) << left << segID
                 << setw(8) << left << bl.getsize()
                 << setw(9) << left << bl.getaddress() << endl;
        }
    }
    catch (const std::out_of_range &)
    {
        // 捕获作业不存在的异常
        cout << "错误：作业" << name << "不存在！" << endl;
    }
}

const MemoryBlock &MemoryManager::getTheBlock(int id) const
{
    for (const auto &blk : allocatedList)
    {
        if (blk.getid() == id)
        {
            return blk;
        }
    }
    for (const auto &blk : freeList)
    {
        if (blk.getid() == id)
        {
            return blk;
        }
    }
    throw std::runtime_error("分区号 " + to_string(id) + " 不存在！");
}

class Handle
{
public:
    MemoryManager mm;
    Handle(int total, int start) : mm(total, start) {}
    void handleAllocate();   // 内存分配
    void handleDeallocate(); // 内存去配
};

void Handle::handleAllocate()
{
    int segID = 1;
    string name;
    cout << "请输入作业名：";
    cin >> name;
    int size;
    cout << "请输入" << name << "需要分配的主存大小：";
    cin >> size;
    if (size > mm.freeSize)
    {
        cout << "内存不足，无法分配" << endl;
        return;
    }
    cout << "请输入要分成几段：";
    int segNum, segSize;
    cin >> segNum;
    for (int i = 0; i < segNum; i++)
    {
        if (i != segNum - 1)
        {
            cout << "剩余" << size << "KB的内存，请输入第" << i + 1 << "段的大小：";
            cin >> segSize;
            while (segSize > size)
            {
                cout << "段的大小不能大于作业的大小，请重新输入：";
                cin >> segSize;
            }
        }
        else
        {
            cout << "剩余" << size << "KB的内存，请输入最后一段的大小：";
        }
        size -= segSize;
        bool flag = mm.allocate(name, i + 1, segSize, segID++);
        if (!flag)
        {
            cout << "内存分配失败" << endl;
            return;
        }
        mm.printMemory();
        if (size == 0)
        {
            cout << "内存已分配完毕";
            mm.printSegmentTable(name);
            return;
        }
    }
}

void Handle::handleDeallocate()
{
    mm.printMemory();
    cout << "请输入要释放的作业名：";
    string name;
    cin >> name;
    mm.deallocate(name);
}

void showMenu()
{
    cout << "------可变分区管理------"
         << "\n\t1.内存分配\n"
         << "\t2. 内存去配\n"
         << "\t0.退出\n"
         << "请输入选择：";
}

int main()
{
    int total, start, choice;
    cout << "请输入内存大小：";
    cin >> total;
    cout << "请输入起始地址：";
    cin >> start;
    Handle h(total, start);
    while (true)
    {
        showMenu();
        cin >> choice;
        switch (choice)
        {
        case 1:
            h.handleAllocate();
            break;
        case 2:
            h.handleDeallocate();
            h.mm.printMemory();
            break;
        case 0:
            return 0;
        default:
            cout << "输入错误，请重新输入" << endl;
        }
    }
}