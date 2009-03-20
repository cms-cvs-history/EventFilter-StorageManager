// Created by Markus Klute on 2007 Jan 09.
// $Id: ProgressMarker.cc,v 1.1.22.2 2009/03/13 14:00:19 mommsen Exp $

#include <EventFilter/StorageManager/interface/ProgressMarker.h>

using stor::ProgressMarker;
using std::string;

ProgressMarker *inst = 0;

ProgressMarker::ProgressMarker()
{
  reading_     = false;
  writing_     = false;
  processing_  = false;
}


ProgressMarker *ProgressMarker::instance()
{ // not thread save
  if (inst == 0) inst = new ProgressMarker();
  return inst;
}


void ProgressMarker::instance(ProgressMarker *anInstance)
{ // not thread save
  delete inst;
  inst = anInstance;
}


string ProgressMarker::status()
{
  if (processing_) return "Process";
  if (reading_)    return "Input";
  if (writing_)    return "Output";
  return "Idle";
}
