#pragma once

#include <cassert>
#include <cstdint>

namespace heddle {
    class SplitMix64 final {
    public:
        explicit constexpr SplitMix64(std::uint64_t seed) noexcept : state_{seed} {
        }

        [[nodiscard]] constexpr std::uint64_t next() noexcept {
            // increment the state variable
            std::uint64_t z = (state_ += 0x9e3779b97f4a7c15);
            // xor the variable with the variable right bit shifted 30 then multiply by a constant
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
            // xor the variable with the variable right bit shifted 27 then multiply by a constant
            z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
            // return the variable xored with itself right bit shifted 31
            return z ^ (z >> 31);
        }

        [[nodiscard]] constexpr std::uint64_t bounded(std::uint64_t n) noexcept {
            assert(n > 0);
            const std::uint64_t threshold = (std::uint64_t{0} - n) % n;
            for (;;) {
                const std::uint64_t x = next();
                if (x >= threshold) {
                    return x % n;
                }
            }
        }

    private:
        std::uint64_t state_;
    };

    using Rng = SplitMix64;
}
