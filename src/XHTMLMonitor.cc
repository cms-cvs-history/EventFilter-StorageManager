// $Id: XHTMLMonitor.cc,v 1.3 2009/07/20 13:07:28 mommsen Exp $
/// @file: XHTMLMonitor.cc

#include "EventFilter/StorageManager/interface/XHTMLMonitor.h"

#include <xercesc/util/PlatformUtils.hpp>

using namespace xercesc;

stor::XHTMLMonitor::XHTMLMonitor()
{
  XMLPlatformUtils::Initialize();
}

stor::XHTMLMonitor::~XHTMLMonitor()
{
  XMLPlatformUtils::Terminate();
}
