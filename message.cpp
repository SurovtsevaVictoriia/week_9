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

////how many unread messges process has(actually needs to be in shm)
//int New_Messages = 0;

//if true then process wont be sendeing messages anymore
bool IsEnded = false;


const std::string shared_memory_name = "n_shared_memory";
const std::string mutex_name = "n_mutex";
const std::string process_num_name = "n_process_num";
const std::string new_messages_name = "n_new_msgs";
const std::string Chat_name = "n_Chat";
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
void thread_func_read() {

    using namespace boost::interprocess;
    managed_shared_memory segment(open_only, shared_memory_name.c_str());

    while (true) {
        if (IsEnded == true) return;

        
       /* for (int i = 0; i < New_Messages; ++i) {
            m_mutex.lock();
            std::cout << Chat[Chat.size() -1 - i];
            m_mutex.unlock();
        }*/
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
        return;
   
}

//pushes getlined text in vector in shared memory
void thread_func_write() {

    using namespace boost::interprocess;
    managed_shared_memory segment(open_only, shared_memory_name.c_str());
   
    std::string text;
    while(std::getline(std::cin, text) && (text != "cancel")){


        segment.find<boost::interprocess::interprocess_mutex>("n_mutex").first.lock();
        Chat.push_back(Message(text, id));
        New_Messages++;
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
   
    
   
   
    //shread memory created
    managed_shared_memory shared_memory(open_or_create, shared_memory_name.c_str(), 10000 * sizeof(char));

    mapped_region region(shared_memory,read_write);

    //named mutex created 
   
    auto mutex =
        shared_memory.find_or_construct < boost::interprocess::interprocess_mutex >(
            mutex_name.c_str())();


    //process counter
    int* process_num = shared_memory.find_or_construct<int>(process_num_name.c_str())();
    *process_num++;

    //unread messages counter(?)
    int* new_messages = shared_memory.find_or_construct<int>(new_messages_name.c_str())();
    //chat history itself
    chat* ptr = shared_memory.find_or_construct<chat>(Chat_name.c_str())();

    //started separate the thread in process fo reading and writing
    std::thread read_thread(thread_func_read);
    std::thread write_thread(thread_func_write);

    *process_num--;
    if (*process_num == 0 ) shared_memory_object::remove(shared_memory_name.c_str()); // !
    

    return EXIT_SUCCESS;
}
