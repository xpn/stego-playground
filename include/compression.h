#pragma once

#include <zlib.h>
#include <functional>

class Compression {
public:
    Compression();
    bool compress(unsigned char *data, int length, std::function<void (char *, int)> callback);
    bool decompress(unsigned char *data, int length, int maxOutputLength, std::function<void (char *, int)> callback);
private:
    z_stream inflatestrm;
    z_stream deflatestrm;
};
