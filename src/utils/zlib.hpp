#pragma once
#include <cstdint>
#include <span>
#include <vector>

#include <zlib.h>
#include <spdlog/spdlog.h>

namespace utils {
[[nodiscard]] inline bool decompress_zlib(
    std::span<const std::byte> compressed_data,
    std::vector<std::byte>& decompressed_data,
    std::uint32_t expected_size = 0
) {
    if (compressed_data.empty()) {
        spdlog::warn("Compressed data is empty");
        return false;
    }

    z_stream stream{};
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<std::byte*>(compressed_data.data()));
    stream.avail_in = static_cast<uInt>(compressed_data.size());

    // Initialize window bits (15 for max window size, +16 for gzip format)
    // The standard zlib format uses -MAX_WBITS, but Growtopia appears to use standard deflate
    constexpr int window_bits = MAX_WBITS;

    if (inflateInit(&stream) != Z_OK) {
        spdlog::error("Failed to initialize zlib decompression");
        return false;
    }

    if (expected_size > 0) {
        decompressed_data.resize(expected_size);
    }
    else {
        decompressed_data.clear();
    }

    std::vector<std::byte> temp_buffer(8192);
    int ret{};

    do {
        stream.next_out = reinterpret_cast<Bytef*>(temp_buffer.data());
        stream.avail_out = static_cast<uInt>(temp_buffer.size());

        ret = inflate(&stream, Z_NO_FLUSH);

        if (ret == Z_STREAM_ERROR) {
            spdlog::error("Decompression stream error");
            inflateEnd(&stream);
            return false;
        }

        const auto have{ temp_buffer.size() - stream.avail_out };
        decompressed_data.insert(decompressed_data.end(), temp_buffer.begin(), temp_buffer.begin() + have);
    } while (stream.avail_out == 0);

    inflateEnd(&stream);

    if (ret != Z_STREAM_END) {
        spdlog::error("Decompression failed, incomplete data (error code: {})", ret);
        return false;
    }

    if (expected_size > 0 && decompressed_data.size() != expected_size) {
        spdlog::error(
            "Decompressed size ({}) != expected size ({})",
            decompressed_data.size(),
            expected_size
        );
        return false;
    }

    spdlog::debug(
        "Successfully decompressed {} bytes to {} bytes",
        compressed_data.size(),
        decompressed_data.size()
    );

    return true;
}

[[nodiscard]] inline bool compress_zlib(
    std::span<const std::byte> data,
    std::vector<std::byte>& compressed_data,
    const int compression_level = Z_DEFAULT_COMPRESSION
) {
    if (data.empty()) {
        spdlog::warn("Input data is empty");
        return false;
    }

    // Estimate compressed size (slightly larger than input to be safe)
    const uLongf max_compressed_size = compressBound(static_cast<uLong>(data.size()));
    compressed_data.resize(max_compressed_size);

    z_stream stream{};
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<std::byte*>(data.data()));
    stream.avail_in = static_cast<uInt>(data.size());
    stream.next_out = reinterpret_cast<Bytef*>(compressed_data.data());
    stream.avail_out = static_cast<uInt>(compressed_data.size());

    if (deflateInit(&stream, compression_level) != Z_OK) {
        spdlog::error("Failed to initialize zlib compression");
        return false;
    }

    if (deflate(&stream, Z_FINISH) != Z_STREAM_END) {
        spdlog::error("Compression failed");
        deflateEnd(&stream);
        return false;
    }

    deflateEnd(&stream);

    // Resize to actual compressed size
    compressed_data.resize(stream.total_out);

    spdlog::debug(
        "Successfully compressed {} bytes to {} bytes (ratio: {:.2f}%)",
        data.size(),
        compressed_data.size(),
        (static_cast<double>(compressed_data.size()) / data.size()) * 100.0
    );

    return true;
}
}
