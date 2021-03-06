#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <stdexcept>
#include <exception>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/errors.hpp>
#include <boost/interprocess/exceptions.hpp>

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/errors.hpp>
#include <stdexcept>
#include <new>

//D:\boost_1_75_0\boost_1_75_0\stage\lib\libboost_date_time-vc142-mt-x64-1_75.lib

using chat = std::vector <struct Message>;

int Id = -1;

//if true then process wont be sendeing messages anymore
bool IsEnded = false;

const std::string line = "-------------------------------------\n";

const std::string shared_memory_name = "n_shared_memory";
const std::string mutex_name = "n_mutex";
const std::string process_num_name = "n_process_num";
const std::string new_messages_name = "n_new_msgs";
const std::string Chat_name = "n_Chat";
const std::string condition_variable_name = "n_cond_var";

//message struct
struct Message {

    Message() { };
    Message(std::string text_, int sender_id = 0) {
        sender = sender_id;
        text = text_;
    }

    void print() {
        std::cout << line << "sender process: " << this->sender << "\ntext: " << this->text << "\n" << line;
    }

    std::string text = "-";
    unsigned int sender = 0;
};

void print_chat(int N) {

    using namespace boost::interprocess;
    managed_shared_memory segment(open_only, shared_memory_name.c_str());
    chat* m_Chat = segment.find<chat>(Chat_name.c_str()).first;
    interprocess_mutex* m_mutex = segment.find<interprocess_mutex>(mutex_name.c_str()).first;

    int m = (m_Chat->size() > N) ? N : m_Chat->size();

    if (m == 0) {
        std::cout << line << "no previous messages\n" << line;
        return;
    }

    std::cout << line << m << " previous messages: \n";

    for (int i = 0; i < m; ++i) {

        m_mutex->lock();
        std::cout << "1111" << __FUNCTION__ ;
        (*m_Chat)[m_Chat->size() - i].print();
        std::cout << "222" <<__FUNCTION__;
        m_mutex->unlock();
    }
    return;

}

//output operator
std::ostream& operator<< (std::ostream& out, const Message& message) {
    return out << "sender process: " << message.sender << "\ntext:" << message.text << "\n";
};

//reads last messages and outputs on screen
void thread_func_read() {

    std::cout << "read thread engaged\n" << std::endl;


    using namespace boost::interprocess;
    managed_shared_memory segment(open_only, shared_memory_name.c_str());
    chat* m_Chat = segment.find<chat>(Chat_name.c_str()).first;
    interprocess_mutex* m_mutex = segment.find<interprocess_mutex>(mutex_name.c_str()).first;
    int* m_New_Messages = segment.find<int>(new_messages_name.c_str()).first;

    while (true) {
        if (IsEnded) return;

            for (int i = 0; i < *m_New_Messages; ++i) {

                m_mutex->lock();
                std::cout << "pre_print" << __FUNCTION__ << std::endl;
                (*m_Chat)[m_Chat->size() -1 - i].print();
                std::cout << "after_print" << __FUNCTION__ << std::endl;
                m_mutex->unlock();
            }

            *m_New_Messages = 0;
       

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2000ms);
    }
    return;

}

//pushes getlined text in vector in shared memory
void thread_func_write(int& id) {

    std::cout << "write thread engaged\n" << std::endl;

    std::cout << "you can write your messages now\n";

    using namespace boost::interprocess;
    try {
        std::cout << "in try section:\n";
        managed_shared_memory segment(open_only, shared_memory_name.c_str());
        std::cout << "managed shared memory accessed\n";
        chat* m_Chat = segment.find<chat>(Chat_name.c_str()).first;
        std::cout << "chat accessed\n";
        interprocess_mutex* m_mutex = segment.find<interprocess_mutex>(mutex_name.c_str()).first;
        std::cout << "mutex accessed\n";
        int* m_New_Messages = segment.find<int>(new_messages_name.c_str()).first;
        std::cout << "new messages accessed\n";

        std::string text;

        std::cout << "entering write while section\n";

        while (std::getline(std::cin, text) && (text != "cancel")) {
            std::cout << __LINE__<< __FUNCTION__;
            m_mutex->lock();
            m_Chat->push_back(Message(text, id));
            (*m_New_Messages)++;
            m_mutex->unlock();
            std::cout << __FUNCTION__ << std::endl;
        }
    }
    catch (interprocess_exception) // ???????????????????? ???????????????????? 
    {
        std::cerr << "Error: \n";
        return;
    }

    std::cout << "after write try section\n";
    IsEnded = true;
    return;
}



int main(int argc, char** argv) {



    using namespace boost::interprocess;

    shared_memory_object::remove(shared_memory_name.c_str());

    /* if ((argc != 2)) {
         std::cout << "incorrect number of arguments\n";
         return -1;
     }*/

    Id = atoi(argv[0]);
    std::cout << " process ID: " << Id << "\n";


    //shread memory created
    managed_shared_memory shared_memory(open_or_create, shared_memory_name.c_str(), 10000 * sizeof(char));
    std::cout << "shared memory created" << std::endl;

    //mapped_region region(shared_memory,read_write);

    //named mutex created 

    auto mutex =
        shared_memory.find_or_construct < boost::interprocess::interprocess_mutex >(
            mutex_name.c_str())();
    std::cout << "mutex created" << std::endl;




    //process counter
    int* process_num = shared_memory.find_or_construct<int>(process_num_name.c_str())();
    (*process_num)++;

    //unread messages counter(?)
    int* new_messages = shared_memory.find_or_construct<int>(new_messages_name.c_str())();
    //chat history itself
    chat* ptr = shared_memory.find_or_construct<chat>(Chat_name.c_str())();

    print_chat(5);
    std::cout << "chat printed " << std::endl;

    //started separate the thread in process fo reading and writing

    std::thread read_thread(thread_func_read);
    std::thread write_thread(thread_func_write, std::ref(Id));

    read_thread.join();
    write_thread.join();

    std::cout << "threads joined\n";


    (*process_num)--;
    if (*process_num == 0) {
        std::cout << "last process closed\n";
        shared_memory_object::remove(shared_memory_name.c_str());
    }
    // !


    return 0;
}
