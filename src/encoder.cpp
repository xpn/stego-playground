#include "pngstego.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <functional>
#include <queue>
#include <zlib.h>

int main(int argc, char **argv) {

  printf("PNG Stego Encoder POC\n");
  printf("  Created by @_xpn_ to support the blog post: https://blog.xpnsec.com/png-steganography/\n\n");

  if (argc < 3) {
    printf("Usage: %s <png file> <payload file>\n", argv[0]);
    exit(1);
  }

  int outputLength = 0;
  int payloadLength = 0;
  char *payload = NULL;

  // Load our payload
  FILE *payloadFile = fopen(argv[2], "rb");
  fseek(payloadFile, 0, SEEK_END);
  payloadLength = ftell(payloadFile);
  fseek(payloadFile, 0, SEEK_SET);
  payload = (char *)malloc(payloadLength);
  fread(payload, 1, payloadLength, payloadFile);
  fclose(payloadFile);

  // Load the file
  PNGStego *png = new PNGStego(argv[1], PNGStegoMode::ENCODE);
  if (!png->load()) {
    printf("[!] Failed to load PNG file\n");
    return 1;
  }

  // Load and encode the payload to be hidden
  char *outData = png->encode(payload, payloadLength, &outputLength);

  // Write the output to a new file
  FILE *outputFile = fopen("output.png", "wb");
  fwrite(outData, 1, outputLength, outputFile);
  fclose(outputFile);

  free(payload);
  delete png;
}