/*

Copyright (c) 2022, StreamLogic LLC.
All Rights Reserved.

This source may not be used without the explicit permission of StreamLogic LLC.

*/

#include <stdlib.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

extern int16_t* read_wav_i16(const char* filename, uint32_t* sizep);
extern py::array_t<float> spectrogram(int16_t* audio, uint32_t size, int N, int S);

py::array_t<float> wav_spectrogram(const char *filename, int N, int S) {
  uint32_t size;

  auto audio = read_wav_i16(filename, &size);
  auto result = spectrogram(audio, size, N, S);
  free(audio);

  return result;
}

py::array_t<float> raw_spectrogram(py::array_t<float> data, int N, int S) {
  py::buffer_info buf = data.request();

  uint32_t size = buf.shape[0];
  float* ptr = static_cast<float*>(buf.ptr);

  int16_t* audio = (int16_t*)malloc(size * sizeof(int16_t));
  for (uint32_t i = 0; i < size; i++) {
    audio[i] = (int16_t)(std::numeric_limits<int16_t>::max() * ptr[i]);
    //printf("%f %d ", ptr[i], audio[i]);
  }
  //printf("\n");

  auto result = spectrogram(audio, size, N, S);

  free(audio);

  return result;
}

PYBIND11_MODULE(pySxaudio, m) {
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------
        .. currentmodule:: pySxaudio
        .. autosummary::
           :toctree: _generate
           wav_spectrogram
    )pbdoc";

    m.def("wav_spectrogram", &wav_spectrogram, R"pbdoc(
        Load audio file and construct spectrogram
    )pbdoc");

    m.def("spectrogram", &raw_spectrogram, R"pbdoc(
        Construct spectrogram from audio data
    )pbdoc");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}