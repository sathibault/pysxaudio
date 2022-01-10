/*

Copyright (c) 2021, StreamLogic LLC.
All Rights Reserved.

This source may not be used without the explicit permission of StreamLogic LLC.

*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

extern void rFFT16(const int N, const int M,
		   int16_t *win, uint16_t *fft,
		   int16_t *fr, int16_t *fi);

static size_t log2i(size_t n) {
  return ( (n<2) ? 0 : 1+log2i(n/2));
}

py::array_t<float> spectrogram(int16_t *audio, uint32_t size, int N, int S) {
  int M = log2i(N);

  int16_t *fr = (int16_t *)malloc(N*sizeof(int16_t));
  int16_t *fi = (int16_t *)malloc(N*sizeof(int16_t));
  uint16_t *fft = (uint16_t *)malloc((N/2+1)*sizeof(int16_t));

  // Number of windows
  uint32_t NW = ((size + (S - 1) - (N  - 1)) / S);

  // N/2+1 outputs per window
  auto result = py::array_t<float>(NW * (N/2 + 1));
  py::buffer_info buf = result.request();
  float *ptr = static_cast<float *>(buf.ptr);

  int idx = 0;
  for (int win = 0; (win + N) <= size; win += S) {
    rFFT16(N, M, audio+win, fft, fr, fi);
    for (int i = 0; i < (N/2+1); i++)
      ptr[idx++]  = fft[i];
  }

  free(fft);
  free(fi);
  free(fr);

  return result;
}