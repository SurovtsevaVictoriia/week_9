#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>


#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

using chat = std::vector <struct Message>;

int Id = -1;


//if true then process wont be sendeing messages anymore
bool IsEnded = false;

//message struct
struct Message {

    Message() {};
    Message(std::string text_,int sender_id) {
        sender = sender_id;
        text = text_;
    }
   
    std::string text;
    unsigned int sender;
};

//output operator
std::ostream& operator<< (std::ostream& out, const Message& message) {

    return out << "sender process: " << message.sender << "\n " << message.text << "\n";
};

//reads last messages and outputs on screen
void thread_func_read( std::string shared_memory_name, chat& Chat, boost::interprocess::named_mutex& m_mutex) {

    if (IsEnded == true) return;
  
    m_mutex.lock();
    std::cout << Chat.back();
    m_mutex.unlock();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    return;
}

//pushes getlined text in vector in shared memory
void thread_func_write(int id, std::string shared_memory_name, chat& Chat, boost::interprocess::named_mutex & m_mutex) {

   
    std::string text;
    while(std::getline(std::cin, text) && (text != "cancel")){

        m_mutex.lock();
        Chat.push_back(Message(text, id));
        m_mutex.unlock();
      
    }

    IsEnded = true;
    return;
}



int main(int argc, char** argv) {

    using namespace boost::interprocess;

    if (argc != 0) {
        std::cout << "incorrect number of arguments\n";
        return -1;
    }

    Id = atoi(argv[0]);
    std::cout << " process ID: " << Id << "\n";
   
    //named mutex created 
    const std::string mutex_name = "n_mutex";
    named_mutex m_mutex(
       open_or_create, mutex_name.c_str());

    //system("pause");

    //shread memory created
    const std::string shared_memory_name = "n_shared_memory";
    //if (process_num == 1) boost::interprocess::shared_memory_object::remove(shared_memory_name.c_str()); // !
    managed_shared_memory shared_memory(open_or_create, shared_memory_name.c_str(), 10000 * sizeof(char));

    mapped_region region(shared_memory,read_write);

    //process counter
    int* process_num = shared_memory.find_or_construct<int>("n_process_num")();
    *process_num++;

    chat* ptr = shared_memory.find_or_construct<chat>("n_Chat")();

    //started separate the thread in process fo reading and writing
    std::thread read_thread(thread_func_read);
    std::thread write_thread(thread_func_write);

    *process_num--;
    if (*process_num == 0 ) shared_memory_object::remove(shared_memory_name.c_str()); // !
    

    return EXIT_SUCCESS;
}
