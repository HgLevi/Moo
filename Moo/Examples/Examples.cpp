#include "Logger.h"

#include <iostream>
#include <thread>
#include <array>

using namespace moo;
using namespace std;

int main()
{
    clog << "Hello Console!" << endl;

    {
        Logger::DefaultLogPath("LogExample.log");
        Logger logger;
        Logger::AssertLock(false);
        //Logger::Lock lock;

        clog << "Hello Log!" << endl;

        cout << "This shouldn't go to file" << endl;

        cerr << "This is an ERROR!" << endl;

        clog << "Goodbye Log!" << endl;
    }

    clog << "Goodbye Console!" << endl;

    {
        Logger logger;
        //Logger::Lock lock;

        clog << "---Second block---" << endl;
    }

    {
        Logger logger("LogExample2.log");

        Logger::AssertLock(true);

        constexpr size_t threadCount = 8;
        array<thread, threadCount> threads;
        for (size_t i = 0; i < threadCount; ++i)
        {
            threads[i] = thread([i]()
                {
                    {
                        Logger::Lock lock;
                        clog << "Thread " << i << ": started" << endl;
                    }

                    for (size_t j = 0; j < 10; ++j)
                    {
                        Logger::Lock lock;
                        clog << "Thread " << i << " loop: " << j << endl;
                        this_thread::sleep_for(chrono::milliseconds(10));
                    }
                });
        }

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    system("pause");
    return 0;
}
