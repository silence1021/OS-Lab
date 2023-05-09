#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

static const int repository_size = 10; // Circular queue size
static const int item_total = 20;      // The number of products to produce

std::mutex mtx; // mutex to protect the production buffer
std::mutex producer_count_mtx;
std::mutex consumer_count_mtx;

std::condition_variable repo_not_full;  // Condition variable indicates that the product buffer is not satisfied
std::condition_variable repo_not_empty; // Condition variable indicates that the product buffer is not empty

int item_buffer[repository_size]; // products buffer, here a circular queue is used

static std::size_t read_position = 0;  // The consumer reads the product's location
static std::size_t write_position = 0; // Where the producer writes the product

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
        repo_not_full.wait(lck); // Producer waits for "product library buffer not full" to happen
    }

    item_buffer[write_position] = i; // Write to the product
    write_position++;

    if (write_position == repository_size) // Reset location if last in queue
    {
        write_position = 0;
    }

    repo_not_empty.notify_all(); // Notify the consumer that the products library is not empty

    lck.unlock(); // Unlock
}

int consume_item()
{
    int data;
    std::unique_lock<std::mutex> lck(mtx);
    // item buffer is empty, just wait here.
    while (write_position == read_position)
    {
        std::cout << "Consumer is waiting for items..." << std::endl;
        repo_not_empty.wait(lck); // Consumer waits for "product library buffer not empty" to happen.
    }

    data = item_buffer[read_position]; // Read products
    read_position++;

    if (read_position >= repository_size)
    {
        read_position = 0;
    }

    repo_not_full.notify_all(); // notify the product repository of dissatisfaction
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
            std::cout << "Producer thread " << std::this_thread::get_id()
                      << "Make the " << produced_item_counter << " product" << std::endl;
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
            std::cout << "Consumer thread" << std::this_thread::get_id()
                      << "Consume the " << item << "product" << std::endl;
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
        thread_vector1.push_back(std::thread(Producer_thread)); // Create producer thread
        thread_vector2.push_back(std::thread(Consumer_thread)); // Create consumer thread
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