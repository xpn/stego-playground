## Test Cases

The following are the test cases used to verify that the encoder/decoder were working as expected. Output was run through `pngcheck` to verify that the PNGs were valid.

### Test Case 1

#### imagetest.png

```
File: ./output1.png (30347 bytes)
  chunk IHDR at offset 0x0000c, length 13
    1920 x 1080 image, 24-bit RGB, non-interlaced
  chunk pHYs at offset 0x00025, length 9: 2835x2835 pixels/meter (72 dpi)
  chunk cHRM at offset 0x0003a, length 32
    White x = 0.31269 y = 0.32899,  Red x = 0.63999 y = 0.33001
    Green x = 0.3 y = 0.6,  Blue x = 0.15 y = 0.05999
  chunk IDAT at offset 0x00066, length 30225
    zlib: deflated, 32K window, default compression
  chunk IEND at offset 0x07683, length 0
No errors detected in ./output1.png (5 chunks, 99.5% compression).
```

#### imagetest2.png

```
File: ./output2.png (44631 bytes)
  chunk IHDR at offset 0x0000c, length 13
    1920 x 1080 image, 32-bit RGB+alpha, non-interlaced
  chunk sRGB at offset 0x00025, length 1
    rendering intent = perceptual
  chunk gAMA at offset 0x00032, length 4: 0.45455
  chunk pHYs at offset 0x00042, length 9: 4724x4724 pixels/meter (120 dpi)
  chunk IDAT at offset 0x00057, length 44524
    zlib: deflated, 32K window, default compression
  chunk IEND at offset 0x0ae4f, length 0
No errors detected in ./output2.png (6 chunks, 99.5% compression).
```

#### imagetest3.png

```
File: ./output3.png (85915 bytes)
  chunk IHDR at offset 0x0000c, length 13
    1920 x 1080 image, 32-bit RGB+alpha, non-interlaced
  chunk sRGB at offset 0x00025, length 1
    rendering intent = perceptual
  chunk pHYs at offset 0x00032, length 9: 2835x2835 pixels/meter (72 dpi)
  chunk tIME at offset 0x00047, length 7: 30 May 2011 10:11:26 UTC
  chunk tEXt at offset 0x0005a, length 25, keyword: Comment
  chunk bKGD at offset 0x0007f, length 6
    red = 0x00ff, green = 0x00ff, blue = 0x00ff
  chunk IDAT at offset 0x00091, length 85750
    zlib: deflated, 32K window, default compression
  chunk IEND at offset 0x14f93, length 0
No errors detected in ./output3.png (8 chunks, 99.0% compression).
```

#### imagetest4.png

```
File: ./output4.png (948125 bytes)
  chunk IHDR at offset 0x0000c, length 13
    1920 x 1080 image, 32-bit RGB+alpha, non-interlaced
  chunk sRGB at offset 0x00025, length 1
    rendering intent = perceptual
  chunk eXIf at offset 0x00032, length 80: EXIF metadata, big-endian (MM) format
  chunk iTXt at offset 0x0008e, length 345, keyword: XML:com.adobe.xmp
    uncompressed, no language tag
    no translated keyword, 324 bytes of UTF-8 text
  chunk IDAT at offset 0x001f3, length 947606
    zlib: deflated, 32K window, default compression
  chunk IEND at offset 0xe7795, length 0
No errors detected in ./output4.png (6 chunks, 88.6% compression).
```

#### imagetest5.png

```
File: ./output5.png (368360 bytes)
  chunk IHDR at offset 0x0000c, length 13
    640 x 426 image, 32-bit RGB+alpha, non-interlaced
  chunk sRGB at offset 0x00025, length 1
    rendering intent = perceptual
  chunk IDAT at offset 0x00032, length 368290
    zlib: deflated, 32K window, default compression
  chunk IEND at offset 0x59ee0, length 0
No errors detected in ./output5.png (4 chunks, 66.2% compression).
```

#### imagetest6.png

```
File: ./output6.png (2052441 bytes)
  chunk IHDR at offset 0x0000c, length 13
    2399 x 1601 image, 16-bit grayscale+alpha, non-interlaced
  chunk pHYs at offset 0x00025, length 9: 2835x2835 pixels/meter (72 dpi)
  chunk bKGD at offset 0x0003a, length 2
    gray = 0x00ff
  chunk IDAT at offset 0x00048, length 2052349
    zlib: deflated, 32K window, default compression
  chunk IEND at offset 0x1f5151, length 0
No errors detected in ./output6.png (5 chunks, 73.3% compression).
```

#### imagetest7.png

```
File: ./output7.png (52022 bytes)
  chunk IHDR at offset 0x0000c, length 13
    256 x 256 image, 8-bit palette, non-interlaced
  chunk gAMA at offset 0x00025, length 4: 0.45455
  chunk sRGB at offset 0x00035, length 1
    rendering intent = perceptual
  chunk PLTE at offset 0x00042, length 363: 121 palette entries
  chunk IDAT at offset 0x001b9, length 51561
    zlib: deflated, 32K window, default compression
  chunk IEND at offset 0x0cb2e, length 0
No errors detected in ./output7.png (6 chunks, 20.6% compression).
```

#### imagetest8.png

```
File: ./output8.png (226177 bytes)
  chunk IHDR at offset 0x0000c, length 13
    768 x 512 image, 8-bit grayscale, non-interlaced
  chunk IDAT at offset 0x00025, length 226120
    zlib: deflated, 32K window, default compression
  chunk IEND at offset 0x37379, length 0
No errors detected in ./output8.png (3 chunks, 42.5% compression).
```