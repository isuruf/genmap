#include <stddef.h>

#include "test.h"
#include "types.h"
#include "name.h"
#include "gs_defs.h"
#include "gs_local.h"
#include "tensor.h"
#include "fail.h"
#include "comm.h"
//------------------------------------------------------------------------------
int test_1() {
  slong id;
  id = 1;

  return id;
}
//------------------------------------------------------------------------------
int test_2() {
  struct comm c;

  comm_init(&c, MPI_COMM_WORLD);

  comm_free(&c);

  return 1;
}
//------------------------------------------------------------------------------
int main() {
  run_test(&test_1,"gs_link1");
  run_test(&test_2,"gs_comm");

  return 0;
}