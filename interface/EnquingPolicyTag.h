// $Id: ConcurrentQueue.h,v 1.1.2.7 2009/02/16 17:37:42 paterno Exp $

#ifndef EventFilter_StorageManager_EnquingPolicy_t
#define EventFilter_StorageManager_EnquingPolicy_t

namespace stor
{

  /**
     This enumeration is used to denote which queuing discipline is
     used for enquing items when the queue in question is full.

     $Author: biery $
     $Revision: 1.1.2.5 $
     $Date: 2009/03/01 20:36:29 $
   */

  namespace enquing_policy
    {
      enum PolicyTag
	{
	  DiscardNew,
	  DiscardOld,
	  FailIfFull,
	  Max
	};
    } // namespace enquing_policy
} // namespace stor

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -


