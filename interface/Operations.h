// -*- c++ -*-

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <string>

// Abstract base class

class Operations
{

public:

  virtual ~Operations() = 0;
  virtual void handle_I2O_event_message() const = 0;
  virtual const std::string& state_name() const = 0;

};

#endif
