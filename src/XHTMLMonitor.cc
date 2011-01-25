// $Id: XHTMLMonitor.cc,v 1.3.14.1 2011/01/24 12:18:39 mommsen Exp $
/// @file: XHTMLMonitor.cc

#include "EventFilter/StorageManager/interface/XHTMLMonitor.h"

#include <xercesc/util/PlatformUtils.hpp>

using namespace xercesc;

boost::mutex stor::XHTMLMonitor::_xhtmlMakerMutex;

stor::XHTMLMonitor::XHTMLMonitor()
{
  _xhtmlMakerMutex.lock();
  XMLPlatformUtils::Initialize();
}

stor::XHTMLMonitor::~XHTMLMonitor()
{
  XMLPlatformUtils::Terminate();
  _xhtmlMakerMutex.unlock();
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
