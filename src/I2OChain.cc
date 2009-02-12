// $Id: I2OChain.cc,v 1.1.2.8 2009/02/11 21:00:50 biery Exp $

#include <algorithm>
#include "EventFilter/StorageManager/interface/Exception.h"
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
      enum BitMasksForFaulty { CORRUPT_HEADER = 0x1,
                               TOTAL_COUNT_MISMATCH = 0x2,
                               DUPLICATE_FRAGMENT = 0x4,
                               INCOMPLETE_MESSAGE = 0x8,
                               EXTERNALLY_REQUESTED = 0x10000 };

    public:
      explicit ChainData(toolbox::mem::Reference* pRef);
      virtual ~ChainData();
      bool empty() const;
      bool complete() const;
      bool faulty() const;
      void addToChain(ChainData const& newpart);
      void markComplete();
      void markFaulty();
      void markCorrupt();
      unsigned long* getBufferData();
      void swap(ChainData& other);
      unsigned char getMessageCode() const {return _messageCode;}
      FragKey const& getFragmentKey() const {return _fragKey;}
      unsigned int getFragmentCount() const {return _fragmentCount;}

    private:
      std::vector<QueueID> _streamTags;
      std::vector<QueueID> _dqmEventConsumerTags;
      std::vector<QueueID> _eventConsumerTags;

    protected:
      toolbox::mem::Reference* _ref;

      bool _complete;
      unsigned int _faultyBits;

      unsigned char _messageCode;
      FragKey _fragKey;
      unsigned int _fragmentCount;
      unsigned int _expectedNumberOfFragments;
    };

    // A ChainData object may or may not contain a Reference.
    inline ChainData::ChainData(toolbox::mem::Reference* pRef) :
      _streamTags(),
      _dqmEventConsumerTags(),
      _eventConsumerTags(),
      _ref(pRef),
      _complete(false),
      _faultyBits(0),
      _messageCode(Header::INVALID),
      _fragKey(Header::INVALID,0,0,0,0,0),
      _fragmentCount(0),
      _expectedNumberOfFragments(1)
    {
      if (pRef)
        {
          _fragmentCount = 1;

          I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
            (I2O_SM_MULTIPART_MESSAGE_FRAME*) pRef->getDataLocation();
          _expectedNumberOfFragments = smMsg->numFrames;
          if (smMsg->numFrames < 1 ||
              smMsg->frameCount >= smMsg->numFrames) {
            markCorrupt();
          }
          else if (smMsg->numFrames == 1) {
            markComplete();
          }
        }
    }

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
          //std::cout << std::endl << std::endl << std::hex
          //          << "### releasing 0x" << ((int) _ref)
          //          << std::dec << std::endl << std::endl;
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

    inline bool ChainData::faulty() const
    {
      return (_faultyBits != 0);
    }

    inline void ChainData::addToChain(ChainData const& newpart)
    {
      // this method expects to be called with non-null _ref
      // for both the current chain and the new part!

      bool fragmentWasAdded = false;

      // determine the index of the new fragment
      I2O_SM_MULTIPART_MESSAGE_FRAME *thatMsg =
        (I2O_SM_MULTIPART_MESSAGE_FRAME*) newpart._ref->getDataLocation();
      unsigned int newIndex = thatMsg->frameCount;
      //std::cout << "newIndex = " << newIndex << std::endl;

      // verify that the total fragment counts match
      unsigned int newFragmentTotalCount = thatMsg->numFrames;
      if (newFragmentTotalCount != _expectedNumberOfFragments)
        {
          _faultyBits |= TOTAL_COUNT_MISMATCH;
        }

      // if the new fragment goes at the head of the chain, handle that here
      I2O_SM_MULTIPART_MESSAGE_FRAME *fragMsg =
        (I2O_SM_MULTIPART_MESSAGE_FRAME*) _ref->getDataLocation();
      unsigned int firstIndex = fragMsg->frameCount;
      //std::cout << "firstIndex = " << firstIndex << std::endl;
      if (newIndex < firstIndex)
        {
          newpart._ref->setNextReference(_ref);
          _ref = newpart._ref->duplicate();
          fragmentWasAdded = true;
        }

      else
        {
          // loop over the existing fragments and insert the new one
          // in the correct place
          toolbox::mem::Reference* curRef = _ref;
          for (unsigned int idx = 0; idx < _fragmentCount; ++idx)
            {
              // if we have a duplicate fragment, add it after the existing
              // one and indicate the error
              I2O_SM_MULTIPART_MESSAGE_FRAME *curMsg =
                (I2O_SM_MULTIPART_MESSAGE_FRAME*) curRef->getDataLocation();
              unsigned int curIndex = curMsg->frameCount;
              //std::cout << "curIndex = " << curIndex << std::endl;
              if (newIndex == curIndex) 
                {
                  _faultyBits |= DUPLICATE_FRAGMENT;
                  newpart._ref->setNextReference(curRef->getNextReference());
                  curRef->setNextReference(newpart._ref->duplicate());
                  fragmentWasAdded = true;
                  break;
                }

              // if we have reached the end of the chain, add the
              // new fragment to the end
              //std::cout << "nextRef = " << ((int) nextRef) << std::endl;
              toolbox::mem::Reference* nextRef = curRef->getNextReference();
              if (nextRef == 0)
                {
                  curRef->setNextReference(newpart._ref->duplicate());
                  fragmentWasAdded = true;
                  break;
                }

              I2O_SM_MULTIPART_MESSAGE_FRAME *nextMsg =
                (I2O_SM_MULTIPART_MESSAGE_FRAME*) nextRef->getDataLocation();
              unsigned int nextIndex = nextMsg->frameCount;
              //std::cout << "nextIndex = " << nextIndex << std::endl;
              if (newIndex > curIndex && newIndex < nextIndex)
                {
                  newpart._ref->setNextReference(curRef->getNextReference());
                  curRef->setNextReference(newpart._ref->duplicate());
                  fragmentWasAdded = true;
                  break;
                }

              curRef = nextRef;
            }
        }

      // update the fragment count and check if the chain is now complete
      if (! fragmentWasAdded)
        {
          // this should never happen - if it does, there is a logic
          // error in the loop above
          XCEPT_RAISE(stor::exception::Exception,
                      "A fragment was unable to be added to a chain.");
        }
      ++_fragmentCount;
      if (! faulty() && _fragmentCount == _expectedNumberOfFragments)
        {
          markComplete();
        }
    }

    inline void ChainData::markComplete()
    {
      _complete = true;
    }

    inline void ChainData::markFaulty()
    {
      _faultyBits |= EXTERNALLY_REQUESTED;
    }

    inline void ChainData::markCorrupt()
    {
      _faultyBits |= CORRUPT_HEADER;
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
      std::swap(_faultyBits, other._faultyBits);
      std::swap(_messageCode, other._messageCode);
      std::swap(_fragKey, other._fragKey);
      std::swap(_fragmentCount, other._fragmentCount);
      std::swap(_expectedNumberOfFragments, other._expectedNumberOfFragments);
    }

    class InitMsgData : public ChainData
    {
    public:
      explicit InitMsgData(toolbox::mem::Reference* pRef);
      ~InitMsgData() {}

    private:
      void parseI2OHeader();
    };

    inline InitMsgData::InitMsgData(toolbox::mem::Reference* pRef) :
      ChainData(pRef)
    {
      parseI2OHeader();
    }

    inline void InitMsgData::parseI2OHeader()
    {
      if (_ref)
        {
          _messageCode = Header::INIT;
          I2O_SM_PREAMBLE_MESSAGE_FRAME *i2oMsg =
            (I2O_SM_PREAMBLE_MESSAGE_FRAME*) _ref->getDataLocation();
          _fragKey.code_ = _messageCode;
          _fragKey.run_ = 0;
          _fragKey.event_ = i2oMsg->hltTid;
          _fragKey.secondaryId_ = i2oMsg->outModID;
          _fragKey.originatorPid_ = i2oMsg->fuProcID;
          _fragKey.originatorGuid_ = i2oMsg->fuGUID;
        }
    }

    class EventMsgData : public ChainData
    {
    public:
      explicit EventMsgData(toolbox::mem::Reference* pRef);
      ~EventMsgData() {}

    private:
      void parseI2OHeader();
    };

    inline EventMsgData::EventMsgData(toolbox::mem::Reference* pRef) :
      ChainData(pRef)
    {
      parseI2OHeader();
    }

    inline void EventMsgData::parseI2OHeader()
    {
      if (_ref)
        {
          _messageCode = Header::EVENT;
          I2O_SM_DATA_MESSAGE_FRAME *i2oMsg =
            (I2O_SM_DATA_MESSAGE_FRAME*) _ref->getDataLocation();
          _fragKey.code_ = _messageCode;
          _fragKey.run_ = i2oMsg->runID;
          _fragKey.event_ = i2oMsg->eventID;
          _fragKey.secondaryId_ = i2oMsg->outModID;
          _fragKey.originatorPid_ = i2oMsg->fuProcID;
          _fragKey.originatorGuid_ = i2oMsg->fuGUID;
        }
    }

    class DQMEventMsgData : public ChainData
    {
    public:
      explicit DQMEventMsgData(toolbox::mem::Reference* pRef);
      ~DQMEventMsgData() {}

    private:
      void parseI2OHeader();
    };

    inline DQMEventMsgData::DQMEventMsgData(toolbox::mem::Reference* pRef) :
      ChainData(pRef)
    {
      parseI2OHeader();
    }

    inline void DQMEventMsgData::parseI2OHeader()
    {
      if (_ref)
        {
          _messageCode = Header::DQM_EVENT;
          I2O_SM_DQM_MESSAGE_FRAME *i2oMsg =
            (I2O_SM_DQM_MESSAGE_FRAME*) _ref->getDataLocation();
          _fragKey.code_ = _messageCode;
          _fragKey.run_ = i2oMsg->runID;
          _fragKey.event_ = i2oMsg->eventAtUpdateID;
          _fragKey.secondaryId_ = i2oMsg->folderID;
          _fragKey.originatorPid_ = i2oMsg->fuProcID;
          _fragKey.originatorGuid_ = i2oMsg->fuGUID;
        }
    }

    class ErrorEventMsgData : public ChainData
    {
    public:
      explicit ErrorEventMsgData(toolbox::mem::Reference* pRef);
      ~ErrorEventMsgData() {}

    private:
      void parseI2OHeader();
    };

    inline ErrorEventMsgData::ErrorEventMsgData(toolbox::mem::Reference* pRef) :
      ChainData(pRef)
    {
      parseI2OHeader();
    }

    inline void ErrorEventMsgData::parseI2OHeader()
    {
      if (_ref)
        {
          _messageCode = Header::ERROR_EVENT;
          I2O_SM_DATA_MESSAGE_FRAME *i2oMsg =
            (I2O_SM_DATA_MESSAGE_FRAME*) _ref->getDataLocation();
          _fragKey.code_ = _messageCode;
          _fragKey.run_ = i2oMsg->runID;
          _fragKey.event_ = i2oMsg->eventID;
          _fragKey.secondaryId_ = i2oMsg->outModID;
          _fragKey.originatorPid_ = i2oMsg->fuProcID;
          _fragKey.originatorGuid_ = i2oMsg->fuGUID;
        }
    }
  } // namespace detail


  // A default-constructed I2OChain has a null (shared) pointer.
  I2OChain::I2OChain() :
    _data()
  { 
  }

  I2OChain::I2OChain(toolbox::mem::Reference* pRef)
  {
    if (pRef)
      {
        I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
          (I2O_PRIVATE_MESSAGE_FRAME*) pRef->getDataLocation();
        unsigned short i2oMessageCode = pvtMsg->XFunctionCode;

        switch (i2oMessageCode)
          {

          case I2O_SM_PREAMBLE:
            {
              _data.reset(new detail::InitMsgData(pRef));
              break;
            }

          case I2O_SM_DATA:
            {
              _data.reset(new detail::EventMsgData(pRef));
              break;
            }

          case I2O_SM_DQM:
            {
              _data.reset(new detail::DQMEventMsgData(pRef));
              break;
            }

          case I2O_SM_ERROR:
            {
              _data.reset(new detail::ErrorEventMsgData(pRef));
              break;
            }

          default:
            {
              _data.reset(new detail::ChainData(pRef));
              _data->markCorrupt();
              break;
            }

          }
      }
  }

  I2OChain::I2OChain(I2OChain const& other) :
    _data(other._data)
  { }

  I2OChain::~I2OChain()
  { }

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


  bool I2OChain::faulty() const
  {
    if (!_data) return false;
    return _data->faulty();
  }


  void I2OChain::addToChain(I2OChain &newpart)
  {
    // fragments can not be added to empty, complete, or faulty chains.
    if (empty())
      {
        XCEPT_RAISE(stor::exception::Exception,
                    "A fragment may not be added to an empty chain.");
      }
    if (complete())
      {
        XCEPT_RAISE(stor::exception::Exception,
                    "A fragment may not be added to a complete chain.");
      }
    if (faulty())
      {
        XCEPT_RAISE(stor::exception::Exception,
                    "A fragment may not be added to a faulty chain.");
      }

    // empty, complete, or faulty new parts can not be added to chains
    if (newpart.empty())
      {
        XCEPT_RAISE(stor::exception::Exception,
                    "An empty chain may not be added to an existing chain.");
      }
    if (newpart.complete())
      {
        XCEPT_RAISE(stor::exception::Exception,
                    "A complete chain may not be added to an existing chain.");
      }
    if (newpart.faulty())
      {
        XCEPT_RAISE(stor::exception::Exception,
                    "A faulty chain may not be added to an existing chain.");
      }

    // require the new part and this chain to have the same fragment key
    FragKey thisKey = getFragmentKey();
    FragKey thatKey = newpart.getFragmentKey();
    // should change this to != once we implement that operator in FragKey
    if (thisKey < thatKey || thatKey < thisKey)
      {
        std::stringstream msg;
        msg << "A fragment key mismatch was detected when trying to add "
            << "a chain link to an existing chain. "
            << "Existing key values = ("
            << thisKey.code_ << "," << thisKey.run_ << ","
            << thisKey.event_ << "," << thisKey.secondaryId_ << ","
            << thisKey.originatorPid_ << "," << thisKey.originatorGuid_
            << "), new key values = ("
            << thatKey.code_ << "," << thatKey.run_ << ","
            << thatKey.event_ << "," << thatKey.secondaryId_ << ","
            << thatKey.originatorPid_ << "," << thatKey.originatorGuid_
            << ").";
        XCEPT_RAISE(stor::exception::Exception, msg.str());
      }

    // add the fragment to the current chain
    _data->addToChain(*(newpart._data));
    newpart.release();
  }

  //void I2OChain::markComplete()
  //{
  //  // TODO:: Should we throw an exception if _data is null? If so, what
  //  // type? Right now, we do nothing if _data is null.
  //  if (_data) _data->markComplete();
  //}

  void I2OChain::markFaulty()
  {
    // TODO:: Should we throw an exception if _data is null? If so, what
    // type? Right now, we do nothing if _data is null.
    if (_data) _data->markFaulty();
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

  unsigned char I2OChain::getMessageCode() const
  {
    if (!_data) return Header::INVALID;
    return _data->getMessageCode();
  }

  FragKey I2OChain::getFragmentKey() const
  {
    if (!_data) return FragKey(Header::INVALID,0,0,0,0,0);
    return _data->getFragmentKey();
  }

  unsigned int I2OChain::getFragmentCount() const
  {
    if (!_data) return 0;
    return _data->getFragmentCount();
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
