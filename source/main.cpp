#include "lockless_pc_queue.hpp"
#include <string>
#include <thread>
#include <iostream>


void prod(Lockless::PC_Queue<std::string> &s_queue);
void consume(Lockless::PC_Queue<std::string> &s_queue);

int main()
{
    Lockless::PC_Queue<std::string> s_queue;
    Lockless::PC_Queue<int> i_queue;

    std::thread t_prod(prod, std::ref(s_queue));
    std::thread t_consum(consume, std::ref(s_queue));

    t_prod.join();
    t_consum.join();
}

void prod(Lockless::PC_Queue<std::string> &s_queue)
{
    s_queue.produce("Test");
    s_queue.produce("Test2");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    s_queue.produce("Test");
    s_queue.produce("Test2");
}

void consume(Lockless::PC_Queue<std::string> &s_queue)
{
    int i = 0;

    while (i < 10) {
        auto res = s_queue.consume();

        if (!res) {
            std::cout << "Empty" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } else
            std::cout << *res << std::endl;
        i++;
    }
}
