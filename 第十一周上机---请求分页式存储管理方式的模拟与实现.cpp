#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class Record
{
private:
    vector<vector<int>> page_frame_seq;
    vector<int> page_que;
    vector<bool> if_fault;
    vector<int> page_throw;
    int page_num;
    int frame_num;

public:
    Record(int page_num, int frame_num, const vector<int> &queue) : page_frame_seq(frame_num, vector<int>(page_num, -1)), if_fault(page_num, false), page_throw(page_num, -1), page_num(page_num), frame_num(frame_num) { page_que = queue; }
    void per_page_record(int page, queue<int> que);
    void per_page_record(int page, const list<int> &que);
    void fault_record(int page);
    void throw_record(int page, int i);
    void print() const;
};

void Record::per_page_record(int page, queue<int> que)
{
    int j = 0;
    while (!que.empty())
    {
        int x = que.front();
        page_frame_seq[j++][page] = x;
        que.pop();
    }
}

void Record::per_page_record(int page, const list<int> &que)
{
    int j = 0;
    for (auto i : que)
    {
        page_frame_seq[j++][page] = i;
    }
}

void Record::fault_record(int page)
{
    if_fault[page] = true;
}

void Record::throw_record(int page, int i)
{
    page_throw[i] = page;
}

void Record::print() const
{
    cout << "    ";
    for (int i = 0; i < page_num; i++)
    {
        cout << setw(4) << i;
    }
    cout << endl;
    cout << "    ";
    for (auto i : page_que)
    {
        cout << setw(4) << i;
    }
    cout << endl;
    for (int i = 0; i < frame_num; i++)
    {
        cout << setw(4) << i;
        for (auto k : page_frame_seq[i])
        {
            cout << setw(4) << (k == -1 ? " " : to_string(k));
        }
        cout << endl;
    }
    cout << setw(4);
    for (auto i : if_fault)
    {
        cout << setw(4) << (i ? '+' : ' ');
    }
    cout << endl;
    cout << "    ";
    for (auto i : page_throw)
    {
        cout << setw(4) << (i == -1 ? " " : to_string(i));
    }
    cout << endl;
}

class PageReplacer
{
protected:
    unsigned int frame_num; // 块数
    vector<int> page_seq;   // 页面序列
    string job_name;
    int page_faults; // 缺页中断次数
public:
    PageReplacer(int count, const vector<int> &queue, string name) : frame_num(count), page_seq(queue), job_name(name), page_faults(0) {}
    virtual ~PageReplacer() = default;
    virtual float get_fault_rate() const { return static_cast<float>(page_faults) / page_seq.size(); }
    virtual void simulate() = 0;
    virtual void print() const = 0;
};
void PageReplacer::print() const
{
    cout << "\n缺页中断率为：" << fixed << setprecision(2)
         << get_fault_rate() * 100 << "%"
         << "  修改次数：" << page_faults << endl;
}

class FIFO : public PageReplacer
{
private:
    queue<int> frame_queue;
    unordered_map<int, bool> in_memory;

public:
    using PageReplacer::PageReplacer;
    void simulate() override;
    void print() const override;
};

void FIFO::simulate()
{
    int n = page_seq.size();
    Record r(n, frame_num, page_seq);
    for (int i = 0; i < n; i++)
    {
        auto page = page_seq[i];
        if (!in_memory[page])
        {
            ++page_faults;
            r.fault_record(i);
            frame_queue.push(page);
            in_memory[page] = true;
            if (frame_queue.size() > frame_num)
            {
                auto temp = frame_queue.front();
                frame_queue.pop();
                in_memory[temp] = false;
                r.throw_record(temp, i);
            }
        }
        r.per_page_record(i, frame_queue);
    }
    r.print();
}

void FIFO::print() const
{
    cout << job_name << "的FIFO调度结果:\n";
    cout << "缺页中断次数: " << page_faults
         << "\n缺页中断率为： " << get_fault_rate() << "\n\n";
}

class LFU : public PageReplacer
{
private:
    list<int> lru_list;
    unordered_map<int, list<int>::iterator> page_map;
    void reflesh(int page); // 更新页面
public:
    using PageReplacer::PageReplacer;
    void simulate() override;
    void print() const override;
};

void LFU::reflesh(int page)
{
    if (page_map.find(page) != page_map.end())
    {
        lru_list.erase(page_map[page]);
    }
    lru_list.push_front(page);
    page_map[page] = lru_list.begin();
}

void LFU::simulate()
{
    int n = page_seq.size();
    Record r(n, frame_num, page_seq);
    for (int i = 0; i < n; i++)
    {
        auto page = page_seq[i];
        if (page_map.find(page) == page_map.end())
        {
            ++page_faults;
            r.fault_record(i);
            if (lru_list.size() >= frame_num)
            {
                auto temp = lru_list.back();
                lru_list.pop_back();
                page_map.erase(temp);
                r.throw_record(temp, i);
            }
        }
        reflesh(page);
        r.per_page_record(i, lru_list);
    }
}

void LFU::print() const
{
    cout << job_name << "的LFU调度结果:\n";
    cout << "缺页中断次数: " << page_faults
         << "\n缺页中断率为： " << get_fault_rate() << "\n"
         << endl;
}

unique_ptr<PageReplacer> create_replacer(int type, int frames, const vector<int> &seq, std::string name)
{
    if (type == 1)
    {
        return make_unique<FIFO>(frames, seq, move(name));
    }
    return make_unique<LFU>(frames, seq, move(name));
}
void print_menu()
{
    cout << "------请求分页式存储管理------\n"
         << "1. FIFO\n"
         << "2. LFU\n"
         << "0. 退出\n"
         << "请输入选项：";
}

int main()
{
    int frame_num, type, page_len;
    string job_name;
    vector<int> page_seq;
    cout << "请输入需要的物理块数：";
    cin >> frame_num;
    while (true)
    {
        print_menu();
        cin >> type;
        if (type == 1 || type == 2)
        {
            cout << "请输入作业名：";
            cin >> job_name;
            cout << "请输入作业页面长度：";
            cin >> page_len;
            page_seq.resize(page_len);
            cout << "请输入作业页面顺序：";
            for (auto &page : page_seq)
            {
                cin >> page;
            }
            auto replacer = create_replacer(type, frame_num, page_seq, job_name);
            replacer->simulate();
            replacer->print();
        }
        else if (type == 0)
        {
            break;
        }
        else
        {
            cout << "输入错误，请重新输入" << endl;
        }
    }
}
