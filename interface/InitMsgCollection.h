#ifndef STOR_INITMSG_COLLECTION_H
#define STOR_INITMSG_COLLECTION_H

/**
 * This class is used to manage the set of INIT messages that have
 * been received by the storage manager and will be sent to event
 * consumers and written to output streams.
 *
 * $Id: InitMsgCollection.h,v 1.4.12.1 2008/12/22 19:17:59 biery Exp $
 */

#include "IOPool/Streamer/interface/InitMessage.h"
#include "boost/shared_ptr.hpp"
#include "boost/thread/thread.hpp"
#include <vector>
#include <map>

namespace stor
{
  typedef std::vector<unsigned char> InitMsgBuffer;
  typedef boost::shared_ptr<InitMsgBuffer> InitMsgSharedPtr;

  class InitMsgCollection
  {

  public:

    InitMsgCollection();
    ~InitMsgCollection();

    bool addIfUnique(InitMsgView const& initMsgView);
    InitMsgSharedPtr getElementForOutputModule(std::string requestedOMLabel);
    InitMsgSharedPtr getLastElement();
    InitMsgSharedPtr getElementAt(unsigned int index);
    InitMsgSharedPtr getFullCollection() { return serializedFullSet_; }

    void clear();
    int size();

    std::string getSelectionHelpString();
    std::string getOutputModuleName(uint32 outputModuleId);
    static std::string stringsToText(Strings const& list,
                                     unsigned int maxCount = 0);

  private:

    void add(InitMsgView const& initMsgView);

    std::vector<InitMsgSharedPtr> initMsgList_;
    InitMsgSharedPtr serializedFullSet_;

    std::map<uint32, std::string> outModNameTable_;

    boost::mutex listLock_;

  };
}

#endif
