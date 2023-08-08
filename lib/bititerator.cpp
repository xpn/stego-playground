#include "bititerator.h"

BitIterator::BitIterator(char *data) {
  this->data = data;
  this->dataLength = -1;
  this->index = 0;
}

BitIterator::BitIterator(char *data, int dataLength) {
  this->data = data;
  this->dataLength = dataLength;
  this->index = 0;
}

void BitIterator::reset() {
  this->index = 0;
}

bool BitIterator::getNextBit(bool &eof) {

  char *buffer = this->data;
  int bufferLength = this->dataLength;

  if (this->index >= bufferLength * 8) {
    eof = true;
    return false;
  }

  int byteIndex = this->index / 8;
  int bitIndex = this->index % 8;

  int bit = (buffer[byteIndex] >> (7 - bitIndex)) & 1;
  this->index++;

  return bit;
}

bool BitIterator::hasNext() {
  return this->index < this->dataLength * 8;
}

void BitIterator::addBit(char bit) {

  if (this->dataLength < this->index) {
    // Buffer is full, so we stop ingesting
    return;
  }

  int byteIndex = this->index / 8;
  int bitIndex = this->index % 8;

  if (bit) {
    this->data[byteIndex] |= (1 << (7 - bitIndex));
  } else {
    this->data[byteIndex] &= ~(1 << (7 - bitIndex));
  }

  this->index++;
}

char* BitIterator::getData() {
  return this->data;
}