#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>

using std::cin;
using std::condition_variable;
using std::cout;
using std::endl;
using std::mutex;
using std::thread;
using std::unique_lock;

const int NumThreads = 4;

struct sharedData
{
    mutex m;
    condition_variable cv;
    int count;
};

void spam(const int i, sharedData *dat)
{
    // This first call ensures that the mutex m is locked before proceeding
    //  The scoping block is the critical section, and ensures that this entire
    //  set of code will be executed while holding the mutex lock
    {
        unique_lock<mutex> lck(dat->m); // only proceeds after mutex is secured
        cout << "Thread " << i << " read the count to be " << dat->count << endl;
    } // mutex m unlocked

    // attempt to sleep for # seconds corresponding to the thread number
    std::this_thread::sleep_for(std::chrono::seconds(i));

    { // re-lock mutex to modify shared variable
        unique_lock<mutex> lck(dat->m);
        // illustrate the conditional variable by having thread 0 wait
        //  until thread 3 (the last thread) has sent the signal
        if (i == 0)
        {
            dat->cv.wait(lck);
        }
        dat->count++;
        cout << "Thread " << i << " just updated the count to be " << dat->count << endl;
    } // mutex m unlocked
    if (i == NumThreads - 1)
    {
        dat->cv.notify_one();
        // if ALL those waiting on the conditional variable need to be
        //  notified, notify_all() can be called instead
        // cv.notify_all();
    }
}

int main()
{
    sharedData someData;
    someData.count = 0;

    thread spammers[4];         // create space to keep track of 4 threads
    for (int i = 0; i < 4; i++) // run the threads, keeping their ids
    {
        // Each thread is created by passing the function to be executed
        //  along with any parameters required by that function
        spammers[i] = thread(spam, i, &someData);
    }

    // Each thread needs to finish before our main function can continue
    // for(int i = 0; i < 4; i++) //run the threads, keeping their ids
    //{
    //     spammers[i].join();
    // }
    //  The following is equivalent to looping through the 4 indexed spammers and
    //  calling spammer[i].join(); (as commented out above)
    //  Notice the use of the newer C++ keyword auto and the range it will operate on
    //  This could be read "For all spammers s, do the following..."
    for (auto &s : spammers)
    {
        s.join();
    }

    cout << "Back in main the final count is " << someData.count << endl;
    return 0;
}
