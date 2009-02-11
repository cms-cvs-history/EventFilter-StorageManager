// $Id: I2OChain.h,v 1.1.2.7 2009/02/06 22:59:51 biery Exp $

#ifndef StorageManager_I2OChain_h
#define StorageManager_I2OChain_h

#include <vector>

#include "boost/shared_ptr.hpp"

#include "toolbox/mem/Reference.h"

#include "IOPool/Streamer/interface/HLTInfo.h"

namespace toolbox
{
  namespace mem
  {
    class Reference;
  } // namespace mem
} // namespace toolbox

namespace stor {

  /**
   * List of one or multiple I2O messages representing event fragments. 
   *
   * It wraps several toolbox::mem::Reference chained together and 
   * assures that the corresponding release methods are called when 
   * the last instance of I2OChain goes out of scope.
   *
   * $Author: biery $
   * $Revision: 1.1.2.7 $
   * $Date: 2009/02/06 22:59:51 $
   */


  // We need only declare ChainData here; it is defined in I2OChain.cc.
  namespace detail
  {
    class ChainData;
  }

  class I2OChain
  {
  public:


    /**
       A default-constructed I2OChain manages no Reference.
    */
    I2OChain();

    /**
       Create an I2OChain that will manage the Reference at address
       pRef, and assure that release is called on it only once,
       regardless of how many copies of the I2OChain object have been
       made.
    */
    explicit I2OChain(toolbox::mem::Reference* pRef);

    /**
       A copy of an I2OChain shares management of the underlying
       Reference. We avoid calling Reference::duplicate.
     */
    I2OChain(I2OChain const& other);

    /**
       Destroying an I2OChain does not release the managed Reference,
       unless that I2OChain is the last one managing that Reference.
    */
    ~I2OChain();

    /**
       Assigning to an I2OChain causes the left-hand side of the
       assignment to relinquish management of any Reference it might
       have had. If the left-hand side was the only chain managing
       that Reference, it will be released. After the assignment, the
       left-hand side shares management of the underlying Reference of
       the right-hand side.
     */
    I2OChain& operator=(I2OChain const& rhs);

    /**
       Standard swap.
     */
    void swap(I2OChain& other);

    /**
     * Returns true if there is no Reference managed by *this.
     */
    bool empty() const;

    /**
     * Returns true if all fragments of an event are available
     */
    bool complete() const;

    /**
     * Returns true if the chain has been marked faulty (the internal
     * data does not represent a complete, valid message).
     */
    bool faulty() const;

    /**
       Adds fragments from another chain to the current chain taking
       care that all fragments are chained in the right order. This
       destructively modifies newpart so that it no longer is part of
       managing any Reference: newpart is made empty.
     
     */
    void addToChain(I2OChain& newpart);

    /**
       Mark this chain as known to be complete.
     */
    void markComplete();

    /**
       Mark this chain as known to be faulty.  The failure modes that
       result in a chain being marked faulty include chains that have
       duplicate fragments and chains that never become complete after
       a timeout interval.
     */
    void markFaulty();

    /**
       Return the address at which the data in buffer managed by the
       Reference begins. If the chain is empty, a null pointer is
       returned.
    */
    unsigned long* getBufferData();


    /**
       Abandon management of the managed Reference, if there is
       one. After this call, *this will be in the same state as if it
       had been default-constructed.
     */
    void release();

    /**
       Returns the I2O message code for the chain.  Valid values
       are I2O_SM_PREAMBLE, I2O_SM_DATA, I2O_SM_ERROR, and
       I2O_SM_DQM from EventFilter/Utilities/interface/i2oEvfMsgs.h.
     */
    unsigned short getI2OMessageCode() const {return _i2oMessageCode;}

    /**
       Returns the fragment key for the chain.  The fragment key
       is the entity that uniquely identifies all of the fragments
       from a particular event.
     */
    FragKey const& getFragmentKey() const {return _fragKey;}

  private:

    boost::shared_ptr<detail::ChainData> _data;

    void parseI2OHeader();
    unsigned short _i2oMessageCode;
    FragKey _fragKey;
  };
  
} // namespace stor

#endif // StorageManager_I2OChain_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
