/*

Copyright (c) 2021, StreamLogic LLC.
All Rights Reserved.

This source may not be used without the explicit permission of StreamLogic LLC.

*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <limits>

struct wavprefix_s {
  char sRIFF[4];
  uint32_t dwRiffLength;
  char sWAVE[4];
  char sFMT[4];
  uint32_t dwFmtSize;
  uint16_t wFormatTag;
};

struct wav_fmt_s {
  char sRIFF[4];
  uint32_t dwRiffLength;
  char sWAVE[4];
  char sFMT[4];
  uint32_t dwFmtSize;
  uint16_t wFormatTag;
  uint16_t wChannels;
  uint32_t dwSamplesPerSecond;
  uint32_t dwAvgBytesPerSec;
  uint16_t wBlockAlign;
  uint16_t wBitsPerSample;
};

struct wav_data_s {
  char sDATA[4];
  uint32_t dwDataLength;
};

#define WAVE_FORMAT_PCM 0x1
#define WAVE_FORMAT_ADPCM 0x2
#define WAVE_FORMAT_FLOAT_PCM 0x3
#define WAVE_FORMAT_A_LAW 0x6
#define WAVE_FORMAT_U_LAW 0x7

#define READ_CHUNK_SZ 512

#define AUDIO_ERROR(msg,filename) \
  fprintf(stderr, "%s %s\n", msg, filename); \
  exit(1);

template <typename T>
T *read_wav(const char *filename, uint32_t *sizep) {
  char *bp;
  char buf[READ_CHUNK_SZ];
  uint16_t wExtSize;
  uint32_t sz, dwBytesRecorded;
  struct wavprefix_s prefix;
  struct wav_fmt_s hdr;
  struct wav_data_s chunk;
  uint32_t ptr, size;
  bool floats;
  T *samples;

  FILE *fp = fopen(filename,"rb");
  if (fp == NULL) {
    AUDIO_ERROR("Cannot open audio file", filename);
  }

  sz = sizeof(struct wavprefix_s);
  if (fread(&prefix,1,sz,fp) != sz) {
    AUDIO_ERROR("Error reading audio file", filename);
  }
  if (prefix.wFormatTag != WAVE_FORMAT_PCM &&
      prefix.wFormatTag != WAVE_FORMAT_FLOAT_PCM) {
    AUDIO_ERROR("Unsupported audio file", filename);
  }
  floats = prefix.wFormatTag == WAVE_FORMAT_FLOAT_PCM;

  memcpy(&hdr,&prefix,sizeof(struct wavprefix_s));
  sz=sizeof(struct wav_fmt_s)-sizeof(struct wavprefix_s);
  if (fread(((char *)&hdr)+sizeof(struct wavprefix_s),1,sz,fp) != sz) {
    AUDIO_ERROR("Error reading audio file", filename);
  }

  if (floats) {
    if (hdr.wBitsPerSample != 8*sizeof(float)) {
      AUDIO_ERROR("Wrong number of bits per sample", filename);
    }

    if (fread(&wExtSize,1,sizeof(wExtSize),fp) != sizeof(wExtSize)) {
      AUDIO_ERROR("Error reading audio file", filename);
    }
    if (wExtSize!=0) {
      AUDIO_ERROR("Unexpected extension", filename);
    }
  } else {
    if (hdr.wBitsPerSample != 8*sizeof(T)) {
      AUDIO_ERROR("Wrong number of bits per sample", filename);
    }
  }

  sz=sizeof(struct wav_data_s);
  if (fread(&chunk,1,sz,fp) != sz) {
    AUDIO_ERROR("Error reading audio file", filename);
  }

  // Optional fact chunk
  if (memcmp(chunk.sDATA,"fact",4) == 0) {
    if (fseek(fp, chunk.dwDataLength, SEEK_CUR) != 0) {
      AUDIO_ERROR("Error reading audio file", filename);
    }

    sz=sizeof(struct wav_data_s);
    if (fread(&chunk,1,sz,fp) != sz) {
      AUDIO_ERROR("Error reading audio file", filename);
    }
  }

  if (memcmp(chunk.sDATA,"data",4)!=0) {
    AUDIO_ERROR("Malformed audio file", filename);
  }

  /*
  printf("%s: WAV %d %d %d %d %d %d\n", filename,
	 hdr.wChannels, hdr.dwSamplesPerSecond,
	 hdr.dwAvgBytesPerSec, hdr.wBlockAlign, hdr.wBitsPerSample,
	 chunk.dwDataLength);
   */

  dwBytesRecorded=chunk.dwDataLength;
  // hdr.wBlockAlign == bytes per sample
  size = dwBytesRecorded / hdr.wBlockAlign;

  samples = (T *) malloc(size * sizeof(T));
  if (samples == NULL) {
    AUDIO_ERROR("Out of memory", filename);
  }


  ptr = 0;
  while (ptr < size) {
    sz = (size-ptr) * hdr.wBlockAlign;
    if (sz > READ_CHUNK_SZ) sz = READ_CHUNK_SZ;
    if (fread(buf, 1,  sz, fp) != sz) {
      AUDIO_ERROR("Error reading audio file", filename);
    }
    if (floats) {
      bp = buf;
      while (sz > 0) {
	      samples[ptr++] = (T)(std::numeric_limits<T>::max() * *(float *)bp);
	      bp += hdr.wBlockAlign; // bytes per sample
	      sz -= hdr.wBlockAlign;
      }
    } else {
      bp = buf;
      while (sz > 0) {
	      samples[ptr++] = *(T *)bp; // first channel
	      bp += hdr.wBlockAlign; // bytes per sample
	      sz -= hdr.wBlockAlign;
      }
    }
  }
  fclose(fp);

  *sizep = size;
  return samples;
}

int16_t* read_wav_i16(const char* filename, uint32_t* sizep) {
  return read_wav<int16_t>(filename, sizep);
}