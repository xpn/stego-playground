#pragma once

class BitIterator {
public:
    BitIterator(char *data);
    BitIterator(char *data, int dataLength);
    bool getNextBit(bool &eof);
    void reset();
    bool hasNext();
    void addBit(char bit);
    char* getData();

private:
    char *data;
    int dataLength;
    int index;
};
