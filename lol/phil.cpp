#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

const int Max_Phil = 5;
const int Phil_Count = 2;

std::mutex Mutex;
std::condition_variable CV;
bool Phil_States[Max_Phil] = {false};  // false - Thinking, true - Eating
int Phil_Index[2] = {1, 3};
bool Flag_First = false;
bool Flag_Second = false;
std::thread Threads_Array[Max_Phil];
std::chrono::steady_clock::time_point Time_Begin;
std::chrono::steady_clock::time_point Time_End;
std::chrono::milliseconds Time_Sleep;

void Philosopher(int index) {
    while (true) {
        int Cur_Phil = 0;
        {
            std::unique_lock<std::mutex> lock(Mutex);
            if (Flag_First && !Flag_Second) {
                Flag_Second = true;
                Cur_Phil = Phil_Index[1];
            }
            if (!Flag_First) {
                Flag_First = true;
                Cur_Phil = Phil_Index[0];
            }

            if (Time_End <= std::chrono::steady_clock::now() + 3 * Time_Sleep) {
                Flag_First = Flag_Second = false;
                CV.notify_all();
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        {
            std::unique_lock<std::mutex> lock(Mutex);
            std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - Time_Begin)
                             .count()
                      << ':' << Cur_Phil << ":T->E" << std::endl;
        }

        std::this_thread::sleep_for(Time_Sleep);

        {
            std::unique_lock<std::mutex> lock(Mutex);
            std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - Time_Begin)
                             .count()
                      << ':' << Cur_Phil << ":E->T" << std::endl;

            if (Flag_First && Flag_Second) {
                if (Phil_Index[0] == 5)
                    Phil_Index[0] = 1;
                else
                    Phil_Index[0] = Phil_Index[0] + 1;

                if (Phil_Index[0] == 4)
                    Phil_Index[1] = 1;
                else if (Phil_Index[0] == 5)
                    Phil_Index[1] = 2;
                else
                    Phil_Index[1] = Phil_Index[0] + 2;

                Flag_First = false;
            }
            Flag_Second = false;
            CV.notify_all();
        }
    }
}

void Init_Time(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Incorrect input data" << std::endl;
        exit(1);
    } else {
        Time_Begin = std::chrono::steady_clock::now();
        Time_End = Time_Begin + std::chrono::milliseconds(std::stoi(argv[1]));
        Time_Sleep = std::chrono::milliseconds(std::stoi(argv[2]));
    }
}

void Create() {
    for (int i = 0; i < Max_Phil; ++i) {
        Threads_Array[i] = std::thread(Philosopher, i);
    }
}

void Close() {
    for (int i = 0; i < Max_Phil; ++i) {
        Threads_Array[i].join();
    }
}

void ActionMain(int argc, char **argv) {
    Init_Time(argc, argv);
    Create();
    Close();
}

int main(int argc, char **argv) {
    ActionMain(argc, argv);
    return 0;
}
