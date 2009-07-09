// $Id: $

#include "EventFilter/StorageManager/interface/Notifier.h"

#include <fstream>
#include <sstream>

using namespace stor;

void Notifier::localDebug( const std::string& message ) const
{
  std::ostringstream fname_oss;
  fname_oss << "/tmp/storage_manager_debug_" << instanceNumber();
  const std::string fname = fname_oss.str();
  std::ofstream f( fname.c_str() );
  if( f.is_open() )
    {
      try
	{
	  f << message << std::endl;
	  f.close();
	}
      catch(...)
	{
	}
    }
}