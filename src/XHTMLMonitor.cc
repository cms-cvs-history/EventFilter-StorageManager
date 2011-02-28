// $Id: XHTMLMonitor.cc,v 1.3.14.2 2011/01/25 11:29:12 mommsen Exp $
/// @file: XHTMLMonitor.cc

#include "EventFilter/StorageManager/interface/XHTMLMonitor.h"

#include <xercesc/util/PlatformUtils.hpp>

using namespace xercesc;

boost::mutex stor::XHTMLMonitor::xhtmlMakerMutex_;

stor::XHTMLMonitor::XHTMLMonitor()
{
  xhtmlMakerMutex_.lock();
  XMLPlatformUtils::Initialize();
}

stor::XHTMLMonitor::~XHTMLMonitor()
{
  XMLPlatformUtils::Terminate();
  xhtmlMakerMutex_.unlock();
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
