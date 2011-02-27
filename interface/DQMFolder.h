// $Id: DQMFolder.h,v 1.17.8.1 2011/01/24 12:18:39 mommsen Exp $
/// @file: DQMFolder.h 

#ifndef EventFilter_StorageManager_DQMFolder_h
#define EventFilter_StorageManager_DQMFolder_h

#include <string>
#include <vector>
#include <map>

#include "TObject.h"


namespace stor 
{
  
  /**
   * A single DQM folder holding several histograms
   *
   * $Author: mommsen $
   * $Revision: 1.17.8.1 $
   * $Date: 2011/01/24 12:18:39 $
   */

  class DQMFolder
  {
  public:
    DQMFolder();
    ~DQMFolder();
    void addObjects(std::vector<TObject *>);
    void fillObjectVector(std::vector<TObject*>&) const;

  private:

    static std::string getSafeMEName(TObject *object);

    typedef std::map<std::string, TObject*> DQMObjectsMap;
    DQMObjectsMap dqmObjects_;
  }; 

} // namespace stor

#endif // EventFilter_StorageManager_DQMFolder_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -