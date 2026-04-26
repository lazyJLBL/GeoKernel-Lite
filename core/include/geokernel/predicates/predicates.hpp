#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace geokernel {

enum class PredicateMode {
    Eps,
    FilteredExact,
    Exact
};

struct PredicateComparisonResult {
    std::string predicate;
    int epsSign = 0;
    int filteredSign = 0;
    int exactSign = 0;
    double epsEstimate = 0.0;
    double filteredEstimate = 0.0;
    bool epsDiffersFromExact = false;
    bool filteredMatchesExact = false;
    std::string disagreement;
};

inline const char* predicateModeName(PredicateMode mode) {
    switch (mode) {
        case PredicateMode::Eps: return "eps";
        case PredicateMode::FilteredExact: return "filtered_exact";
        case PredicateMode::Exact: return "exact";
    }
    return "unknown";
}

inline PredicateMode predicateModeFromString(const std::string& value) {
    if (value == "eps") return PredicateMode::Eps;
    if (value == "filtered" || value == "filtered_exact") return PredicateMode::FilteredExact;
    if (value == "exact") return PredicateMode::Exact;
    throw std::invalid_argument("unknown predicate mode: " + value);
}

namespace predicate_detail {

inline void requireFinite(double value, const char* name) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument(std::string("predicate input is not finite: ") + name);
    }
}

inline int rawSign(double value) {
    if (value > 0.0) return 1;
    if (value < 0.0) return -1;
    return 0;
}

inline int epsSign(double value, double eps) {
    if (value > eps) return 1;
    if (value < -eps) return -1;
    return 0;
}

struct BigUnsigned {
    static constexpr uint32_t BASE_BITS = 16;
    static constexpr uint32_t BASE = 1u << BASE_BITS;
    static constexpr uint32_t MASK = BASE - 1u;

    std::vector<uint32_t> limbs;

    bool isZero() const {
        return limbs.empty();
    }

    void trim() {
        while (!limbs.empty() && limbs.back() == 0u) {
            limbs.pop_back();
        }
    }

    static BigUnsigned fromUint64(uint64_t value) {
        BigUnsigned out;
        while (value != 0u) {
            out.limbs.push_back(static_cast<uint32_t>(value & MASK));
            value >>= BASE_BITS;
        }
        return out;
    }

    static int compare(const BigUnsigned& a, const BigUnsigned& b) {
        if (a.limbs.size() != b.limbs.size()) {
            return a.limbs.size() < b.limbs.size() ? -1 : 1;
        }
        for (std::size_t i = a.limbs.size(); i > 0; --i) {
            const uint32_t av = a.limbs[i - 1];
            const uint32_t bv = b.limbs[i - 1];
            if (av != bv) return av < bv ? -1 : 1;
        }
        return 0;
    }

    static BigUnsigned add(const BigUnsigned& a, const BigUnsigned& b) {
        BigUnsigned out;
        const std::size_t n = std::max(a.limbs.size(), b.limbs.size());
        out.limbs.resize(n);
        uint32_t carry = 0u;
        for (std::size_t i = 0; i < n; ++i) {
            const uint32_t av = i < a.limbs.size() ? a.limbs[i] : 0u;
            const uint32_t bv = i < b.limbs.size() ? b.limbs[i] : 0u;
            const uint32_t sum = av + bv + carry;
            out.limbs[i] = sum & MASK;
            carry = sum >> BASE_BITS;
        }
        if (carry != 0u) out.limbs.push_back(carry);
        out.trim();
        return out;
    }

    static BigUnsigned subtract(const BigUnsigned& a, const BigUnsigned& b) {
        BigUnsigned out;
        out.limbs.resize(a.limbs.size());
        int32_t borrow = 0;
        for (std::size_t i = 0; i < a.limbs.size(); ++i) {
            const int32_t av = static_cast<int32_t>(a.limbs[i]);
            const int32_t bv = i < b.limbs.size() ? static_cast<int32_t>(b.limbs[i]) : 0;
            int32_t diff = av - bv - borrow;
            if (diff < 0) {
                diff += static_cast<int32_t>(BASE);
                borrow = 1;
            } else {
                borrow = 0;
            }
            out.limbs[i] = static_cast<uint32_t>(diff);
        }
        out.trim();
        return out;
    }

    static BigUnsigned shiftLeftBits(const BigUnsigned& value, std::size_t bits) {
        if (value.isZero() || bits == 0u) return value;
        const std::size_t limbShift = bits / BASE_BITS;
        const uint32_t bitShift = static_cast<uint32_t>(bits % BASE_BITS);
        BigUnsigned out;
        out.limbs.assign(limbShift, 0u);
        uint32_t carry = 0u;
        for (uint32_t limb : value.limbs) {
            const uint32_t shifted = static_cast<uint32_t>((limb << bitShift) & MASK);
            out.limbs.push_back(shifted | carry);
            carry = bitShift == 0u ? 0u : static_cast<uint32_t>(limb >> (BASE_BITS - bitShift));
        }
        if (carry != 0u) out.limbs.push_back(carry);
        out.trim();
        return out;
    }

    static BigUnsigned multiply(const BigUnsigned& a, const BigUnsigned& b) {
        if (a.isZero() || b.isZero()) return {};
        BigUnsigned out;
        out.limbs.assign(a.limbs.size() + b.limbs.size(), 0u);
        for (std::size_t i = 0; i < a.limbs.size(); ++i) {
            uint64_t carry = 0u;
            for (std::size_t j = 0; j < b.limbs.size(); ++j) {
                const uint64_t cur = static_cast<uint64_t>(out.limbs[i + j]) +
                                     static_cast<uint64_t>(a.limbs[i]) * b.limbs[j] + carry;
                out.limbs[i + j] = static_cast<uint32_t>(cur & MASK);
                carry = cur >> BASE_BITS;
            }
            std::size_t pos = i + b.limbs.size();
            while (carry != 0u) {
                if (pos >= out.limbs.size()) out.limbs.push_back(0u);
                const uint64_t cur = static_cast<uint64_t>(out.limbs[pos]) + carry;
                out.limbs[pos] = static_cast<uint32_t>(cur & MASK);
                carry = cur >> BASE_BITS;
                ++pos;
            }
        }
        out.trim();
        return out;
    }
};

struct SignedBigInt {
    int sign = 0;
    BigUnsigned mag;

    static SignedBigInt fromSignedMagnitude(int signValue, BigUnsigned magnitude) {
        magnitude.trim();
        if (magnitude.isZero() || signValue == 0) return {};
        return {signValue > 0 ? 1 : -1, std::move(magnitude)};
    }

    static SignedBigInt fromMantissa(int signValue, uint64_t mantissa) {
        return fromSignedMagnitude(signValue, BigUnsigned::fromUint64(mantissa));
    }

    SignedBigInt negated() const {
        return sign == 0 ? *this : SignedBigInt{-sign, mag};
    }

    static SignedBigInt shiftedLeft(const SignedBigInt& value, std::size_t bits) {
        if (value.sign == 0) return {};
        return fromSignedMagnitude(value.sign, BigUnsigned::shiftLeftBits(value.mag, bits));
    }

    static SignedBigInt add(const SignedBigInt& a, const SignedBigInt& b) {
        if (a.sign == 0) return b;
        if (b.sign == 0) return a;
        if (a.sign == b.sign) {
            return fromSignedMagnitude(a.sign, BigUnsigned::add(a.mag, b.mag));
        }
        const int cmp = BigUnsigned::compare(a.mag, b.mag);
        if (cmp == 0) return {};
        if (cmp > 0) return fromSignedMagnitude(a.sign, BigUnsigned::subtract(a.mag, b.mag));
        return fromSignedMagnitude(b.sign, BigUnsigned::subtract(b.mag, a.mag));
    }

    static SignedBigInt subtract(const SignedBigInt& a, const SignedBigInt& b) {
        return add(a, b.negated());
    }

    static SignedBigInt multiply(const SignedBigInt& a, const SignedBigInt& b) {
        if (a.sign == 0 || b.sign == 0) return {};
        return fromSignedMagnitude(a.sign * b.sign, BigUnsigned::multiply(a.mag, b.mag));
    }
};

struct BigDyadic {
    SignedBigInt coeff;
    int exponent = 0;

    static BigDyadic zero() {
        return {};
    }

    static BigDyadic fromDouble(double value, const char* name) {
        requireFinite(value, name);
        if (value == 0.0) return zero();
        const int sign = std::signbit(value) ? -1 : 1;
        int exp = 0;
        const double frac = std::frexp(std::fabs(value), &exp);
        uint64_t mantissa = static_cast<uint64_t>(std::ldexp(frac, 53));
        int dyadicExp = exp - 53;
        while ((mantissa & 1u) == 0u) {
            mantissa >>= 1u;
            ++dyadicExp;
        }
        return {SignedBigInt::fromMantissa(sign, mantissa), dyadicExp};
    }

    BigDyadic negated() const {
        return {coeff.negated(), exponent};
    }

    static BigDyadic add(const BigDyadic& a, const BigDyadic& b) {
        if (a.coeff.sign == 0) return b;
        if (b.coeff.sign == 0) return a;
        const int exp = std::min(a.exponent, b.exponent);
        const auto ashift = static_cast<std::size_t>(a.exponent - exp);
        const auto bshift = static_cast<std::size_t>(b.exponent - exp);
        return {
            SignedBigInt::add(SignedBigInt::shiftedLeft(a.coeff, ashift),
                              SignedBigInt::shiftedLeft(b.coeff, bshift)),
            exp
        };
    }

    static BigDyadic subtract(const BigDyadic& a, const BigDyadic& b) {
        return add(a, b.negated());
    }

    static BigDyadic multiply(const BigDyadic& a, const BigDyadic& b) {
        return {SignedBigInt::multiply(a.coeff, b.coeff), a.exponent + b.exponent};
    }

    int sign() const {
        return coeff.sign;
    }
};

inline BigDyadic exactDiff(double a, double b, const char* aname, const char* bname) {
    return BigDyadic::subtract(BigDyadic::fromDouble(a, aname), BigDyadic::fromDouble(b, bname));
}

inline int orient2dExactCoordinates(double ax, double ay, double bx, double by, double cx, double cy) {
    const BigDyadic abx = exactDiff(bx, ax, "bx", "ax");
    const BigDyadic aby = exactDiff(by, ay, "by", "ay");
    const BigDyadic acx = exactDiff(cx, ax, "cx", "ax");
    const BigDyadic acy = exactDiff(cy, ay, "cy", "ay");
    const BigDyadic det = BigDyadic::subtract(BigDyadic::multiply(abx, acy),
                                              BigDyadic::multiply(aby, acx));
    return det.sign();
}

inline int incircleExactCoordinates(double ax, double ay, double bx, double by, double cx, double cy, double dx, double dy) {
    const BigDyadic adx = exactDiff(ax, dx, "ax", "dx");
    const BigDyadic ady = exactDiff(ay, dy, "ay", "dy");
    const BigDyadic bdx = exactDiff(bx, dx, "bx", "dx");
    const BigDyadic bdy = exactDiff(by, dy, "by", "dy");
    const BigDyadic cdx = exactDiff(cx, dx, "cx", "dx");
    const BigDyadic cdy = exactDiff(cy, dy, "cy", "dy");

    const BigDyadic alift = BigDyadic::add(BigDyadic::multiply(adx, adx), BigDyadic::multiply(ady, ady));
    const BigDyadic blift = BigDyadic::add(BigDyadic::multiply(bdx, bdx), BigDyadic::multiply(bdy, bdy));
    const BigDyadic clift = BigDyadic::add(BigDyadic::multiply(cdx, cdx), BigDyadic::multiply(cdy, cdy));

    const BigDyadic term1 = BigDyadic::multiply(
        adx,
        BigDyadic::subtract(BigDyadic::multiply(bdy, clift), BigDyadic::multiply(blift, cdy)));
    const BigDyadic term2 = BigDyadic::multiply(
        ady,
        BigDyadic::subtract(BigDyadic::multiply(bdx, clift), BigDyadic::multiply(blift, cdx)));
    const BigDyadic term3 = BigDyadic::multiply(
        alift,
        BigDyadic::subtract(BigDyadic::multiply(bdx, cdy), BigDyadic::multiply(bdy, cdx)));
    const BigDyadic det = BigDyadic::add(BigDyadic::subtract(term1, term2), term3);
    return det.sign();
}

}  // namespace predicate_detail

inline double orient2dDeterminantEstimate(double ax, double ay, double bx, double by, double cx, double cy) {
    predicate_detail::requireFinite(ax, "ax");
    predicate_detail::requireFinite(ay, "ay");
    predicate_detail::requireFinite(bx, "bx");
    predicate_detail::requireFinite(by, "by");
    predicate_detail::requireFinite(cx, "cx");
    predicate_detail::requireFinite(cy, "cy");
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

inline int orient2dEps(double ax, double ay, double bx, double by, double cx, double cy, double eps = 1e-9) {
    return predicate_detail::epsSign(orient2dDeterminantEstimate(ax, ay, bx, by, cx, cy), eps);
}

inline int orient2dExact(double ax, double ay, double bx, double by, double cx, double cy) {
    return predicate_detail::orient2dExactCoordinates(ax, ay, bx, by, cx, cy);
}

inline int orient2dFiltered(double ax, double ay, double bx, double by, double cx, double cy) {
    const double abx = bx - ax;
    const double aby = by - ay;
    const double acx = cx - ax;
    const double acy = cy - ay;
    const double det = orient2dDeterminantEstimate(ax, ay, bx, by, cx, cy);
    const double permanent = std::fabs(abx * acy) + std::fabs(aby * acx);
    const double errbound = (16.0 * std::numeric_limits<double>::epsilon()) * permanent;
    if (std::fabs(det) > errbound) return predicate_detail::rawSign(det);
    return orient2dExact(ax, ay, bx, by, cx, cy);
}

inline int orient2d(double ax, double ay, double bx, double by, double cx, double cy, PredicateMode mode = PredicateMode::FilteredExact) {
    switch (mode) {
        case PredicateMode::Eps: return orient2dEps(ax, ay, bx, by, cx, cy);
        case PredicateMode::FilteredExact: return orient2dFiltered(ax, ay, bx, by, cx, cy);
        case PredicateMode::Exact: return orient2dExact(ax, ay, bx, by, cx, cy);
    }
    return orient2dFiltered(ax, ay, bx, by, cx, cy);
}

inline double incircleDeterminantEstimate(double ax, double ay, double bx, double by, double cx, double cy, double dx, double dy) {
    predicate_detail::requireFinite(ax, "ax");
    predicate_detail::requireFinite(ay, "ay");
    predicate_detail::requireFinite(bx, "bx");
    predicate_detail::requireFinite(by, "by");
    predicate_detail::requireFinite(cx, "cx");
    predicate_detail::requireFinite(cy, "cy");
    predicate_detail::requireFinite(dx, "dx");
    predicate_detail::requireFinite(dy, "dy");
    const double adx = ax - dx;
    const double ady = ay - dy;
    const double bdx = bx - dx;
    const double bdy = by - dy;
    const double cdx = cx - dx;
    const double cdy = cy - dy;
    const double alift = adx * adx + ady * ady;
    const double blift = bdx * bdx + bdy * bdy;
    const double clift = cdx * cdx + cdy * cdy;
    return adx * (bdy * clift - blift * cdy) -
           ady * (bdx * clift - blift * cdx) +
           alift * (bdx * cdy - bdy * cdx);
}

inline int incircleEps(double ax, double ay, double bx, double by, double cx, double cy, double dx, double dy, double eps = 1e-9) {
    return predicate_detail::epsSign(incircleDeterminantEstimate(ax, ay, bx, by, cx, cy, dx, dy), eps);
}

inline int incircleExact(double ax, double ay, double bx, double by, double cx, double cy, double dx, double dy) {
    return predicate_detail::incircleExactCoordinates(ax, ay, bx, by, cx, cy, dx, dy);
}

inline int incircleFiltered(double ax, double ay, double bx, double by, double cx, double cy, double dx, double dy) {
    const double adx = ax - dx;
    const double ady = ay - dy;
    const double bdx = bx - dx;
    const double bdy = by - dy;
    const double cdx = cx - dx;
    const double cdy = cy - dy;
    const double alift = adx * adx + ady * ady;
    const double blift = bdx * bdx + bdy * bdy;
    const double clift = cdx * cdx + cdy * cdy;
    const double det = incircleDeterminantEstimate(ax, ay, bx, by, cx, cy, dx, dy);
    const double permanent =
        std::fabs(adx) * (std::fabs(bdy) * clift + blift * std::fabs(cdy)) +
        std::fabs(ady) * (std::fabs(bdx) * clift + blift * std::fabs(cdx)) +
        alift * (std::fabs(bdx) * std::fabs(cdy) + std::fabs(bdy) * std::fabs(cdx));
    const double errbound = (64.0 * std::numeric_limits<double>::epsilon()) * permanent;
    if (std::fabs(det) > errbound) return predicate_detail::rawSign(det);
    return incircleExact(ax, ay, bx, by, cx, cy, dx, dy);
}

inline int incircle(double ax, double ay, double bx, double by, double cx, double cy, double dx, double dy, PredicateMode mode = PredicateMode::FilteredExact) {
    switch (mode) {
        case PredicateMode::Eps: return incircleEps(ax, ay, bx, by, cx, cy, dx, dy);
        case PredicateMode::FilteredExact: return incircleFiltered(ax, ay, bx, by, cx, cy, dx, dy);
        case PredicateMode::Exact: return incircleExact(ax, ay, bx, by, cx, cy, dx, dy);
    }
    return incircleFiltered(ax, ay, bx, by, cx, cy, dx, dy);
}

template <typename Point>
inline int orient2dEps(const Point& a, const Point& b, const Point& c, double eps = 1e-9) {
    return orient2dEps(a.x, a.y, b.x, b.y, c.x, c.y, eps);
}

template <typename Point>
inline int orient2dFiltered(const Point& a, const Point& b, const Point& c) {
    return orient2dFiltered(a.x, a.y, b.x, b.y, c.x, c.y);
}

template <typename Point>
inline int orient2dExact(const Point& a, const Point& b, const Point& c) {
    return orient2dExact(a.x, a.y, b.x, b.y, c.x, c.y);
}

template <typename Point>
inline int orient2d(const Point& a, const Point& b, const Point& c, PredicateMode mode = PredicateMode::FilteredExact) {
    return orient2d(a.x, a.y, b.x, b.y, c.x, c.y, mode);
}

template <typename Point>
inline int incircleEps(const Point& a, const Point& b, const Point& c, const Point& d, double eps = 1e-9) {
    return incircleEps(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y, eps);
}

template <typename Point>
inline int incircleFiltered(const Point& a, const Point& b, const Point& c, const Point& d) {
    return incircleFiltered(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y);
}

template <typename Point>
inline int incircleExact(const Point& a, const Point& b, const Point& c, const Point& d) {
    return incircleExact(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y);
}

template <typename Point>
inline int incircle(const Point& a, const Point& b, const Point& c, const Point& d, PredicateMode mode = PredicateMode::FilteredExact) {
    return incircle(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y, mode);
}

inline PredicateComparisonResult compareOrient2d(double ax, double ay, double bx, double by, double cx, double cy, double eps = 1e-9) {
    PredicateComparisonResult result;
    result.predicate = "orient2d";
    result.epsEstimate = orient2dDeterminantEstimate(ax, ay, bx, by, cx, cy);
    result.filteredEstimate = result.epsEstimate;
    result.epsSign = orient2dEps(ax, ay, bx, by, cx, cy, eps);
    result.filteredSign = orient2dFiltered(ax, ay, bx, by, cx, cy);
    result.exactSign = orient2dExact(ax, ay, bx, by, cx, cy);
    result.epsDiffersFromExact = result.epsSign != result.exactSign;
    result.filteredMatchesExact = result.filteredSign == result.exactSign;
    result.disagreement = result.epsDiffersFromExact ? "eps_collapsed_nonzero_orientation" : "";
    return result;
}

inline PredicateComparisonResult compareIncircle(double ax, double ay, double bx, double by, double cx, double cy, double dx, double dy, double eps = 1e-9) {
    PredicateComparisonResult result;
    result.predicate = "incircle";
    result.epsEstimate = incircleDeterminantEstimate(ax, ay, bx, by, cx, cy, dx, dy);
    result.filteredEstimate = result.epsEstimate;
    result.epsSign = incircleEps(ax, ay, bx, by, cx, cy, dx, dy, eps);
    result.filteredSign = incircleFiltered(ax, ay, bx, by, cx, cy, dx, dy);
    result.exactSign = incircleExact(ax, ay, bx, by, cx, cy, dx, dy);
    result.epsDiffersFromExact = result.epsSign != result.exactSign;
    result.filteredMatchesExact = result.filteredSign == result.exactSign;
    result.disagreement = result.epsDiffersFromExact ? "eps_collapsed_nonzero_incircle" : "";
    return result;
}

template <typename Point>
inline PredicateComparisonResult compareOrient2d(const Point& a, const Point& b, const Point& c, double eps = 1e-9) {
    return compareOrient2d(a.x, a.y, b.x, b.y, c.x, c.y, eps);
}

template <typename Point>
inline PredicateComparisonResult compareIncircle(const Point& a, const Point& b, const Point& c, const Point& d, double eps = 1e-9) {
    return compareIncircle(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y, eps);
}

}  // namespace geokernel
