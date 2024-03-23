#include "gds2defs.hpp"
#include <cmath>
#include <cassert>
#include <bit>

namespace LunaCore::GDS2
{

GDS2Float IEEE2GDSFloat(double value)
{
    GDS2Float result{.m_data{0,0,0,0,0,0,0,0}};

    const bool negative = value < 0.0;
    if (negative)
    {
        result.m_data[0] |= 0x80;   // set sign bit
        value = -value;
    }

    // figure out the exponent
    int exp = 0;
    if (value < 1e-77)
    {
        value = 0;  // round down to zero
    }
    else
    {
        auto const log2_16 = 4.0;
        auto bits = log2(value) / log2_16;
        exp = static_cast<int>(std::ceil(bits));
        if (exp == static_cast<int>(log2_16))
        {
            exp++;
        }
    }

    value = value / std::pow(16.0, static_cast<double>(exp-14));
    assert(exp >= -64);
    assert(exp < 64);

    // set the exponent, preserve the sign bit
    result.m_data[0] |= (exp + 64) & 0x7f;

    // generate and round mantissa
    uint64_t mantissa = static_cast<uint64_t>(value + 0.5);

    for(int i=7; i>0; i--)
    {
        result.m_data[i] = mantissa & 0xff;
        mantissa >>= 8;
    }

    return result;
}

double GDS2Float2IEEE(const GDS2Float &value)
{
    // VAX float format(?)
    // SEEEEEEE | MMMMMMMM | MMMMMMMM | MMMMMMMM ...
    // MMMMMMMM | MMMMMMMM | MMMMMMMM | MMMMMMMM
    //
    // exponent is encoded as an excess-64 number
    //
    // value = mantissa * 16^exponent
    //
    //

    static_assert(std::endian::native == std::endian::little, "GDS2Float2IEEE does not work on big-endian machines yet");

    double  result = 0;

    // determine if the value is negative
    bool    negative = (value.m_data[0] & 0x80) != 0;

    // determine the sign of the exponent
    uint8_t sexp = value.m_data[0] ^ 0x40;

    // construct the mantissa
    uint64_t mantissa = static_cast<uint64_t>(value.m_data[1]) << 48;
    mantissa |= static_cast<uint64_t>(value.m_data[2]) << 40;
    mantissa |= static_cast<uint64_t>(value.m_data[3]) << 32;
    mantissa |= static_cast<uint64_t>(value.m_data[4]) << 24;
    mantissa |= static_cast<uint64_t>(value.m_data[5]) << 16;
    mantissa |= static_cast<uint64_t>(value.m_data[6]) << 8;
    mantissa |= static_cast<uint64_t>(value.m_data[7]);

    // construct the exponent
    int8_t exponent = (sexp & 0x7F) | ((sexp << 1) & 0x80);

    const double factor = std::pow(2.0, -56.0);
    result = static_cast<double>(mantissa)*std::pow(16.0, static_cast<double>(exponent))*factor;
    if (negative) return -result;
    return result;
}


};