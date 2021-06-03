#pragma once

#include <string>
#include <vector>

namespace ChipDB
{

#if 0
    using InstanceIndex = int32_t;
    using PinIndex      = int32_t;
    using NetIndex      = int32_t;
    using CellIndex     = int32_t;
    using ModuleIndex   = int32_t;
    using LayerInfoIndex = int32_t;
    using SiteInfoIndex  = int32_t;
    using RegionIndex    = int32_t;
    using RowIndex      = int32_t;
    using ObstructionIndex = int32_t;
#endif

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


    /** 64-bit coordinate expressed in nanometers 
     *  use this for absolute position information
    */
    struct Coord64
    {
        Coord64() : m_x(0), m_y(0) {}
        Coord64(const int64_t &x, const int64_t &y) : m_x(x), m_y(y) {}
        Coord64(const Coord64 &c) : m_x(c.m_x), m_y(c.m_y) {}

        int64_t m_x;   ///< x coordinate in nanometers
        int64_t m_y;   ///< y coordinate in nanometers

        Coord64& operator+=(const Coord64& rhs) noexcept
        { 
            m_x += rhs.m_x;
            m_y += rhs.m_y;
            return *this;
        }

        Coord64& operator-=(const Coord64& rhs) noexcept
        { 
            m_x -= rhs.m_x;
            m_y -= rhs.m_y;
            return *this;
        }

        Coord64 operator+(const Coord64& rhs) const noexcept
        { 
            return Coord64{m_x + rhs.m_x, m_y + rhs.m_y};
        }

        Coord64 operator-(const Coord64& rhs) const noexcept
        {
            return Coord64{m_x - rhs.m_x, m_y - rhs.m_y};
        } 

        // unary minus
        Coord64 operator-() const noexcept
        {
            return Coord64{-m_x, -m_y};
        } 

        bool operator==(const Coord64& other) const noexcept
        {
            return (m_x == other.m_x) && (m_y == other.m_y);
        }   

        bool operator!=(const Coord64& other) const noexcept
        {
            return (m_x != other.m_x) || (m_y != other.m_y);
        }           
    };

    

    struct Rect64
    {
        Rect64() : m_ll{0,0}, m_ur{0,0} {}
        Rect64(const Coord64 &ll, const Coord64 &ur) :
            m_ll(ll), m_ur(ur) {}

        Coord64 m_ll;   ///< lower left
        Coord64 m_ur;   ///< upper right

        void setLL(const Coord64 &p)
        {
            m_ll = p;
        }

        void setUL(const Coord64 &p)
        {
            m_ll.m_x = p.m_x;
            m_ur.m_y = p.m_y;
        }

        void setUR(const Coord64 &p)
        {
            m_ur = p;
        }    

        void setLR(const Coord64 &p)
        {
            m_ll.m_y = p.m_y;
            m_ur.m_x = p.m_x;
        }  

        void setSize(const Coord64 &s)
        {
            m_ur.m_x = m_ll.m_x + s.m_x;
            m_ur.m_y = m_ll.m_y + s.m_y;
        }

        Coord64 getSize() const
        {
            return Coord64{m_ur.m_x - m_ll.m_x, m_ur.m_y - m_ll.m_y};
        }

        [[nodiscard]] Rect64 moveTo(const Coord64 &p) const
        {
            auto delta = p - m_ll;
            return {p, m_ur + delta};
        }

        void moveTo(const Coord64 &p)
        {
            auto delta = p - m_ll;
            m_ll = p;
            m_ur = m_ur + delta;
        }


        void moveBy(const Coord64 &offset)
        {
            m_ll = m_ll + offset;
            m_ur = m_ur + offset;
        }

        [[nodiscard]] Rect64 moveBy(const Coord64 &offset) const 
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

        Coord64 pos() const noexcept
        {
            return m_ll;
        }   

        Coord64 center() const noexcept
        {
            return Coord64{(m_ll.m_x + m_ur.m_x)/2, (m_ll.m_y + m_ur.m_y)/2};
        }

        Rect64& operator+=(const Coord64& rhs) noexcept
        { 
            m_ll += rhs;
            m_ur += rhs;
            return *this;
        }

        Rect64& operator-=(const Coord64& rhs) noexcept
        { 
            m_ll -= rhs;
            m_ur -= rhs;
            return *this;
        }

        Rect64 operator+(const Coord64& rhs) const noexcept
        { 
            return Rect64{m_ll + rhs, m_ur + rhs};
        }

        Rect64 operator-(const Coord64& rhs) const noexcept
        { 
            return Rect64{m_ll - rhs, m_ur - rhs};
        }        
    };

    struct Polygon64
    {
        std::vector<Coord64> m_points;
    };

    /** Test if two rectangles intersect. */
    bool intersects(const Rect64 &p, const Rect64 &q) noexcept;

    /** Test if a point is within a rectangle */
    bool isInsideRect(const ChipDB::Coord64 &p, const ChipDB::Rect64 &r) noexcept;

    ChipDB::Rect64 unionRect(const ChipDB::Rect64 &r1, const ChipDB::Rect64 &r2) noexcept;

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

    enum Orientation : uint8_t
    {
        ORIENT_UNDEFINED = 0,
        ORIENT_R0,
        ORIENT_R90,
        ORIENT_R180,
        ORIENT_R270,
        ORIENT_MX,
        ORIENT_MX90,
        ORIENT_MY,
        ORIENT_MY90
    };

    enum PlacementInfo : uint8_t
    {
        PLACEMENT_IGNORE = 0,       ///< ignore instance during placement
        PLACEMENT_UNPLACED,         ///< instance unplaced
        PLACEMENT_PLACED,           ///< instance placed but still movable
        PLACEMENT_PLACEDANDFIXED,   ///< instance placed and not movable
    };

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

    enum CellClass : uint8_t
    {
        CLASS_CORE = 0,
        CLASS_COVER,
        CLASS_RING,
        CLASS_PAD,
        CLASS_ENDCAP,
        CLASS_BLOCK
    };

    enum CellSubclass : uint8_t
    {
        SUBCLASS_NONE = 0,
        SUBCLASS_BUMP,          ///< COVER subtype
        SUBCLASS_BLACKBOX,      ///< BLOCK subtype
        SUBCLASS_SOFT,          ///< BLOCK subtype
        SUBCLASS_INPUT,         ///< PAD subtype
        SUBCLASS_OUTPUT,        ///< PAD subtype
        SUBCLASS_INOUT,         ///< PAD subtype
        SUBCLASS_POWER,         ///< PAD subtype
        SUBCLASS_SPACER,        ///< PAD or CORE subtype
        SUBCLASS_AREAIO,        ///< PAD subtype
        SUBCLASS_FEEDTHRU,      ///< CORE subtype
        SUBCLASS_TIEHIGH,       ///< CORE subtype
        SUBCLASS_TIELOW,        ///< CORE subtype
        SUBCLASS_ANTENNACELL,   ///< CORE subtype
        SUBCLASS_WELLTAP,       ///< CORE subtype
        SUBCLASS_PRE,           ///< ENDCAP subtype
        SUBCLASS_POST,          ///< ENDCAP subtype
        SUBCLASS_TOPLEFT,       ///< ENDCAP subtype
        SUBCLASS_TOPRIGHT,      ///< ENDCAP subtype
        SUBCLASS_BOTTOMLEFT,    ///< ENDCAP subtype
        SUBCLASS_BOTTOMRIGHT,   ///< ENDCAP subtype
    };

    std::string toString(const CellSubclass &v);
    std::string toString(const CellClass &v);

    /** create a new string with upper case characters */
    std::string toUpper(const std::string &txt);
};

ChipDB::SymmetryFlags& operator+=(ChipDB::SymmetryFlags &lhs, const uint8_t &rhs);

std::ostream& operator<<(std::ostream& os, const ChipDB::Coord64& r);
std::ostream& operator<<(std::ostream& os, const ChipDB::Rect64& r);