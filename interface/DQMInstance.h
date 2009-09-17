// $Id: DQMInstance.h,v 1.12 2009/09/16 11:04:22 mommsen Exp $
/// @file: DQMInstance.h 

#ifndef StorageManager_DQMInstance_h
#define StorageManager_DQMInstance_h

#include <string>
#include <vector>
#include <map>

#include "FWCore/PluginManager/interface/ProblemTracker.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageService/interface/MessageServicePresence.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "TFile.h"
#include "TTimeStamp.h"
#include "TObject.h"

namespace stor 
{

  /**
   * A single DQM folder holding several histograms
   *
   * $Author: mommsen $
   * $Revision: 1.12 $
   * $Date: 2009/09/16 11:04:22 $
   */

  class DQMFolder
  {
    public:
      DQMFolder();
     ~DQMFolder();
      std::map<std::string, TObject *> dqmObjects_;
  }; 



  /**
   * A collection of DQM Folders under the same top-level name.
   *
   * $Author: mommsen $
   * $Revision: 1.12 $
   * $Date: 2009/09/16 11:04:22 $
   */

  class DQMGroup
  {
    public:
      DQMGroup(int readyTime, int expectedUpdates);
     ~DQMGroup();
      std::map<std::string, DQMFolder *> dqmFolders_;
      int getNUpdates()             { return(nUpdates_);}
      int getReadyTime()            { return(readyTime_);}
      int getLastEvent()            { return(lastEvent_);}
      void setLastEvent(int lastEvent);
      TTimeStamp * getFirstUpdate() { return(firstUpdate_);}
      TTimeStamp * getLastUpdate()  { return(lastUpdate_);}
      TTimeStamp * getLastServed()  { return(lastServed_);}
      bool isReady(int currentTime);
      bool wasServedSinceUpdate()   { return(wasServedSinceUpdate_);}
      void setServedSinceUpdate();
      void setLastServed()          { lastServed_->Set();}

    protected:
      TTimeStamp            *firstUpdate_;
      TTimeStamp            *lastUpdate_;
      TTimeStamp            *lastServed_;
      int                    nUpdates_;
      int                    readyTime_;
      int                    expectedUpdates_;
      int                    lastEvent_;
      bool                   wasServedSinceUpdate_;
  }; 



  /**
   * Container class for one snapshot instance of a collection of 
   * collated DQM groups
   *
   * $Author: mommsen $
   * $Revision: 1.12 $
   * $Date: 2009/09/16 11:04:22 $
   */

  class DQMInstance
  {
    public:
      DQMInstance(int runNumber, 
		  int lumiSection, 
		  int instance,
		  int purgeTime,
                  int readyTime,
                  int expectedUpdates);

     ~DQMInstance();

      int getRunNumber()            { return(runNumber_);}
      int getLastEvent()            { return(lastEvent_);}
      int getLumiSection()          { return(lumiSection_);}
      int getInstance()             { return(instance_);}
      int getPurgeTime()            { return(purgeTime_);}
      int getReadyTime()            { return(readyTime_);}

      TTimeStamp * getFirstUpdate() { return(firstUpdate_);}
      TTimeStamp * getLastUpdate()  { return(lastUpdate_);}
      int updateObject(std::string groupName,
		       std::string objectDirectory,
		       TObject   * object,
		       int         eventNumber);
      double writeFile(std::string filePrefix, bool endRunFlag);
      DQMGroup * getDQMGroup(std::string groupName);
      bool isReady(int currentTime);
      bool isStale(int currentTime);
      std::map<std::string, DQMGroup *> dqmGroups_;

      static std::string getSafeMEName(TObject *object);

    protected:  
      int                    runNumber_;
      int                    lastEvent_;
      int                    lumiSection_;
      int                    instance_;
      TTimeStamp            *firstUpdate_;
      TTimeStamp            *lastUpdate_;
      int                    nUpdates_;
      int                    purgeTime_;
      int                    readyTime_;
      int                    expectedUpdates_;
  }; 

  class DQMGroupDescriptor
  {
    public:
      DQMGroupDescriptor(DQMInstance *instance,DQMGroup *group);
     ~DQMGroupDescriptor();
      DQMInstance *instance_;
      DQMGroup    *group_;
  };
}


#endif // StorageManager_DQMInstance_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
