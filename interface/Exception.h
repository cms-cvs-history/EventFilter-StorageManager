// $Id: Exception.h,v 1.1.2.1 2009/02/04 17:51:25 mommsen Exp $

#ifndef _storagemanager_Exception_h_
#define _storagemanager_Exception_h_


#include "xcept/Exception.h"

// The following macro is defined in newer xdaq versions
#ifndef XCEPT_DEFINE_EXCEPTION
#define XCEPT_DEFINE_EXCEPTION(NAMESPACE1, EXCEPTION_NAME) \
namespace NAMESPACE1 { \
namespace exception { \
class EXCEPTION_NAME: public xcept::Exception \
{\
        public: \
        EXCEPTION_NAME( std::string name, std::string message, std::string module, int line, std::string function ): \
                xcept::Exception(name, message, module, line, function) \
        {} \
        EXCEPTION_NAME( std::string name, std::string message, std::string module, int line, std::string function, xcept::Exception & e ): \
                xcept::Exception(name, message, module, line, function,e) \
        {} \
}; \
} \
}
#endif



/**
 * Generic exception raised by the storage manager
 */
XCEPT_DEFINE_EXCEPTION(stor, Exception)

/**
 * Exception raised in case of a SOAP error
 */
XCEPT_DEFINE_EXCEPTION(stor, SoapMessage)

/**
 * Exception raised in case of a finite state machine error
 */
XCEPT_DEFINE_EXCEPTION(stor, StateMachine)

/**
 * Exception raised in case of a monitoring error
 */
XCEPT_DEFINE_EXCEPTION(stor, Monitoring)

/**
 * Exception raised in case of problems accessing the info space
 */
XCEPT_DEFINE_EXCEPTION(stor, Infospace)


#endif // _storagemanager_Exception_h_


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
