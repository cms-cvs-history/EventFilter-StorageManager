// $Id: I2OChain.cc,v 1.1.2.3 2009/02/06 02:23:04 paterno Exp $

#include <algorithm>
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/Types.h"

namespace stor
{
  namespace detail
  {
    ///////////////////////////////////////////////////////////////////
    //
    // class ChainData is responsible for managing a single chain of
    // References and associated status information (such as tags
    // applied to the 'event' that lives in the buffer(s) managed by the
    // Reference(s).
    //
    // Only one ChainData object ever manages a given
    // Reference. Furthermore, ChainData makes use of References such
    // that a duplicate of a given Reference is never made. Thus when a
    // ChainData object is destroyed, any and all References managed by
    // that object are immediately released.
    //
    ///////////////////////////////////////////////////////////////////
    class ChainData
    {
    public:
      explicit ChainData(toolbox::mem::Reference* pRef);
      ~ChainData();
      bool empty() const;
      bool complete() const;
      void markComplete();
      unsigned long* getBufferData();
      void swap(ChainData& other);

    private:
      std::vector<QueueID> _streamTags;
      std::vector<QueueID> _dqmEventConsumerTags;
      std::vector<QueueID> _eventConsumerTags;
    
      toolbox::mem::Reference* _ref;
    
      bool  _complete;
    };

    // A ChainData object may or may not contain a Reference.
    inline ChainData::ChainData(toolbox::mem::Reference* pRef) :
      _streamTags(),
      _dqmEventConsumerTags(),
      _eventConsumerTags(),
      _ref(pRef),
      _complete(false)
    { }

    // A ChainData that has a Reference is in charge of releasing
    // it. Because releasing a Reference can throw an exception, we have
    // to be prepared to swallow the exception. This is fairly gross,
    // because we lose any exception information. But allowing an
    // exception to escape from a destructor is even worse, so we must
    // do what we must do.
    //
    inline ChainData::~ChainData()
    {
      if (_ref) 
        {
          try { _ref->release(); }
          catch (...) { /* swallow any exception. */ }
        }
    }

    inline bool ChainData::empty() const
    {
      return !_ref;
    }

    inline bool ChainData::complete() const
    {
      return _complete;
    }

    inline void ChainData::markComplete()
    {
      _complete = true;
    }

    inline unsigned long* ChainData::getBufferData()
    {
      return _ref 
        ?  static_cast<unsigned long*>(_ref->getDataLocation()) 
        : 0UL;
    }

    inline void ChainData::swap(ChainData& other)
    {
      _streamTags.swap(other._streamTags);
      _dqmEventConsumerTags.swap(other._dqmEventConsumerTags);
      _eventConsumerTags.swap(other._eventConsumerTags);
      std::swap(_ref, other._ref);
      std::swap(_complete, other._complete);
    }
  } // namespace detail


  // A default-constructed I2OChain has a null (shared) pointer.
  I2OChain::I2OChain() :
    _data()
  { 
  }

  I2OChain::I2OChain(toolbox::mem::Reference* pRef) :
    _data(new detail::ChainData(pRef))
  
  { 
  }

  I2OChain::I2OChain(I2OChain const& other) :
    _data(other._data)
  { }

  I2OChain::~I2OChain()
  {
  }

  I2OChain& I2OChain::operator=(I2OChain const& rhs)
  {
    // This is the standard copy/swap algorithm, to obtain the strong
    // exception safety guarantee.
    I2OChain temp(rhs);
    swap(temp);
    return *this;
  }
  
  void I2OChain::swap(I2OChain& other)
  {
    _data.swap(other._data);
  }

  bool I2OChain::empty() const
  {
    // We're empty if we have no ChainData, or if the ChainData object
    // we have is empty.
    return !_data || _data->empty();
  }


  bool I2OChain::complete() const
  {
    if (!_data) return false;
    return _data->complete();
  }


  void I2OChain::addToChain(I2OChain &chain)
  {
  }

  void I2OChain::markComplete()
  {
    // TODO:: Should we throw an exception if _data is null? If so, what
    // type? Right now, we do nothing if _data is null.
    if (_data) _data->markComplete();
  }

  unsigned long* I2OChain::getBufferData()
  {
    return _data ?  _data->getBufferData() : 0UL;
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
