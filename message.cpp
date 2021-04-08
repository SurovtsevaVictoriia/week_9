#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/containers/map.hpp>
//#include <D:\boost_1_75_0\boost_1_75_0\stage\lib\libboost_date_time-vc142-mt-x64-1_75.lib>

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

//message struct
struct Message {

    Message() { };
    Message(std::string text_,int sender_id = 0) {
        sender = sender_id;
        text = text_;
    }

    void print() {
        std::cout << line << "sender process: " << this->sender << "\n " << this->text << "\n" << line;
    }
   
    std::string text = "-";
    unsigned int sender = 0;
};

void print_chat(int N) {

    using namespace boost::interprocess;
    managed_shared_memory segment(open_only, shared_memory_name.c_str());
    chat* m_Chat = segment.find<chat>(Chat_name.c_str()).first;
    interprocess_mutex* m_mutex = segment.find<interprocess_mutex>(mutex_name.c_str()).first;

    std::cout << N << " previous messages: \n";

    for (int i = 0; i < N; ++i) {

        m_mutex->lock();
        std::cout << (m_Chat + m_Chat->size() - i);
        m_mutex->unlock();
    }
    
}

//output operator
std::ostream& operator<< (std::ostream& out, const Message& message) {
    return out << "sender process: " << message.sender << "\n " << message.text << "\n";
};

//reads last messages and outputs on screen
void thread_func_read() {

    using namespace boost::interprocess;
    managed_shared_memory segment(open_only, shared_memory_name.c_str());
    chat* m_Chat = segment.find<chat>(Chat_name.c_str()).first;
    interprocess_mutex* m_mutex = segment.find<interprocess_mutex>(mutex_name.c_str()).first;
    int* m_New_Messages = segment.find<int>(new_messages_name.c_str()).first;

    while (true) {
        if (IsEnded == true) return;

        
        for (int i = 0; i < *m_New_Messages; ++i) {

            m_mutex->lock();            
            std::cout << (m_Chat + m_Chat->size() - i) ;
            m_mutex->unlock();
        }

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
        return;
   
}

//pushes getlined text in vector in shared memory
void thread_func_write(int& id) {
    std::cout << "you can write your messages now\n";

    using namespace boost::interprocess;
    managed_shared_memory segment(open_only, shared_memory_name.c_str());
    chat* m_Chat = segment.find<chat>(Chat_name.c_str()).first;
    interprocess_mutex* m_mutex = segment.find<interprocess_mutex>(mutex_name.c_str()).first;
    int* m_New_Messages = segment.find<int>(new_messages_name.c_str()).first;

    std::string text;

    while(std::getline(std::cin, text) && (text != "cancel")){

        m_mutex->lock();
        m_Chat->push_back(Message(text, id));
        (*m_New_Messages)++;
        m_mutex->unlock();

    }

    IsEnded = true;
    return;
}



int main(int argc, char** argv) {

    using namespace boost::interprocess;

    if ((argc != 2)) {
        std::cout << "incorrect number of arguments\n";
        return -1;
    }

    Id = atoi(argv[0]);
    std::cout << " process ID: " << Id << "\n";
   
    //shread memory created
    managed_shared_memory shared_memory(open_or_create, shared_memory_name.c_str(), 10000 * sizeof(char));

    //mapped_region region(shared_memory,read_write);

    //named mutex created 
   
    auto mutex =
        shared_memory.find_or_construct < boost::interprocess::interprocess_mutex >(
            mutex_name.c_str())();

    //print_chat(5);

    //process counter
    int* process_num = shared_memory.find_or_construct<int>(process_num_name.c_str())();
    (*process_num)++;

    //unread messages counter(?)
    int* new_messages = shared_memory.find_or_construct<int>(new_messages_name.c_str())();
    //chat history itself
    chat* ptr = shared_memory.find_or_construct<chat>(Chat_name.c_str())();

    //started separate the thread in process fo reading and writing
    std::thread read_thread(thread_func_read);
    std::thread write_thread(thread_func_write, std::ref(Id));

    (*process_num)--;
    if (*process_num == 0 ) shared_memory_object::remove(shared_memory_name.c_str()); // !
    

    return EXIT_SUCCESS;
}
