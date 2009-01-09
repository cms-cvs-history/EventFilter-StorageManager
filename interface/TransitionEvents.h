// -*- c++ -*-

#ifndef TRANSITIONEVENTS_H
#define TRANSITIONEVENTS_H

#include <boost/statechart/event.hpp>
#include <boost/statechart/transition.hpp>

namespace bsc = boost::statechart;

// Transition event classes

class Configure : public bsc::event<Configure> {};
class Enable    : public bsc::event<Enable> {};
class Stop      : public bsc::event<Stop> {};
class Halt      : public bsc::event<Halt> {};
class Fail      : public bsc::event<Fail> {};

#endif
