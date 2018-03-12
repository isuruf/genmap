#include <math.h>
#include <assert.h>
#include <float.h>

#include "laplacian.h"
#include "lanczos.h"

//------------------------------------------------------------------------------
void lanczos(Vector *alpha, Vector *beta, Vector **q, struct comm *c,
             struct header* mapheader, struct element *elements, Vector *init,
             int32 iter) {
  assert(alpha->size == iter);
  assert(alpha->size == beta->size + 1);

  Vector q0, q1, u;
  struct gs_data *goph;
  double norm_q1, b = 0.;

  int32 n = init->size;
  gop_init(&goph, c);

  // Set q_0 and beta_0 to zero (both uses 0-indexing)
  zeros_vector(&q0, n);
  beta->vv[0] = 0.;

  *q = malloc(sizeof(q0) * iter);

  // Create vector u and q1
  create_vector(&u, n);
  create_vector(&q1, n);
  copy_vector(&q1, init);

  norm_q1 = dot_vector(&q1, &q1);
  gop(&norm_q1, goph, gs_double, gs_add, 0); norm_q1 = sqrt(norm_q1);
  scale_vector(&q1, &q1, 1. / norm_q1);

  int32 nc = mapheader->nc;
  int32 lelt = mapheader->lelt;
  int32 *glo_num; glo_num = malloc(sizeof(int32) * lelt);
  for(int32 i = 0; i < lelt; i++) {
    glo_num[i] = elements[i].globalId;
  }
  double *weights = NULL;

  struct gs_data *axh;
  ax_init(&axh, &weights, c, nc * lelt, lelt, glo_num);

  for(int32 k = 0; k < iter; k++) {
    // Store q1
    create_vector(*q + k, n);
    copy_vector(*q + k, &q1);

    // Multiplication by the laplacian
    ax(&u, &q1, axh, weights, nc);

    alpha->vv[k] = dot_vector(&q1, &u);
    gop(&alpha->vv[k], goph, gs_double, gs_add, 0);

    z_axpby_vector(&u, &u, 1., &q0, -b);
    z_axpby_vector(&u, &u, 1., &q1, -alpha->vv[k]);

    b = dot_vector(&u, &u);
    gop(&b, goph, gs_double, gs_add, 0);
    b = sqrt(b);
    if(k < iter - 1) {
      beta->vv[k] = b;
    }

    copy_vector(&q0, &q1);

//    if (abs(beta->vv[k]) < DBL_EPSILON) {
//      beta->size = k;
//      alpha->size = k + 1;
//      return;
//    }

    scale_vector(&q1, &u, 1. / beta->vv[k]);
  }
}
//------------------------------------------------------------------------------
void lanczos_old(Vector *alpha, Vector *beta, Vector **q, struct comm *c,
                 int32 *glo_num, Vector *init, int32 nc, int32 lelt, int32 iter) {
  assert(alpha->size == iter);
  assert(alpha->size == beta->size + 1);

  int32 n = init->size;

  struct gs_data *goph; gop_init(&goph, c);

  // Remove components of (1, 1, ... 1)
//  double sum = 0.0;
//  for (int32 i = 0; i < n; i++) {
//    sum += init->vv[i];
//  }
//  gop(&sum, goph, gs_double, gs_add, 0);
//  sum /= sqrt(nel);
//  for (int32 i = 0; i < n; i++) {
//    init->vv[i] -= sum;
//  }

  double norm_q1, b = 0.;
  Vector q0, q1, u;

  // Set q_0 and beta_0 to zero (both uses 0-indexing)
  zeros_vector(&q0, n);
  beta->vv[0] = 0.;

  *q = malloc(sizeof(q0) * iter);

  // Create vector u and q1
  create_vector(&u, n);
  create_vector(&q1, n);
  copy_vector(&q1, init);

  norm_q1 = dot_vector(&q1, &q1);
  gop(&norm_q1, goph, gs_double, gs_add, 0); norm_q1 = sqrt(norm_q1);
  scale_vector(&q1, &q1, 1. / norm_q1);

  struct gs_data *axh; double *weights = NULL;
  ax_init(&axh, &weights, c, nc * lelt, lelt, glo_num);

  for(int32 k = 0; k < iter; k++) {
    // Store q1
    create_vector(*q + k, n);
    copy_vector(*q + k, &q1);

    // Multiplication by the laplacian
    ax(&u, &q1, axh, weights, nc);

    alpha->vv[k] = dot_vector(&q1, &u);
    gop(&alpha->vv[k], goph, gs_double, gs_add, 0);

    z_axpby_vector(&u, &u, 1., &q0, -b);
    z_axpby_vector(&u, &u, 1., &q1, -alpha->vv[k]);

    b = dot_vector(&u, &u);
    gop(&b, goph, gs_double, gs_add, 0);
    b = sqrt(b);
    if(k < iter - 1) {
      beta->vv[k] = b;
    }

    copy_vector(&q0, &q1);

//    if (abs(beta->vv[k]) < DBL_EPSILON) {
//      beta->size = k;
//      alpha->size = k + 1;
//      return;
//    }

    scale_vector(&q1, &u, 1. / beta->vv[k]);
  }
}

//------------------------------------------------------------------------------
void lanczos_serial(Vector *alpha, Vector *beta, CSRMatrix *A, Vector *init) {
  /* Asserts:
       - #Rows of A == size of alpha  == size of beta + 1 = size of init
  */
  assert(A->nrows == alpha->size);
  assert(A->nrows == init->size);
  assert(A->nrows == beta->size + 1);

  int32 n = A->nrows;
  double norm_q1, b = 0.;
  Vector q0, q1, u;

  // Set q_0 and beta_0 to zero (both uses 0-indexing)
  zeros_vector(&q0, n);
  beta->vv[0] = 0.;

  // Create vector u
  create_vector(&u, n);

  // Set q1 to normalized initial vector
  create_vector(&q1,    n);
  copy_vector(&q1, init);
  norm_q1 = norm_vector(&q1, 2);
  mult_scalar_add_vector(&q1, 0., &q1, 1. / norm_q1);

  for(int32 k = 0; k < n; k++) {
    csr_matrix_vector_multiply(&u, A, &q1);

    alpha->vv[k] = dot_vector(&q1, &u);

    mult_scalar_add_vector(&u, 1., &q0, -b);
    mult_scalar_add_vector(&u, 1., &q1, -alpha->vv[k]);

    b = norm_vector(&u, 2);
    if(k < n - 1) {
      beta->vv[k] = b;
    }

    copy_vector(&q0, &q1);

//    if (beta->vv[k] < DBL_EPSILON) {
//      return;
//    }

    mult_scalar_add_vector(&q1, 0., &u, 1. / beta->vv[k]);
  }
}

//------------------------------------------------------------------------------
void lanczos_serial2(Vector *alpha, Vector *beta, CSRMatrix *A, Vector *init) {
  /* Asserts:
       - #Rows of A == size of alpha  == size of beta + 1 = size of init
  */
  assert(A->nrows == alpha->size);
  assert(A->nrows == beta->size + 1);
  assert(A->nrows == init->size);

  int32 i = 0, n = A->nrows;
  double b1 = 1.;
  Vector v, r, v1, p;

  create_vector(&v, n); create_vector(&r, n); create_vector(&p, n);
  copy_vector(&v, init);
  mult_scalar_add_vector(&v, 0., &v, 1. / norm_vector(&v, 2));
  copy_vector(&r, &v);
  zeros_vector(&v1, n);

  while(i < n - 1) {
    mult_scalar_add_vector(&v, 0., &r, 1. / b1);

    csr_matrix_vector_multiply(&p, A, &v);

    alpha->vv[i] = dot_vector(&p, &v);

    copy_vector(&r, &p);
    mult_scalar_add_vector(&r, 1., &v, -alpha->vv[i]);
    mult_scalar_add_vector(&r, 1., &v1, -b1);

    b1 = norm_vector(&r, 2);
    beta->vv[i] = b1;

    copy_vector(&v1, &v);
    i = i + 1;
  }

  mult_scalar_add_vector(&v, 0., &r, 1. / b1);
  csr_matrix_vector_multiply(&p, A, &v);
  alpha->vv[i] = dot_vector(&p, &v);
}

//------------------------------------------------------------------------------
