#include "fillerhandler.h"
#include <algorithm>
#include "common/logging.h"

using namespace LunaCore;


FillerHandler::FillerHandler(const ChipDB::CellLib &cellLib)
{
}

void FillerHandler::identifyFillers(const ChipDB::CellLib &cellLib)
{
    for(auto const& cell : cellLib)
    {
        if ((cell->m_class == ChipDB::CellClass::CORE) &&
           (cell->m_subclass == ChipDB::CellSubclass::SPACER))
        {
            m_fillers.emplace_back();
            m_fillers.back().m_name     = cell->name();
            m_fillers.back().m_cellKey  = cell.key();
            m_fillers.back().m_size     = cell->m_size;
        }
    }

    sortFillers();

    // report fillers
    Logging::doLog(Logging::LogType::VERBOSE, "FillerHandler identified the following filler cells:\n");
    for(auto const& filler : m_fillers)
    {
        Logging::doLog(Logging::LogType::VERBOSE,
            "Filler %s width %d nm\n", filler.m_name.c_str(), filler.m_size.m_x);
    }
}

void FillerHandler::sortFillers()
{
    // sort fillers in width, largest first
    std::sort(m_fillers.begin(), m_fillers.end(), 
        [this](const FillerInfo &filler1, const FillerInfo &filler2)
        {
            return filler1.m_size.m_x > filler2.m_size.m_x;
        }
    );
}

bool FillerHandler::addFillerByName(const ChipDB::CellLib &cellLib, const std::string &fillerName)
{
    // check if the filler isn't already in the list
    if (isFillerAlreadyInList(fillerName))
    {
        return true;
    }    

    auto cellPtr = cellLib.lookupCell(fillerName);

    if (!cellPtr.isValid()) return false;

    m_fillers.emplace_back();
    m_fillers.back().m_name = cellPtr->name();
    m_fillers.back().m_cellKey = cellPtr.key();
    m_fillers.back().m_size = cellPtr->m_size;

    sortFillers();

    return true;
}

bool FillerHandler::isFillerAlreadyInList(const std::string &name) const
{
    auto iter = std::find_if(m_fillers.begin(), m_fillers.end(), 
        [&name](const auto &fillerInfo)
        {
            return fillerInfo.m_name == name;
        }
    );

    return iter != m_fillers.end();
}

bool FillerHandler::placeFillers(ChipDB::Design &design, const ChipDB::Region &region, ChipDB::Netlist &netlist)
{
    if (m_fillers.size() == 0)
    {
        Logging::doLog(Logging::LogType::ERROR, "FillerHandler has no filler cells!\n");
        return false;
    }

    for(auto ins : netlist.m_instances)
    {
        ins->m_flags = 0;
    }

    // FIXME: let the row have a list of cells
    for(auto const& row : region.m_rows)
    {
        std::vector<ChipDB::ObjectKey> cellsInRow;

        for(auto ins : netlist.m_instances)
        {
            if (!ins->isCell()) continue;       // only cells can be places in a row
            if (ins->m_flags != 0) continue;    // instance already found previously

            if (row.m_rect.contains(ins->getCenter()))
            {
                cellsInRow.push_back(ins.key());
            }
        }

        // sort cells in x direction
        std::sort(cellsInRow.begin(), cellsInRow.end(), 
            [&cellsInRow, &netlist](auto cellKey1, auto cellKey2)
            {
                const auto& ins1 = netlist.m_instances.at(cellKey1);
                const auto& ins2 = netlist.m_instances.at(cellKey2);
                return ins1->m_pos.m_x < ins2->m_pos.m_x;
            }
        );

        bool debug = true;
        if (debug) Logging::doLog(Logging::LogType::INFO, "  Row: \n");

        ChipDB::CoordType leftPos = row.m_rect.m_ll.m_x;
        for(auto cellKey : cellsInRow)
        {
            const auto& ins = netlist.m_instances.at(cellKey);
            if (debug) Logging::doLog(Logging::LogType::INFO, "    ins: %s at %d,%d\n", ins->name().c_str(), ins->m_pos.m_x, ins->m_pos.m_y);
            if (ins->m_pos.m_x > leftPos)
            {
                ChipDB::Coord64 lowerLeftPos{leftPos, row.m_rect.m_ll.m_y};
                auto gapWidth = ins->m_pos.m_x - leftPos;
                if (!fillSpaceWithFillers(design, netlist, lowerLeftPos, gapWidth))
                {
                    return false;
                }
                ins->m_flags = 1;   // mark visited
                leftPos += gapWidth + ins->instanceSize().m_x;
            }
            else
            {
                leftPos += ins->instanceSize().m_x;
            }            
        }

        // see if there is space left at the end of the row
        // if so .. fill it.
        if (leftPos < row.m_rect.m_ur.m_x)
        {
            ChipDB::Coord64 lowerLeftPos{leftPos, row.m_rect.m_ll.m_y};
            auto gapWidth = row.m_rect.m_ur.m_x - leftPos;
            if (!fillSpaceWithFillers(design, netlist, lowerLeftPos, gapWidth))
            {
                return false;
            }            
        }
    }

    return true;
}

bool FillerHandler::fillSpaceWithFillers(const ChipDB::Design &design, 
    ChipDB::Netlist &netlist, 
    const ChipDB::Coord64 &lowerLeftPos, 
    const ChipDB::CoordType width)
{
    auto currentPos = lowerLeftPos;
    auto spaceRemaining = width;
    while(spaceRemaining > 0)
    {
        // find largest filler that is smaller or equal to space remaining
        std::size_t fillerIdx = 0;
        for(auto const& filler : m_fillers)
        {
            if (filler.m_size.m_x <= spaceRemaining) break;
            fillerIdx++;
        }

        if (fillerIdx < m_fillers.size())
        {
            // found filler

            auto cellPtr = design.m_cellLib->lookupCell(m_fillers.at(fillerIdx).m_cellKey);

            std::stringstream ss;
            ss << "_filler_" << m_fillerID++;

            auto fillerInstance = std::make_shared<ChipDB::Instance>(ss.str(), 
                ChipDB::InstanceType::CELL,
                cellPtr);

            fillerInstance->m_pos = currentPos;
            fillerInstance->m_placementInfo = ChipDB::PlacementInfo::PLACED;

            netlist.m_instances.add(fillerInstance);
            const auto x_delta = fillerInstance->instanceSize().m_x;
            spaceRemaining -= x_delta;
            currentPos.m_x += x_delta;
        }
        else
        {
            // no fillers to plug this remaining size :-/
            Logging::doLog(Logging::LogType::WARNING,"FillerHandler cannot find a filler to fill width %d\n", spaceRemaining);
            return true;
        }
    }

    return true;
}
