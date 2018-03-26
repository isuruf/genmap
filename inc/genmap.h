#ifndef _GENMAP_H_
#define _GENMAP_H_
#endif

//------------------------------------------------------------------------------
// Genmap types
//
typedef int GenmapInt32;
typedef unsigned int GenmapUInt32;

typedef long GenmapInt64;
typedef unsigned long GenmapUInt64;

typedef double GenmapScalar;

typedef struct GenmapVector_private *GenmapVector;

//
// Genmap tolerances
//
#define GENMAP_TOL 1e-12
//
// GenmapVector operations
//
int GenmapCreateVector(GenmapVector *x, GenmapInt32 size);
int GenmapSetVector(GenmapVector x, GenmapScalar *array);
int GenmapGetVector(GenmapVector x, GenmapScalar *array);

int GenmapCreateRandomVector(GenmapVector *x, GenmapInt32 size,
                             GenmapInt32 seed);
int GenmapCreateOnesVector(GenmapVector *x, GenmapInt32 size);
int GenmapCreateZerosVector(GenmapVector *x, GenmapInt32 size);

int GenmapScaleVector(GenmapVector y, GenmapVector x, GenmapScalar alpha);
int GenmapAxpbyVector(GenmapVector z, GenmapVector x, GenmapScalar alpha,
                      GenmapVector *y, GenmapScalar beta);

int GenmapVectorsEqual(GenmapVector x, GenmapVector y, GenmapScalar tol);
int GenmapCopyVector(GenmapVector x, GenmapVector y);
GenmapScalar GenmapDotVector(GenmapVector x, GenmapVector y);
GenmapScalar GenmapNormVector(GenmapVector x, GenmapInt32 p);

int GenmapPrintVector(GenmapVector x);
int GenmapDestroyVector(GenmapVector x);
//
// Linear solve
//
int GenmapSymTriDiagSolve(GenmapVector x, GenmapVector b, GenmapVector alpha,
                          GenmapVector beta);
//
// Power and inverse power iterations
//
int GenmapPowerIter(GenmapVector eVector, GenmapVector alpha,
                    GenmapVector beta, GenmapVector init, GenmapInt32 iter);

int GenmapInvPowerIter(GenmapVector eVector, GenmapVector alpha,
                       GenmapVector beta, GenmapVector init, GenmapInt32 iter);
