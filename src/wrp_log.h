
#include<stdarg.h>


#define LEVEL_ERROR	0
#define LEVEL_INFO	1
#define LEVEL_DEBUG	2
#define MAX_BUF_SIZE	4096

#define WrpError(...)	__WRP_LOG(LEVEL_ERROR, __VA_ARGS__)
#define WrpInfo(...)	__WRP_LOG(LEVEL_INFO, __VA_ARGS__)
#define WrpPrint(...)	__WRP_LOG(LEVEL_DEBUG, __VA_ARGS__)

/** 
* @brief Handler used by wrp_log_set_handler to receive all log
* notifications produced by the library on this function.
*
*/

typedef void (*wrpLogHandler) (int level,const char * log_msg);

/**
* @brief Allows to define a log handler that will receive all logs
* produced under the provided content.
* 
* @param handler The handler to be called for each log to be
* notified. Passing in NULL is allowed to remove any previously
* configured handler.
*/

void wrp_log_set_handler(wrpLogHandler handler);

/**
* @brief handle log message based on log level
* 
* @param level of log is info,debug,error
* @param logging message
*/
void __WRP_LOG(int level ,const char *msg, ...);
