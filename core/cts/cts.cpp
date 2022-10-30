#include <cassert>
#include "common/logging.h"
#include "cts.h"
//#include "cts_private.h"

using namespace LunaCore::CTS;

void MeanAndMedianCTS::recursiveSubdivision(const ChipDB::Netlist &netlist, CTSNodeList &nodes,
    SegmentList &segments, SegmentIndex topSegIndex, int level)
{
    if (nodes.size() == 1) 
    {        
        return;
    }

    if (nodes.size() == 0) 
    {
        return;
    }

    auto LeftRight = nodes.split(netlist, CTSNodeList::Axis::X);
    auto &left  = LeftRight.first;
    auto &right = LeftRight.second;

    assert(left.size() >= 1);
    assert(right.size() >= 1);

    auto center     = nodes.mean(netlist);
    auto leftCoord  = left.mean(netlist);
    auto rightCoord = right.mean(netlist);

    // we don't need the incoming list anymore
    nodes.clear();

    // route from center to leftCoord
    auto leftSegIndex = segments.createSegment(center, leftCoord, topSegIndex, level + 1);

    // route from center to rightCoord
    auto rightSegIndex = segments.createSegment(center, rightCoord, topSegIndex, level + 1);

    segments.at(topSegIndex).addChild(leftSegIndex);
    segments.at(topSegIndex).addChild(rightSegIndex);

    auto BLTL = left.split(netlist, CTSNodeList::Axis::Y);  // bottom and top left
    auto BRTR = right.split(netlist, CTSNodeList::Axis::Y); // bottom and top right
    auto &bl  = BLTL.first;
    auto &tl  = BLTL.second;
    auto &br  = BRTR.first;
    auto &tr  = BRTR.second;

    auto blCoord = bl.mean(netlist);
    auto tlCoord = tl.mean(netlist);
    auto brCoord = br.mean(netlist);
    auto trCoord = tr.mean(netlist);

    // route from leftCoord  -> blCoord, leftCoord  -> tlCoord
    SegmentIndex bl_top = -1;
    if (!bl.empty()) 
    {
        if (bl.size() == 1)
            bl_top = segments.createSegment(leftCoord, blCoord, leftSegIndex, bl.front(), level + 2);
        else
            bl_top = segments.createSegment(leftCoord, blCoord, leftSegIndex, level + 2);

        segments.at(leftSegIndex).addChild(bl_top);
    }

    SegmentIndex tl_top = -1;
    if (!tl.empty()) 
    {
        if (tl.size() == 1)
            tl_top = segments.createSegment(leftCoord, tlCoord, leftSegIndex, tl.front(), level + 2);
        else
            tl_top = segments.createSegment(leftCoord, tlCoord, leftSegIndex, level + 2);

        segments.at(leftSegIndex).addChild(tl_top);
    }
    
    // route from rightCoord -> brCoord, rightCoord -> trCoord
    SegmentIndex br_top = -1;
    if (!br.empty()) 
    {
        if (br.size() == 1)
            br_top = segments.createSegment(rightCoord, brCoord, rightSegIndex, br.front(), level + 2);
        else
            br_top = segments.createSegment(rightCoord, brCoord, rightSegIndex, level + 2);

        segments.at(rightSegIndex).addChild(br_top);
    }
    
    SegmentIndex tr_top = -1;
    if (!tr.empty()) 
    {
        if (tr.size() == 1)
            tr_top = segments.createSegment(rightCoord, trCoord, rightSegIndex, tr.front(), level + 2);
        else
            tr_top = segments.createSegment(rightCoord, trCoord, rightSegIndex, level + 2);

        segments.at(rightSegIndex).addChild(tr_top);
    }
    
    // we don't need the intermediate lists anymore
    left.clear();
    right.clear();

    recursiveSubdivision(netlist, bl, segments, bl_top, level+2);
    recursiveSubdivision(netlist, tl, segments, tl_top, level+2);
    recursiveSubdivision(netlist, br, segments, br_top, level+2);
    recursiveSubdivision(netlist, tr, segments, tr_top, level+2);
}

std::optional<MeanAndMedianCTS::SegmentList> MeanAndMedianCTS::generateTree
    (const std::string &clockNetName, ChipDB::Netlist &netlist)
{
    // create list of nodes
    CTSNodeList topList;

    auto clockNet = netlist.lookupNet(clockNetName);
    if (!clockNet.isValid())
    {
        Logging::doLog(Logging::LogType::ERROR, "CTS cannot find the specified clock net %s\n", clockNetName.c_str());
        return std::nullopt;
    }

    CTSNodeList clkNodes;
    clkNodes.reserve(clockNet->numberOfConnections());

    std::size_t driverCount = 0;
    ChipDB::Coord64     driverPos;
    ChipDB::ObjectKey   driverPinKey{ChipDB::ObjectNotFound};

    for(auto conn : *clockNet)
    {
        auto ins = netlist.lookupInstance(conn.m_instanceKey);
        if (!ins)
        {
            Logging::doLog(Logging::LogType::ERROR, "CTS cannot find instance with key %d\n", conn.m_instanceKey);
            return std::nullopt;
        }

        if (!ins->isPlaced())
        {
            Logging::doLog(Logging::LogType::ERROR, "CTS: instance %s (%s) has not been placed - aborting!\n", ins->name().c_str(), ins->getArchetypeName().c_str());
            return std::nullopt;
        }

        auto pin = ins->getPin(conn.m_pinKey);
        if (!pin.isValid())
        {
            Logging::doLog(Logging::LogType::ERROR, "CTS: pin with key %ld on instance %s is invalid - aborting!\n", conn.m_pinKey, ins->name().c_str());
            return std::nullopt;            
        }

        if (!pin.m_pinInfo)
        {
            Logging::doLog(Logging::LogType::ERROR, "CTS: pin info with key %ld on instance %s is invalid - aborting!\n", conn.m_pinKey, ins->name().c_str());
            return std::nullopt;            
        }

        if (pin.m_pinInfo->isOutput())
        {
            driverCount++;
            driverPos = ins->m_pos;
            driverPinKey = conn.m_pinKey;
        }
        else
        {
            clkNodes.push_back(conn.m_instanceKey, conn.m_pinKey, pin.m_pinInfo->m_cap);
        }
    }

    if (driverCount == 0)
    {
        Logging::doLog(Logging::LogType::ERROR, "CTS: could not find a driver for clock net - aborting!\n");
        return std::nullopt;                    
    }

    if (driverCount > 1)
    {
        Logging::doLog(Logging::LogType::ERROR, "CTS: clock net has more than one driver - aborting!\n");
        return std::nullopt;                    
    }

    if (clkNodes.size() == 0)
    {
        Logging::doLog(Logging::LogType::ERROR, "CTS: did not find any instances - aborting!\n");
        return std::nullopt;
    }

    SegmentList segments;
    segments.createSegment(driverPos, {0,0}, -1, 0);   // add the driver node
    recursiveSubdivision(netlist, clkNodes, segments);

    if (segments.size() > 1)
    {
        // connect the driver to the
        // rest of the network
        segments.at(0).m_end = segments.at(1).m_start;
    }

    // add a segment from the net driver to the first destination
    return std::move(segments);
}

void CTSNodeList::insertAtEnd(ContainerType::const_iterator s, ContainerType::const_iterator e)
{
    m_nodes.insert(m_nodes.end(), s,e);
}

/** split the node list into two equal parts, based on the median location along the specified axis */
std::pair<CTSNodeList, CTSNodeList> CTSNodeList::split(const ChipDB::Netlist &netlist, Axis axis)
{
    CTSNodeList leftList;   // or bottom, depending on axis
    CTSNodeList rightList;  // or top

    auto expectedNodes = (m_nodes.size() + 1) / 2;
    leftList.reserve(expectedNodes);
    rightList.reserve(expectedNodes);

    sortAlongAxis(netlist, axis);
    leftList.insertAtEnd(m_nodes.begin(), m_nodes.begin() + expectedNodes);
    rightList.insertAtEnd(m_nodes.begin() + expectedNodes, m_nodes.end());

    auto const diff = std::abs(static_cast<int>(leftList.size()) - static_cast<int>(rightList.size()));
    assert(diff < 2);

    return {leftList, rightList};
};

void CTSNodeList::sortAlongAxis(const ChipDB::Netlist &netlist, const Axis axis)
{
    auto const& instances = netlist.m_instances;
    if (axis == Axis::X)
    {
        std::sort(m_nodes.begin(), m_nodes.end(), 
            [&instances](const CTSNode &node1, const CTSNode &node2)
            {
                return instances[node1.m_insKey]->m_pos.m_x < instances[node2.m_insKey]->m_pos.m_x;
            }
        );

        if (m_nodes.size() >= 2)
        {
            assert(instances[m_nodes.at(0).m_insKey]->m_pos.m_x <= instances[m_nodes.at(1).m_insKey]->m_pos.m_x);
        }
    }    
    else
    {
        std::sort(m_nodes.begin(), m_nodes.end(), 
            [&instances](const CTSNode &node1, const CTSNode &node2)
            {
                return instances[node1.m_insKey]->m_pos.m_y < instances[node2.m_insKey]->m_pos.m_y;
            }
        );        

        if (m_nodes.size() >= 2)
        {
            assert(instances[m_nodes.at(0).m_insKey]->m_pos.m_y <= instances[m_nodes.at(1).m_insKey]->m_pos.m_y);
        }        
    }
}


ChipDB::Coord64 CTSNodeList::mean(const ChipDB::Netlist &netlist) const
{
    // prevent a division by zero later on..
    if (m_nodes.size() == 0) return ChipDB::Coord64{0,0};

    float total_x{0};
    float total_y{0};
    auto const& instances = netlist.m_instances;
    for(auto const& node : m_nodes)
    {   
        auto const &ins = instances[node.m_insKey];
        assert(ins);
        total_x += static_cast<float>(ins->m_pos.m_x);
        total_y += static_cast<float>(ins->m_pos.m_y);
    }

    total_x /= static_cast<float>(m_nodes.size());
    total_y /= static_cast<float>(m_nodes.size());

    return {static_cast<ChipDB::CoordType>(total_x), 
        static_cast<ChipDB::CoordType>(total_y)};
}

float MeanAndMedianCTS::calcSegmentLoadCapacitance(SegmentIndex index, const SegmentList &segments, 
    const ChipDB::Netlist &netlist) const
{
    auto const& seg = segments.at(index);
    
    // if the segment has a cell, return the pin capacitance
    if (seg.hasCell())
    {
        return seg.m_cell.m_capacitance; 
    }

    float totalCap = 0.0;
    for(auto childIndex : seg.m_children)
    {
        totalCap += calcSegmentLoadCapacitance(childIndex, segments, netlist);
    }
    return totalCap;
}

MeanAndMedianCTS::BufferResult MeanAndMedianCTS::insertBuffers(SegmentList &segments, 
        SegmentIndex segIndex, 
        ChipDB::Netlist &netlist,
        const CTSInfo &ctsInfo)
{
    BufferResult bresult;

    // depth first, bottom up buffer insertion
    auto const& seg = segments.at(segIndex);

    // if this segments has a cell, it is a leaf / terminal
    // add it to the sink list.
    if (seg.hasCell())
    {
        bresult.m_list.push_back({seg.m_cell.m_insKey, seg.m_cell.m_pinKey});
        bresult.m_totalCapacitance = seg.m_cell.m_capacitance;
        return bresult;
    }

    for(auto const& child : seg.m_children)
    {
        auto subtreeResult = insertBuffers(segments, child, netlist, ctsInfo);
        
        bresult.m_totalCapacitance += subtreeResult.m_totalCapacitance;

        // add the sinks to the current list
        bresult.m_list.insert(bresult.m_list.end(), subtreeResult.m_list.begin(), subtreeResult.m_list.end());
    }

    // if we exceed the max capacitance,
    // we should insert a buffer here.
    if (bresult.m_totalCapacitance >= ctsInfo.m_maxCap)
    {
        std::stringstream ss;
        ss << "ctsbuffer_L" << seg.m_level << "_" << netlist.createUniqueID();
        auto bufferIns = std::make_shared<ChipDB::Instance>(ss.str(), 
            ChipDB::InstanceType::CELL, ctsInfo.m_bufferCell);
        auto bufInsKeyPtrOpt = netlist.m_instances.add(bufferIns);
        if (!bufInsKeyPtrOpt)
        {
            throw std::runtime_error("cts cannot create buffer");
        }
        auto bufInsKeyPtr = bufInsKeyPtrOpt.value();

        if (!bufInsKeyPtr.isValid())
        {
            throw std::runtime_error("cts cannot create buffer");
        }
        
        // place the buffer at the segment end position.
        bufferIns->m_pos = seg.m_end;
        bufferIns->m_placementInfo = ChipDB::PlacementInfo::PLACED;

        // connect all sinks to this buffer via a new net
        ss.str("");
        ss << "ctsnet_" << netlist.createUniqueID();        
        auto bufNet = netlist.createNet(ss.str());

        Logging::doLog(Logging::LogType::VERBOSE, "  created net %s key=%ld\n", bufNet->name().c_str(), bufNet.key());

        if (!bufNet.isValid())
        {
            std::stringstream sserr;
            sserr << "error creating buffer net " << ss.str() << "\n";
            throw std::runtime_error(sserr.str());
        }

        bufNet->setClockNet(true);

        auto origClkNet = netlist.lookupNet(ctsInfo.m_clkNetKey);
        for(auto const& sink : bresult.m_list)
        {
            // connect net to instance
            bufNet->addConnection(sink.m_instanceKey, sink.m_pinKey);

            // remove instance from original clock net
            origClkNet->removeConnection(sink.m_instanceKey, sink.m_pinKey);

            // connect instance to net
            auto connInsKeyPtr = netlist.lookupInstance(sink.m_instanceKey);
            connInsKeyPtr->setPinNet(sink.m_pinKey, bufNet.key());
        }

        // connect net to buffer
        bufNet->addConnection(bufInsKeyPtr.key(), ctsInfo.m_outputPinKey);

        // connect buffer to net
        if (!bufferIns->setPinNet(ctsInfo.m_outputPinKey, bufNet.key()))
        {
            std::stringstream sserr;
            sserr << "error setting output pin of buffer instance to net key " << bufNet.key() << "\n";
            throw std::runtime_error(sserr.str());
        }

        // clear the sink list and put the input to the buffer in the list
        bresult.m_list.clear();
        bresult.m_list.push_back({bufInsKeyPtr.key(), ctsInfo.m_inputPinKey});
        bresult.m_totalCapacitance = ctsInfo.m_pinCapacitance;
    }

    return bresult;
}
