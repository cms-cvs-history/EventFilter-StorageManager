#include <signal.h>
#include <iostream>
#include <string>
#include "toolbox/Condition.h" 
#include "EventFilter/StorageManager/interface/SyncQueue.h"
#include "toolbox/exception/QueueFull.h"
#include "toolbox/exception/Timeout.h"

// This test is taken from the XDAQ original, and modified to use
// stor::SyncQueue rather than toolbox::SyncQueue.
//  --- Marc Paterno <paterno@fnal.gov>


int main() 
{ 
  toolbox::Condition cond_;
  stor::SyncQueue<int>  synch_(1000); // define synrchonized queue for 1000 entries 

  std::cout << "Going to wait for 5 seconds on condition..." << std::endl ; 

  try 
    { 
      cond_.timedwait(5,0) ; // wait on condition with timeout of 5 sec. 
    } 
  catch (toolbox::exception::Timeout& e) 
    {
      std::cout << "Received timeout" << std::endl ;  
    } 

  std::cout << "After timeout." << std::endl ; 
  std::cout << "Pushing 0,1,2 .... 2000 items into synch queue, that has space for only 1000 entries... " << std::endl ; 

  int i = 0; 

  try 
    { 
      for (i=0;i<2000;i++) 
	synch_.push(i) ;
    } 
  catch (toolbox::exception::QueueFull& e) 
    {
      std::cout << i << "th entry. Can not push more ... synch queue is full" << std::endl ;
    }

  std::cout << "After pushing entries, queue size is now: " << synch_.size() << std::endl ;

  std::cout << "Now popping entries with 3 seconds timeout..." << std::endl;

  try 
    { 
      for (i=0;i<2000;i++) 
	synch_.pop(3,0) ; // pop element with 3 sec timeout      
    } 
  catch (toolbox::exception::Timeout& e) 
    {
      std::cout << "Pop produced timeout after pop operation " << i << std::endl ; 
      std::cout << "Synch queue has now: " << synch_.size()  << " entries." << std::endl ; 
    } 

  return 0 ; 
} 
