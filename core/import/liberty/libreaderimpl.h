/*
    Copyright (c) 2020, Niels Moseley <n.a.moseley@moseleyinstruments.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
    
*/

#pragma once

#include <stack>
#include "common/dbtypes.h"
#include "celllib/pin.h"
#include "design/design.h"
#include "libparser.h"

namespace ChipDB
{
    class CellLib;
    class TechLib;
    class PinInfo;
    class Cell;
    class Design;
}

namespace ChipDB::Liberty
{

class ReaderImpl : public Parser
{
public:
    ReaderImpl(Design *design);

    /** Called for groups without a name/parameter */
    virtual void onGroup(const std::string &group) override;

    /** called for groups with a name/parameter, such as library(libname).
     *  group = "library", name = libname.
    */
    virtual void onGroup(const std::string &group, const std::string &name) override;

    /** Called for a simple attribute. name : value */
    virtual void onSimpleAttribute(const std::string &name, const std::string &value) override;

    /** Called for complex attributes, such as timing tables */
    virtual void onComplexAttribute(const std::string &attrname, const std::vector<std::string> &list) override;
    
    /** Called at the end of every group */
    virtual void onEndGroup() override;

    /** Called at the end of parsing */
    virtual void onEndParse() override;

protected:
    Design  *m_design;
    
    std::shared_ptr<Cell>       m_curCell;
    std::shared_ptr<PinInfo>    m_curPin;

    enum groupType : int32_t
    {
        GT_NONE = 0,
        GT_LIBRARY,
        GT_CELL,
        GT_PIN
    };

    void parseLeakagePowerUnit(const std::string &value);
    void parseCapacitanceUnit(const std::string &value, const std::string &unit);

    std::stack<groupType> m_groupStack;
    double m_leakagePowerUnit;
    double m_capacitanceUnit;
};

};  // namespace

