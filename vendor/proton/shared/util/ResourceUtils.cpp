//#include <cassert>
//#include "zlib.h"
//#include "brotli/encode.h"
//#include "brotli/decode.h"
//
//#include "ResourceUtils.h"
//
//// You must SAFE_DELETE_ARRAY what it returns
//uint8_t *brotliCompressToMemory(const uint8_t *pInput, int sizeBytes, int *pSizeCompressedOut, int compressionQuality) {
//    if (pInput == nullptr || pSizeCompressedOut == nullptr || sizeBytes <= 0 || (compressionQuality < BROTLI_MIN_QUALITY && compressionQuality > BROTLI_MAX_QUALITY)) {
//        return nullptr;
//    }
//
//    size_t sizeOut = BrotliEncoderMaxCompressedSize(sizeBytes);
//    auto *pOut = new uint8_t[sizeOut];
//
//    BROTLI_BOOL ret = BrotliEncoderCompress(compressionQuality, BROTLI_DEFAULT_WINDOW, BROTLI_MODE_GENERIC, sizeBytes, pInput, &sizeOut, pOut);
//    if (ret == false) {
//        delete[] pOut;
//        return nullptr;
//    }
//
//    *pSizeCompressedOut = (int)sizeOut;
//    return pOut;
//}
//
//// You must SAFE_DELETE_ARRAY what it returns
//uint8_t *brotliDecompressToMemory(const uint8_t *pInput, int compressedSize, int decompressedSize) {
//    if (pInput == nullptr || compressedSize <= 0 || decompressedSize <= 0) {
//        return nullptr;
//    }
//
//    auto *pDestBuff = new uint8_t[decompressedSize + 1]; // Room for extra null at the end;
//	pDestBuff[decompressedSize] = 0; // Add the extra null, if we decompressed a text file this can be useful
//
//    BrotliDecoderResult ret = BrotliDecoderDecompress(compressedSize, pInput, (size_t *)&decompressedSize, pDestBuff);
//    if (ret != BROTLI_DECODER_RESULT_SUCCESS) {
//        delete[] pDestBuff;
//        return nullptr;
//    }
//
//    return pDestBuff;
//}
//
//// You must SAFE_DELETE_ARRAY what it returns
//uint8_t *zlibDeflateToMemory(uint8_t *pInput, int sizeBytes, int *pSizeCompressedOut) {
//	z_stream strm;
//	int ret;
//
//	/* allocate deflate state */
//	strm.zalloc = Z_NULL;
//	strm.zfree = Z_NULL;
//	strm.opaque = Z_NULL;
//
//	ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
//	if (ret != Z_OK) {
//		return nullptr;
//	}
//
//#define ZLIB_PADDING_BYTES (5*1024)
//
//	auto *pOut = new uint8_t[sizeBytes + ZLIB_PADDING_BYTES];  // Some extra padding in case the compressed version is larger than the decompressed version for some reason
//
//	strm.avail_in = sizeBytes;
//	strm.next_in = pInput;
//	strm.avail_out = sizeBytes + ZLIB_PADDING_BYTES;
//	strm.next_out = pOut;
//
//	ret = deflate(&strm, Z_FINISH);
//	assert(ret != Z_STREAM_ERROR); /* State not clobbered */
//
//	// assert(ret == Z_STREAM_END);
//	deflateEnd(&strm);
//	*pSizeCompressedOut = (int)strm.total_out;
//	return pOut;
//}
//
//// You must SAFE_DELETE_ARRAY what it returns
//uint8_t *zLibInflateToMemory(uint8_t *pInput, unsigned int compressedSize, unsigned int decompressedSize) {
//	int ret;
//	z_stream strm;
//
//	strm.zalloc = Z_NULL;
//	strm.zfree = Z_NULL;
//	strm.opaque = Z_NULL;
//	strm.avail_in = 0;
//	strm.next_in = Z_NULL;
//	ret = inflateInit(&strm);
//	if (ret != Z_OK) {
//		return nullptr;
//	}
//
//    auto *pDestBuff = new uint8_t[decompressedSize + 1]; // Room for extra null at the end;
//	pDestBuff[decompressedSize] = 0; // Add the extra null, if we decompressed a text file this can be useful
//
//	strm.avail_in = compressedSize;
//	strm.next_in = pInput;
//	strm.avail_out = decompressedSize;
//	strm.next_out = pDestBuff;
//
//	ret = inflate(&strm, Z_NO_FLUSH);
//	if (!(ret == Z_OK || ret == Z_STREAM_END)) {
//		delete[] pDestBuff;
//		return nullptr;
//	}
//
//	inflateEnd(&strm);
//	return pDestBuff;
//}
