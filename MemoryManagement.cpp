#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>

const int MEMORY_SIZE = 102400;
const int MIN_REQUEST_SIZE = 2;
const int MAX_REQUEST_SIZE = 1024;

struct Request
{
    int id;
    int size;
    int persistence;
};

class MemoryManager
{
    int pageSize;
    int numPages;
    std::vector<int> pageTable;
    std::queue<Request> activeRequests;

public:
    MemoryManager(int pageSize) : pageSize(pageSize)
    {
        numPages = MEMORY_SIZE / pageSize;
        pageTable.resize(numPages, -1);
    }

    bool allocateMemory(Request request)
    {
        int pagesNeeded = (request.size + pageSize - 1) / pageSize;

        int freePages = 0;
        for (int i = 0; i < numPages; i++)
        {
            if (pageTable[i] == -1)
            {
                freePages++;
            }
        }

        if (freePages < pagesNeeded)
        {
            return false;
        }

        int allocatedPages = 0;
        for (int i = 0; i < numPages && allocatedPages < pagesNeeded; i++)
        {
            if (pageTable[i] == -1)
            {
                pageTable[i] = request.id;
                allocatedPages++;
            }
        }

        activeRequests.push(request);
        return true;
    }

    void releaseMemory()
    {
        while (!activeRequests.empty() && activeRequests.front().persistence <= 0)
        {
            Request request = activeRequests.front();
            activeRequests.pop();

            for (int i = 0; i < numPages; i++)
            {
                if (pageTable[i] == request.id)
                {
                    pageTable[i] = -1;
                }
            }
        }
    }

    void decrementPersistence()
    {
        std::queue<Request> newQueue;
        while (!activeRequests.empty())
        {
            Request request = activeRequests.front();
            activeRequests.pop();
            request.persistence--;
            if (request.persistence > 0)
            {
                newQueue.push(request);
            }
        }
        activeRequests = newQueue;
    }
};

int main()
{
    srand(time(0));
    std::vector<int> pageSizes = {128, 256, 512, 1024};

    for (int pageSize : pageSizes)
    {
        MemoryManager memoryManager(pageSize);
        int numRequests = 1000;
        int successfulRequests = 0;

        for (int i = 0; i < numRequests; i++)
        {
            int requestSize = rand() % (MAX_REQUEST_SIZE - MIN_REQUEST_SIZE + 1) + MIN_REQUEST_SIZE;
            int persistence = rand() % 10 + 1;

            Request request = {i, requestSize, persistence};

            bool success = memoryManager.allocateMemory(request);
            if (success)
            {
                successfulRequests++;
            }
            memoryManager.decrementPersistence();
            memoryManager.releaseMemory();
        }

        std::cout << "Page size: " << pageSize << ", Successful requests: " << successfulRequests << std::endl;
    }

    return 0;
}
