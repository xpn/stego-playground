#include <stdio.h>
#include <functional>
#include <zlib.h>
#include <stdint.h>
#include <string.h>
#include "compression.h"

Compression::Compression() {
  memset(&this->inflatestrm, 0, sizeof(z_stream));
  memset(&this->deflatestrm, 0, sizeof(z_stream));
}

bool Compression::compress(unsigned char *data, int length, std::function<void (char *, int)> callback) {
  
  int ret;
  unsigned char *buffer;
  uLongf bufferLength;
  
  bufferLength = compressBound(length);
  buffer = (unsigned char *)malloc(bufferLength);

  ret = ::compress((Bytef *)buffer, (uLongf *)&bufferLength, (Bytef *)data, length);
  if (ret != Z_OK) {
    printf("[!] Error: compress returned %d\n", ret);
    return false;
  }

  callback((char *)buffer, bufferLength);
  free(buffer);

  return true;
}

bool Compression::decompress(unsigned char *data, int length, int maxOutputLength, std::function<void (char *, int)> callback) {

  int ret;
  unsigned char *buffer;
  int bufferLength = maxOutputLength;
  int outputLength;

  this->inflatestrm.avail_in = length;
  this->inflatestrm.next_in = (Bytef *)data;

  if (this->inflatestrm.state == NULL) {
    this->inflatestrm.zalloc = Z_NULL;
    this->inflatestrm.zfree = Z_NULL;
    this->inflatestrm.opaque = Z_NULL;
    ret = inflateInit(&this->inflatestrm);
    if (ret != Z_OK) {
      printf("Error: inflateInit returned %d\n", ret);
      return false;
    }
  }

  buffer = (unsigned char *)malloc(bufferLength);
  this->inflatestrm.avail_out = bufferLength;
  this->inflatestrm.next_out = buffer;

  ret = inflate(&this->inflatestrm, Z_SYNC_FLUSH);
  if (ret != Z_OK && ret != Z_STREAM_END) {
    printf("Error: inflate returned %d\n", ret);
    return false;
  }

  outputLength = bufferLength - this->inflatestrm.avail_out;
  callback((char *)buffer, outputLength);
  free(buffer);

  if (this->inflatestrm.avail_in != 0) {
    printf("Error: inflate did not consume all input\n");
    return false;
  }

  inflateEnd(&this->inflatestrm);

  return true;
}
