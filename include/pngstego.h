#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <functional>
#include <stdint.h>
#include "compression.h"
#include "bititerator.h"

#define LONG_ENDINESS(x) (((x) >> 24) | (((x) >> 8) & 0xff00) | (((x) << 8) & 0xff0000) | ((x) << 24))

#define PNG_SIGNATURE 0xa1a0a0d474e5089
#define PNG_SIGNATURE_LENGTH 8

#define IHDR 0x52444849
#define PLTE 0x45544c50
#define IDAT 0x54414449
#define IEND 0x444e4549

#define PNG_LENGTH_FIELD_SIZE 4
#define PNG_TYPE_FIELD_SIZE 4
#define PNG_CRC_FIELD_SIZE 4

#define PNG_CHUNK_HEADER_SIZE (PNG_LENGTH_FIELD_SIZE + PNG_TYPE_FIELD_SIZE)
#define PNG_CHUNK_FOOTER_SIZE PNG_CRC_FIELD_SIZE

struct Chunk {
  uint32_t length;
  uint32_t type;
  uint8_t *data;
  uint32_t crc;
} __attribute__((packed));

typedef struct __attribute__((packed)) _IHDR {
  uint32_t width;
  uint32_t height;
  uint8_t bitDepth;
  uint8_t colorType;
  uint8_t compressionMethod;
  uint8_t filterMethod;
  uint8_t interlaceMethod;
} Chunk_IHDR;

typedef struct __attribute__((packed)) _PLTE {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} Chunk_PLTE;

enum PNGStegoMode {
    ENCODE,
    DECODE
};

class PNGStego {
public:
    PNGStego(const char *filename, PNGStegoMode encodingMode);
    ~PNGStego();

    bool load();
    char* encode(char *data, int dataLen, int *outLength);
    char* decode();

private:
    void compress(unsigned char *data, int length, std::function<void (char *, int)> callback);
    char* processIHDR(char *input, int *inputOffset, char *output, int *outputOffset);
    char* processIDAT(char *input, int *inputOffset, char *output, int *outputOffset);
    char* processIEND(char *input, int *inputOffset, char *output, int *outputOffset);
    char* processUnknownChunk(char *input, int *inputOffset, char *output, int *outputOffset);
    void handleFilters();
    void unfilterSub(unsigned char *scanline, int length);
    void unfilterUp(unsigned char *scanline, unsigned char *previousScanline, int length);
    void unfilterAverage(unsigned char *scanline, unsigned char *previousScanline, int length);
    void unfilterPaeth(unsigned char *scanline, unsigned char *previousScanline, int length);
    void filterSub(unsigned char *scanline, int length);
    void filterUp(unsigned char *scanline, unsigned char *previousScanline, int length);
    void filterAverage(unsigned char *scanline, unsigned char *previousScanline, int length);
    void filterPaeth(unsigned char *scanline, unsigned char *previousScanline, int length);
    unsigned char paethPredictor(unsigned char a, unsigned char b, unsigned char c);
    void encodeDataIntoScanline(unsigned char *scanline, int scanlineLength);
    void decodeDataFromScanline(unsigned char *scanline, int scanlineLength);

    // Input image filename
    const char *filename;  

    // Loaded input image data  
    char *fileContent;
    int fileLength;

    // Data to be hidden in image
    char *inputData;
    int inputDataLength;

    // Length of each scanline in bytes
    int scanlineLength;

    // Uncompressed image data
    char *uncompressedData;
    int uncompressedDataLength;

    // Dimensions of the image
    int width;
    int height;
    int bytesPerPixel;

    // Operating mode
    PNGStegoMode mode;

    Compression *compression;
    BitIterator *bitIterator;
};
