#include <genmap-impl.h>

#include <math.h>
#include <stdio.h>
//
// Algorithms
//
// Power and inverse power iterations
int GenmapPowerIter(GenmapVector eVector, GenmapVector alpha,
                    GenmapVector beta, GenmapVector init, GenmapInt iter) {
  assert(alpha->size == beta->size + 1);
  assert(alpha->size == eVector->size);

  GenmapInt n = alpha->size;

  GenmapVector x, y;
  GenmapCreateVector(&x, n);
  GenmapCreateVector(&y, n);
  GenmapCopyVector(x, init);

  if(n == 1) {
    eVector->data[0] = alpha->data[0];
    return 0;
  } else {
    for(GenmapInt j = 0; j < iter; j++) {
      // y = Ax
      y->data[0] = alpha->data[0] * x->data[0] + beta->data[0] * x->data[1];
      for(GenmapInt i = 1; i < n - 1; i++) {
        y->data[i] = beta->data[i - 1] * x->data[i - 1] + alpha->data[i] *
                     x->data[i] +
                     beta->data[i] * x->data[i + 1];
      }
      y->data[n - 1] = beta->data[n - 2] * x->data[n - 2] + alpha->data[n - 1]
                       *
                       x->data[n - 1];

      // Normalize by inf-norm(y)
      if(j != iter - 1) {
        GenmapScalar norm = GenmapNormVector(y, -1);
        GenmapScaleVector(y, y, 1.0 / norm);
      }

      GenmapCopyVector(x, y);
    }
  }

  GenmapCopyVector(eVector, y);

  GenmapDestroyVector(x);
  GenmapDestroyVector(y);

  return 0;
}

int GenmapPowerIterNew(GenmapVector eVector, void (*Ax)(GenmapVector ax,
                       GenmapVector x, void* data), void *data,
                       GenmapVector init, GenmapInt iter) {
  assert(eVector->size == init->size);
  GenmapInt n = init->size;

  GenmapVector x, y;
  GenmapCreateVector(&x, n);
  GenmapCreateVector(&y, n);
  GenmapCopyVector(x, init);

  for(GenmapInt j = 0; j < iter; j++) {
    // y = Ax
    Ax(y, x, data);
    // Normalize by inf-norm(y)
    if(j != iter - 1)
      GenmapScaleVector(y, y, 1.0 / GenmapNormVector(y, -1));

    GenmapCopyVector(x, y);
  }

  GenmapCopyVector(eVector, y);

  GenmapDestroyVector(x);
  GenmapDestroyVector(y);

  return 0;
}

int GenmapInvPowerIter(GenmapVector eVector, GenmapVector alpha,
                       GenmapVector beta, GenmapVector init, GenmapInt iter) {
  assert(alpha->size == beta->size + 1);
  assert(alpha->size == eVector->size);

  GenmapInt n = alpha->size;

  GenmapVector x, y;

  if(n == 1) {
    eVector->data[0] = alpha->data[0];
    return 0;
  } else {
    GenmapCreateVector(&x, n);
    GenmapCreateVector(&y, n);

    GenmapCopyVector(x, init);
    for(GenmapInt j = 0; j < iter; j++) {
      // Ay = x
      GenmapSymTriDiagSolve(y, x, alpha, beta);

      // Normalize by inf-norm(y)
      if(j != iter - 1)
        GenmapScaleVector(y, y, 1.0 / GenmapNormVector(y, -1));

      GenmapCopyVector(x, y);
    }
  }

  GenmapCopyVector(eVector, y);

  GenmapDestroyVector(x);
  GenmapDestroyVector(y);

  return 0;
}
//
// Linear solve for Symmetric Tridiagonal Matrix
//
int GenmapSymTriDiagSolve(GenmapVector x, GenmapVector b,
                          GenmapVector alpha,
                          GenmapVector beta) {
  assert((x->size == b->size) && (x->size == alpha->size));
  assert(alpha->size == beta->size + 1);
  assert(b->size > 0);

  GenmapInt n = b->size;

  GenmapVector diag;
  GenmapCreateVector(&diag, n);
  GenmapCopyVector(diag, alpha);

  GenmapCopyVector(x, b);

  for(GenmapInt i = 0; i < n - 1; i++) {
    GenmapScalar m = (beta->data[i] / diag->data[i]);
    x->data[i + 1] = x->data[i + 1] - m * x->data[i];
    diag->data[i + 1] = diag->data[i + 1] - m * beta->data[i];
  }

  x->data[n - 1] = x->data[n - 1] / diag->data[n - 1];

  for(GenmapInt i = n - 2; i >= 0; i--) {
    x->data[i] = (x->data[i] - beta->data[i] * x->data[i + 1]) /
                 diag->data[i];
  }

  GenmapDestroyVector(diag);
  return 0;
}
//
//
void GenmapLanczos(GenmapHandle h, GenmapComm c, GenmapVector init,
                   GenmapInt iter, GenmapVector **q, GenmapVector alpha,
                   GenmapVector beta) {
  assert(alpha->size == iter);
  assert(alpha->size == beta->size + 1);
  assert(init->size == h->header->lelt);

  GenmapVector q0, q1, u;
  GenmapScalar normq1, b = 0.;

  GenmapInt lelt = h->header->lelt;

  // Create vector u and q1
  GenmapCreateVector(&q1, lelt);
  GenmapCopyVector(q1, init);
  GenmapCreateVector(&u, lelt);

  // Set q_0 and beta_0 to zero (both uses 0-indexing)
  GenmapCreateZerosVector(&q0, lelt);
  beta->data[0] = 0.;

  // Allocate memory for q-vectors
  if(*q == NULL)
    GenmapMalloc(iter, q);

  normq1 = GenmapDotVector(q1, q1);
  h->Gop(c, &normq1, 1, GENMAP_SUM);
  normq1 = sqrt(normq1);
  GenmapScaleVector(q1, q1, 1. / normq1);

  // Store Local Laplacian weights
  GenmapVector weights;
  GenmapCreateVector(&weights, lelt);
  h->AxInit(h, c, weights);

  for(GenmapInt k = 0; k < iter; k++) {
    // Store q1
    GenmapCreateVector(&(*q)[k], lelt);
    GenmapCopyVector((*q)[k], q1);

    // Multiplication by the laplacian
    h->Ax(h, c, q1, weights, u);

    alpha->data[k] = GenmapDotVector(q1, u);
    h->Gop(c, &alpha->data[k], 1, GENMAP_SUM);

    GenmapAxpbyVector(u, u, 1., q0, -b);
    GenmapAxpbyVector(u, u, 1., q1, -alpha->data[k]);

    b = GenmapDotVector(u, u);
    h->Gop(c, &b, 1, GENMAP_SUM);
    b = sqrt(b);

    if(k < iter - 1) {
      beta->data[k] = b;
    }

    GenmapCopyVector(q0, q1);

    if(fabs(beta->data[k]) < GENMAP_TOL) {
      beta->size = k;
      alpha->size = k + 1;
      GenmapDestroyVector(q0);
      GenmapDestroyVector(q1);
      GenmapDestroyVector(u);
      GenmapDestroyVector(weights);
      return;
    }

    if(k < iter - 1) {
      GenmapScaleVector(q1, u, 1. / beta->data[k]);
    }
  }

  GenmapDestroyVector(q0);
  GenmapDestroyVector(q1);
  GenmapDestroyVector(u);
  GenmapDestroyVector(weights);
}

void GenmapPrimeFactors(GenmapInt n, GenmapInt *primes) {
}

void GenmapRQI(GenmapHandle h, GenmapVector v) {
  // Calculate Lv, v^T(Lv)
}

void GenmapFiedlerMinMax(GenmapHandle h, GenmapScalar *min,
                         GenmapScalar *max) {
  *min = 1; *max = -1;

  GenmapElements e = GenmapGetElements(h);
  for(int i = 0; i < h->header->lelt; i++) {
    if(e[i].fiedler < *min) {
      *min = e[i].fiedler;
    }
    if(e[i].fiedler > *max) {
      *max = e[i].fiedler;
    }
  }
  h->Gop(h->local, min, 1, GENMAP_MIN);
  h->Gop(h->local, max, 1, GENMAP_MAX);
}

GenmapInt GenmapSetProcessorId(GenmapHandle h) {
  GenmapScalar min, max;
  GenmapFiedlerMinMax(h, &min, &max);
  GenmapScalar range = max - min;

  GenmapInt np = h->Np(h->local);
  GenmapInt nbins = np;
  GenmapInt lelt = h->header->lelt;
  GenmapElements elements = GenmapGetElements(h);

  for(GenmapElements p = elements, e = p + lelt; p != e; p++) {
    GenmapInt id;
    for(id = 0; id < np; id++) {
      GenmapScalar start = min + (range * id) / nbins;
      GenmapScalar end = min + (range * (id + 1)) / nbins;
      if(start <= p->fiedler && p->fiedler < end) {
        p->proc = id;
        break;
      }
    }
    if(id == np) p->proc = np - 1;
  }

  return 0;
}

void GenmapFiedler(GenmapHandle h, GenmapComm c, int global) {
  // 1. Do lanczos in local communicator.
  GenmapInt iter = 500;
  GenmapInt lelt = h->header->lelt;
  GenmapVector initVec, alphaVec, betaVec;

  if(lelt == 0) return;

  GenmapCreateVector(&initVec, h->header->lelt);
  GenmapScalar sum = 0.0;
  GenmapElements elements = GenmapGetElements(h);
  for(GenmapInt i = 0;  i < lelt; i++) {
    if(global)
      initVec->data[i] = elements[i].globalId;
    else
      initVec->data[i] = elements[i].fiedler;
    sum += initVec->data[i];
  }

  h->Gop(c, &sum, 1, GENMAP_SUM);

  for(GenmapInt i = 0;  i < lelt; i++) {
    initVec->data[i] -= sum / h->header->nel;
  }

  GenmapCreateVector(&alphaVec, iter);
  GenmapCreateVector(&betaVec, iter - 1);
  GenmapVector *q = NULL;
  // TODO: Lanczos doesn't work well for smaller matrices
  // We need to fix this
  GenmapLanczos(h, c, initVec, iter, &q, alphaVec, betaVec);
  iter = alphaVec->size;

  // 2. Do inverse power iteration on local communicator and find
  // local Fiedler vector.
  GenmapVector evLanczos, evTriDiag, evInit;
  GenmapCreateVector(&evTriDiag, iter);
  GenmapCreateVector(&evInit, iter);
  sum = 0.0;
  for(GenmapInt i = 0; i < iter; i++) {
    evInit->data[i] = i + 1;
    sum += evInit->data[i];
  }
  for(GenmapInt i = 0;  i < iter; i++) {
    evInit->data[i] -= sum / iter;
  }

  GenmapInvPowerIter(evTriDiag, alphaVec, betaVec, evInit, 100);

  // Multiply tri-diagonal matrix by [q1, q2, ...q_{iter}]
  GenmapCreateZerosVector(&evLanczos, lelt);
  for(GenmapInt i = 0; i < lelt; i++) {
    for(GenmapInt j = 0; j < iter; j++) {
      evLanczos->data[i] += q[j]->data[i] * evTriDiag->data[j];
    }
  }

  GenmapScalar lNorm = 0;
  for(GenmapInt i = 0; i < lelt; i++) {
    lNorm += evLanczos->data[i] * evLanczos->data[i];
  }

  h->Gop(c, &lNorm, 1, GENMAP_SUM);
  GenmapScaleVector(evLanczos, evLanczos, 1. / sqrt(lNorm));
  for(GenmapInt i = 0; i < lelt; i++) {
    elements[i].fiedler = evLanczos->data[i];
  }

  // n. Destory the data structures
  GenmapDestroyVector(initVec);
  GenmapDestroyVector(alphaVec);
  GenmapDestroyVector(betaVec);
  GenmapDestroyVector(evLanczos);
  GenmapDestroyVector(evTriDiag);
  GenmapDestroyVector(evInit);
  for(GenmapInt i = 0; i < iter; i++) {
    GenmapDestroyVector(q[i]);
  }
  GenmapFree(q);
}

void GenmapRSB(GenmapHandle h) {
  GenmapInt id = h->Id(h->local);
  GenmapInt np = h->Np(h->local);
  GenmapInt lelt = h->header->lelt;
  GenmapInt nel = h->header->nel;
  GenmapInt start = h->header->start;
  GenmapElements elements = GenmapGetElements(h);

  // Data needed to use gslib
  struct crystal cr;
  crystal_init(&cr, &(h->local->gsComm));
  GenmapInt out[2][1], buf[2][1];

  // Calculate the global Fiedler vector, local communicator
  // must be initialized using the global communicator, we never
  // touch global communicator
  while(h->Np(h->local) > 1) {
    GenmapFiedler(h, h->local, 1);
    // sort locally according to Fiedler vector
    buffer buf0 = null_buffer;
    sarray_sort_2(struct GenmapElement_private, elements, lelt, fiedler,
                  TYPE_DOUBLE, globalId, TYPE_INT, &buf0);
    // We proceed with bisection only if we have more than 1 processor in
    // communicator; otherwise, return and set values for Fortran rsb
    // Transfer elements to processors based on fiedler value to find the
    // median
    GenmapSetProcessorId(h);
    sarray_transfer(struct GenmapElement_private, &(h->elementArray), proc,
                    0, &cr);
    elements = GenmapGetElements(h);
    lelt = h->header->lelt = h->elementArray.n;

    // sort locally again -- now we have everything sorted
    buffer buf1 = null_buffer;
    sarray_sort_2(struct GenmapElement_private, elements, lelt, fiedler,
                  TYPE_DOUBLE, globalId, TYPE_INT, &buf1);

    id = h->Id(h->local);
    np = h->Np(h->local);
    GenmapInt bin = (id >= (np + 1) / 2);

    // find the median -- first do a scan to find position of my
    // elements. Combine these two into one.
    comm_scan(out, &(h->local->gsComm), gs_int, gs_add, &lelt, 1, buf);
    start = h->header->start = out[0][0];
    GenmapInt medianPos = (nel + 1) / 2;

    if(start <=  medianPos && medianPos <= start + lelt) {
      GenmapInt medianId;
      medianId = h->Id(h->local);
      // sarray_transfer -- elements with higher fiedler values are sent
      // into processor with medianId+1 if it exist, otherwise elements
      // with lower fiedler values are sent into processor medianId-1
      if(bin == 0) {
        for(int i = 0; i < medianPos - start; i++) {
          elements[i].proc = medianId;
        }
        for(int i = medianPos - start; i < lelt; i++) {
          elements[i].proc = medianId + 1;
        }
      } else {
        for(int i = 0; i < medianPos - start; i++) {
          elements[i].proc = medianId - 1;
        }
        for(int i = medianPos - start; i < lelt; i++) {
          elements[i].proc = medianId;
        }
      }
    } else {
      for(int i = 0; i < lelt; i++) {
        elements[i].proc = h->Id(h->local);
      }
    }

    // Do the transfer once again
    sarray_transfer(struct GenmapElement_private, &(h->elementArray), proc,
                    0, &cr);
    elements = GenmapGetElements(h);
    lelt = h->header->lelt = h->elementArray.n;

    // Now it is time to split the communicator
    GenmapCommExternal local;
#ifdef GENMAP_MPI
    MPI_Comm_split(h->local->gsComm.c, bin, id, &local);
#else
    local = 0;
#endif
    // finalize the crystal router
    crystal_free(&cr);
    GenmapDestroyComm(h->local);

    // Create new communicator
    GenmapCreateComm(&(h->local), local);
    crystal_init(&cr, &(h->local->gsComm));

    comm_scan(out, &(h->local->gsComm), gs_int, gs_add, &lelt, 1, buf);
    start = h->header->start = out[0][0];
    nel = h->header->nel = out[1][0];
    id = h->Id(h->local);
    np = h->Np(h->local);
    elements = GenmapGetElements(h);

    GenmapInt pNel = nel / np;
    GenmapInt nrem = nel - pNel * np;
    GenmapInt idCount = 0;
    while(idCount * pNel + ((idCount < nrem) ? idCount : nrem) < start)
      idCount++;

    GenmapInt upLimit = idCount * pNel + ((idCount < nrem) ? idCount :
                                          nrem);
    GenmapInt downLimit = start;
    do {
      GenmapInt end = upLimit < lelt ? upLimit : lelt;
      for(int i = downLimit - start; i < end;
          i++) elements[i].proc = idCount - 1;
      downLimit = upLimit;
      idCount++;
      upLimit = idCount * pNel + ((idCount < nrem) ? idCount : nrem);
    } while(downLimit < start + lelt);

    sarray_transfer(struct GenmapElement_private, &(h->elementArray), proc,
                    0, &cr);
    elements = GenmapGetElements(h);
    lelt = h->header->lelt = h->elementArray.n;

    comm_scan(out, &(h->local->gsComm), gs_int, gs_add, &lelt, 1, buf);
    start = h->header->start = out[0][0];
    nel = h->header->nel = out[1][0];
  }
}
