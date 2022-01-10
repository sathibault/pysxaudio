/*

Copyright (c) 2021, StreamLogic LLC.
All Rights Reserved.

This source may not be used without the explicit permission of StreamLogic LLC.

*/

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <map>
#include <vector>


static std::map<int,std::vector<int8_t> *> twiddles;

static const std::vector<int8_t>& get_twiddle(int N) {
  auto entry = twiddles.find(N);
  if (entry == twiddles.end()) {
    int halfPi = N/4;
    int count = N/2 + halfPi;
    double k = -6.283185307179586/N;
    auto tw = new std::vector<int8_t>();
    for (int i = 0; i < count; i++)
      tw->push_back((int)round(cos(i*k)*64));
    twiddles[N] = tw;
    return *tw;
  } else
    return *(entry->second);
}

static int bitrev(int a, int bits) {
  uint16_t b = 0;
  for (int i = 0; i < bits; i++) {
    b = b << 1;
    if (a&1) b |= 1;
    a = a >> 1;
  }
  return b;
}

static uint32_t cpxrot(int8_t co, int16_t r, int8_t si, int16_t i) {
  int pr = co*r - si*i;
  int pi = si*r + co*i;
  int16_t fxr = pr>>6;
  int16_t fxi = pi>>6;
  uint32_t ret = (((uint32_t)fxr)<<16)|(uint16_t)fxi;
  return ret;
}

void rFFT16(const int N, const int M,
	    int16_t *win, uint16_t *fft,
	    int16_t *fr, int16_t *fi) {
  uint32_t count;
  uint32_t m;
  uint32_t k;
  uint32_t k0;
  uint32_t stp;
  uint32_t hi;
  uint32_t lo;
  uint32_t g;
  uint32_t j;
  uint32_t a1;
  uint32_t a2;
  int16_t samp;
  int16_t r1;
  int16_t i1;
  int16_t r2;
  int16_t i2;
  uint32_t prd;
  int16_t pr;
  int16_t pi;
  int8_t si;
  int8_t co;
  int16_t rout;
  int16_t iout;
  uint16_t mgr;
  uint16_t mgi;
  uint16_t mg;
  uint32_t wmsk = N-1;
  uint32_t hwmsk = (N/2)-1;

  const std::vector<int8_t>& twiddle=get_twiddle(N);

  for (count = 0; count < N; count++) {
    samp = win[count];
    //printf("[%d][%d] %d\n", bitrev(count, M), count, samp);
    fr[bitrev(count, M)] = samp;
    fi[count] = 0;
  }

  stp = N/2;
  lo = 0;
  hi = N-2;
  g = 1;
  m = 0;
  while (1) {
    k = 0;
    j = 0;
    while (1) {
      if (j == (N/2)) break;
      k0 = k;
      k = (k + stp) & hwmsk;
      a1 = ((j << 1) & hi) | (j & lo);
      a2 = a1 + g;
      j = j + 1;
      
      r2 = fr[a2];
      i2 = fi[a2];
      co = twiddle[k0];
      si = twiddle[(k0 + (N/4))];
      prd = cpxrot(co, r2, si, i2);
      pr = (int16_t)(prd >> 16) >> 1;
      pi = (int16_t)prd >> 1;
      
      r1 = fr[a1];
      i1 = fi[a1];
      fr[a1] = (r1 >> 1) + pr;
      fi[a1] = (i1 >> 1) + pi;
      fr[a2] = (r1 >> 1) - pr;
      fi[a2] = (i1 >> 1) - pi;
    }
    stp = stp >> 1;
    g = g << 1;
    lo = (lo << 1) | 1;
    hi = (hi << 1) & wmsk;
    m = m + 1;
    if (m == M) break;
  }

  for (count = 0; count < (N/2+1); count++) {
    rout = fr[count];
    iout = fi[count];
    //printf("%d + %di\n", rout, iout);
    if (rout >= 0)
      mgr = rout;
    else
      mgr = -rout;
    if (iout >= 0)
      mgi = iout;
    else
      mgi = -iout;
    mg = mgr + mgi;
    fft[count] = mg;
  }
}
