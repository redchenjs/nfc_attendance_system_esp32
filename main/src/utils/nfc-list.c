/*-
 * Free/Libre Near Field Communication (NFC) library
 *
 * Libnfc historical contributors:
 * Copyright (C) 2009      Roel Verdult
 * Copyright (C) 2009-2013 Romuald Conty
 * Copyright (C) 2010-2012 Romain Tartière
 * Copyright (C) 2010-2013 Philippe Teuwen
 * Copyright (C) 2012-2013 Ludovic Rousseau
 * See AUTHORS file for a more comprehensive list of contributors.
 * Additional contributors of this file:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  1) Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  2 )Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Note that this license only applies on the examples, NFC library itself is under LGPL
 *
 */

/**
 * @file nfc-list.c
 * @brief Lists the first target present of each founded device
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <buses/emdev.h>
//#include <stm32f10x.h>

#include "nfc/nfc.h"

#define MAX_TARGET_COUNT 16

extern nfc_emdev emdev;   
   
//static nfc_device *pnd;


void
print_nfc_target(const nfc_target *pnt, bool verbose)
{
  char *s;
  if(str_nfc_target(&s, pnt, verbose) < 0)
  {
    printf("ram size not enough\n");
    return ;
  }

  printf("%s", s);     
  nfc_free(s);
}

nfc_target ant[MAX_TARGET_COUNT];
int res = 0;  
  bool verbose = true; 
  int mask = 0xFF;
  
int
list_passive_targets()
{
  /*@verbose  indicates whether print in details.*/
  nfc_device *pnd;

  
/* mask value
 * poll only for types according to bitfield:
 *   1: ISO14443A
 *   2: Felica (212 kbps)
 *   4: Felica (424 kbps)
 *   8: ISO14443B
 *   16: ISO14443B'
 *   32: ISO14443B-2 ST SRx
 *   64: ISO14443B-2 ASK CTx
 *   128: Jewel
 * So 255 (default) polls for all types.
 * Note that if 16, 32 or 64 then 8 is selected too.
 **/
  
  // Force TypeB for all derivatives of B
  if (mask & 0x70)
    mask |= 0x08;
//      mask = 0x0f;
  pnd = nfc_open(&emdev);

  if (pnd == NULL) {
    return -1;
  }
  if (nfc_initiator_init(pnd) < 0) {
    nfc_perror(pnd, "nfc_initiator_init");
    return -1;
  }

  printf("NFC device opened\n");

  nfc_modulation nm;

  if (mask & 0x1) {
    nm.nmt = NMT_ISO14443A;
    nm.nbr = NBR_106;
    // List ISO14443A targets

    if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
     
      int n;

      if (verbose || (res > 0)) {
        printf("%d ISO14443A passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
      }

      for (n = 0; n < res; n++) {
        print_nfc_target(&ant[n], verbose);
        
        printf("\n");
      }
    }

  }
/*
  if (mask & 0x02) {
    nm.nmt = NMT_FELICA;
    nm.nbr = NBR_212;
    // List Felica tags
    if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
      int n;
      if (verbose || (res > 0)) {
        printf("%d Felica (212 kbps) passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
      }
      for (n = 0; n < res; n++) {
        print_nfc_target(&ant[n], verbose);
        printf("\n");
      }
    }
  }

  if (mask & 0x04) {
    nm.nmt = NMT_FELICA;
    nm.nbr = NBR_424;
    if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
      int n;
      if (verbose || (res > 0)) {
        printf("%d Felica (424 kbps) passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
      }
      for (n = 0; n < res; n++) {
        print_nfc_target(&ant[n], verbose);
        printf("\n");
      }
    }
  }

  if (mask & 0x08) {
    nm.nmt = NMT_ISO14443B;
    nm.nbr = NBR_106;
    // List ISO14443B targets
    if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
      int n;
      if (verbose || (res > 0)) {
        printf("%d ISO14443B passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
      }
      for (n = 0; n < res; n++) {
        print_nfc_target(&ant[n], verbose);
        printf("\n");
      }
    }
  }
  
#if 0
  if (mask & 0x10) {
    nm.nmt = NMT_ISO14443BI;
    nm.nbr = NBR_106;
    // List ISO14443B' targets
    if ((res =  nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
      int n;
      if (verbose || (res > 0)) {
        printf("%d ISO14443B' passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
      }
      for (n = 0; n < res; n++) {
        print_nfc_target(&ant[n], verbose);
        printf("\n");
      }
    }
  }

  if (mask & 0x20) {
    nm.nmt = NMT_ISO14443B2SR;
    nm.nbr = NBR_106;
    // List ISO14443B-2 ST SRx family targets
    if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
      int n;
      if (verbose || (res > 0)) {
        printf("%d ISO14443B-2 ST SRx passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
      }
      for (n = 0; n < res; n++) {
        print_nfc_target(&ant[n], verbose);
        printf("\n");
      }
    }
  }

  if (mask & 0x40) {
    nm.nmt = NMT_ISO14443B2CT;
    nm.nbr = NBR_106;
    // List ISO14443B-2 ASK CTx family targets
    if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
      int n;
      if (verbose || (res > 0)) {
        printf("%d ISO14443B-2 ASK CTx passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
      }
      for (n = 0; n < res; n++) {
        print_nfc_target(&ant[n], verbose);
        printf("\n");
      }
    }
  }
#endif

  if (mask & 0x80) {
    nm.nmt = NMT_JEWEL;
    nm.nbr = NBR_106;
    // List Jewel targets
    if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
      int n;
      if (verbose || (res > 0)) {
        printf("%d Jewel passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
      }
      for (n = 0; n < res; n++) {
        print_nfc_target(&ant[n], verbose);
        printf("\n");
      }
    }
  }
  */
  nfc_close(pnd);

  return 0;
}
