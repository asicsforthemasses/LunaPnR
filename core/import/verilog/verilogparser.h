
#pragma once

#include <stdint.h>
#include <vector>
#include "veriloglexer.h"

namespace ChipDB::Verilog
{

/// \cond HIDE_INTERNALS

class Parser
{
public:
    Parser() : m_idx(0), m_tokens(nullptr) {}

    bool execute(const std::vector<Lexer::token> &tokens);

    /** callback for each module encountered in the netlist */
    virtual void onModule(const std::string &modName,
        const std::vector<std::string> &ports) {};

    /** callback for each module instance in the netlist */
    virtual void onInstance(const std::string &modname, const std::string &insname) {};

    /** callback for each module instance in the netlist */
    virtual void onInstancePort(uint32_t portIndex, const std::string &netname) {};

    /** callback for each module instance in the netlist */
    virtual void onInstanceNamedPort(const std::string &portname, const std::string &netname) {};

    /** callback for each wire in the netlist with [start:stop] range */
    virtual void onWire(const std::string &netname, uint32_t start, uint32_t stop) {};

    /** callback for each wire in the netlist that does not have a range */
    virtual void onWire(const std::string &netname) {};

    /** callback for each module input port with optional [start:stop] range */
    virtual void onInput(const std::string &netname, uint32_t start, uint32_t stop) {};

    /** callback for each module single wire input port */
    virtual void onInput(const std::string &netname) {};

    /** callback for each module output port with optional [start:stop] range */
    virtual void onOutput(const std::string &netname, uint32_t start, uint32_t stop) {};

    /** callback for each module single wire output port */
    virtual void onOutput(const std::string &netname) {};

    /** callback for each assignment statement */
    virtual void onAssign(const std::string &left, const std::string &right) {};

    /** callback for each attribute (belonging to the next statement) */
    virtual void onAttribute(const std::string &attr) {};

protected:
    // grammar rules
    bool module();
    bool moduleItems();
    bool pInput();
    bool pOutput();
    bool pWire();
    bool pAssign();
    bool pInstance();
    bool pInstanceOrderedPortList();
    bool pInstanceNamedPortList();

    Lexer::token previous() const;
    Lexer::token peek() const;
    void advance();
    bool match(Lexer::tokenId tok);
    bool atEnd() const;

    void error(const std::string &error);

    uint32_t m_idx;
    const std::vector<Lexer::token> *m_tokens;
};

/// \endcond

}; // namespace

