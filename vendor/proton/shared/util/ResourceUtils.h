#pragma once
#include <cstdint>
#include <string>

uint8_t *brotliCompressToMemory(const uint8_t *pInput, int sizeBytes, int *pSizeCompressedOut, int compressionQuality = 2);
uint8_t *brotliDecompressToMemory(const uint8_t *pInput, int compressedSize, int decompressedSize);

uint8_t *zlibDeflateToMemory(uint8_t *pInput, int sizeBytes, int *pSizeCompressedOut); // You must SAFE_DELETE_ARRAY what it returns
uint8_t *zLibInflateToMemory(uint8_t *pInput, unsigned int compressedSize, unsigned int decompressedSize); // You must SAFE_DELETE_ARRAY what it returns
