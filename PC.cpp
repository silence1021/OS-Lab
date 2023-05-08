#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

static const int repository_size = 10; // 循环队列的大小
static const int item_total = 20;      // 要生产的产品数目

std::mutex mtx; // 互斥量，保护产品缓冲区
std::mutex producer_count_mtx;
std::mutex consumer_count_mtx;

std::condition_variable repo_not_full;  // 条件变量指示产品缓冲区不满
std::condition_variable repo_not_empty; // 条件变量指示产品缓冲区不为空，就是缓冲区有产品

int item_buffer[repository_size]; // 产品缓冲区，这里使用了一个循环队列

static std::size_t read_position = 0;  // 消费者读取产品的位置
static std::size_t write_position = 0; // 生产者写入产品的位置

static size_t produced_item_counter = 0;
static size_t consumed_item_counter = 0;

std::chrono::seconds t(1); // a  new feature of c++ 11 standard
std::chrono::microseconds t1(1000);

void produce_item(int i)
{
    std::unique_lock<std::mutex> lck(mtx);
    // item buffer is full, just wait here.
    while (((write_position + 1) % repository_size) == read_position)
    {
        std::cout << "Producer is waiting for an empty slot..." << std::endl;
        repo_not_full.wait(lck); // 生产者等待"产品库缓冲区不为满"这一条件发生.
    }                            // 当缓冲区满了之后我们就不能添加产品了

    item_buffer[write_position] = i; // 写入产品
    write_position++;

    if (write_position == repository_size) // 写入的位置如果在队列最后则重新设置
    {
        write_position = 0;
    }

    repo_not_empty.notify_all(); // 通知消费者产品库不为空

    lck.unlock(); // 解锁
}

int consume_item()
{
    int data;
    std::unique_lock<std::mutex> lck(mtx);
    // item buffer is empty, just wait here.
    while (write_position == read_position)
    {
        std::cout << "Consumer is waiting for items..." << std::endl;
        repo_not_empty.wait(lck); // 消费者等待"产品库缓冲区不为空"这一条件发生.
    }

    data = item_buffer[read_position]; // 读取产品
    read_position++;

    if (read_position >= repository_size)
    {
        read_position = 0;
    }

    repo_not_full.notify_all(); // 通知产品库不满
    lck.unlock();

    return data;
}

void Producer_thread()
{
    bool ready_to_exit = false;
    while (1)
    {
        // std::this_thread::sleep_for(t);
        std::unique_lock<std::mutex> lock(producer_count_mtx);
        if (produced_item_counter < item_total)
        {
            ++produced_item_counter;
            produce_item(produced_item_counter);
            std::cout << "生产者线程 " << std::this_thread::get_id()
                      << "生产第  " << produced_item_counter << "个产品" << std::endl;
        }
        else
        {
            ready_to_exit = true;
        }

        lock.unlock();

        if (ready_to_exit == true)
        {
            break;
        }
    }

    std::cout << "Producer thread " << std::this_thread::get_id()
              << " is exiting..." << std::endl;
}

void Consumer_thread()
{
    bool read_to_exit = false;
    while (1)
    {
        std::this_thread::sleep_for(t1);
        std::unique_lock<std::mutex> lck(consumer_count_mtx);
        if (consumed_item_counter < item_total)
        {
            int item = consume_item();
            ++consumed_item_counter;
            std::cout << "消费者线程" << std::this_thread::get_id()
                      << "消费第" << item << "个产品" << std::endl;
        }
        else
        {
            read_to_exit = true;
        }

        if (read_to_exit == true)
        {
            break;
        }
    }

    std::cout << "Consumer thread " << std::this_thread::get_id()
              << " is exiting..." << std::endl;
}

int main()
{
    std::vector<std::thread> thread_vector1;
    std::vector<std::thread> thread_vector2;
    for (int i = 0; i != 5; ++i)
    {
        thread_vector1.push_back(std::thread(Producer_thread)); // 创建生产者线程.
        thread_vector2.push_back(std::thread(Consumer_thread)); // 创建消费者线程.
    }

    for (auto &thr1 : thread_vector1)
    {
        thr1.join();
    }

    for (auto &thr2 : thread_vector2)
    {
        thr2.join();
    }
}