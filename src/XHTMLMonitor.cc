// $Id: XHTMLMonitor.cc,v 1.1.2.1 2009/02/18 10:38:06 dshpakov Exp $

#include "EventFilter/StorageManager/interface/XHTMLMonitor.h"

#include <xercesc/util/PlatformUtils.hpp>

using namespace xercesc;

XHTMLMonitor::XHTMLMonitor()
{
  XMLPlatformUtils::Initialize();
}

XHTMLMonitor::~XHTMLMonitor()
{
  XMLPlatformUtils::Terminate();
}
