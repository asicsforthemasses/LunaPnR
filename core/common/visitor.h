#pragma once


namespace ChipDB
{

// pre-declarations
class Cell;
class Module;
class Instance;
class PinInfo;
class Netlist;
class Net;

class Visitor
{
public:
    virtual void visit(Instance *instance) = 0;
    virtual void visit(Cell    *cell) = 0;
    virtual void visit(Module  *module) = 0;
    virtual void visit(PinInfo *pin) = 0;
    virtual void visit(Netlist *nl) = 0;
    virtual void visit(Net *net) = 0;
};

class ConstVisitor
{
public:
    virtual void visit(const Instance *instance) = 0;
    virtual void visit(const Cell    *cell) = 0;
    virtual void visit(const Module  *module) = 0;
    virtual void visit(const PinInfo *pin) = 0;
    virtual void visit(const Netlist *nl) = 0;
    virtual void visit(const Net *net) = 0;
};


/** IMPLEMENT_ACCEPT creates 'virtual void accept(..)' functions for Visitor and ConstVisitor classes */
#define IMPLEMENT_ACCEPT \
    virtual void accept(Visitor *visitor) { visitor->visit(this); }; \
    virtual void accept(ConstVisitor *visitor) const { visitor->visit(this); };

/** IMPLEMENT_ACCEPT_VERRIDE creates 'virtual void accept(..) override' functions for Visitor and ConstVisitor classes */
#define IMPLEMENT_ACCEPT_OVERRIDE \
    virtual void accept(Visitor *visitor) override { visitor->visit(this); }; \
    virtual void accept(ConstVisitor *visitor) const override { visitor->visit(this); };

};