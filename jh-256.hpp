#include <cstdint>
#include <string>
#include <format>
#include <vector>
#include <algorithm>
#include <iostream>

static const uint32_t H_INITIAL[8] = {0x21fba37b, 0x43ab9fb6, 0x75a9f91d, 0x86305019, 0xd7cd8173, 0x07fe00ff, 0x379f513f, 0x66b651a8};
static const uint32_t K_CONSTANTS[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 
    0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 
    0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 
    0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

uint32_t rotr32(uint32_t value, unsigned int count) {
    return (value >> count) | (value << (32 - count));
}

uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return ((x & z) | (y & ~z));
}

uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return ((z & y) ^ (~x & y) | (z & x));
}

uint32_t sigma0(uint32_t x) {
    return rotr32(x, 17) ^ rotr32(x, 23) ^ rotr32(x, 30);
}

uint32_t sigma1(uint32_t x) {
    return rotr32(x, 3) ^ rotr32(x, 19) ^ rotr32(x, 28);
}

uint32_t sigma_ms0(uint32_t x) {
    return rotr32(x, 11) ^ rotr32(x, 24) ^ (x >> 3);  
}

uint32_t sigma_ms1(uint32_t x) {
    return rotr32(x, 29) ^ rotr32(x, 31) ^ (x >> 10);
}

uint32_t bytes_to_uint32_be(const unsigned char* block_ptr) {
    return (static_cast<uint32_t>(block_ptr[0]) << 24) |
           (static_cast<uint32_t>(block_ptr[1]) << 16) |
           (static_cast<uint32_t>(block_ptr[2]) << 8)  |
           (static_cast<uint32_t>(block_ptr[3]));
}

class jh_256 {
private:
    uint32_t chaining_vars[8];
    std::vector<unsigned char> buffer;
    uint64_t total_bits_len;

public:
    jh_256(): buffer(), total_bits_len(0) {
        std::copy(std::begin(H_INITIAL), std::end(H_INITIAL), std::begin(chaining_vars));
    }

    void update(const std::string& data_str) {
        std::vector<unsigned char> data(data_str.begin(), data_str.end());

        buffer.insert(buffer.end(), data.begin(), data.end());
        total_bits_len += data.size() * 8;

        while (buffer.size() >= 64) {
            std::vector<unsigned char> block(buffer.begin(), buffer.begin() + 64);
            processBlock(block);
            buffer.erase(buffer.begin(), buffer.begin() + 64);
        }
    }

    std::string finalize() {
        uint64_t original_bits_len = total_bits_len;

        buffer.push_back(0x80);

        if (buffer.size() > 56) {
            while (buffer.size() < 64) {
                buffer.push_back(0);
            }
            processBlock(buffer);
            buffer.clear();
        }

        while (buffer.size() < 56)
            buffer.push_back(0);

        for (int i = 7; i >= 0; i--)
            buffer.push_back(static_cast<unsigned char>(original_bits_len >> (i * 8)));

        processBlock(buffer);

        std::string digest = "";
        for (size_t i = 0; i < 8; i++)
            digest += std::format("{:08x}", chaining_vars[i]);

        return digest;
    }

private:
    void processBlock(const std::vector<unsigned char>& block) {
        uint32_t w[64];

        for (size_t t = 0; t < 16; t++) {
            const unsigned char* chunk_ptr = &block[t*4];
            w[t] = bytes_to_uint32_be(chunk_ptr);
        }

        for (size_t t = 16; t < 64; t++) {
            uint32_t term1 = sigma_ms1(w[t-2]);
            uint32_t term2 = w[t-7];
            uint32_t term3 = sigma_ms0(w[t-15]);
            uint32_t term4 = w[t-16];

            w[t] = term1 + term2 + term3 + term4;
        }

        uint32_t a = chaining_vars[0];
        uint32_t b = chaining_vars[1];
        uint32_t c = chaining_vars[2];
        uint32_t d = chaining_vars[3];
        uint32_t e = chaining_vars[4];
        uint32_t f = chaining_vars[5];
        uint32_t g = chaining_vars[6];
        uint32_t h = chaining_vars[7];

        for (size_t t = 0; t < 64; t++) {
            uint32_t t1 = h + sigma1(e) + ch(e, f, g) + K_CONSTANTS[t] + w[t];
            uint32_t t2 = sigma0(a) + maj(a, b, c);

            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        chaining_vars[0] += a;
        chaining_vars[1] += b;
        chaining_vars[2] += c;
        chaining_vars[3] += d;
        chaining_vars[4] += e;
        chaining_vars[5] += f;
        chaining_vars[6] += g;
        chaining_vars[7] += h;
    }
};

