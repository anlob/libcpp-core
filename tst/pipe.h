#ifndef TEST_PIPE_H_INCLUDED
#define TEST_PIPE_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_PIPE == 1) || (_TEST_PIPE_IO == 1)
void test_pipe_io();
#endif


#endif // TEST_PIPE_H_INCLUDED
