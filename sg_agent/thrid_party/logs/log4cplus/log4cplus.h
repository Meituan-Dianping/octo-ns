#ifndef __LOG4CPLUS_H__
#define __LOG4CPLUS_H__

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/property.h>

/*采用宏对目前的底层日志库进行封装，如果未来有需要更换日志库，在这里修改即可*/
namespace meituan_mns{
using std::auto_ptr;
using log4cplus::Logger;
using log4cplus::ConsoleAppender;
using log4cplus::FileAppender;
using log4cplus::Appender;
using log4cplus::Layout;
using log4cplus::PatternLayout;
using log4cplus::helpers::SharedObjectPtr;

extern Logger debug_instance;
extern Logger info_instance;
extern Logger warn_instance;
extern Logger error_instance;
extern Logger fatal_instance;
extern Logger stat_instance;

#define NS_LOG_DEBUG(debugContent) {LOG4CPLUS_DEBUG(debug_instance,debugContent);}
#define NS_LOG_INFO(infoContent) {LOG4CPLUS_INFO(info_instance, infoContent);}
#define NS_LOG_WARN(warnContent) {LOG4CPLUS_WARN(warn_instance, warnContent);}
#define NS_LOG_ERROR(errorContent) {LOG4CPLUS_ERROR(error_instance,errorContent);}
#define NS_LOG_FATAL(fatalContent) {LOG4CPLUS_ERROR(fatal_instance,fatalContent);}
#define NS_LOG_STAT(statContent) {LOG4CPLUS_INFO(stat_instance, statContent);}

}



#endif
