#include <iostream>
#include <vector>
#include <queue>
#include <ctime>
#include <mutex>
#include <cstring>

using namespace std;

const int TOTAL_MEMORY_SIZE = 102400; // Total memory size in BLIPS (units of size)
const int PAGE_SIZE_128 = 128;        // Page size in BLIPS

struct PageTableEntry
{
    bool is_allocated;    // Whether the page is allocated or free
    int process_id;       // ID of the process that owns this page (if allocated)
    int page_num;         // Page number
    time_t last_accessed; // Timestamp of the last access
};

class MemoryAllocator
{
private:
    int num_pages;
    int page_size;
    vector<PageTableEntry> page_table;
    vector<char> memory;
    queue<int> free_page_list;
    mutex page_table_mutex;

public:
    MemoryAllocator(int page_size)
    {
        this->page_size = page_size;
        this->num_pages = TOTAL_MEMORY_SIZE / page_size;
        page_table.resize(num_pages);
        memory.resize(TOTAL_MEMORY_SIZE);
        for (int i = 0; i < num_pages; i++)
        {
            page_table[i].is_allocated = false;
            page_table[i].page_num = i;
            free_page_list.push(i);
        }
    }

    int allocate(int process_id, int size)
    {
        int num_pages_needed = (size + page_size - 1) / page_size;
        if (num_pages_needed > free_page_list.size())
        {
            return -1; // Failed to allocate memory
        }
        page_table_mutex.lock();
        int first_page = free_page_list.front();
        free_page_list.pop();
        for (int i = 0; i < num_pages_needed; i++)
        {
            int page_num = first_page + i;
            page_table[page_num].is_allocated = true;
            page_table[page_num].process_id = process_id;
            page_table[page_num].last_accessed = time(NULL);
        }
        page_table_mutex.unlock();
        return first_page * page_size;
    }

    void free(int process_id, int address, int size)
    {
        int first_page = address / page_size;
        int num_pages = (size + page_size - 1) / page_size;
        page_table_mutex.lock();
        for (int i = 0; i < num_pages; i++)
        {
            int page_num = first_page + i;
            if (page_table[page_num].is_allocated && page_table[page_num].process_id == process_id)
            {
                page_table[page_num].is_allocated = false;
                free_page_list.push(page_num);
            }
        }
        page_table_mutex.unlock();
    }

    int find_free_page()
    {
        page_table_mutex.lock();
        if (free_page_list.empty())
        {
            page_table_mutex.unlock();
            return -1;
        }
        int page_num = free_page_list.front();
        free_page_list.pop();
        page_table_mutex.unlock();
        return page_num;
    }

    void handle_page_fault(int process_id, int address)
    {
        int page_num = address / page_size;
        if (!page_table[page_num].is_allocated)
        {
            int new_page_num = find_free_page();
            if (new_page_num == -1)
            {
                // Page replacement needed
                new_page_num = page_replacement(process_id);
            }
            copy_page(page_num, new_page_num);
            page_table_mutex.lock();
            page_table[new_page_num].is_allocated = true;
            page_table[new_page_num].process_id = process_id;
            page_table[new_page_num].last_accessed = time(NULL);
            page_table_mutex.unlock();
        }
        else
        {
            page_table_mutex.lock();
            page_table[page_num].last_accessed = time(NULL);
            page_table_mutex.unlock();
        }
    }
    int page_replacement(int process_id)
    {
        // Simplest page replacement algorithm: FIFO
        int oldest_page_num = -1;
        time_t oldest_timestamp = time(NULL);
        for (int i = 0; i < num_pages; i++)
        {
            if (page_table[i].is_allocated && page_table[i].process_id == process_id && page_table[i].last_accessed < oldest_timestamp)
            {
                oldest_page_num = i;
                oldest_timestamp = page_table[i].last_accessed;
            }
        }
        if (oldest_page_num == -1)
        {
            // Shouldn't happen if memory allocation is managed correctly
            return -1;
        }
        return oldest_page_num;
    }

    void copy_page(int old_page_num, int new_page_num)
    {
        int start_address = old_page_num * page_size;
        int end_address = start_address + page_size;
        for (int i = start_address; i < end_address; i++)
        {
            memory[new_page_num * page_size + (i - start_address)] = memory[i];
        }
    }

    void release_expired_memory(int expiration_time)
    {
        time_t current_time = time(NULL);
        page_table_mutex.lock();
        for (int i = 0; i < num_pages; i++)
        {
            if (page_table[i].is_allocated && current_time - page_table[i].last_accessed >= expiration_time)
            {
                page_table[i].is_allocated = false;
                free_page_list.push(i);
            }
        }
        page_table_mutex.unlock();
    }

    void read(int process_id, int address, int size, char *buffer)
    {
        int start_address = address / page_size * page_size;
        int end_address = (address + size - 1) / page_size * page_size;
        for (int i = start_address; i <= end_address; i += page_size)
        {
            int page_num = i / page_size;
            if (!page_table[page_num].is_allocated || page_table[page_num].process_id != process_id)
            {
                handle_page_fault(process_id, i);
            }
        }
        memcpy(buffer, &memory[address], size);
        page_table_mutex.lock();
        for (int i = start_address; i <= end_address; i += page_size)
        {
            int page_num = i / page_size;
            page_table[page_num].last_accessed = time(NULL);
        }
        page_table_mutex.unlock();
    }

    void write(int process_id, int address, int size, char *buffer)
    {
        int start_address = address / page_size * page_size;
        int end_address = (address + size - 1) / page_size * page_size;
        for (int i = start_address; i <= end_address; i += page_size)
        {
            int page_num = i / page_size;
            if (!page_table[page_num].is_allocated || page_table[page_num].process_id != process_id)
            {
                handle_page_fault(process_id, i);
            }
        }
        memcpy(&memory[address], buffer, size);
        page_table_mutex.lock();
        for (int i = start_address; i <= end_address; i += page_size)
        {
            int page_num = i / page_size;
            page_table[page_num].last_accessed = time(NULL);
        }
        page_table_mutex.unlock();
    }
};

int main()
{
    MemoryAllocator memory_allocator(PAGE_SIZE_128);
    int process_id = 1;
    char buffer[1024];
    memory_allocator.write(process_id, 0, 1024, buffer);
    memory_allocator.read(process_id, 256, 512, buffer);
    memory_allocator.free(process_id, 0, 1024);
    memory_allocator.release_expired_memory(60);
    return 0;
}
