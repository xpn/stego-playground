#include "pngstego.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <functional>
#include <queue>
#include <zlib.h>

// Prints a hex dump of the input with each line prefixed with the line number
// and each line being 16 bytes long
// Thanks: https://gist.github.com/ccbrown/9722406
void hexdump(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

int main(int argc, char **argv) {

  printf("PNG Stego Decoder POC\n");
  printf("  Created by @_xpn_ to support the blog post: https://blog.xpnsec.com/png-steganography/\n\n");

  if (argc < 2) {
    printf("Usage: %s <png file>\n", argv[0]);
    exit(1);
  }

  // We're going to decode the payload from the image
  PNGStego *png = new PNGStego(argv[1], PNGStegoMode::DECODE);
  if (!png->load()) {
    printf("[!] Failed to load PNG file\n");
    return 1;
  }

  char *output = png->decode();
  if (output == NULL) {
    printf("[!] Error decoding PNG\n");
    return 1;
  }

  // Usually we'd spawn te payload off the back of this, but for now we'll just print the output in hex
  printf("[*] 0x100 bytes of decoded Payload:\n");
  hexdump(output, 0x100); 

  free(output);
  delete png;
}