// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "enums.h"

#ifdef NO_SSIZE_T
#include <type_traits>
typedef std::make_signed<size_t>::type ssize_t;
#endif

namespace ChipDB
{
    using ObjectKey         = int32_t;
    using NetObjectKey      = int32_t;
    using PinObjectKey      = int32_t;
    using InstanceObjectKey = int32_t;
    using CellObjectKey     = int32_t;
    using ModuleObjectKey   = int32_t;

    static constexpr ObjectKey ObjectNotFound = -1;
    static constexpr ObjectKey ObjectAlreadyExists = -2;
    static constexpr ObjectKey ObjectUnspecified = -3;

    using LayerID   = int32_t;

    /** base object that provides a getName() function */
    class NamedObject
    {
    public:
        NamedObject(const std::string &name) : m_name(name) {};

        virtual ~NamedObject()
        {
        }

        constexpr const std::string& getName() const noexcept
        {
            return m_name;
        }

        void setName(const std::string &name) noexcept
        {
            m_name = name;
        }

    protected:
        std::string m_name;
    };

    using CoordType = int64_t;

    /** 64-bit coordinate expressed in nanometers
     *  use this for absolute position information
    */
    struct Coord64
    {
        constexpr Coord64() : m_x(0), m_y(0) {}
        constexpr Coord64(const CoordType &x, const CoordType &y) : m_x(x), m_y(y) {}
        constexpr Coord64(const Coord64 &c) : m_x(c.m_x), m_y(c.m_y) {}

        CoordType m_x;   ///< x coordinate in nanometers
        CoordType m_y;   ///< y coordinate in nanometers

        constexpr Coord64& operator+=(const Coord64& rhs) noexcept
        {
            m_x += rhs.m_x;
            m_y += rhs.m_y;
            return *this;
        }

        constexpr Coord64& operator-=(const Coord64& rhs) noexcept
        {
            m_x -= rhs.m_x;
            m_y -= rhs.m_y;
            return *this;
        }

        [[nodiscard]] constexpr Coord64 operator+(const Coord64& rhs) const noexcept
        {
            return Coord64{m_x + rhs.m_x, m_y + rhs.m_y};
        }

        [[nodiscard]] constexpr Coord64 operator-(const Coord64& rhs) const noexcept
        {
            return Coord64{m_x - rhs.m_x, m_y - rhs.m_y};
        }

        /** Manhattan/rectilinear length of (0,0) to this point. */
        [[nodiscard]] int64_t manhattanLength() const noexcept
        {
            return std::abs(m_x) + std::abs(m_y);
        }

        /** Manhattan/rectilinear distance between two points */
        [[nodiscard]] int64_t manhattanDistance(const Coord64& rhs) const noexcept
        {
            return std::abs(m_x-rhs.m_x) + std::abs(m_y-rhs.m_y);
        }

        // unary minus
        constexpr Coord64 operator-() const noexcept
        {
            return Coord64{-m_x, -m_y};
        }

        [[nodiscard]]  constexpr bool operator==(const Coord64& other) const noexcept
        {
            return (m_x == other.m_x) && (m_y == other.m_y);
        }

        [[nodiscard]] constexpr bool operator!=(const Coord64& other) const noexcept
        {
            return (m_x != other.m_x) || (m_y != other.m_y);
        }

        [[nodiscard]] constexpr bool isNullSize() const noexcept
        {
            return (m_x == 0) && (m_y == 0);
        }

    };

    using Size64 = Coord64;

    struct Margins64
    {
        Margins64() : m_top(0), m_bottom(0), m_left(0), m_right(0) {}

        Margins64(int64_t top, int64_t bottom, int64_t left, int64_t right)
            : m_top(top), m_bottom(bottom), m_left(left), m_right(right)
            {}

        /** true if all margins are zero */
        [[nodiscard]] constexpr bool isNull() const noexcept
        {
            return (m_top == 0) && (m_bottom == 0) && (m_left == 0) && (m_right == 0);
        }

        [[nodiscard]] constexpr auto& top() noexcept { return m_top; }
        [[nodiscard]] constexpr auto& bottom() noexcept { return m_bottom; }
        [[nodiscard]] constexpr auto& left() noexcept { return m_left; }
        [[nodiscard]] constexpr auto& right() noexcept { return m_right; }

        [[nodiscard]] constexpr auto top() const noexcept { return m_top; }
        [[nodiscard]] constexpr auto bottom() const noexcept { return m_bottom; }
        [[nodiscard]] constexpr auto left() const noexcept { return m_left; }
        [[nodiscard]] constexpr auto right() const noexcept { return m_right; }

        int64_t m_top;      ///< in nm
        int64_t m_bottom;   ///< in nm
        int64_t m_left;     ///< in nm
        int64_t m_right;    ///< in nm
    };

    struct Rect64
    {
        constexpr Rect64() : m_ll{0,0}, m_ur{0,0} {}
        constexpr Rect64(const Coord64 &ll, const Coord64 &ur) :
            m_ll(ll), m_ur(ur) {}

        Coord64 m_ll;   ///< lower left
        Coord64 m_ur;   ///< upper right

        constexpr void setLL(const Coord64 &p)
        {
            m_ll = p;
        }

        constexpr void setUL(const Coord64 &p)
        {
            m_ll.m_x = p.m_x;
            m_ur.m_y = p.m_y;
        }

        constexpr void setUR(const Coord64 &p)
        {
            m_ur = p;
        }

        constexpr void setLR(const Coord64 &p)
        {
            m_ll.m_y = p.m_y;
            m_ur.m_x = p.m_x;
        }

        constexpr void setLeft(const int64_t c)
        {
            m_ll.m_x = c;
        }

        constexpr void setRight(const int64_t c)
        {
            m_ur.m_x = c;
        }

        constexpr void setTop(const int64_t c)
        {
            m_ur.m_y = c;
        }

        constexpr void setBottom(const int64_t c)
        {
            m_ll.m_y = c;
        }

        constexpr void setSize(const Coord64 &s)
        {
            m_ur.m_x = m_ll.m_x + s.m_x;
            m_ur.m_y = m_ll.m_y + s.m_y;
        }

        /** get lower left coordinate */
        constexpr Coord64 getLL() const
        {
            return m_ll;
        }

        /** get upper left coordinate */
        constexpr Coord64 getUL() const
        {
            return Coord64{m_ll.m_x, m_ur.m_y};
        }

        /** get lower right coordinate */
        constexpr Coord64 getLR() const
        {
            return Coord64{m_ur.m_x, m_ll.m_y};
        }

        /** get upper right coordinate */
        constexpr Coord64 getUR() const
        {
            return m_ur;
        }

        /** return the size of the rectangle */
        constexpr Coord64 getSize() const
        {
            return Coord64{m_ur.m_x - m_ll.m_x, m_ur.m_y - m_ll.m_y};
        }

        /** return a rectangle that is positioned at the given coordinate
         *  with it´s lower left vertex
        */
        [[nodiscard]] constexpr Rect64 movedTo(const Coord64 &p) const
        {
            auto delta = p - m_ll;
            return {p, m_ur + delta};
        }

        /** move the rectangle to the given coordinate
         *  with it´s lower left vertex.
        */
        constexpr void moveTo(const Coord64 &p) noexcept
        {
            auto delta = p - m_ll;
            m_ll = p;
            m_ur = m_ur + delta;
        }

        /** return a rectangle that is moved by the given offset.
        */
        constexpr void moveBy(const Coord64 &offset) noexcept
        {
            m_ll = m_ll + offset;
            m_ur = m_ur + offset;
        }

        /** move the rectangle by the given offset.
        */
        [[nodiscard]] Rect64 movedBy(const Coord64 &offset) const
        {
            return {m_ll + offset, m_ur + offset};
        }

        constexpr int64_t height() const noexcept
        {
            return m_ur.m_y - m_ll.m_y;
        }

        constexpr int64_t width() const noexcept
        {
            return m_ur.m_x - m_ll.m_x;
        }

        constexpr int64_t left() const noexcept
        {
            return m_ll.m_x;
        }

        constexpr int64_t right() const noexcept
        {
            return m_ur.m_x;
        }

        constexpr int64_t top() const noexcept
        {
            return m_ur.m_y;
        }

        constexpr int64_t bottom() const noexcept
        {
            return m_ll.m_y;
        }

        /** return the (lower left vertex) position of the rectangle */
        constexpr Coord64 pos() const noexcept
        {
            return m_ll;
        }

        /** return the center coordinate of the rectangle */
        constexpr Coord64 center() const noexcept
        {
            return Coord64{(m_ll.m_x + m_ur.m_x)/2, (m_ll.m_y + m_ur.m_y)/2};
        }

        constexpr Rect64& operator+=(const Coord64& rhs) noexcept
        {
            m_ll += rhs;
            m_ur += rhs;
            return *this;
        }

        constexpr Rect64& operator-=(const Coord64& rhs) noexcept
        {
            m_ll -= rhs;
            m_ur -= rhs;
            return *this;
        }

        constexpr Rect64 operator+(const Coord64& rhs) const noexcept
        {
            return Rect64{m_ll + rhs, m_ur + rhs};
        }

        constexpr Rect64 operator-(const Coord64& rhs) const noexcept
        {
            return Rect64{m_ll - rhs, m_ur - rhs};
        }

        /** return a rectangle that has been decreased in size by the margins */
        [[nodiscard]] constexpr Rect64 contracted(const Margins64 &margins) const noexcept
        {
            auto newRect = *this;
            newRect.m_ll.m_x += margins.m_left;
            newRect.m_ur.m_x -= margins.m_right;
            newRect.m_ll.m_y += margins.m_bottom;
            newRect.m_ur.m_y -= margins.m_top;
            return newRect;
        }

        /** return a rectangle that has been increased in size by the margins */
        [[nodiscard]] constexpr Rect64 expanded(const Margins64 &margins) const noexcept
        {
            auto newRect = *this;
            newRect.m_ll.m_x -= margins.m_left;
            newRect.m_ur.m_x += margins.m_right;
            newRect.m_ll.m_y -= margins.m_bottom;
            newRect.m_ur.m_y += margins.m_top;
            return newRect;
        }

        /** reduce the rectangle by the given margins */
        constexpr void contract(const Margins64 &margins) noexcept
        {
            m_ll.m_x += margins.m_left;
            m_ur.m_x -= margins.m_right;
            m_ll.m_y += margins.m_bottom;
            m_ur.m_y -= margins.m_top;
        }

        /** increase the rectangle by the given margins */
        constexpr void expand(const Margins64 &margins) noexcept
        {
            m_ll.m_x -= margins.m_left;
            m_ur.m_x += margins.m_right;
            m_ll.m_y -= margins.m_bottom;
            m_ur.m_y += margins.m_top;
        }

        [[nodiscard]] bool contains(const ChipDB::Coord64 &p) const noexcept
        {
            bool inx = ((p.m_x >= m_ll.m_x) && (p.m_x <= m_ur.m_x));
            if (!inx)
                return false;

            bool iny = ((p.m_y >= m_ll.m_y) && (p.m_y <= m_ur.m_y));
            if (!iny)
                return false;

            return true;
        }

    };

#if 0
    struct Polygon64
    {
        std::vector<Coord64> m_points;
    };

    /** Test if two rectangles intersect. */
    bool intersects(const Rect64 &p, const Rect64 &q) noexcept;

    /** Test if a point is within a rectangle */
    bool isInsideRect(const ChipDB::Coord64 &p, const ChipDB::Rect64 &r) noexcept;

    ChipDB::Rect64 unionRect(const ChipDB::Rect64 &r1, const ChipDB::Rect64 &r2) noexcept;
#endif

#if 0
    /* StrEnum for cell/site symmetry */
    struct SymmetryType : public StrEnum<SymmetryType>
    {
        SymmetryType() : StrEnum() {}
        SymmetryType(const std::string &s) { *this = s; }

        // FIXME: these should be flags as some are not mutually exclusive.
        static constexpr std::array<std::pair<int32_t, const char *>, 5> m_options =
        {{
            {0,"UNDEFINED"},
            {1,"X"},
            {2,"Y"},
            {3,"XY"},
            {4,"R90"}
        }};

        StrEnumOverloads(SymmetryType);
    };
#endif

    struct SymmetryFlags
    {
        SymmetryFlags() : m_flags(0) {}

        uint8_t m_flags;

        static constexpr uint8_t SYM_UNKNOWN = 0;
        static constexpr uint8_t SYM_NONE = 0;
        static constexpr uint8_t SYM_X = 1;
        static constexpr uint8_t SYM_Y = 2;
        static constexpr uint8_t SYM_R90 = 4;
    };

#if 0
    enum class CellClass : uint8_t
    {
        CORE = 0,
        COVER,
        RING,
        PAD,
        ENDCAP,
        BLOCK
    };

    enum class CellSubclass : uint8_t
    {
        NONE = 0,
        BUMP,          ///< COVER subtype
        BLACKBOX,      ///< BLOCK subtype
        SOFT,          ///< BLOCK subtype
        INPUT,         ///< PAD subtype
        OUTPUT,        ///< PAD subtype
        INOUT,         ///< PAD subtype
        POWER,         ///< PAD subtype
        SPACER,        ///< PAD or CORE subtype
        DECAP,         ///< CORE subtype (unique to LunaPnR)
        AREAIO,        ///< PAD subtype
        FEEDTHRU,      ///< CORE subtype
        TIEHIGH,       ///< CORE subtype
        TIELOW,        ///< CORE subtype
        ANTENNACELL,   ///< CORE subtype
        WELLTAP,       ///< CORE subtype
        PRE,           ///< ENDCAP subtype
        POST,          ///< ENDCAP subtype
        TOPLEFT,       ///< ENDCAP subtype
        TOPRIGHT,      ///< ENDCAP subtype
        BOTTOMLEFT,    ///< ENDCAP subtype
        BOTTOMRIGHT,   ///< ENDCAP subtype
    };
#endif

    std::string toString(const SymmetryFlags &v);

    struct XAxisAccessor
    {
        static constexpr ChipDB::CoordType get(const ChipDB::Coord64 &pos) noexcept
        {
            return pos.m_x;
        }

        static constexpr const char* m_name= "X";

    };

    struct YAxisAccessor
    {
        static constexpr ChipDB::CoordType get(const ChipDB::Coord64 &pos) noexcept
        {
            return pos.m_y;
        }

        static constexpr const char* m_name = "Y";
    };

    constexpr ChipDB::Coord64 rotate180(const ChipDB::Coord64 &p) noexcept
    {
        ChipDB::Coord64 result;
        result.m_x = -p.m_x;
        result.m_y = -p.m_y;
        return result;
    };

    constexpr ChipDB::Coord64 rotate90(const ChipDB::Coord64 &p) noexcept
    {
        ChipDB::Coord64 result;
        result.m_x = -p.m_y;
        result.m_y = p.m_x;
        return result;
    };

    constexpr ChipDB::Coord64 rotate270(const ChipDB::Coord64 &p) noexcept
    {
        ChipDB::Coord64 result;
        result.m_x = p.m_y;
        result.m_y = -p.m_x;
        return result;
    };
};

ChipDB::SymmetryFlags& operator+=(ChipDB::SymmetryFlags &lhs, const uint8_t &rhs);


std::ostream& operator<<(std::ostream& os, const ChipDB::Coord64& r);
std::ostream& operator<<(std::ostream& os, const ChipDB::Rect64& r);
std::ostream& operator<<(std::ostream& os, const ChipDB::Margins64& m);
std::ostream& operator<<(std::ostream& os, const ChipDB::PlacementInfo& pi);
std::ostream& operator<<(std::ostream& os, const ChipDB::Orientation& orientation);
