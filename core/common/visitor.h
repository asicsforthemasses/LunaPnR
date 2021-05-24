#pragma once


namespace ChipDB
{

// pre-declarations
class AbstractInstance;
class CellInstance;
class ModuleInstance;
class PinInfo;
class Netlist;

class Visitor
{
public:
    virtual void visit(AbstractInstance *instance) = 0;
    virtual void visit(CellInstance *instance) = 0;
    virtual void visit(ModuleInstance *instance) = 0;
    virtual void visit(PinInfo *pin) = 0;
    virtual void visit(Netlist *nl) = 0;
};

class ConstVisitor
{
public:
    virtual void visit(const AbstractInstance *instance) = 0;
    virtual void visit(const CellInstance *instance) = 0;
    virtual void visit(const ModuleInstance *instance) = 0;
    virtual void visit(const PinInfo *pin) = 0;
    virtual void visit(const Netlist *nl) = 0;
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