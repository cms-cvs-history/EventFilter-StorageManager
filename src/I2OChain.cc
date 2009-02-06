// $Id: I2OChain.cc,v 1.1.2.5 2009/02/06 17:59:05 paterno Exp $

#include <algorithm>
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/Types.h"
#include "EventFilter/Utilities/interface/i2oEvfMsgs.h"
#include "IOPool/Streamer/interface/MsgHeader.h"

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
    _data(),
    _i2oMessageCode(0),
    _fragKey(0,0,0,0,0,0)
  { 
  }

  I2OChain::I2OChain(toolbox::mem::Reference* pRef) :
    _data(new detail::ChainData(pRef)),
    _i2oMessageCode(0),
    _fragKey(0,0,0,0,0,0)
  {
    parseI2OHeader();
  }

  I2OChain::I2OChain(I2OChain const& other) :
    _data(other._data),
    _i2oMessageCode(other._i2oMessageCode),
    _fragKey(other._fragKey)
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
    std::swap(_i2oMessageCode, other._i2oMessageCode);
    std::swap(_fragKey, other._fragKey);
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

  void I2OChain::release()
  {
    // A default-constructed chain controls no resources; we can
    // relinquish our control over any controlled Reference by
    // becoming like a default-constructed chain.
    I2OChain().swap(*this);
  }

  void I2OChain::parseI2OHeader()
  {
    if (empty())
      {
        _i2oMessageCode = 0;
        _fragKey.code_ = 0;
        _fragKey.run_ = 0;
        _fragKey.event_ = 0;
        _fragKey.secondaryId_ = 0;
        _fragKey.originatorPid_ = 0;
        _fragKey.originatorGuid_ = 0;
      }
    else
      {
        I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
          (I2O_PRIVATE_MESSAGE_FRAME*) getBufferData();
        _i2oMessageCode = pvtMsg->XFunctionCode;

        switch (_i2oMessageCode)
          {

          case I2O_SM_PREAMBLE:
            {
              I2O_SM_PREAMBLE_MESSAGE_FRAME *i2oMsg =
                (I2O_SM_PREAMBLE_MESSAGE_FRAME*) pvtMsg;
              _fragKey.code_ = Header::INIT;
              _fragKey.run_ = 0;
              _fragKey.event_ = i2oMsg->hltTid;
              _fragKey.secondaryId_ = i2oMsg->outModID;
              _fragKey.originatorPid_ = i2oMsg->fuProcID;
              _fragKey.originatorGuid_ = i2oMsg->fuGUID;
              break;
            }

          case I2O_SM_DATA:
            {
              I2O_SM_DATA_MESSAGE_FRAME *i2oMsg =
                (I2O_SM_DATA_MESSAGE_FRAME*) pvtMsg;
              _fragKey.code_ = Header::EVENT;
              _fragKey.run_ = i2oMsg->runID;
              _fragKey.event_ = i2oMsg->eventID;
              _fragKey.secondaryId_ = i2oMsg->outModID;
              _fragKey.originatorPid_ = i2oMsg->fuProcID;
              _fragKey.originatorGuid_ = i2oMsg->fuGUID;
              break;
            }

          case I2O_SM_ERROR:
            {
              I2O_SM_DATA_MESSAGE_FRAME *i2oMsg =
                (I2O_SM_DATA_MESSAGE_FRAME*) pvtMsg;
              _fragKey.code_ = Header::ERROR_EVENT;
              _fragKey.run_ = i2oMsg->runID;
              _fragKey.event_ = i2oMsg->eventID;
              _fragKey.secondaryId_ = i2oMsg->outModID;
              _fragKey.originatorPid_ = i2oMsg->fuProcID;
              _fragKey.originatorGuid_ = i2oMsg->fuGUID;
              break;
            }

          case I2O_SM_DQM:
            {
              I2O_SM_DQM_MESSAGE_FRAME *i2oMsg =
                (I2O_SM_DQM_MESSAGE_FRAME*) pvtMsg;
              _fragKey.code_ = Header::DQM_EVENT;
              _fragKey.run_ = i2oMsg->runID;
              _fragKey.event_ = i2oMsg->eventAtUpdateID;
              _fragKey.secondaryId_ = i2oMsg->folderID;
              _fragKey.originatorPid_ = i2oMsg->fuProcID;
              _fragKey.originatorGuid_ = i2oMsg->fuGUID;
              break;
            }

          default:
            {
              _fragKey.code_ = 0;
              _fragKey.run_ = 0;
              _fragKey.event_ = 0;
              _fragKey.secondaryId_ = 0;
              _fragKey.originatorPid_ = 0;
              _fragKey.originatorGuid_ = 0;
              break;
            }

          }
      }
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
