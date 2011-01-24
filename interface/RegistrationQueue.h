// $Id: RegistrationQueue.h,v 1.5.14.1 2011/01/21 15:51:20 mommsen Exp $
/// @file: RegistrationQueue.h 

#ifndef EventFilter_StorageManager_RegistrationQueue_h
#define EventFilter_StorageManager_RegistrationQueue_h

#include <boost/shared_ptr.hpp>
#include <EventFilter/StorageManager/interface/ConcurrentQueue.h>
#include <EventFilter/StorageManager/interface/RegistrationInfoBase.h>

namespace stor
{

  /**
     Concurrent queue holding consumer registrations

     $Author: mommsen $
     $Revision: 1.5.14.1 $
     $Date: 2011/01/21 15:51:20 $
  */
  typedef boost::shared_ptr<RegistrationInfoBase> RegInfoBasePtr;
  typedef ConcurrentQueue<RegInfoBasePtr> RegistrationQueue;
  typedef boost::shared_ptr<RegistrationQueue> RegistrationQueuePtr;

} // namespace stor

#endif // EventFilter_StorageManager_RegistrationQueue_h

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
