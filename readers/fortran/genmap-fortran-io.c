#include <genmap-readers.h>

#include <stdio.h>
//
// GenmapHeader: Create, Destroy
//
int GenmapCreateHeader_fortran(GenmapHeader *h) {
  GenmapMalloc(1, h);

  return 0;
}

int GenmapDestroyHeader_fortran(GenmapHeader h) {
  GenmapFree(h);
  return 0;
}
//
// GenmapElements: Create, Destroy
//
int GenmapCreateElements_fortran(GenmapElements *e) {
//  GenmapMalloc(1, e);
  return 0;
}

int GenmapDestroyElements_fortran(GenmapElements e) {
//  GenmapFree(e);
//  array_free(e);
  return 0;
}
GenmapElements GenmapGetElements_fortran(GenmapHandle h) {
  return (GenmapElements) h->elementArray.ptr;
}
//
// Do File I/O in parallel
//
int GenmapRead_fortran(GenmapHandle h, char *name) {
#ifdef MPI
  MPI_File fh;
  MPI_Offset offset;
  MPI_Status st;

  MPI_File_open(h->global->gsComm.c, name, MPI_MODE_RDONLY, MPI_INFO_NULL,
                &fh);

  MPI_File_seek(fh, 0, MPI_SEEK_SET);
  MPI_File_get_size(fh, &offset);
#else
  FILE *fp = fopen(name, "rb");
  if(fp == NULL) {
    printf("Unable to open the file.\n");
    exit(1);
  }
#endif

  GenmapInt *headerArray;
  GenmapMalloc(GENMAP_HEADER_SIZE, &headerArray);
#ifdef MPI
  MPI_File_read(fh, headerArray, GENMAP_HEADER_SIZE, MPI_INT, &st);
#else
  GenmapInt result = fread(headerArray, sizeof(GenmapInt),
                           GENMAP_HEADER_SIZE,
                           fp);
#endif

  // nel, nactive, depth, d2, npts, nrank, noutflow
  h->header->nel = headerArray[GENMAP_NEL];
  h->header->nactive = headerArray[GENMAP_NACTIVE];
  h->header->depth = headerArray[GENMAP_DEPTH];
  h->header->d2 = headerArray[GENMAP_D2];
  h->header->npts = headerArray[GENMAP_NPTS];
  h->header->nrank = headerArray[GENMAP_NRANK];
  h->header->noutflow = headerArray[GENMAP_NOUTFLOW];

  GenmapInt nel = headerArray[GENMAP_NEL];
  GenmapInt nc = headerArray[GENMAP_NPTS] / nel;
  GenmapInt lelt = nel / h->Np(h->global);

#ifdef MPI
  GenmapInt start = h->Id(h->global) * lelt * (2 * nc + 1);
  if(h->Id(h->global) == h->Np(h->global) - 1)
    lelt = nel - h->Id(h->global) * lelt;
#endif

  GenmapInt out[2][1], buf[2][1];
  comm_scan(out, &(h->global->gsComm), gs_int, gs_add, &lelt, 1, buf);

  h->header->nc = nc;
  h->header->lelt = lelt;
  h->header->start = out[0][0];

  array_reserve(struct GenmapElement_private, &(h->elementArray), lelt);
  h->elementArray.n = lelt;
  GenmapElements elements = GenmapGetElements(h);

#ifdef MPI
  MPI_File_seek(fh, (GENMAP_HEADER_SIZE + start)*sizeof(GenmapInt),
                MPI_SEEK_SET);
#endif

  for(GenmapInt i = 0; i < lelt; i++) {
#ifdef MPI
    MPI_File_read(fh, &(elements[i].globalId), 1, MPI_INT, &st);
    MPI_File_read(fh, elements[i].vertices, nc, MPI_INT, &st);
    MPI_File_read(fh, elements[i].edges, nc, MPI_INT, &st);
#else
    result += fread(&(elements[i].globalId), sizeof(GenmapInt), 1, fp);
    result += fread(elements[i].vertices, sizeof(GenmapInt), nc, fp);
    result += fread(elements[i].edges, sizeof(GenmapInt), nc, fp);
#endif
  }

#ifdef MPI
  MPI_File_close(&fh);
#else
  fclose(fp);
#endif

  GenmapFree(headerArray);

  return 0;
}
