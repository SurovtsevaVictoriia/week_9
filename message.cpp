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
//make global variable for isned(= true)

bool IsEnded = false;

struct Message {

    Message() {};
    Message(std::string text_,int sender_id) {
        sender = sender_id;
        text = text_;
    }
   

    std::string text;
    unsigned int sender;
};

std::ostream& operator<< (std::ostream& out, const Message& message) {

    return out << "sender: " << message.sender << "\n " << message.text << "\n";
};

void thread_func_read( std::string shared_memory_name, chat& Chat, boost::interprocess::named_mutex& m_mutex) {

    if (IsEnded == true) return;
    //when write-mutex unlocks, lock mutex, read(last object in container), cout, then unlock, then sleep.
     //kill process (?)


    m_mutex.lock();
    std::cout << Chat.back();
    m_mutex.unlock();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);


    //after sleep (if is ended  = true)  
}

void thread_func_write(int id, std::string shared_memory_name, chat& Chat, boost::interprocess::named_mutex & m_mutex) {

    

    std::string text;
    while(std::getline(std::cin, text)){

        //lock mutex
        m_mutex.lock();
        //put text into container located in shared memory. 
        Chat.push_back(Message(text, id));
        //(pushback in container)
        m_mutex.unlock();
        //unlock mutex    
    }

    // make global variable
    IsEnded = true;
    return;
}



int main(int argc, char** argv) {

    if (argc != 0) {
        std::cout << "incorrect number of arguments\n";
        return -1;
    }

    Id = atoi(argv[0]);
    std::cout << " process ID: " << Id << "\n";
   

    const std::string mutex_name = "m_mutex";

    boost::interprocess::named_mutex m_mutex(
        boost::interprocess::open_or_create, mutex_name.c_str());

   // system("pause");

    const std::string shared_memory_name = "shared_memory";

    boost::interprocess::managed_shared_memory shared_memory(
        boost::interprocess::open_or_create, shared_memory_name.c_str(), 10000 * sizeof(char));

    boost::interprocess::mapped_region region(shared_memory, boost::interprocess::read_write);

    std::cout << "shared memory ready" << std::endl;

    system("pause");

   // boost::interprocess::shared_memory_object::remove(shared_memory_name.c_str()); // !

   //make two threads with one mutex 
   //wait two threads to join (join when completed)


    return EXIT_SUCCESS;
}
