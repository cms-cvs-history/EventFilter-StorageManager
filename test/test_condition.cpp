// $Id: test_condition.cpp,v 1.1.2.1 2009/02/05 23:08:05 paterno Exp $

// This test is taken from the XDAQ original, and modified to use
// stor::SyncQueue rather than toolbox::SyncQueue.
//  --- Marc Paterno <paterno@fnal.gov>

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "toolbox/Task.h"
#include "toolbox/TaskAttributes.h"
#include "toolbox/TaskGroup.h"
#include "EventFilter/StorageManager/interface/SyncQueue.h"

#define PRODUCER 0
#define CONSUMER 1

class ProducerConsumer: public toolbox::Task
{
public:
	
  ProducerConsumer (int which, char* type, stor::SyncQueue<int>* q): toolbox::Task(type)
  {
    queue_ = q;
    which_ = which;
  }
	
  ~ProducerConsumer()
  {
  }

  int svc()
  {
    if (which_ == PRODUCER) 
      {
	int i = 0;
	sleep(5); // wait for consumer to ask for something.
// 	queue_->push (i++);
// 	queue_->push (i++);
// 	queue_->push (i++);
// 	queue_->push (i++);
// 	queue_->push (i++);
// 	std::cerr << "Producer has put 5 elements onto queue\n";
			
	while (i < 10)
	  {
	    std::cerr << "Producer going to put " << i
		      << " onto queue\n";
	    queue_->push (i++);
	    std::cerr << "Producer has put " << i 
		      << " onto queue, going to sleep...\n";
	    this->sleep(1);
	    std::cerr << "Producer has awoken, i = " << i << '\n';
	  }
      }
    else  // consumer part
      {
	for (int i = 0; i < 10; i++) 
	  {
	    int x = queue_->pop();
	    int size = queue_->size();
	    std::cout << "Read: " << x << ", queue size: " 
		      << size << std::endl;
	  }
      }
    return 0;
  }	
	
private:
		
  int 		which_;
  stor::SyncQueue<int>* queue_;
	

};

int main (int argc, char* argv[])
{
  stor::SyncQueue<int> queue;
  toolbox::TaskGroup group;
	
  std::cout << "Create two tasks, producer and consumer" << std::endl;
	
  ProducerConsumer p(PRODUCER, "Producer", &queue);
  ProducerConsumer c(CONSUMER, "Consumer", &queue);	
	
  std::cout << "Activate tasks" << std::endl;
  //aTask.activate(0, "Ciao", 0);
	
  p.initTaskGroup(&group);
  c.initTaskGroup(&group);

		
  p.activate();
  c.activate();
	
  group.join();
	
  std::cout << "Shutdown tasks" << std::endl;
	
  p.kill();
  c.kill();	
	
  std::cout << "End of test." << std::endl;
  return 0;
}
