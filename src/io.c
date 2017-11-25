#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
//------------------------------------------------------------------------------
long file_open(char* name, int mode)
{
#ifdef MPI
  MPI_Status status;
  MPI_File fh;
  MPI_Offset offset;

  MPI_File_open(MPI_COMM_WORLD, name, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
  MPI_File_close(&fh);
#else
  printf("Serial !!!!\n");
#endif
}
//------------------------------------------------------------------------------
long file_read(long fh)
{
  return 1;
}
//------------------------------------------------------------------------------
long file_write(long fh)
{
  return 1;
}
//------------------------------------------------------------------------------
long file_close(long fh)
{
  return 1;
}
//------------------------------------------------------------------------------
void readmap_dev(int nid, long *npts, long *nel, long **glo_num,
            char* name)
{
  long fp;
  long nactive, depth, d2, nrank, noutflow;
  long nc, cnt, jnk;

  fp = file_open(name, MPI_MODE_RDONLY);
  if (fp == -1) {
    fprintf(stderr, "Unable to open the file.\n");
  }

//  cnt = file_read(fp, "%ld %ld %ld %ld %ld %ld %ld\n",
//        nel, &nactive, &depth, &d2, npts, &nrank, &noutflow);
//  if (cnt != 7) {
//    fprintf(stderr, "Unable to read .map file.\n");
//  }
//
//  nc  = *npts/(*nel);
//
//  *glo_num = malloc(sizeof(long)*(*npts));
//
//  for (long i = 0; i < *nel; i++) {
//      cnt = fscanf(fp, "%ld", &jnk);
//      if (cnt != 1) {
//        fprintf(stderr, "Unable to read .map file.\n");
//      }
//      for (long j = 0; j < nc - 1; j++) {
//        cnt = fscanf(fp, "%ld", *glo_num + (i*nc + j));
//        if (cnt != 1) {
//          fprintf(stderr, "Unable to read .map file.\n");
//        }
//      }
//      cnt = fscanf(fp, "%ld\n", *glo_num + (i*nc + nc - 1));
//      if (cnt != 1) {
//        fprintf(stderr, "Unable to read .map file.\n");
//      }
//  }
//
//  file_close(fp);
}
//------------------------------------------------------------------------------
void readmap(long *npts, long *nelt, long **glo_num, char* name)
{
    readmap_dev(0, npts, nelt, glo_num, name);
}
//------------------------------------------------------------------------------
