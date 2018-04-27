#include <default.h>

#include <stdio.h>
//
// GenmapHandle: Create, Destroy
//
int GenmapCreateHandle_default(GenmapHandle h) {
  h->global = NULL;
  h->local = NULL;
  h->CreateComm = GenmapCreateComm_default;
  h->DestroyComm = GenmapDestroyComm_default;

  h->header = NULL;
  h->CreateHeader = GenmapCreateHeader_default;
  h->DestroyHeader = GenmapDestroyHeader_default;

  h->elements = NULL;
  h->CreateElements = GenmapCreateElements_default;
  h->DestroyElements = GenmapDestroyElements_default;

  h->Id = GenmapId_default;
  h->Np = GenmapNp_default;

  h->Ax = GenmapAx_default;
  h->AxInit = GenmapAxInit_default;

  h->Gop = GenmapGop_default;
  h->Read = GenmapRead_default;

  h->Destroy = GenmapDestroyHandle_default;

  return 0;
}

int GenmapDestroyHandle_default(GenmapHandle h) {
  return 0;
}