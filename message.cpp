#include <iostream>
#include <string>
#include <thread>
#include <mutex>


#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

int Id = -1;
//make global variable for isned(= true)

bool IsEnded = false;

struct Message {
    std::string text;
    unsigned int sender;
};

void thread_func_read() {

    if (IsEnded == true) return;
    //when write-mutex unlocks, lock mutex, read(last object in container), cout, then unlock, then sleep.
     //kill process (?)
    
    //after sleep (if is ended  = true)  
}

void thread_func_write(std::string shared_memory_name) {



    std::string text;
    while(std::getline(std::cin, text)){

        //lock mutex
        //put text into container located in shared memory. 
        //(pushback in container)
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

    

//    boost::interprocess::managed_shared_memory managed_shm(boost::interprocess::create_only, "MySharedMemory", 10000 * sizeof(char));


    const std::string mutex_name = "mutex";

    boost::interprocess::named_mutex mutex(
        boost::interprocess::open_or_create, mutex_name.c_str());

    system("pause");

    const std::string shared_memory_name = "shared_memory";

   // boost::interprocess::shared_memory_object::remove(shared_memory_name.c_str()); // !

    boost::interprocess::managed_shared_memory shared_memory(
        boost::interprocess::open_or_create, shared_memory_name.c_str(), 10000 * sizeof(char));

   //shared_memory.truncate(1024*1024*sizeof(char));

    boost::interprocess::mapped_region region(shared_memory, boost::interprocess::read_write);

   // std::memset(region.get_address(), 1, region.get_size()); // !

    std::cout << "shared memory ready" << std::endl;

    system("pause");

   // boost::interprocess::shared_memory_object::remove(shared_memory_name.c_str()); // !

   //make two threads with one mutex 
   //wait two threads to join (join when completed)


    return EXIT_SUCCESS;
}
