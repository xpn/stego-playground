## Info

A PNG Stego encoder/decoder, written as an acompaniment to my blog post on the subject:

https://blog.xpnsec.com/png-steganography/

This is a POC tool meant to show the concept of hiding data in the IDAT chunk of a PNG file. It is not meant to be a fully featured tool and may be buggy.

## Building

To build the encoder and decoder, you'll need:

* CMake
* zlib

We can then build with:

```
mkdir build
cd build
cmake ..
cmake --build .
```

## Running

### Encoder

```
./png_encoder ../testdata/imagetest.png payload
```

This will create a new PNG as `output.png`

### Decoder

```
./png_decoder ./output.png
```