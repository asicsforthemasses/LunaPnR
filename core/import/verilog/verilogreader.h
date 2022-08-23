// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <iostream>
#include "veriloglexer.h"
#include "verilogparser.h"
#include "celllib/celllib.h"
#include "celllib/pin.h"
#include "design/design.h"

/** Namespace for the Verilog importers and exporters */
namespace ChipDB::Verilog
{

/// \cond HIDE_INTERNALS

class ReaderImpl : public Parser
{
public:
    ReaderImpl(Design &design);

    /** callback for each module encountered in the netlist */
    virtual void onModule(const std::string &modName,
        const std::vector<std::string> &ports) override;

    /** callback for each module instance in the netlist */
    virtual void onInstance(const std::string &modname, const std::string &insname) override;

    /** callback for each module instance in the netlist */
    virtual void onInstancePort(uint32_t portIndex, const std::string &netname) override;

    /** callback for each module instance in the netlist */
    virtual void onInstanceNamedPort(const std::string &portname, const std::string &netname) override;

    /** callback for each wire in the netlist with optional [start:stop] range */
    virtual void onWire(const std::string &netname, uint32_t start, uint32_t stop) override;

    /** callback for each wire in the netlist */
    virtual void onWire(const std::string &netname) override;

    /** callback for each module input port with [start:stop] range */
    virtual void onInput(const std::string &netname, uint32_t start, uint32_t stop) override;

    /** callback for each module single wire input port */
    virtual void onInput(const std::string &netname) override;

    /** callback for each module output port with [start:stop] range */
    virtual void onOutput(const std::string &netname, uint32_t start, uint32_t stop) override;

    /** callback for each module single wire output port */
    virtual void onOutput(const std::string &netname) override;

    /** callback for each assignment statement */
    virtual void onAssign(const std::string &left, const std::string &right) override;

    /** callback for each attribute (belonging to the next statement) */
    virtual void onAttribute(const std::string &attr) override {};

protected:
    void throwOnModuleIsNullptr();
    void throwOnCurInstanceIsNullptr();

    Design                      &m_design;
    std::shared_ptr<Module>     m_currentModule;
    KeyObjPair<InstanceBase>    m_currentInsKeyObjPair;
};

/// \endcond

/** read a Verilog netlist into a Design */
class Reader
{
public:
    /** read a Verilog netlist into a Design */
    static bool load(Design &design, std::istream &is);
};

};
