#ifndef _testI2OReceiver_h_
#define _testI2OReceiver_h_

/*
   Author: Harry Cheung, FNAL

   Description:
     Header file used by test XDAQ application that will receive I2O
     frames and write out a data file.

   Modification:
     version 1.1 2005/11/23
       Initial implementation. Needs changes for production version.

*/


#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"

#include "toolbox/mem/Reference.h"

#include <vector>
#include <string>
#include <fstream>

struct SMI2ORecFrames;

class testI2OReceiver: public xdaq::Application  
{
  public:
	
  testI2OReceiver(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);

  virtual ~testI2OReceiver(){}

  private:

  vector<SMI2ORecFrames> SMframeFragments_;
  string filename_;
  ofstream ost_;
  void receiveRegistryMessage(toolbox::mem::Reference *ref);
  void receiveDataMessage(toolbox::mem::Reference *ref);
  void receiveOtherMessage(toolbox::mem::Reference *ref);
  void testCompleteChain(vector<SMI2ORecFrames>::iterator pos);
  void writeCompleteChain(toolbox::mem::Reference *ref);

};
#endif
