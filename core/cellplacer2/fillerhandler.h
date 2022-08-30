#pragma once

#include <string>
#include <vector>
#include "common/dbtypes.h"
#include "celllib/celllib.h"
#include "design/design.h"

namespace LunaCore
{

class FillerHandler
{
public:
    FillerHandler(const ChipDB::CellLib &cellLib);

    std::size_t numberOfFillers() const
    {
        return m_fillers.size();
    }

    bool addFillerByName(const ChipDB::CellLib &cellLib, const std::string &fillerName);

    bool placeFillers(ChipDB::Design &design, const ChipDB::Region &region, ChipDB::Netlist &netlist);

protected:

    void identifyFillers(const ChipDB::CellLib &cellLib);

    void sortFillers();

    [[nodiscard]] bool isFillerAlreadyInList(const std::string &name) const;

    [[nodiscard]] bool fillSpaceWithFillers(const ChipDB::Design &design, 
        ChipDB::Netlist &netlist, 
        const ChipDB::Coord64 &lowerLeftPos, 
        const ChipDB::CoordType width);

    struct FillerInfo
    {
        std::string         m_name;     ///< name of filler cell
        ChipDB::ObjectKey   m_cellKey;  ///< filler cell key in celllib
        ChipDB::Coord64     m_size;     ///< size of filler cell
    };

    std::vector<FillerInfo> m_fillers;      ///< fillers sorted by width
    std::size_t m_fillerID{0};
};

};