#include <iostream>
#include <sstream>     // for stringstream
#include <vector>      // for vectors
#include <thread>      // for threads
#include <functional>  // for std::ref
#include <chrono>      // for thread sleep time

using namespace std;

void printStuff()
{
    string stuff;
    float cycleWaste = 2;
    for(int i = 0; i < 50; ++i)
    {
        stuff += char('a'+(i % 26));
        cout << stuff << "\n";

	this_thread::yield();
    }
}

void letMeAddThatForYou(string prefix, int& num)
{
    stringstream ss;

    ss << prefix << ": ";

    for(int i = 0; i < 10; ++i)
    {
        ++num;
        ss << num << ", ";
        cout << ss.str() << "\n";
	this_thread::yield();
    }
}

class ThreadWorker
{
    public:
        ThreadWorker() : m_num(0), m_stop(false), m_count(0) {};

        void workStuff(const int& numToAdd)
        {
            stringstream ss;

            while(!m_stop)
            {
                m_num += numToAdd;
                ss << m_num << "--";
                cout << ss.str() << "\n";

                ++m_count;
                if(m_count%5 == 0)
                {
                    takeABreak();
                }
            }
        }
        
        void takeABreak()
        {
            this_thread::sleep_for(chrono::seconds(3));
        }

        int getNum() {return m_num;}

        void okStopThat() {m_stop = true;}

    private:
        int m_num;
        bool m_stop;
        int m_count;
};

int main()
{
    //example one:
    cout << "Example 1: giving the threads a function, no arguments\n"
         << "(press enter to start)\n";
    cin.get();
    cout << "\n=================================================================\n";
    
    thread t1(printStuff), t2(printStuff);
    this_thread::yield();
    printStuff();
    t1.join();
    t2.join();
    
    cout << "\n=================================================================\n"
         << "(press enter to continue)\n";
    cin.get();
    



    //example two:
    cout << "\n\nExample 2: giving the threads a function with some arguments, one pass by value, one pass by reference\n"
         << "(press enter to start)\n";
    cin.get();
    cout << "\n=================================================================\n";

    int x = 10;
    string prefix;
    vector<thread> threadVec;

    prefix = "   (t3)";
    threadVec.push_back(thread(letMeAddThatForYou, prefix, ref(x)));
    prefix = "      (t4)";
    threadVec.push_back(thread(letMeAddThatForYou, prefix, ref(x)));
    prefix = "(main)";
    this_thread::yield();
    letMeAddThatForYou(prefix, std::ref(x));
    threadVec[0].join();
    threadVec[1].join();

    cout << "\n=================================================================\n"
         << "(press enter to continue)\n";
    cin.get();




    //example three:
    cout << "\n\nExample 3: giving the threads an object to work with\n"
         << "(press enter to start)\n";
    cin.get();
    cout << "\n=================================================================\n";

    ThreadWorker tw;
    int add = 1, add2 = 100;
    thread t3(&ThreadWorker::workStuff, &tw, ref(add)), t4(&ThreadWorker::workStuff, &tw, ref(add2));
    this_thread::sleep_for(chrono::seconds(15));
    tw.okStopThat();
    t3.join();
    t4.join();

    cout << "\n=================================================================\n\n";

    return 0;
}
