// $Id: FileRecord.h,v 1.9.4.5 2009/03/14 01:32:36 biery Exp $

#ifndef StorageManager_FileRecord_h
#define StorageManager_FileRecord_h

#include <EventFilter/StorageManager/interface/Utils.h>
#include <EventFilter/StorageManager/interface/Configuration.h>

#include <boost/shared_ptr.hpp>

#include <string>


namespace stor {

  /**
   * Holds the information for a physical file
   *
   * $Author: biery $
   * $Revision: 1.9.4.5 $
   * $Date: 2009/03/14 01:32:36 $
   */

  class FileRecord
    {
    public:

      /**
       * Enumaration why the given file was closed
       */
      enum ClosingReason
      {
        notClosed = 0,
        stop,
        Nminus2lumi,
        timeout,
        size
      };

      FileRecord(const uint32_t lumiSection, const std::string &file, DiskWritingParams dwParams);


      //////////////////////
      // File bookkeeping //
      //////////////////////

      /**
       * Write summary information in file catalog
       */
      void writeToSummaryCatalog() const;

      /**
       * Write command to update the file information in the CMS_STOMGR.TIER0_INJECTION table
       * into the logFile_.
       */
      void updateDatabase() const;

      /**
       * Write command to insert a new file into the CMS_STOMGR.TIER0_INJECTION table
       * into the logFile_.
       */
      void insertFileInDatabase() const;



      ////////////////////////////
      // File parameter setters //
      ////////////////////////////

      /**
       * Sets the run number
       */
      void setRunNumber(const uint32_t i)
      { runNumber_ = i; }

      /**
       * Sets the current file system
       */
      void setFileSystem(const unsigned int i);

      /**
       * Sets the file counter
       */
      void setFileCounter(const unsigned int i)
      { fileCounter_ = i; }

      /**
       * Set the stream label
       */
      void setStreamLabel(const std::string &s)
      { streamLabel_ = s;}

      /**
       * Set the reason why the file was closed
       */
      void setWhyClosed(ClosingReason w)
      { whyClosed_  = w; }

      /**
       * Set the adler checksum for the file
       */
      void setadler(unsigned int s, unsigned int i)
      { adlerstream_ = s; adlerindex_ = i; }

      /**
       * Add number of bytes to file size
       */
      void increaseFileSize(const long long size)
      { fileSize_ += size; }

      /**
       * Increment number of events stored in the file
       */
      void increaseEventCount()
      { events_++; }

      /**
       * Time when first event was added
       */
      void firstEntry(utils::time_point_t d)
      { firstEntry_ = d; }

      /**
       * Time when latest event was added
       */
      void lastEntry(utils::time_point_t d)
      { lastEntry_ = d; }



      ////////////////////////////
      // File parameter getters //
      ////////////////////////////

      /**
       * Returns the number of events in the file
       */
      const int events() const
      { return events_; } 

      /**
       * Returns the luminosity section the file belongs to
       */
      const uint32_t lumiSection() const
      { return lumiSection_; }

      /**
       * Returns the size of the file in bytes
       */
      const long long fileSize() const
      { return fileSize_; }

      /**
       * Returns the full path where the file resides
       */
      const std::string filePath() const;

      /**
       * Returns the complete file name and path w/o file ending
       */
      const std::string completeFileName() const;

      /**
       * Returns the time when first event was added
       */
      const utils::time_point_t firstEntry() const
      { return firstEntry_; }

      /**
       * Returns the time when latest event was added
       */
      const utils::time_point_t lastEntry() const
      { return lastEntry_; }



      /////////////////////////////
      // File system interaction //
      /////////////////////////////

      /**
       * Move index and streamer file to "closed" directory
       */
      void moveFileToClosed();

      /**
       * Move error event file to "closed" directory
       */
      void moveErrorFileToClosed();

      /**
       * Checks if all directories needed for the file output are available.
       * Throws a cms::Exception when a directory does not exist.
       */
      void checkDirectories() const;



      /////////////////////////////
      // File information dumper //
      /////////////////////////////

      /**
       * Adds file summary information to ostream
       */
      void info(std::ostream &os) const;

      /**
       * Dump status information to ostream
       */
      void report(std::ostream &os, int indentation) const;



    private:
      std::string  fileName_;                         // file name (w/o ending)
      std::string  basePath_;                         // base path name
      std::string  fileSystem_;                       // file system directory
      std::string  workingDir_;                       // current working directory
      std::string  logPath_;                          // log path
      std::string  logFile_;                          // log file including path
      std::string  streamLabel_;                      // datastream label
      std::string  cmsver_;                           // CMSSW version string
      uint32_t     lumiSection_;                      // luminosity section  
      uint32_t     runNumber_;                        // runNumber
      unsigned int fileCounter_;                      // number of files with fileName_ as name
      long long    fileSize_;                         // current file size in ??? bytes
      uint32_t     events_;                           // total number of events
      utils::time_point_t firstEntry_;                // time when first event was writen
      utils::time_point_t lastEntry_;                 // time when latest event was writen

      ClosingReason whyClosed_;                       // record why file was closed 

      unsigned int adlerstream_;                      // adler32 checksum for streamer file
      unsigned int adlerindex_;                       // adler32 checksum for index file

      /**
       * Returns the fileName_ and fileCounter_
       */
      const std::string qualifiedFileName() const;

      /**
       * Returns the name of the log file
       */
      const std::string logFile(stor::DiskWritingParams const&) const;

      /**
       * Throws a cms::Exception when the directory does not exist
       */
      void checkDirectory(const std::string &) const;

      /**
       * Returns the relative difference btw to file sizes
       */
      const double calcPctDiff(long long, long long) const;

      stor::DiskWritingParams diskWritingParams_;
   };
 
} // stor namespace

#endif // StorageManager_FileRecord_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
