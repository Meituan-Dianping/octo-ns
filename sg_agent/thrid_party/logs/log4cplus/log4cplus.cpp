#include "log4cplus.h"

namespace meituan_mns{

Logger debug_instance = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("debug"));
Logger info_instance = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("info"));
Logger warn_instance = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("warn"));
Logger error_instance = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("error"));
Logger fatal_instance = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("fatal"));

//non root Logger
Logger stat_instance = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("statLogger"));

}

