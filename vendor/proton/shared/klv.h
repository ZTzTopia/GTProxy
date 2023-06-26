// https://github.com/Nuron-bit/KLV-Generator

#pragma once
#include <array>
#include <string>
#include <algorithm>
#include <openssl/evp.h>
#include <openssl/sha.h>

namespace hash {
std::string sha256(const std::string& input) {
    std::array<unsigned char, SHA256_DIGEST_LENGTH> digest{};

    SHA256_CTX ctx{};
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, input.data(), input.length());
    SHA256_Final(digest.data(), &ctx);

    std::string sha256{};
    sha256.reserve(SHA256_DIGEST_LENGTH * 2);

    for (int i{ 0 }; i < SHA256_DIGEST_LENGTH; i++) {
        sha256 += fmt::format("{:02x}", digest[i]);
    }

    return sha256;
}

std::string md5(const std::string& input) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
    EVP_DigestUpdate(ctx, input.c_str(), input.length());
    EVP_DigestFinal_ex(ctx, digest, &digest_len);
    EVP_MD_CTX_free(ctx);

    char md5string[33];
    for (int i = 0; i < 16; i++) {
        sprintf(&md5string[i * 2], "%02x", (unsigned int)digest[i]);
    }
    md5string[32] = '\0';

    std::string hash = std::string(md5string);
    std::transform(hash.begin(), hash.end(), hash.begin(), ::toupper);
    return hash;
}
}

namespace proton {
std::string generate_klv(const int protocol, const std::string& version, const std::string& rid) {
    constexpr std::array salts = {
        "e9fc40ec08f9ea6393f59c65e37f750aacddf68490c4f92d0d2523a5bc02ea63",
        "c85df9056ee603b849a93e1ebab5dd5f66e1fb8b2f4a8caef8d13b9f9e013fa4",
        "3ca373dffbf463bb337e0fd768a2f395b8e417475438916506c721551f32038d",
        "73eff5914c61a20a71ada81a6fc7780700fb1c0285659b4899bc172a24c14fc1"
    };

    static std::array constant_values = {
        hash::sha256(hash::md5(hash::sha256(std::to_string(protocol)))),
        hash::sha256(hash::sha256(version)),
        hash::sha256(hash::sha256(std::to_string(protocol)) + salts[3])
    };

    return hash::sha256(constant_values[0]
                        + salts[0]
                        + constant_values[1]
                        + salts[1]
                        + hash::sha256(hash::md5(hash::sha256(rid)))
                        + salts[2]
                        + constant_values[2]
    );
}
}
