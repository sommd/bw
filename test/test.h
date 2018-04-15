#ifndef TEST_H
#define TEST_H

#include <string.h>
#include <errno.h>

/* Abort with error message from errno if check is false. */
#define test_check_errno(check) if (!(check)) ck_abort_msg(strerror(errno))

#endif
