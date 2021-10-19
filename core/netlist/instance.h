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

class InstanceBase
{
public:
    enum InstanceType : uint8_t
    {
        INS_ABSTRACT = 0,
        INS_CELL,
        INS_MODULE,
        INS_PIN
    } m_insType;

    InstanceBase() : m_parent(nullptr), m_insType(INS_ABSTRACT), m_orientation(Orientation::R0), 
        m_placementInfo(PlacementInfo::UNPLACED), m_id(-1), m_flags(0) {}
    
    InstanceBase(InstanceBase *parent) : m_parent(parent), m_insType(INS_ABSTRACT), m_orientation(Orientation::R0), 
        m_placementInfo(PlacementInfo::UNPLACED), m_id(-1), m_flags(0) {}

    virtual ~InstanceBase() = default;

    IMPLEMENT_ACCEPT

    /** returns true if the instance is a module */
    bool isModule() const noexcept
    {
        return m_insType == INS_MODULE;
    }

    /** get area in um² */
    virtual double getArea() const noexcept = 0;

    /** return the underlying cell/module name */
    virtual std::string getArchetypeName() const = 0;
    
    /** get pin information from the underlying cell or module 
     *  returns nullptr if pin not found.
    */
    virtual const PinInfo* getPinInfo(ssize_t pinIndex) const = 0;

    /** get pin information from the underlying cell or module 
     *  returns nullptr if pin not found.
    */
    virtual const PinInfo* getPinInfo(const std::string &pinName) const = 0;

    /** get pin index by name. returns -1 when not found. 
    */
    virtual const ssize_t getPinIndex(const std::string &pinName) const = 0;

    /** get the number of pins on this instance */
    virtual const size_t getNumberOfPins() const = 0;

    /** connect pin with specified index to the given net. 
     *  returns true if succesful.
    */
    virtual bool connect(ssize_t pinIndex, Net *net) = 0;

    /** connect pin with specified name to the given net. 
     *  returns true if succesful.
    */    
    virtual bool connect(const std::string &pinName, Net *net) = 0;

    /** returns the net connected to a pin with a given index.
     *  if the pin does not exist, it return nullptr.
    */
    virtual Net* getConnectedNet(ssize_t pinIndex) = 0;

    /** returns the net connected to a pin with a given index.
     *  if the pin does not exist, it return nullptr.
    */
    virtual const Net* getConnectedNet(ssize_t pinIndex) const = 0;

    /** return the size of the instance in nm */
    virtual const Coord64 instanceSize() const = 0;

    /** return the center position of the instance */
    virtual Coord64 getCenter() const = 0;

    std::string m_name;     ///< name of the instance
    InstanceBase *m_parent; ///< parent instance

    Coord64         m_pos;              ///< lower-left position of the instance
    Orientation     m_orientation;      ///< orientation of the cell instance
    PlacementInfo   m_placementInfo;    ///< placement status
    int32_t         m_id;               ///< unique id for each instance
    uint32_t        m_flags;            ///< non-persistent generic flags that can be used by algorithms
};

class Instance : public InstanceBase
{
public:
    
    Instance(const Cell *cell) : m_cell(cell)
    {
        if (cell->isModule())
            m_insType = INS_MODULE;
        else
            m_insType = INS_CELL;

        m_pinToNet.resize(cell->m_pins.size());
    }

    Instance(const Cell *cell, InstanceBase *parent) : InstanceBase(parent), m_cell(cell)
    {
        if (cell->isModule())
            m_insType = INS_MODULE;
        else
            m_insType = INS_CELL;

        m_pinToNet.resize(cell->m_pins.size());
    }    

    virtual ~Instance() = default;

    IMPLEMENT_ACCEPT_OVERRIDE;

    /** get access to the cell/module */
    const Cell* cell() const
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
    
    /** get pin information from the underlying cell or module 
     *  returns nullptr if pin not found.
    */
    const PinInfo* getPinInfo(ssize_t pinIndex) const override;

    /** get pin information from the underlying cell or module 
     *  returns nullptr if pin not found.
    */
    const PinInfo* getPinInfo(const std::string &pinName) const override;

    /** get pin index by name. returns -1 when not found. 
    */
    const ssize_t getPinIndex(const std::string &pinName) const override;

    /** get the number of pins on this instance */
    const size_t getNumberOfPins() const override;

    /** connect pin with specified index to the given net. 
     *  returns true if succesful.
    */
    bool connect(ssize_t pinIndex, Net *net) override;

    /** connect pin with specified name to the given net. 
     *  returns true if succesful.
    */    
    bool connect(const std::string &pinName, Net *net) override;

    /** returns the net connected to a pin with a given index.
     *  if the pin does not exist, it return nullptr.
    */
    Net* getConnectedNet(ssize_t pinIndex) override;

    /** returns the net connected to a pin with a given index.
     *  if the pin does not exist, it return nullptr.
    */
    const Net* getConnectedNet(ssize_t pinIndex) const override;

protected:
    std::vector<Net*>   m_pinToNet;  ///< connections from pin to net
    const Cell* m_cell;
};

class PinInstance : public InstanceBase
{
public:
    
    PinInstance(const std::string &name)
    {
        m_name = name;
        m_insType = INS_PIN;
        m_connectedNet = nullptr;
        m_pinInfo.m_name = name;
    }

    PinInstance(const std::string &name, InstanceBase *parent) : InstanceBase(parent)
    {
        m_name = name;
        m_insType = INS_PIN;
        m_connectedNet = nullptr;
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
    
    /** get pin information from the underlying cell or module 
     *  returns nullptr if pin not found.
    */
    virtual const PinInfo* getPinInfo(ssize_t pinIndex) const override;

    /** get pin information from the underlying cell or module 
     *  returns nullptr if pin not found.
    */
    virtual const PinInfo* getPinInfo(const std::string &pinName) const override;

    /** get pin index by name. returns -1 when not found. 
    */
    virtual const ssize_t getPinIndex(const std::string &pinName) const override;

    /** get the number of pins on this instance */
    virtual const size_t getNumberOfPins() const override;

    /** connect pin with specified index to the given net. 
     *  returns true if succesful.
    */
    virtual bool connect(ssize_t pinIndex, Net *net) override;

    /** connect pin with specified name to the given net. 
     *  returns true if succesful.
    */    
    virtual bool connect(const std::string &pinName, Net *net) override;

    /** returns the net connected to a pin with a given index.
     *  if the pin does not exist, it return nullptr.
    */
    Net* getConnectedNet(ssize_t pinIndex) override;

    /** returns the net connected to a pin with a given index.
     *  if the pin does not exist, it return nullptr.
    */
    const Net* getConnectedNet(ssize_t pinIndex) const override;

    void setPinIOType(IOType iotype)
    {
        m_pinInfo.m_iotype = iotype;
    }

protected:
    PinInfo m_pinInfo;
    Net*    m_connectedNet;  ///< connection from pin to net
};

};
