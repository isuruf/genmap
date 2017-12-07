#include <stdio.h>

#include "test.h"
#include "laplacian.h"
#include "linalg.h"
#include "mpiwrapper.h"
#include "gswrapper.h"
//------------------------------------------------------------------------------
int32 main(int32 argc, char **argv) {
  struct comm c;

  init_genmap(&c, argc, argv);

  gop_init(&c);

  int32 rank, np;
  rank = c.id;
  np = c.np;
  printf("rank = %d\n", rank);
  printf("np = %d\n", np);

  double sum = rank;
  gop(&sum, gs_double, gs_add, 0);
  if (rank == 0) {
    printf("Sum is : %lf\n", sum);
  }

  double min = rank;
  gop(&min, gs_double, gs_min, 0);
  if (rank == 0) {
    printf("Min is : %lf\n", min);
  }

  double max = rank;
  gop(&max, gs_double, gs_max, 0);
  if (rank == 0) {
    printf("Max is : %lf\n", max);
  }

  gop_finalize();

  finalize_genmap(&c);

  return 0;
}
//------------------------------------------------------------------------------
