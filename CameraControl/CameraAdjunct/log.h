#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

/*
 *  Put once in global scope.
 */
#define LOG_DECLARE FILE *fp_;

/*
 *  Called once each.
 */
#define LOG_INIT(fName) fp_=fopen(fName,"a");
#define LOG_CLOSE() fclose(fp_);

/*
 *  Put at top of any file that needs to access log file,
 *  except the file that has the LOG_DECLARE statment.
 */
#define LOG_USE extern FILE *fp_;

/*
 *  Various logging levels.
 */
#define LOG_MSG(msg)           fprintf(fp_,"message: %-12s %4d %s\n",__FILE__,__LINE__,msg);
#define LOG_WARN(msg)          fprintf(fp_,"warn:    %-12s %4d %s\n",__FILE__,__LINE__,msg);
#define LOG_ERROR(msg)         fprintf(fp_,"error:   %-12s %4d %s\n",__FILE__,__LINE__,msg);
#define LOG_MSG_FLOAT(msg,v)   fprintf(fp_,"message: %-12s %4d %s %12.4f\n",__FILE__,__LINE__,msg,v);
#define LOG_WARN_FLOAT(msg,v)  fprintf(fp_,"warn:    %-12s %4d %s %12.4f\n",__FILE__,__LINE__,msg,v);
#define LOG_ERROR_FLOAT(msg,v) fprintf(fp_,"error:   %-12s %4d %s %12.4f\n",__FILE__,__LINE__,msg,v);
#define LOG_MSG_INT(msg,v)     fprintf(fp_,"message: %-12s %4d %s %d\n",__FILE__,__LINE__,msg,v);
#define LOG_WARN_INT(msg,v)    fprintf(fp_,"warn:    %-12s %4d %s %d\n",__FILE__,__LINE__,msg,v);
#define LOG_ERROR_INT(msg,v)   fprintf(fp_,"error:   %-12s %4d %s %d\n",__FILE__,__LINE__,msg,v);
#define LOG_MSG_STR(msg,s)     fprintf(fp_,"message: %-12s %4d %s %s\n",__FILE__,__LINE__,msg,s);
#define LOG_WARN_STR(msg,s)    fprintf(fp_,"warn:    %-12s %4d %s %s\n",__FILE__,__LINE__,msg,s);
#define LOG_ERROR_STR(msg,s)   fprintf(fp_,"error:   %-12s %4d %s %s\n",__FILE__,__LINE__,msg,s);

#endif // _LOG_H_
