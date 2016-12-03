#include <stdio.h>
#include <stdlib.h>
#include "wrp_log.h"

wrpLogHandler handler = NULL;

void __WRP_LOG(int level ,const char *msg, ...)
{
    char *tempChar = NULL;
    int nbytes;
    va_list arg_ptr;
    tempChar=(char *)malloc(MAX_BUF_SIZE);
    if(tempChar)
    {
       va_start(arg_ptr,msg);
       nbytes = vsnprintf(tempChar,MAX_BUF_SIZE,msg,arg_ptr);
       va_end(arg_ptr);
       if(nbytes<0)
       {
         printf("Error occurred in vsnprintf\n");
       }
       else
       {
         if(handler != NULL)
         {
          handler(level,tempChar);
         }
         else
         {
          if(level == LEVEL_ERROR)
	        {
	         printf("Error :%s",tempChar);
	        }
	        if(level == LEVEL_INFO)
	        {
	         printf("Info :%s",tempChar);
	        }
	        if(level == LEVEL_DEBUG)
	        {
	         printf("Debug :%s",tempChar);
	        }
         } 
       }
       free(tempChar);
    }
    else
    {
      printf("Malloc failed in __WRP_LOG\n");
    }
}

void wrp_log_set_handler(wrpLogHandler logHandler)
{
	if(logHandler == NULL)
	{
	  handler  = NULL;
	  return;
	}
	else
	{
	  printf("Enabled log handler for wrp-c \n");
	  handler  = logHandler;
	}
	
}
