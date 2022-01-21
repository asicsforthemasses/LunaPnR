#pragma once

#include <string>
#include <vector>
#include <memory>
#include "common/visitor.h"
#include "celllib/cell.h"
#include "celllib/module.h"
#include "net.h"

namespace ChipDB
{

enum class InstanceType
{
    UNKNOWN = 0,
    ABSTRACT,
    CELL,
    MODULE,
    PIN
};

class InstanceBase
{
public:
    InstanceType m_insType = InstanceType::UNKNOWN;

    InstanceBase() : m_insType(InstanceType::UNKNOWN), m_orientation(Orientation::R0), 
        m_placementInfo(PlacementInfo::UNPLACED), m_flags(0) {}
    
    InstanceBase(const std::string &name) : m_name(name), m_insType(InstanceType::UNKNOWN), m_orientation(Orientation::R0), 
        m_placementInfo(PlacementInfo::UNPLACED), m_flags(0) {}

    virtual ~InstanceBase() = default;

    IMPLEMENT_ACCEPT

    /** returns true if the instance is a module */
    bool isModule() const noexcept
    {
        return m_insType == InstanceType::MODULE;
    }

    /** get area in um² */
    virtual double getArea() const noexcept = 0;

    /** return the underlying cell/module name */
    virtual std::string getArchetypeName() const = 0;
    
    /** access the pin information */
    //virtual PinInfoList& pinInfo() const = 0;

    /** return the size of the instance in nm */
    virtual const Coord64 instanceSize() const = 0;

    /** return the center position of the instance */
    virtual Coord64 getCenter() const = 0;

    /** get the name of the instance */
    std::string name() const noexcept
    {
        return m_name;
    }

    struct Pin
    {
        constexpr bool isValid() const
        {
            return ((m_pinKey != ObjectNotFound) && m_pinInfo);
        }

        std::string name() const
        {
            if (m_pinInfo)
            {
                return m_pinInfo->name();
            }
            return "INVALID PININFO";
        }
        
        NetObjectKey    m_netKey = ObjectNotFound;  ///< key of connected net
        PinObjectKey    m_pinKey = ObjectNotFound;  ///< key of this pin
        std::shared_ptr<PinInfo> m_pinInfo;         ///< information about the pin

        constexpr PinObjectKey pinKey() const
        {
            return m_pinKey;
        }

        constexpr NetObjectKey netKey() const
        {
            return m_netKey;
        }
    };

    virtual Pin getPin(PinObjectKey pinKey) const = 0;
    virtual Pin getPin(const std::string &pinName) const = 0;
    virtual bool setPinNet(PinObjectKey pinKey, NetObjectKey netKey) = 0;
    virtual size_t getNumberOfPins() const = 0;

    Coord64         m_pos;              ///< lower-left position of the instance
    Orientation     m_orientation;      ///< orientation of the cell instance
    PlacementInfo   m_placementInfo;    ///< placement status
    uint32_t        m_flags;            ///< non-persistent generic flags that can be used by algorithms

protected:
    std::string m_name;     ///< name of the instance
};

class Instance : public InstanceBase
{
public:
    Instance(const std::string &name, const std::shared_ptr<Cell> cell) : InstanceBase(name), m_cell(cell)
    {
        if (cell->isModule())
            m_insType = InstanceType::MODULE;
        else
            m_insType = InstanceType::CELL;

        m_pinToNet.resize(cell->m_pins.size());
    }

    virtual ~Instance() = default;

    IMPLEMENT_ACCEPT_OVERRIDE;

    /** get access to the cell/module, if there is one */
    const std::shared_ptr<Cell> cell() const
    {
        return m_cell;
    }

    /** return the cell size of the instance */
    const Coord64 instanceSize() const override
    {
        if (m_cell == nullptr)
            return Coord64{0,0};
            
        return m_cell->m_size;
    }

    /** return the center position of the instance */
    Coord64 getCenter() const override
    {
        if (m_cell != nullptr)
            return Coord64{m_pos.m_x + m_cell->m_size.m_x/2, m_pos.m_y + m_cell->m_size.m_y/2};
        else
            return m_pos;
    }

    /** get area in um² */
    double getArea() const noexcept override;

    /** return the underlying cell/module name */
    std::string getArchetypeName() const override;

    Pin getPin(PinObjectKey key) const override;
    Pin getPin(const std::string &pinName) const override;
    bool setPinNet(PinObjectKey pinKey, NetObjectKey netKey) override;
    size_t getNumberOfPins() const override;

protected:

    std::vector<NetObjectKey>   m_pinToNet;  ///< connections from pin to net
    const std::shared_ptr<Cell> m_cell;
};

class PinInstance : public InstanceBase
{
public:
    
    PinInstance(const std::string &name) : InstanceBase(name)
    {
        m_insType = InstanceType::PIN;
        m_pinInfo.m_name = name;
    }

    virtual ~PinInstance() {};

    IMPLEMENT_ACCEPT_OVERRIDE;

    /** return {0,0} for pins */
    const Coord64 instanceSize() const override
    {
        return Coord64{0,0};
    }

    /** return the position of the pin */
    Coord64 getCenter() const override
    {
        return m_pos;
    }

    /** get area in um² */
    double getArea() const noexcept override
    {
        return 0.0;
    }

    /** return the underlying cell/module name */
    virtual std::string getArchetypeName() const override;
    
    void setPinIOType(IOType iotype)
    {
        m_pinInfo.m_iotype = iotype;
    }

    Pin getPin(PinObjectKey key) const override;
    Pin getPin(const std::string &pinName) const override;
    bool setPinNet(PinObjectKey pinKey, NetObjectKey netKey) override;
    size_t getNumberOfPins() const override;

protected:
    PinInfo         m_pinInfo;
    NetObjectKey    m_connectedNet = ObjectNotFound;  ///< connection from pin to net
};

};
