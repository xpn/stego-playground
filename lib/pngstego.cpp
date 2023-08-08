#include "pngstego.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <functional>
#include <queue>
#include <zlib.h>
#include <stdint.h>
#include <string.h>

PNGStego::PNGStego(const char *filename, PNGStegoMode encodingMode) {
  this->filename = filename;
  this->uncompressedData = NULL;
  this->uncompressedDataLength = 0;
  this->compression = new Compression();
  this->mode = encodingMode;
}

PNGStego::~PNGStego() {
  
  if (this->uncompressedData != NULL) {
    free(this->uncompressedData);
  }

  if (this->fileContent != NULL) {
    free(this->fileContent);
  }

  delete this->compression;
  delete this->bitIterator;
}

// Load data from the PNG file into memory
bool PNGStego::load() {
  FILE *f = fopen(this->filename, "rb");
  if (f == NULL) {
    return false;
  }

  fseek(f, 0, SEEK_END);
  this->fileLength = ftell(f);
  fseek(f, 0, SEEK_SET);
  this->fileContent = (char *)malloc(this->fileLength);
  if (this->fileContent == NULL) {
    return false;
  }
  
  fread(this->fileContent, this->fileLength, 1, f);
  fclose(f);

  return true;
}

// Filters a scanline using the sub filter type
// scanline - the scanline to filter
// length - the length of the scanline
void PNGStego::filterSub(unsigned char *scanline, int length) {
  int i;
  unsigned char previousPixel;

  unsigned char *origScanline = (unsigned char *)malloc(length);
  memcpy(origScanline, scanline, length);

  for (i = 0; i < length; i++) {
    if (i < bytesPerPixel) {
      previousPixel = 0;
      scanline[i] -= previousPixel;
    } else {
      previousPixel = origScanline[i - bytesPerPixel];
      scanline[i] -= previousPixel;
    }
  }

  free(origScanline);
}

// Unfilters a scanline using the sub filter type
// scanline - the scanline to unfilter
// length - the length of the scanline
void PNGStego::unfilterSub(unsigned char *scanline, int length) {
  int i;
  unsigned char previousPixel;

  for (i = 0; i < length; i++) {
    if (i < bytesPerPixel) {
      previousPixel = 0;
      scanline[i] += previousPixel;
    } else {
      previousPixel = scanline[i - bytesPerPixel];
      scanline[i] += previousPixel;
    }
  }
}

// Filters a scanline using the up filter type
// scanline - the scanline to filter
// previousScanline - the previous scanline
// length - the length of the scanline
void PNGStego::filterUp(unsigned char *scanline, unsigned char *previousScanline, int length) {
  int i;
  unsigned char previousPixel;

  for (i = 0; i < length; i++) {
    previousPixel = previousScanline[i];
    scanline[i] -= previousPixel;
  }
}

// Unfilters a scanline using the up filter type
// scanline - the scanline to unfilter
// previousScanline - the previous scanline
// length - the length of the scanline
void PNGStego::unfilterUp(unsigned char *scanline, unsigned char *previousScanline, int length) {
  int i;
  unsigned char previousPixel;

  for (i = 0; i < length; i++) {
    previousPixel = previousScanline[i];
    scanline[i] += previousPixel;
  }
}

// Filters a scanline using the average filter type
// scanline - the scanline to filter
// previousScanline - the previous scanline
// length - the length of the scanline
void PNGStego::filterAverage(unsigned char *scanline, unsigned char *previousScanline, int length) {
  int i;
  unsigned char previousPixel;
  unsigned char previousPixelUp;
  
  unsigned char *origScanline = (unsigned char *)malloc(length);
  memcpy(origScanline, scanline, length);

  for (i = 0; i < length; i++) {
    if (i < bytesPerPixel) {
      previousPixel = 0;
      previousPixelUp = previousScanline[i];
      scanline[i] -= (previousPixel + previousPixelUp) / 2;
    } else {
      previousPixel = origScanline[i - bytesPerPixel];
      previousPixelUp = previousScanline[i];
      scanline[i] -= (previousPixel + previousPixelUp) / 2;
    }
  }

  free(origScanline);
}

// Unfilters a scanline using the average filter type
// scanline - the scanline to unfilter
// previousScanline - the previous scanline
// length - the length of the scanline
void PNGStego::unfilterAverage(unsigned char *scanline, unsigned char *previousScanline, int length) {
  int i;
  unsigned char previousPixel;
  unsigned char previousPixelUp;

  for (i = 0; i < length; i++) {
    if (i < bytesPerPixel) {
      previousPixel = 0;
      previousPixelUp = previousScanline[i];
      scanline[i] += (previousPixel + previousPixelUp) / 2;
      
    } else {
      previousPixel = scanline[i - bytesPerPixel];
      previousPixelUp = previousScanline[i];
      scanline[i] += (previousPixel + previousPixelUp) / 2;
    }
  }
}

// Filters a scanline using the paeth filter type
// scanline - the scanline to filter
// previousScanline - the previous scanline
// length - the length of the scanline
void PNGStego::filterPaeth(unsigned char *scanline, unsigned char *previousScanline, int length) {
  int i;
  unsigned char previousPixel;
  unsigned char previousPixelUp;
  unsigned char previousPixelUpLeft;

  unsigned char *origScanline = (unsigned char *)malloc(length);
  memcpy(origScanline, scanline, length);

  for (i = 0; i < length; i++) {
    if (i < bytesPerPixel) {
      previousPixel = 0;
      previousPixelUp = previousScanline[i];
      previousPixelUpLeft = 0;
      scanline[i] -= this->paethPredictor(previousPixel, previousPixelUp, previousPixelUpLeft);
    } else {
      previousPixel = origScanline[i - bytesPerPixel];
      previousPixelUp = previousScanline[i];
      previousPixelUpLeft = previousScanline[i - bytesPerPixel];
      scanline[i] -= this->paethPredictor(previousPixel, previousPixelUp, previousPixelUpLeft);
    }
  }

  free(origScanline);
}

// Unfilters a scanline using the paeth filter type
// scanline - the scanline to unfilter
// previousScanline - the previous scanline
// length - the length of the scanline
void PNGStego::unfilterPaeth(unsigned char *scanline, unsigned char *previousScanline, int length) {
  int i;
  unsigned char previousPixel;
  unsigned char previousPixelUp;
  unsigned char previousPixelUpLeft;

  for (i = 0; i < length; i++) {
    if (i < bytesPerPixel) {
      previousPixel = 0;
      previousPixelUp = previousScanline[i];
      previousPixelUpLeft = 0;
      scanline[i] += this->paethPredictor(previousPixel, previousPixelUp, previousPixelUpLeft);
    } else {
      previousPixel = scanline[i - bytesPerPixel];
      previousPixelUp = previousScanline[i];
      previousPixelUpLeft = previousScanline[i - bytesPerPixel];
      scanline[i] += this->paethPredictor(previousPixel, previousPixelUp, previousPixelUpLeft);
    }
  }
}

// Returns the paeth predictor for the given values
unsigned char PNGStego::paethPredictor(unsigned char a, unsigned char b, unsigned char c) {
  int p = a + b - c;
  int pa = abs(p - a);
  int pb = abs(p - b);
  int pc = abs(p - c);

  if (pa <= pb && pa <= pc) {
    return a;
  } else if (pb <= pc) {
    return b;
  } else {
    return c;
  }
}

// Uses the bit iterator to add decoded bits from the image
void PNGStego::encodeDataIntoScanline(unsigned char *scanline, int scanlineLength) {
  int i;
  bool eof = false;

  if (!this->bitIterator->hasNext()) {
    return;
  }

  for (i = 0; i < scanlineLength; i++) {
    char bit = this->bitIterator->getNextBit(eof);
    if (eof) {
      return;
    }

    if (bit) {
      scanline[i] |= 1;
    } else {
      scanline[i] &= 0xFE;
    }
  }
}

// Combines bits from the image into the bit iterator
void PNGStego::decodeDataFromScanline(unsigned char *scanline, int scanlineLength) {
  int i;

  for (i = 0; i < scanlineLength; i++) {
    this->bitIterator->addBit(scanline[i] & 1);
  }
}

// Walks through the decompressed IDAT chunks and processes the filters
void PNGStego::handleFilters() {

  std::queue<int> filterQueue;
  char *prevUncompressedData = NULL;

  prevUncompressedData = (char *)malloc(this->uncompressedDataLength);
  memcpy(prevUncompressedData, this->uncompressedData, this->uncompressedDataLength);

  printf("[*] Running unfilters...\n");

  // First we process the filters for each scanline
  for (int i = 0; i < this->uncompressedDataLength; i += this->scanlineLength) {

    filterQueue.push(this->uncompressedData[i]);

    switch(this->uncompressedData[i]) {
      case 0:
        printf("[*] Scanline Filter: None\n");
        break;

      case 1:
        printf("[*] Scanline Filter: Sub\n");
        this->unfilterSub((unsigned char *)this->uncompressedData + i + 1, this->scanlineLength - 1);
        this->uncompressedData[i] = 0;
        break;

      case 2:
        printf("[*] Scanline Filter: Up\n");
        this->unfilterUp((unsigned char *)this->uncompressedData + i + 1, (unsigned char *)this->uncompressedData + i + 1 - this->scanlineLength, this->scanlineLength - 1);
        this->uncompressedData[i] = 0;
        break;

      case 3:
        printf("[*] Scanline Filter: Average\n");
        this->unfilterAverage((unsigned char *)this->uncompressedData + i + 1, (unsigned char *)this->uncompressedData + i + 1 - this->scanlineLength, this->scanlineLength - 1);
        this->uncompressedData[i] = 0;
        break;

      case 4:
        printf("[*] Scanline Filter: Paeth\n");
        this->unfilterPaeth((unsigned char *)this->uncompressedData + i + 1, (unsigned char *)this->uncompressedData + i + 1 - this->scanlineLength, this->scanlineLength - 1);
        this->uncompressedData[i] = 0;
        break;

      default:
        printf("[*] Scanline Filter: Unknown (%d)\n", *((unsigned char *)uncompressedData + i));
        break;
    }
  }

  for (int i = 0; i < this->uncompressedDataLength; i += this->scanlineLength) {
    if (this->mode == PNGStegoMode::ENCODE) {
      this->encodeDataIntoScanline((unsigned char *)this->uncompressedData + i + 1, this->scanlineLength - 1);
    } else {
      this->decodeDataFromScanline((unsigned char *)this->uncompressedData + i + 1, this->scanlineLength - 1);
      return;
    }
  }

  char *origBytes = (char*)malloc(this->uncompressedDataLength);
  memcpy(origBytes, this->uncompressedData, this->uncompressedDataLength);

  printf("[*] Re-running filters...\n");

  // And then reapply the filter (we'll just repeat from the beginning)
  for (int i = 0; i < this->uncompressedDataLength; i += this->scanlineLength) {

    int filter = filterQueue.front();
    filterQueue.pop();

    switch(filter) {
      case 0:
        printf("[*] Scanline Filter: None\n");
        break;

      case 1:
        printf("[*] Scanline Filter: Sub\n");
        this->filterSub((unsigned char *)this->uncompressedData + i + 1, this->scanlineLength - 1);
        this->uncompressedData[i] = 1;
        break;

      case 2:
        printf("[*] Scanline Filter: Up\n");
        this->filterUp((unsigned char *)this->uncompressedData + i + 1, (unsigned char *)origBytes + i + 1 - this->scanlineLength, this->scanlineLength - 1);
        this->uncompressedData[i] = 2;
        break;

      case 3:
        printf("[*] Scanline Filter: Average\n");
        this->filterAverage((unsigned char *)this->uncompressedData + i + 1, (unsigned char *)origBytes + i + 1 - this->scanlineLength, this->scanlineLength - 1);
        this->uncompressedData[i] = 3;
        break;

      case 4:
        printf("[*] Scanline Filter: Paeth\n");
        this->filterPaeth((unsigned char *)this->uncompressedData + i + 1, (unsigned char *)origBytes + i + 1 - this->scanlineLength, this->scanlineLength - 1);
        this->uncompressedData[i] = 4;
        break;

      default:
        printf("[*] Scanline Filter: Unknown (%d)\n", *((unsigned char *)uncompressedData + i));
        break;
    }
  }

  if (prevUncompressedData != NULL) {
    free(prevUncompressedData);
  }

  if (origBytes != NULL) {
    free(origBytes);
  }
}

// Processes a IHDR chunk
// input - the input buffer containing PNG data
// inputOffset - the offset into the input buffer where the IHDR chunk starts
// output - the output buffer of our new image
// outputOffset - the offset into the output buffer where we should start writing
// returns a new output buffer as we most likely will need to realloc
char* PNGStego::processIHDR(char *input, int *inputOffset, char *output, int *outputOffset) {

Chunk_IHDR *ihdr;
uint32_t chunkLength;
uint32_t chunkType;
uint32_t chunkCRC;
char *chunkData;

  chunkLength = LONG_BITSWAP(*(uint32_t *)(input + *inputOffset));
  chunkType = LONG_BITSWAP(*(uint32_t *)((input + *inputOffset + PNG_LENGTH_FIELD_SIZE)));
  chunkData = input + *inputOffset + PNG_CHUNK_HEADER_SIZE;
  chunkCRC = LONG_BITSWAP(*(uint32_t *)((input + *inputOffset + PNG_CHUNK_HEADER_SIZE + chunkLength)));

  ihdr = (Chunk_IHDR *)(chunkData);

  printf("[*] Dimensions: %dx%d\n", LONG_BITSWAP(ihdr->width), LONG_BITSWAP(ihdr->height));
  printf("[*] Bit Depth: %d\n", ihdr->bitDepth);
  printf("[*] Color Type: %d\n", ihdr->colorType);
  printf("[*] Compression: %d\n", ihdr->compressionMethod);
  printf("[*] FilterMethod: %d\n", ihdr->filterMethod);
  printf("[*] Interlace: %d\n", ihdr->interlaceMethod);

  if (ihdr->compressionMethod != 0) {
    printf("[!] Error: Compression method must be 0\n");
    exit(1);
  }

  if (ihdr->filterMethod != 0) {
    printf("[!] Error: Filter method must be 0\n");
    exit(1);
  }

  if (ihdr->interlaceMethod != 0) {
    // Not currently implemented, because CBA until I see it in the wild
    printf("[!] Error: Interlace method must be 0\n");
    exit(1);
  }

  this->width = LONG_BITSWAP(ihdr->width);
  this->height = LONG_BITSWAP(ihdr->height);

  switch(ihdr->colorType) {
    case 0:
      printf("[*] IHDR Color Type: Grayscale\n");
      this->scanlineLength = LONG_BITSWAP(ihdr->width) + 1;
      this->bytesPerPixel = (1 * ihdr->bitDepth) / 8;
      break;

    case 2:
      printf("[*] IHDR Color Type: RGB\n");
      this->scanlineLength = LONG_BITSWAP(ihdr->width) * 3 + 1;
      this->bytesPerPixel = (3 * ihdr->bitDepth) / 8;
      break;

    case 3:
      printf("[*] IHDR Color Type: Palette\n");
      printf("[!] Pallate not currently supported so results may be screwy!\n");
      this->scanlineLength = LONG_BITSWAP(ihdr->width) + 1;
      this->bytesPerPixel = (1 * ihdr->bitDepth) / 8;
      break;

    case 4:
      printf("[*] IHDR Color Type: Grayscale + Alpha\n");
      this->scanlineLength = LONG_BITSWAP(ihdr->width) * 2 + 1;
      this->bytesPerPixel = (2 * ihdr->bitDepth) / 8;
      break;

    case 6:
      printf("[*] IHDR Color Type: RGB + Alpha\n");
      this->scanlineLength = LONG_BITSWAP(ihdr->width) * 4 + 1;
      this->bytesPerPixel = (4 * ihdr->bitDepth) / 8;
      break;
  }

  printf("[*] Maximum Data Size: %d bytes\n", (this->width * this->height * this->bytesPerPixel));
  if ((this->width * this->height * this->bytesPerPixel) < this->inputDataLength) {
    printf("[!] Error: Payload is too large to fit in the image\n");
    exit(1);
  }

  // If we are decoding, we don't write out any data
  if (this->mode == PNGStegoMode::DECODE) {
    *inputOffset += PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE;
    return output;
  }

  output = (char *)realloc(output, *outputOffset + PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE);
  if (output == NULL) {
    printf("[!] Error: realloc failed\n");
    return NULL;
  }

  // Copy the IHDR chunk to our stego output
  memcpy(output + *outputOffset, input + *inputOffset, PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE);

  // Update the offsets
  *inputOffset += PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE;
  *outputOffset += PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE;

  return output;
}

// Processes a IEND chunk
// input - the input buffer containing PNG data
// inputOffset - the offset into the input buffer where the IEND chunk starts
// output - the output buffer of our new image
// outputOffset - the offset into the output buffer where we should start writing
// returns a new output buffer as we most likely will need to realloc
char* PNGStego::processIEND(char *input, int *inputOffset, char *output, int *outputOffset) {

char *chunkData = NULL;
uint32_t chunkLength;
uint32_t chunkType;
uint32_t chunkCRC;

  chunkLength = LONG_BITSWAP(*(uint32_t *)(input + *inputOffset));
  chunkType = LONG_BITSWAP(*(uint32_t *)((input + *inputOffset + PNG_LENGTH_FIELD_SIZE)));
  chunkData = input + *inputOffset + PNG_CHUNK_HEADER_SIZE;
  chunkCRC = LONG_BITSWAP(*(uint32_t *)((input + *inputOffset + PNG_CHUNK_HEADER_SIZE + chunkLength)));

  // Now we need to handle all that IDAT stuff
  this->handleFilters();

  // If we are decoding, we return here
  if (this->mode == PNGStegoMode::DECODE) {
    *inputOffset += PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE;
    return output;
  }

  this->compression->compress((unsigned char *)this->uncompressedData, this->uncompressedDataLength, [&output, outputOffset](char *compressedData, int compressedLength) {
      
      printf("[*] Compressed Length: %d bytes\n", compressedLength);
      output = (char*)realloc(output, *outputOffset + compressedLength + PNG_CHUNK_HEADER_SIZE + PNG_CRC_FIELD_SIZE);

      *(uint32_t *)(output + *outputOffset) = LONG_BITSWAP(compressedLength);
      *outputOffset += PNG_LENGTH_FIELD_SIZE;

      *(uint32_t *)(output + *outputOffset) = IDAT;
      *outputOffset += PNG_TYPE_FIELD_SIZE;
      
      memcpy(output + *outputOffset, compressedData, compressedLength);
      *outputOffset += compressedLength;

      *(uint32_t *)(output + *outputOffset) = LONG_BITSWAP(crc32(0, (unsigned char *)output + *outputOffset - compressedLength - PNG_LENGTH_FIELD_SIZE, compressedLength + PNG_LENGTH_FIELD_SIZE));
      *outputOffset += PNG_CRC_FIELD_SIZE;
  });

  output = (char*)realloc(output, *outputOffset + chunkLength + PNG_CHUNK_HEADER_SIZE + PNG_CRC_FIELD_SIZE);

  // Copy the IEND chunk to our stego output
  *(uint32_t *)(output + *outputOffset) = LONG_BITSWAP(chunkLength);
  *outputOffset += PNG_LENGTH_FIELD_SIZE;

  *(uint32_t *)(output + *outputOffset) = IEND;
  *outputOffset += PNG_TYPE_FIELD_SIZE;

  memcpy(output + *outputOffset, chunkData, chunkLength);
  *outputOffset += chunkLength;

  *(uint32_t *)(output + *outputOffset) = LONG_BITSWAP(crc32(0, (unsigned char *)output + *outputOffset - chunkLength - PNG_LENGTH_FIELD_SIZE, chunkLength + PNG_LENGTH_FIELD_SIZE));;
  *outputOffset += PNG_CRC_FIELD_SIZE;

  // Update the offsets
  *inputOffset += PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE;

  return output;
}

// Processes a IDAT chunk
// input - the input buffer containing PNG data
// inputOffset - the offset into the input buffer where the IDAT chunk starts
// output - the output buffer of our new image
// outputOffset - the offset into the output buffer where we should start writing
// returns a new output buffer as we most likely will need to realloc
char* PNGStego::processIDAT(char *input, int *inputOffset, char *output, int *outputOffset) {

char *chunkData = NULL;
uint32_t chunkLength;
uint32_t chunkType;
uint32_t chunkCRC;

  chunkLength = LONG_BITSWAP(*(uint32_t *)(input + *inputOffset));
  chunkType = LONG_BITSWAP(*(uint32_t *)((input + *inputOffset + PNG_LENGTH_FIELD_SIZE)));
  chunkData = input + *inputOffset + PNG_CHUNK_HEADER_SIZE;
  chunkCRC = LONG_BITSWAP(*(uint32_t *)((input + *inputOffset + PNG_CHUNK_HEADER_SIZE + chunkLength)));

  // Decompress the data
  this->compression->decompress((unsigned char *)chunkData, chunkLength, (this->width * this->height * this->bytesPerPixel) + this->height, [this](char *decompressedData, int decompressedLength) {

    // Copy the decompressed data for later processing
    printf("[*] Decompressed Length: %d bytes\n", decompressedLength);
    
    this->uncompressedData = (char *)realloc(this->uncompressedData, this->uncompressedDataLength + decompressedLength);
    memcpy(this->uncompressedData + this->uncompressedDataLength, decompressedData, decompressedLength);
    this->uncompressedDataLength += decompressedLength;

  });

  *inputOffset += PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE;
  return output;
}

// Processes any unhandled chunks such as iTXT
// input - the input buffer containing PNG data
// inputOffset - the offset into the input buffer where the chunk starts
// output - the output buffer of our new image
// outputOffset - the offset into the output buffer where we should start writing
// returns a new output buffer as we most likely will need to realloc
char* PNGStego::processUnknownChunk(char *input, int *inputOffset, char *output, int *outputOffset) {

char *chunkData = NULL;
uint32_t chunkLength;
uint32_t chunkType;
uint32_t chunkCRC;

  chunkLength = LONG_BITSWAP(*(uint32_t *)(input + *inputOffset));
  chunkType = LONG_BITSWAP(*(uint32_t *)((input + *inputOffset + PNG_LENGTH_FIELD_SIZE)));
  chunkData = input + *inputOffset + PNG_CHUNK_HEADER_SIZE;
  chunkCRC = LONG_BITSWAP(*(uint32_t *)((input + *inputOffset + PNG_CHUNK_HEADER_SIZE + chunkLength)));

  // If we are decoding, we return here
  if (this->mode == PNGStegoMode::DECODE) {
    *inputOffset += PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE;
    return output;
  }

  output = (char *)realloc(output, *outputOffset + PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE);
  if (output == NULL) {
    printf("[!] Error: realloc failed\n");
    return NULL;
  }

  // Copy the unknown chunk to our stego output
  memcpy(output + *outputOffset, input + *inputOffset, PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE);

  // Update the offsets
  *inputOffset += PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE;
  *outputOffset += PNG_CHUNK_HEADER_SIZE + chunkLength + PNG_CRC_FIELD_SIZE;

  return output;
}

// Encodes the provided data into the PNG image
// data - the data to encode
// dataLen - the length of the data to encode
// outLength - the length of the new encoded image generated
// returns the new encoded image
char* PNGStego::encode(char *data, int dataLen, int *outLength) {

  int inBufferOffset = 0;
  uint32_t pngChunkType = 0;
  int pngChunkLength = 0;
  char *inBuffer = NULL;
  char *outBuffer = NULL;
  int outBufferOffset = 0;

  if (this->mode != PNGStegoMode::ENCODE) {
    printf("[!] Error: encode() method called in decode mode\n");
    return NULL;
  }

  this->inputData = data;
  this->inputDataLength = dataLen;
  this->bitIterator = new BitIterator(this->inputData, this->inputDataLength);

  inBuffer = this->fileContent;
  outBuffer = (char*)malloc(1024);

  // Check the magic
  if (*(uint64_t *)inBuffer != PNG_SIGNATURE) {
    return NULL;
  }

  // Copy the magic to our stego output
  memcpy(outBuffer, inBuffer, PNG_SIGNATURE_LENGTH);
  outBufferOffset += PNG_SIGNATURE_LENGTH;
  inBufferOffset += PNG_SIGNATURE_LENGTH;

  // Iterate through PNG chunks
  while (inBufferOffset < this->fileLength) {

    // Each chunk starts with a 4 byte length and 4 byte type
    // followed by the data (of length bytes) and a 4 byte CRC
    pngChunkLength = LONG_BITSWAP(*(uint32_t *)(inBuffer + inBufferOffset));
    pngChunkType = *(uint32_t *)(inBuffer + inBufferOffset + PNG_LENGTH_FIELD_SIZE);

    switch(pngChunkType) {
      case IHDR:

        printf("[*] IHDR Chunk\n");
        outBuffer = processIHDR(inBuffer, &inBufferOffset, outBuffer, &outBufferOffset);
        break;

      case IDAT:

        printf("[*] IDAT Chunk\n");
        outBuffer = processIDAT(inBuffer, &inBufferOffset, outBuffer, &outBufferOffset);
        break;
      
      case IEND:

        printf("[*] IEND Chunk\n");
        outBuffer = processIEND(inBuffer, &inBufferOffset, outBuffer, &outBufferOffset);
        break;

      default:

        printf("[*] Unknown Chunk: %c%c%c%c\n", (pngChunkType >> 24) & 0xFF, (pngChunkType >> 16) & 0xFF, (pngChunkType >> 8) & 0xFF, pngChunkType & 0xFF);
        outBuffer = processUnknownChunk(inBuffer, &inBufferOffset, outBuffer, &outBufferOffset);
        break;
    }
  }

  *outLength = outBufferOffset;
  return outBuffer;
}

// Decodes the data from the PNG image
// returns the decoded data
char* PNGStego::decode() {

  int inBufferOffset = 0;
  int pngChunkLength = 0;
  char *outBuffer = NULL;
  int outBufferOffset = 0;
  uint32_t pngChunkType = 0;

  if (this->mode != PNGStegoMode::DECODE) {
    printf("[!] Error: decode() method called in encode mode\n");
    return NULL;
  }

  // Allocate the maximum amount of space likely to be taken up by the payload
  // which we can guess isn't going to be larger than the size of the file
  char *output = (char *)malloc(this->fileLength);
  memset(output, 0, this->fileLength);

  this->bitIterator = new BitIterator(output, this->fileLength);

  char *inBuffer = this->fileContent;

  // Check the magic
  if (*(uint64_t *)inBuffer != PNG_SIGNATURE) {
    return NULL;
  }

  inBufferOffset += PNG_SIGNATURE_LENGTH;

  while (inBufferOffset < this->fileLength) {

    // Each chunk starts with a 4 byte length and 4 byte type
    // followed by the data (of length bytes) and a 4 byte CRC
    pngChunkLength = LONG_BITSWAP(*(uint32_t *)(inBuffer + inBufferOffset));
    pngChunkType = *(uint32_t *)(inBuffer + inBufferOffset + PNG_LENGTH_FIELD_SIZE);

    switch(pngChunkType) {

      case IHDR:

        printf("[*] IHDR Chunk\n");
        outBuffer = processIHDR(inBuffer, &inBufferOffset, outBuffer, &outBufferOffset);
        break;

      case IDAT:

        printf("[*] IDAT Chunk\n");
        outBuffer = processIDAT(inBuffer, &inBufferOffset, outBuffer, &outBufferOffset);
        break;
      
      case IEND:

        printf("[*] IEND Chunk\n");
        outBuffer = processIEND(inBuffer, &inBufferOffset, outBuffer, &outBufferOffset);
        break;

      default:

        printf("[*] Unknown Chunk: %c%c%c%c\n", pngChunkType & 0xFF, (pngChunkType >> 8) & 0xFF, (pngChunkType >> 16) & 0xFF, (pngChunkType >> 24) & 0xFF);
        outBuffer = processUnknownChunk(inBuffer, &inBufferOffset, outBuffer, &outBufferOffset);
        break;

    }
  }

  return output;

}