#ifndef TEST_PROCESS_H_INCLUDED
#define TEST_PROCESS_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_PROC == 1) || (_TEST_PROC_EXEC == 1)
void test_proc_exec();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_PROC == 1) || (_TEST_PROC_FN == 1)
void test_proc_fn();
#endif


#endif // TEST_PROCESS_H_INCLUDED
