#include <genmap-impl.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

GenmapUInt32 genmap_srand_initialized = 0;
//
// Vector operations
//
int GenmapCreateVector(GenmapVector *x, GenmapInt size) {
  /* Asserts:
       - size > 0
  */
  assert(size > 0);

  GenmapMalloc(1, x);
  if(*x == NULL) {
    printf("malloc failed in %s:%d", __FILE__, __LINE__);
    return 1;
  }

  (*x)->size = size;
  (*x)->data = NULL;

  GenmapMalloc(size, &(*x)->data);
  if((*x)->data == NULL) {
    printf("malloc failed in %s:%d", __FILE__, __LINE__);
    return 1;
  }

  return 0;
}

int GenmapVectorsEqual(GenmapVector x, GenmapVector y, GenmapScalar tol) {
  /* Asserts:
       - size of y == size of x
  */
  assert(x->size == y->size);

  for(GenmapInt i = 0; i < x->size; i++) {
    assert(!isnan(x->data[i]) && !isnan(y->data[i]));
  }

  GenmapInt n = x->size;
  for(GenmapInt i = 0; i < n; i++) {
    if(fabs(x->data[i] - y->data[i]) > tol) {
      return 0;
    }
  }

  return 1;
}

int GenmapSetVector(GenmapVector x, GenmapScalar *array) {
  memcpy(x->data, array, sizeof(GenmapScalar) * x->size);
  return 0;
}

int GenmapDestroyVector(GenmapVector x) {
  if(x->data) {
    free(x->data);
    x->data = NULL;
  }

  if(x) {
    free(x);
    x = NULL;
  }

  return 0;
}

int GenmapCopyVector(GenmapVector y, GenmapVector x) {
  /* Asserts:
       - size y = size x
  */
  assert(y->size == x->size);

  GenmapInt n = x->size;
  for(GenmapInt i = 0; i < n; i++) {
    y->data[i] = x->data[i];
  }

  return 0;
}

GenmapScalar GenmapNormVector(GenmapVector x, GenmapInt p) {
  assert(x->size > 0);

  GenmapInt n = x->size;
  GenmapScalar norm = 0;

  if(p == 1) {
    for(GenmapInt i = 0; i < n; i++) {
      norm += fabs(x->data[i]);
    }
  } else if(p == 2) {
    for(GenmapInt i = 0; i < n; i++) {
      norm += x->data[i] * x->data[i];
    }
    norm = sqrt(norm);
  } else if(p == -1) {
    norm = fabs(x->data[0]);

    for(GenmapInt i = 1; i < n; i++) {
      if(fabs(x->data[i]) > norm) norm = fabs(x->data[i]);
    }
  }

  return norm;
}

int GenmapScaleVector(GenmapVector y, GenmapVector x,  GenmapScalar alpha) {
  /* asserts:
       - size x = size y
  */
  assert(x->size == y->size);

  GenmapInt n = x->size;
  for(GenmapInt i = 0; i < n; i++) {
    y->data[i] = alpha * x->data[i];
  }

  return 0;
}

int GenmapCreateOnesVector(GenmapVector *x, GenmapInt size) {
  GenmapCreateVector(x, size);

  for(GenmapInt i = 0; i < size; i++) {
    (*x)->data[i] = 1.;
  }

  return 0;
}

int GenmapCreateZerosVector(GenmapVector *x, GenmapInt size) {
  GenmapCreateVector(x, size);

  for(GenmapInt i = 0; i < size; i++) {
    (*x)->data[i] = 0.;
  }

  return 0;
}

int GenmapPrintVector(GenmapVector x) {
  /* Asserts:
       - size x > 0
  */
  assert(x->size > 0);

  printf("(%lf", x->data[0]);
  for(GenmapInt i = 1; i < x->size - 1; i++) {
    printf(", %lf", x->data[i]);
  }

  if(x->size > 1) {
    printf(", %lf)", x->data[x->size - 1]);
  } else {
    printf(")");
  }

  return 0;
}
////------------------------------------------------------------------------------
//void random_vector(Vector *x, int32 size, int32 seed) {
//  create_vector(x, size);
//
//  if(!genmap_srand_initialized) {
//    srand(time(NULL) + seed);
//    genmap_srand_initialized = 1;
//  }
//
//  for(int32 i = 0; i < size; i++) {
//    x->data[i] = (double) rand() / RAND_MAX * 2. - 1.;
//  }
//}
////------------------------------------------------------------------------------
//double dot_vector(Vector *x, Vector *y) {
//  /* Asserts:
//       - size y = size x
//  */
//  assert(y->size == x->size);
//
//  double dot = 0.;
//
//  int32 n = x->size;
//  for(int32 i = 0; i < n; i++) {
//    dot += y->data[i] * x->data[i];
//  }
//
//  return dot;
//}
////------------------------------------------------------------------------------
