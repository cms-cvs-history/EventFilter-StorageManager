// $Id: I2OChain.cc,v 1.1.2.22 2009/02/25 19:51:38 biery Exp $

#include <algorithm>
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/Types.h"
#include "EventFilter/StorageManager/interface/Utils.h"
#include "EventFilter/Utilities/interface/i2oEvfMsgs.h"
#include "IOPool/Streamer/interface/MsgHeader.h"
#include "IOPool/Streamer/interface/InitMessage.h"

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
      enum BitMasksForFaulty { INVALID_INITIAL_REFERENCE = 0x1,
                               CORRUPT_INITIAL_HEADER = 0x2,
                               INVALID_SECONDARY_REFERENCE = 0x4,
                               CORRUPT_SECONDARY_HEADER = 0x8,
                               TOTAL_COUNT_MISMATCH = 0x10,
                               FRAGMENTS_OUT_OF_ORDER = 0x20,
                               DUPLICATE_FRAGMENT = 0x40,
                               INCOMPLETE_MESSAGE = 0x80,
                               EXTERNALLY_REQUESTED = 0x10000 };

    public:
      explicit ChainData(toolbox::mem::Reference* pRef);
      virtual ~ChainData();
      bool empty() const;
      bool complete() const;
      bool faulty() const;
      bool parsable() const;
      void addToChain(ChainData const& newpart);
      void markComplete();
      void markFaulty();
      void markCorrupt();
      unsigned long* getBufferData();
      void swap(ChainData& other);
      unsigned int messageCode() const {return _messageCode;}
      FragKey const& fragmentKey() const {return _fragKey;}
      unsigned int fragmentCount() const {return _fragmentCount;}
      unsigned int rbBufferId() const {return _rbBufferId;}
      double creationTime() const {return _creationTime;}
      double lastFragmentTime() const {return _lastFragmentTime;}
      unsigned long totalDataSize() const;
      unsigned long dataSize(int fragmentIndex) const;
      unsigned char* dataLocation(int fragmentIndex) const;
      unsigned int getFragmentID(int fragmentIndex) const;
      unsigned int copyFragmentsIntoBuffer(std::vector<unsigned char>& buff) const;

      std::string outputModuleLabel() const;
      uint32 outputModuleId() const;
      void hltTriggerNames(Strings& nameList) const;
      void hltTriggerSelections(Strings& nameList) const;
      void l1TriggerNames(Strings& nameList) const;

    private:
      std::vector<QueueID> _streamTags;
      std::vector<QueueID> _dqmEventConsumerTags;
      std::vector<QueueID> _eventConsumerTags;

    protected:
      toolbox::mem::Reference* _ref;

      bool _complete;
      unsigned int _faultyBits;

      unsigned int _messageCode;
      FragKey _fragKey;
      unsigned int _fragmentCount;
      unsigned int _expectedNumberOfFragments;
      unsigned int _rbBufferId;

      double _creationTime;
      double _lastFragmentTime;

      bool validateDataLocation(toolbox::mem::Reference* ref,
                                BitMasksForFaulty maskToUse);
      bool validateMessageSize(toolbox::mem::Reference* ref,
                               BitMasksForFaulty maskToUse);
      bool validateFragmentIndexAndCount(toolbox::mem::Reference* ref,
                                         BitMasksForFaulty maskToUse);
      bool validateExpectedFragmentCount(toolbox::mem::Reference* ref,
                                         BitMasksForFaulty maskToUse);
      bool validateFragmentOrder(toolbox::mem::Reference* ref,
                                 int& indexValue);
      bool validateMessageCode(toolbox::mem::Reference* ref,
                               unsigned short expectedI2OMessageCode);

      virtual unsigned char* do_fragmentLocation(unsigned char* dataLoc) const;

      virtual std::string do_outputModuleLabel() const;
      virtual uint32 do_outputModuleId() const;
      virtual void do_hltTriggerNames(Strings& nameList) const;
      virtual void do_hltTriggerSelections(Strings& nameList) const;
      virtual void do_l1TriggerNames(Strings& nameList) const;
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
      _expectedNumberOfFragments(0),
      _rbBufferId(0),
      _creationTime(-1),
      _lastFragmentTime(-1)
    {
      // Avoid the situation in which all unparsable chains
      // have the same fragment key.  We do this by providing a 
      // variable default value for one of the fragKey fields.
      if (pRef)
        {
          _fragKey.secondaryId_ = (uint32) pRef->getDataLocation();
        }
      else
        {
          _fragKey.secondaryId_ = (uint32) time(0);
        }

      if (pRef)
        {
          _creationTime = utils::getCurrentTime();
          _lastFragmentTime = _creationTime;

          // first fragment in Reference chain
          ++_fragmentCount;
          int workingIndex = -1;

          if (validateDataLocation(pRef, INVALID_INITIAL_REFERENCE) &&
              validateMessageSize(pRef, CORRUPT_INITIAL_HEADER) &&
              validateFragmentIndexAndCount(pRef, CORRUPT_INITIAL_HEADER))
            {
              I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
                (I2O_SM_MULTIPART_MESSAGE_FRAME*) pRef->getDataLocation();
              _expectedNumberOfFragments = smMsg->numFrames;

              validateFragmentOrder(pRef, workingIndex);
            }

          // subsequent fragments in Reference chain
          toolbox::mem::Reference* curRef = pRef->getNextReference();
          while (curRef)
            {
              ++_fragmentCount;

              if (validateDataLocation(curRef, INVALID_SECONDARY_REFERENCE) &&
                  validateMessageSize(curRef, CORRUPT_SECONDARY_HEADER) &&
                  validateFragmentIndexAndCount(curRef, CORRUPT_SECONDARY_HEADER))
                {
                  validateExpectedFragmentCount(curRef, TOTAL_COUNT_MISMATCH);
                  validateFragmentOrder(curRef, workingIndex);
                }

              curRef = curRef->getNextReference();
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

    inline bool ChainData::parsable() const
    {
      return (_ref) && ((_faultyBits & INVALID_INITIAL_REFERENCE) == 0) &&
        ((_faultyBits & CORRUPT_INITIAL_HEADER) == 0);
    }

    // this method currently does NOT support operation on empty chains
    // (both the existing chain and the new part must be non-empty!)
    void ChainData::addToChain(ChainData const& newpart)
    {
      // if either the current chain or the newpart are faulty, we
      // simply append the new stuff to the end of the existing chain
      if (faulty() || newpart.faulty())
        {
          // update our fragment count to include the new fragments
          toolbox::mem::Reference* curRef = newpart._ref;
          while (curRef) {
            ++_fragmentCount;
            curRef = curRef->getNextReference();
          }

          // append the new fragments to the end of the existing chain
          toolbox::mem::Reference* lastRef = _ref;
          curRef = _ref->getNextReference();
          while (curRef) {
            lastRef = curRef;
            curRef = curRef->getNextReference();
          }
          lastRef->setNextReference(newpart._ref->duplicate());

          // merge the faulty flags from the new part into the existing flags
          _faultyBits |= newpart._faultyBits;

          // update the time stamps
          _lastFragmentTime = utils::getCurrentTime();
          if (newpart.creationTime() < _creationTime)
          {
            _creationTime = newpart.creationTime();
          }

          return;
        }

      // loop over the fragments in the new part
      toolbox::mem::Reference* newRef = newpart._ref;
      while (newRef)
        {
          // unlink the next element in the new chain from that chain
          toolbox::mem::Reference* nextNewRef = newRef->getNextReference();
          newRef->setNextReference(0);

          // if the new fragment that we're working with is the first one
          // in its chain, we need to duplicate it (now that it is unlinked)
          // This is necessary since it is still being managed by an I2OChain
          // somewhere.  The subsequent fragments in the new part do not
          // need to be duplicated since we explicitly unlinked them from
          // the first one.
          if (newRef == newpart._ref) {newRef = newpart._ref->duplicate();}

          // we want to track whether the fragment was added (it *always* should be)
          bool fragmentWasAdded = false;

          // determine the index of the new fragment
          I2O_SM_MULTIPART_MESSAGE_FRAME *thatMsg =
            (I2O_SM_MULTIPART_MESSAGE_FRAME*) newRef->getDataLocation();
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
              newRef->setNextReference(_ref);
              _ref = newRef;
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
                      newRef->setNextReference(curRef->getNextReference());
                      curRef->setNextReference(newRef);
                      fragmentWasAdded = true;
                      break;
                    }

                  // if we have reached the end of the chain, add the
                  // new fragment to the end
                  //std::cout << "nextRef = " << ((int) nextRef) << std::endl;
                  toolbox::mem::Reference* nextRef = curRef->getNextReference();
                  if (nextRef == 0)
                    {
                      curRef->setNextReference(newRef);
                      fragmentWasAdded = true;
                      break;
                    }

                  I2O_SM_MULTIPART_MESSAGE_FRAME *nextMsg =
                    (I2O_SM_MULTIPART_MESSAGE_FRAME*) nextRef->getDataLocation();
                  unsigned int nextIndex = nextMsg->frameCount;
                  //std::cout << "nextIndex = " << nextIndex << std::endl;
                  if (newIndex > curIndex && newIndex < nextIndex)
                    {
                      newRef->setNextReference(curRef->getNextReference());
                      curRef->setNextReference(newRef);
                      fragmentWasAdded = true;
                      break;
                    }

                  curRef = nextRef;
                }
            }

          // update the fragment count and check if the chain is now complete
          if (!fragmentWasAdded)
            {
              // this should never happen - if it does, there is a logic
              // error in the loop above
              XCEPT_RAISE(stor::exception::I2OChain,
                          "A fragment was unable to be added to a chain.");
            }
          ++_fragmentCount;
          
          newRef = nextNewRef;
        }

      // update the time stamps
      _lastFragmentTime = utils::getCurrentTime();
      if (newpart.creationTime() < _creationTime)
      {
        _creationTime = newpart.creationTime();
      }
      
      if (!faulty() && _fragmentCount == _expectedNumberOfFragments)
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
      _faultyBits |= CORRUPT_INITIAL_HEADER;
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
      std::swap(_creationTime, other._creationTime);
      std::swap(_lastFragmentTime, other._lastFragmentTime);
    }

    unsigned long ChainData::totalDataSize() const
    {
      unsigned long totalSize = 0;
      toolbox::mem::Reference* curRef = _ref;
      for (unsigned int idx = 0; idx < _fragmentCount; ++idx)
        {
          I2O_MESSAGE_FRAME *i2oMsg =
            (I2O_MESSAGE_FRAME*) curRef->getDataLocation();
          if (!faulty())
            {
              I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
                (I2O_SM_MULTIPART_MESSAGE_FRAME*) i2oMsg;
              totalSize += smMsg->dataSize;
            }
          else if (i2oMsg)
            {
              totalSize += (i2oMsg->MessageSize*4);
            }

          curRef = curRef->getNextReference();
        }
      return totalSize;
    }

    unsigned long ChainData::dataSize(int fragmentIndex) const
    {
      toolbox::mem::Reference* curRef = _ref;
      for (int idx = 0; idx < fragmentIndex; ++idx)
        {
          curRef = curRef->getNextReference();
        }

      I2O_MESSAGE_FRAME *i2oMsg =
        (I2O_MESSAGE_FRAME*) curRef->getDataLocation();
      if (!faulty())
        {
          I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
            (I2O_SM_MULTIPART_MESSAGE_FRAME*) i2oMsg;
          return smMsg->dataSize;
        }
      else if (i2oMsg)
        {
          return (i2oMsg->MessageSize*4);
        }
      return 0;
    }

    unsigned char* ChainData::dataLocation(int fragmentIndex) const
    {
      toolbox::mem::Reference* curRef = _ref;
      for (int idx = 0; idx < fragmentIndex; ++idx)
        {
          curRef = curRef->getNextReference();
        }

      if (!faulty())
        {
          return do_fragmentLocation(static_cast<unsigned char*>
                                       (curRef->getDataLocation()));
        }
      else
        {
          return static_cast<unsigned char*>(curRef->getDataLocation());
        }
    }

    unsigned int ChainData::getFragmentID(int fragmentIndex) const
    {
      toolbox::mem::Reference* curRef = _ref;
      for (int idx = 0; idx < fragmentIndex; ++idx)
        {
          curRef = curRef->getNextReference();
        }

      if (parsable())
        {
          I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
            (I2O_SM_MULTIPART_MESSAGE_FRAME*) curRef->getDataLocation();
          return smMsg->frameCount;
        }
      else
        {
          return 0;
        }
    }

    unsigned int ChainData::
    copyFragmentsIntoBuffer(std::vector<unsigned char>& targetBuffer) const
    {
      unsigned long fullSize = totalDataSize();
      if (targetBuffer.capacity() < fullSize)
        {
          targetBuffer.resize(fullSize);
        }
      unsigned char* targetLoc = (unsigned char*)&targetBuffer[0];

      toolbox::mem::Reference* curRef = _ref;
      while (curRef)
        {
          unsigned char* fragmentLoc =
            (unsigned char*) curRef->getDataLocation();
          unsigned long sourceSize = 0;
          unsigned char* sourceLoc = 0;

          if (!faulty())
            {
              I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
                (I2O_SM_MULTIPART_MESSAGE_FRAME*) fragmentLoc;
              sourceSize = smMsg->dataSize;
              sourceLoc = do_fragmentLocation(fragmentLoc);
            }
          else if (fragmentLoc)
            {
              I2O_MESSAGE_FRAME *i2oMsg = (I2O_MESSAGE_FRAME*) fragmentLoc;
              sourceSize = i2oMsg->MessageSize * 4;
              sourceLoc = fragmentLoc;
            }

          if (sourceSize > 0)
            {
              std::copy(sourceLoc, sourceLoc+sourceSize, targetLoc);
              targetLoc += sourceSize;
            }

          curRef = curRef->getNextReference();
        }

      return static_cast<unsigned int>(fullSize);
    }

    std::string ChainData::outputModuleLabel() const
    {
      return do_outputModuleLabel();
    }

    uint32 ChainData::outputModuleId() const
    {
      return do_outputModuleId();
    }

    void ChainData::hltTriggerNames(Strings& nameList) const
    {
      do_hltTriggerNames(nameList);
    }

    void ChainData::hltTriggerSelections(Strings& nameList) const
    {
      do_hltTriggerSelections(nameList);
    }

    void ChainData::l1TriggerNames(Strings& nameList) const
    {
      do_l1TriggerNames(nameList);
    }

    inline bool
    ChainData::validateDataLocation(toolbox::mem::Reference* ref,
                                    BitMasksForFaulty maskToUse)
    {
      I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
        (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
      if (!pvtMsg)
        {
          _faultyBits |= maskToUse;
          return false;
        }
      return true;
    }

    inline bool
    ChainData::validateMessageSize(toolbox::mem::Reference* ref,
                                   BitMasksForFaulty maskToUse)
    {
      I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
        (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
      if ((size_t)(pvtMsg->StdMessageFrame.MessageSize*4) <
          sizeof(I2O_SM_MULTIPART_MESSAGE_FRAME))
        {
          _faultyBits |= maskToUse;
          return false;
        }
      return true;
    }

    inline bool
    ChainData::validateFragmentIndexAndCount(toolbox::mem::Reference* ref,
                                             BitMasksForFaulty maskToUse)
    {
      I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
        (I2O_SM_MULTIPART_MESSAGE_FRAME*) ref->getDataLocation();
      if (smMsg->numFrames < 1 || smMsg->frameCount >= smMsg->numFrames)
        {
          _faultyBits |= maskToUse;
          return false;
        }
      return true;
    }

    inline bool
    ChainData::validateExpectedFragmentCount(toolbox::mem::Reference* ref,
                                             BitMasksForFaulty maskToUse)
    {
      I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
        (I2O_SM_MULTIPART_MESSAGE_FRAME*) ref->getDataLocation();
      if (smMsg->numFrames != _expectedNumberOfFragments)
        {
          _faultyBits |= maskToUse;
          return false;
        }
      return true;
    }

    inline bool
    ChainData::validateFragmentOrder(toolbox::mem::Reference* ref,
                                     int& indexValue)
    {
      int problemCount = 0;
      I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
        (I2O_SM_MULTIPART_MESSAGE_FRAME*) ref->getDataLocation();
      int thisIndex = static_cast<int>(smMsg->frameCount);
      if (thisIndex == indexValue)
        {
          _faultyBits |= DUPLICATE_FRAGMENT;
          ++problemCount;
        }
      else if (thisIndex < indexValue)
        {
          _faultyBits |= FRAGMENTS_OUT_OF_ORDER;
          ++problemCount;
        }
      indexValue = thisIndex;
      return (problemCount == 0);
    }

    inline bool
    ChainData::validateMessageCode(toolbox::mem::Reference* ref,
                                   unsigned short expectedI2OMessageCode)
    {
      I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
        (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
      if (pvtMsg->XFunctionCode != expectedI2OMessageCode)
        {
          _faultyBits |= CORRUPT_SECONDARY_HEADER;
          return false;
        }
      return true;
    }

    inline unsigned char*
    ChainData::do_fragmentLocation(unsigned char* dataLoc) const
    {
      return dataLoc;
    }

    inline std::string ChainData::do_outputModuleLabel() const
    {
      std::stringstream msg;
      msg << "An output module label is only available from a valid, ";
      msg << "complete INIT message.";
      XCEPT_RAISE(stor::exception::WrongI2OMessageType, msg.str());
    }

    inline uint32 ChainData::do_outputModuleId() const
    {
      std::stringstream msg;
      msg << "An output module ID is only available from a valid, ";
      msg << "complete INIT message.";
      XCEPT_RAISE(stor::exception::WrongI2OMessageType, msg.str());
    }

    inline void ChainData::do_hltTriggerNames(Strings& nameList) const
    {
      std::stringstream msg;
      msg << "The HLT trigger names are only available from a valid, ";
      msg << "complete INIT message.";
      XCEPT_RAISE(stor::exception::WrongI2OMessageType, msg.str());
    }

    inline void ChainData::do_hltTriggerSelections(Strings& nameList) const
    {
      std::stringstream msg;
      msg << "The HLT trigger selections are only available from a valid, ";
      msg << "complete INIT message.";
      XCEPT_RAISE(stor::exception::WrongI2OMessageType, msg.str());
    }

    inline void ChainData::do_l1TriggerNames(Strings& nameList) const
    {
      std::stringstream msg;
      msg << "The L1 trigger names are only available from a valid, ";
      msg << "complete INIT message.";
      XCEPT_RAISE(stor::exception::WrongI2OMessageType, msg.str());
    }

    class InitMsgData : public ChainData
    {
    public:
      explicit InitMsgData(toolbox::mem::Reference* pRef);
      ~InitMsgData() {}

    protected:
      unsigned char* do_fragmentLocation(unsigned char* dataLoc) const;
      std::string do_outputModuleLabel() const;
      uint32 do_outputModuleId() const;
      void do_hltTriggerNames(Strings& nameList) const;
      void do_hltTriggerSelections(Strings& nameList) const;
      void do_l1TriggerNames(Strings& nameList) const;

    private:
      void parseI2OHeader();
      void cacheHeaderFields() const;

      mutable bool _headerFieldsCached;
      mutable std::string _outputModuleLabel;
      mutable uint32 _outputModuleId;
      mutable Strings _hltTriggerNames;
      mutable Strings _hltTriggerSelections;
      mutable Strings _l1TriggerNames;
    };

    inline InitMsgData::InitMsgData(toolbox::mem::Reference* pRef) :
      ChainData(pRef),
      _headerFieldsCached(false)
    {
      parseI2OHeader();

      if (_fragmentCount > 1)
        {
          toolbox::mem::Reference* curRef = _ref->getNextReference();
          while (curRef)
            {
              validateMessageCode(curRef, I2O_SM_PREAMBLE);
              curRef = curRef->getNextReference();
            }
        }

      if (!faulty() && _fragmentCount == _expectedNumberOfFragments)
        {
          markComplete();
        }
    }

    inline unsigned char*
    InitMsgData::do_fragmentLocation(unsigned char* dataLoc) const
    {
      if (parsable())
        {
          I2O_SM_PREAMBLE_MESSAGE_FRAME *smMsg =
            (I2O_SM_PREAMBLE_MESSAGE_FRAME*) dataLoc;
          return (unsigned char*) smMsg->dataPtr();
        }
      else
        {
          return dataLoc;
        }
    }

    std::string InitMsgData::do_outputModuleLabel() const
    {
      if (faulty() || !complete())
        {
          std::stringstream msg;
          msg << "An output module label can not be determined from a ";
          msg << "faulty or incomplete INIT message.";
          XCEPT_RAISE(stor::exception::IncompleteInitMessage, msg.str());
        }

      if (! _headerFieldsCached) {cacheHeaderFields();}
      return _outputModuleLabel;
    }

    uint32 InitMsgData::do_outputModuleId() const
    {
      if (faulty() || !complete())
        {
          std::stringstream msg;
          msg << "An output module ID can not be determined from a ";
          msg << "faulty or incomplete INIT message.";
          XCEPT_RAISE(stor::exception::IncompleteInitMessage, msg.str());
        }

      if (! _headerFieldsCached) {cacheHeaderFields();}
      return _outputModuleId;
    }

    void InitMsgData::do_hltTriggerNames(Strings& nameList) const
    {
      if (faulty() || !complete())
        {
          std::stringstream msg;
          msg << "The HLT trigger names can not be determined from a ";
          msg << "faulty or incomplete INIT message.";
          XCEPT_RAISE(stor::exception::IncompleteInitMessage, msg.str());
        }

      if (! _headerFieldsCached) {cacheHeaderFields();}
      nameList = _hltTriggerNames;
    }

    void InitMsgData::do_hltTriggerSelections(Strings& nameList) const
    {
      if (faulty() || !complete())
        {
          std::stringstream msg;
          msg << "The HLT trigger selections can not be determined from a ";
          msg << "faulty or incomplete INIT message.";
          XCEPT_RAISE(stor::exception::IncompleteInitMessage, msg.str());
        }

      if (! _headerFieldsCached) {cacheHeaderFields();}
      nameList = _hltTriggerSelections;
    }

    void InitMsgData::do_l1TriggerNames(Strings& nameList) const
    {
      if (faulty() || !complete())
        {
          std::stringstream msg;
          msg << "The L1 trigger names can not be determined from a ";
          msg << "faulty or incomplete INIT message.";
          XCEPT_RAISE(stor::exception::IncompleteInitMessage, msg.str());
        }

      if (! _headerFieldsCached) {cacheHeaderFields();}
      nameList = _l1TriggerNames;
    }

    void InitMsgData::cacheHeaderFields() const
    {
      unsigned char* firstFragLoc = dataLocation(0);
      unsigned long firstFragSize = dataSize(0);
      bool useFirstFrag = false;

      // if there is only one fragment, use it
      if (_fragmentCount == 1)
        {
          useFirstFrag = true;
        }
      // otherwise, check if the first fragment is large enough to hold
      // the full INIT message header  (we require some minimal fixed
      // size in the hope that we don't parse garbage when we overlay
      // the InitMsgView on the buffer)
      else if (firstFragSize > 4096)
        {
          InitMsgView view(firstFragLoc);
          if (view.headerSize() <= firstFragSize)
            {
              useFirstFrag = true;
            }
        }

      boost::shared_ptr<InitMsgView> msgView;
      boost::shared_ptr< std::vector<unsigned char> >
        tempBuffer(new std::vector<unsigned char>());
      if (useFirstFrag)
        {
          msgView.reset(new InitMsgView(firstFragLoc));
        }
      else
        {
          copyFragmentsIntoBuffer(*tempBuffer);
          msgView.reset(new InitMsgView(&(*tempBuffer)[0]));
        }

      _outputModuleId = msgView->outputModuleId();
      _outputModuleLabel = msgView->outputModuleLabel();
      msgView->hltTriggerNames(_hltTriggerNames);
      msgView->hltTriggerSelections(_hltTriggerSelections);
      msgView->l1TriggerNames(_l1TriggerNames);

      _headerFieldsCached = true;
    }

    inline void InitMsgData::parseI2OHeader()
    {
      if (parsable())
        {
          _messageCode = Header::INIT;
          I2O_SM_PREAMBLE_MESSAGE_FRAME *smMsg =
            (I2O_SM_PREAMBLE_MESSAGE_FRAME*) _ref->getDataLocation();
          _fragKey.code_ = _messageCode;
          _fragKey.run_ = 0;
          _fragKey.event_ = smMsg->hltTid;
          _fragKey.secondaryId_ = smMsg->outModID;
          _fragKey.originatorPid_ = smMsg->fuProcID;
          _fragKey.originatorGuid_ = smMsg->fuGUID;
          _rbBufferId = smMsg->rbBufferID;
        }
    }

    class EventMsgData : public ChainData
    {
    public:
      explicit EventMsgData(toolbox::mem::Reference* pRef);
      ~EventMsgData() {}

    protected:
      unsigned char* do_fragmentLocation(unsigned char* dataLoc) const;

    private:
      void parseI2OHeader();
    };

    inline EventMsgData::EventMsgData(toolbox::mem::Reference* pRef) :
      ChainData(pRef)
    {
      parseI2OHeader();

      if (_fragmentCount > 1)
        {
          toolbox::mem::Reference* curRef = _ref->getNextReference();
          while (curRef)
            {
              validateMessageCode(curRef, I2O_SM_DATA);
              curRef = curRef->getNextReference();
            }
        }

      if (!faulty() && _fragmentCount == _expectedNumberOfFragments)
        {
          markComplete();
        }
    }

    inline unsigned char*
    EventMsgData::do_fragmentLocation(unsigned char* dataLoc) const
    {
      if (parsable())
        {
          I2O_SM_DATA_MESSAGE_FRAME *smMsg =
            (I2O_SM_DATA_MESSAGE_FRAME*) dataLoc;
          return (unsigned char*) smMsg->dataPtr();
        }
      else
        {
          return dataLoc;
        }
    }

    inline void EventMsgData::parseI2OHeader()
    {
      if (parsable())
        {
          _messageCode = Header::EVENT;
          I2O_SM_DATA_MESSAGE_FRAME *smMsg =
            (I2O_SM_DATA_MESSAGE_FRAME*) _ref->getDataLocation();
          _fragKey.code_ = _messageCode;
          _fragKey.run_ = smMsg->runID;
          _fragKey.event_ = smMsg->eventID;
          _fragKey.secondaryId_ = smMsg->outModID;
          _fragKey.originatorPid_ = smMsg->fuProcID;
          _fragKey.originatorGuid_ = smMsg->fuGUID;
          _rbBufferId = smMsg->rbBufferID;
        }
    }

    class DQMEventMsgData : public ChainData
    {
    public:
      explicit DQMEventMsgData(toolbox::mem::Reference* pRef);
      ~DQMEventMsgData() {}

    protected:
      unsigned char* do_fragmentLocation(unsigned char* dataLoc) const;

    private:
      void parseI2OHeader();
    };

    inline DQMEventMsgData::DQMEventMsgData(toolbox::mem::Reference* pRef) :
      ChainData(pRef)
    {
      parseI2OHeader();

      if (_fragmentCount > 1)
        {
          toolbox::mem::Reference* curRef = _ref->getNextReference();
          while (curRef)
            {
              validateMessageCode(curRef, I2O_SM_DQM);
              curRef = curRef->getNextReference();
            }
        }

      if (!faulty() && _fragmentCount == _expectedNumberOfFragments)
        {
          markComplete();
        }
    }

    inline unsigned char*
    DQMEventMsgData::do_fragmentLocation(unsigned char* dataLoc) const
    {
      if (parsable())
        {
          I2O_SM_DQM_MESSAGE_FRAME *smMsg =
            (I2O_SM_DQM_MESSAGE_FRAME*) dataLoc;
          return (unsigned char*) smMsg->dataPtr();
        }
      else
        {
          return dataLoc;
        }
    }

    inline void DQMEventMsgData::parseI2OHeader()
    {
      if (parsable())
        {
          _messageCode = Header::DQM_EVENT;
          I2O_SM_DQM_MESSAGE_FRAME *smMsg =
            (I2O_SM_DQM_MESSAGE_FRAME*) _ref->getDataLocation();
          _fragKey.code_ = _messageCode;
          _fragKey.run_ = smMsg->runID;
          _fragKey.event_ = smMsg->eventAtUpdateID;
          _fragKey.secondaryId_ = smMsg->folderID;
          _fragKey.originatorPid_ = smMsg->fuProcID;
          _fragKey.originatorGuid_ = smMsg->fuGUID;
          _rbBufferId = smMsg->rbBufferID;
        }
    }

    class ErrorEventMsgData : public ChainData
    {
    public:
      explicit ErrorEventMsgData(toolbox::mem::Reference* pRef);
      ~ErrorEventMsgData() {}

    protected:
      unsigned char* do_fragmentLocation(unsigned char* dataLoc) const;

    private:
      void parseI2OHeader();
    };

    inline ErrorEventMsgData::ErrorEventMsgData(toolbox::mem::Reference* pRef) :
      ChainData(pRef)
    {
      parseI2OHeader();

      if (_fragmentCount > 1)
        {
          toolbox::mem::Reference* curRef = _ref->getNextReference();
          while (curRef)
            {
              validateMessageCode(curRef, I2O_SM_ERROR);
              curRef = curRef->getNextReference();
            }
        }

      if (!faulty() && _fragmentCount == _expectedNumberOfFragments)
        {
          markComplete();
        }
    }

    inline unsigned char*
    ErrorEventMsgData::do_fragmentLocation(unsigned char* dataLoc) const
    {
      if (parsable())
        {
          I2O_SM_DATA_MESSAGE_FRAME *smMsg =
            (I2O_SM_DATA_MESSAGE_FRAME*) dataLoc;
          return (unsigned char*) smMsg->dataPtr();
        }
      else
        {
          return dataLoc;
        }
    }

    inline void ErrorEventMsgData::parseI2OHeader()
    {
      if (parsable())
        {
          _messageCode = Header::ERROR_EVENT;
          I2O_SM_DATA_MESSAGE_FRAME *smMsg =
            (I2O_SM_DATA_MESSAGE_FRAME*) _ref->getDataLocation();
          _fragKey.code_ = _messageCode;
          _fragKey.run_ = smMsg->runID;
          _fragKey.event_ = smMsg->eventID;
          _fragKey.secondaryId_ = smMsg->outModID;
          _fragKey.originatorPid_ = smMsg->fuProcID;
          _fragKey.originatorGuid_ = smMsg->fuGUID;
          _rbBufferId = smMsg->rbBufferID;
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
        if (!pvtMsg || ((size_t)(pvtMsg->StdMessageFrame.MessageSize*4) <
                        sizeof(I2O_SM_MULTIPART_MESSAGE_FRAME)))
          {
            _data.reset(new detail::ChainData(pRef));
            return;
          }

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
        XCEPT_RAISE(stor::exception::I2OChain,
                    "A fragment may not be added to an empty chain.");
      }
    if (complete())
      {
        XCEPT_RAISE(stor::exception::I2OChain,
                    "A fragment may not be added to a complete chain.");
      }

    // empty, complete, or faulty new parts can not be added to chains
    if (newpart.empty())
      {
        XCEPT_RAISE(stor::exception::I2OChain,
                    "An empty chain may not be added to an existing chain.");
      }
    if (newpart.complete())
      {
        XCEPT_RAISE(stor::exception::I2OChain,
                    "A complete chain may not be added to an existing chain.");
      }

    // require the new part and this chain to have the same fragment key
    FragKey thisKey = fragmentKey();
    FragKey thatKey = newpart.fragmentKey();
    // should change this to != once we implement that operator in FragKey
    if (thisKey < thatKey || thatKey < thisKey)
      {
        std::stringstream msg;
        msg << "A fragment key mismatch was detected when trying to add "
            << "a chain link to an existing chain. "
            << "Existing key values = ("
            << ((int)thisKey.code_) << "," << thisKey.run_ << ","
            << thisKey.event_ << "," << thisKey.secondaryId_ << ","
            << thisKey.originatorPid_ << "," << thisKey.originatorGuid_
            << "), new key values = ("
            << ((int)thatKey.code_) << "," << thatKey.run_ << ","
            << thatKey.event_ << "," << thatKey.secondaryId_ << ","
            << thatKey.originatorPid_ << "," << thatKey.originatorGuid_
            << ").";
        XCEPT_RAISE(stor::exception::I2OChain, msg.str());
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

  unsigned int I2OChain::messageCode() const
  {
    if (!_data) return Header::INVALID;
    return _data->messageCode();
  }

  bool I2OChain::hasValidRBBufferId() const
  {
    if (!_data) return false;
    return _data->parsable();
  }

  unsigned int I2OChain::rbBufferId() const
  {
    if (!_data) return 0;
    return _data->rbBufferId();
  }

  FragKey I2OChain::fragmentKey() const
  {
    if (!_data) return FragKey(Header::INVALID,0,0,0,0,0);
    return _data->fragmentKey();
  }

  unsigned int I2OChain::fragmentCount() const
  {
    if (!_data) return 0;
    return _data->fragmentCount();
  }

  double I2OChain::creationTime() const
  {
    if (!_data) return -1;
    return _data->creationTime();
  }

  double I2OChain::lastFragmentTime() const
  {
    if (!_data) return -1;
    return _data->lastFragmentTime();
  }

  unsigned long I2OChain::totalDataSize() const
  {
    if (!_data) return 0UL;
    return _data->totalDataSize();
  }

  unsigned long I2OChain::dataSize(int fragmentIndex) const
  {
    if (!_data) return 0UL;
    return _data->dataSize(fragmentIndex);
  }

  unsigned char* I2OChain::dataLocation(int fragmentIndex) const
  {
    if (!_data) return 0UL;
    return _data->dataLocation(fragmentIndex);
  }

  unsigned int I2OChain::getFragmentID(int fragmentIndex) const
  {
    if (!_data) return 0;
    return _data->getFragmentID(fragmentIndex);
  }

  unsigned int I2OChain::
  copyFragmentsIntoBuffer(std::vector<unsigned char>& targetBuffer) const
  {
    if (!_data) return 0;
    return _data->copyFragmentsIntoBuffer(targetBuffer);
  }

  std::string I2OChain::outputModuleLabel() const
  {
    if (!_data)
      {
        XCEPT_RAISE(stor::exception::I2OChain,
          "The output module label can not be determined from an empty I2OChain.");
      }
    return _data->outputModuleLabel();
  }

  uint32 I2OChain::outputModuleId() const
  {
    if (!_data)
      {
        XCEPT_RAISE(stor::exception::I2OChain,
          "The output module ID can not be determined from an empty I2OChain.");
      }
    return _data->outputModuleId();
  }

  void I2OChain::hltTriggerNames(Strings& nameList) const
  {
    if (!_data)
      {
        XCEPT_RAISE(stor::exception::I2OChain,
          "HLT trigger names can not be determined from an empty I2OChain.");
      }
    _data->hltTriggerNames(nameList);
  }

  void I2OChain::hltTriggerSelections(Strings& nameList) const
  {
    if (!_data)
      {
        XCEPT_RAISE(stor::exception::I2OChain,
          "HLT trigger selections can not be determined from an empty I2OChain.");
      }
    _data->hltTriggerSelections(nameList);
  }

  void I2OChain::l1TriggerNames(Strings& nameList) const
  {
    if (!_data)
      {
        XCEPT_RAISE(stor::exception::I2OChain,
          "L1 trigger names can not be determined from an empty I2OChain.");
      }
    _data->l1TriggerNames(nameList);
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
