// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <fstream>
#include <chrono>

#include "version.h"
#include "common/logging.h"
#include "database/database.h"
#include "spefwriter.h"

static std::string quoted(const std::string &str)
{
    return "\"" + str + "\"";
}

static std::string escapeSPEFString(const std::string &str)
{
    std::string result;
    result.reserve(str.size());
    for(auto c : str)
    {
        if (c == '[')
        {
            result += '\\';
        }
        else if (c == ']')
        {
            result += '\\';
        }
        result += c;
    }
    return result;
}

bool LunaCore::SPEF::write(std::ostream &os, const std::shared_ptr<ChipDB::Module> module)
{
    if (!os.good())
    {
        Logging::doLog(Logging::LogType::ERROR, "SPEF writer: output stream is invalid!\n");
        return false;
    }

    if (!module)
    {
        Logging::doLog(Logging::LogType::ERROR, "SPEF writer: module is nullptr!\n");
        return false;
    }

    auto currentTime = std::chrono::system_clock::now();
    auto currentTime_t = std::chrono::system_clock::to_time_t(currentTime);
    auto timeString = std::string(std::ctime(&currentTime_t));

    // remove end of lines from the time string
    timeString.erase(std::remove(timeString.begin(), timeString.end(), '\n'), timeString.cend());

    os << "*SPEF        " << quoted("IEEE 1481-2009") << "\n";
    os << "*DESIGN      " << quoted(module->name()) << "\n";
    os << "*DATE        " << quoted(timeString) << "\n";
    os << "*VENDOR      " << quoted(LUNAVERSIONSTRING) << "\n";
    os << "*PROGRAM     " << quoted(LUNAVERSIONSTRING) << "\n";
    os << "*VERSION     " << quoted("1.1.0") << "\n";
    //os << "*DESIGN_FLOW " << quoted("EXTERNAL_LOADS") << "\n";
    os << "*DESIGN_FLOW " << quoted("") << "\n";
    os << "*DIVIDER /\n";
    os << "*DELIMITER :\n";
    os << "*BUS_DELIMITER [ ]\n";
    os << "*T_UNIT 1 NS\n";
    os << "*C_UNIT 1 PF\n";
    os << "*R_UNIT 1 OHM\n";
    os << "*L_UNIT 1 HENRY\n";
    os << "\n";

    const float FaradToPicofarad = 1.0e12f;

    // FIXME: do a much better job
    // assume all pins on instances are in the center..

    auto const &nets = module->m_netlist->m_nets;
    auto const &instances = module->m_netlist->m_instances;

    os << "*PORTS\n";
    for(auto const portPins : module->m_pins)
    {
        os << portPins->name() << " ";
        switch(portPins->m_iotype)
        {
        case ChipDB::IOType::INPUT:
            os << "I\n";
            break;
        case ChipDB::IOType::OUTPUT:
            os << "O\n";
            break;
        case ChipDB::IOType::OUTPUT_TRI:
        case ChipDB::IOType::IO:
        case ChipDB::IOType::ANALOG:
            os << "B\n";
            break;
        case ChipDB::IOType::POWER:
            break;
        case ChipDB::IOType::GROUND:
            break;
        case ChipDB::IOType::UNKNOWN:
            break;
        }
    }

    os << "\n";

    std::size_t resCounter = 0;

    for(auto const netKeyPair : nets)
    {
        float totalCap = 0.0f;
        std::stringstream osDNETBody;

        // write the connections
        // and while we are iteration over the nets
        // collect the resistive parasitics too..
        osDNETBody << "*CONN\n";

        bool portNet = netKeyPair->m_isPortNet;
        for(auto const& conn : *netKeyPair)
        {
            auto const ins = instances.at(conn.m_instanceKey);
            auto const pin = ins->getPin(conn.m_pinKey);

            if ((portNet) && (ins->getArchetypeName() == std::string("__PIN")))
            {
                osDNETBody << "*P " << ins->name()<< " ";
                // note direction of port pins is reversed!
                if (pin.m_pinInfo->isInput())
                {
                    osDNETBody << "O *C " << ins->m_pos.m_x/1000.0 << " " << ins->m_pos.m_y/1000.0 << "\n";
                }
                else
                {
                    osDNETBody << "I *C " << ins->m_pos.m_x/1000.0 << " " << ins->m_pos.m_y/1000.0 << "\n";
                }
            }
            else
            {
                osDNETBody << "*I " << ins->name() << ":" << pin.name() << " ";
                if (pin.m_pinInfo->isInput())
                {
                    osDNETBody << "I *C " << ins->m_pos.m_x/1000.0 << " " << ins->m_pos.m_y/1000.0 << " *L " << pin.m_pinInfo->m_cap*FaradToPicofarad << "\n";
                    totalCap += pin.m_pinInfo->m_cap*FaradToPicofarad;
                }
                else
                {
                    osDNETBody << "O *C " << ins->m_pos.m_x/1000.0 << " " << ins->m_pos.m_y/1000.0 << " ";
                    osDNETBody << "*D " << ins->getArchetypeName() << "\n";
                }
            }
        }

        // write DNET now we know the total capacitance...
        os << "*D_NET " << escapeSPEFString(netKeyPair->name()) << " " << totalCap << "\n";
        os << osDNETBody.str();

        // create resistor section
        // we simply take the first connection and calculate the
        // distance between all the other connections (manhattan)
        // based on this length, we connect it all together
        // FIXME: make a better network based on the PRIM/GlobalRouter segment network!

        auto connIter = netKeyPair->begin();
        if (connIter != netKeyPair->end())
        {
            os << "*RES\n";
            auto const srcIns = instances.at(connIter->m_instanceKey);
            auto const srcPin = srcIns->getPin(connIter->m_pinKey);

            if (!srcPin.isValid())
            {
                Logging::doLog(Logging::LogType::ERROR, "Invalid pin %ld key on instance %s of type %s\n",
                    connIter->m_pinKey, srcIns->name().c_str(), srcIns->getArchetypeName().c_str());
            }

            if (!srcIns)
            {
                Logging::doLog(Logging::LogType::ERROR, "Invalid instance key: %ld\n", connIter->m_instanceKey);
            }

            std::string srcName;
            if (srcIns->getArchetypeName() == std::string("__PIN"))
            {
                srcName = escapeSPEFString(srcIns->name());
            }
            else
            {
                srcName = srcIns->name() + ":" + srcPin.name();
            }

            connIter++;
            while(connIter != netKeyPair->end())
            {
                auto const dstIns = instances.at(connIter->m_instanceKey);
                auto const dstPin = dstIns->getPin(connIter->m_pinKey);

                if (!dstPin.isValid())
                {
                    Logging::doLog(Logging::LogType::ERROR, "Invalid pin %ld key on instance %s of type %s\n",
                        connIter->m_pinKey, dstIns->name().c_str(), dstIns->getArchetypeName().c_str());
                }

                if (!dstIns)
                {
                    Logging::doLog(Logging::LogType::ERROR, "Invalid instance key: %ld\n", connIter->m_instanceKey);
                }

                std::string dstName;
                if (dstIns->getArchetypeName() == std::string("__PIN"))
                {
                    dstName = escapeSPEFString(dstIns->name());
                }
                else
                {
                    dstName = dstIns->name() + ":" + dstPin.name();
                }

                // we calculate parasitics here based on the manahattan distance
                // length is in nm
                auto d = srcIns->m_pos.manhattanDistance(dstIns->m_pos);

                //FIXME:
                // for now we take the OSU/TSMC180 value of 0.08 ohm per square
                // each wire is 300nm wide, so a length of 300 nm is 0.08 ohms
                const float RperSqInOhms = 0.08f;
                const float trackWidth   = 300.0f;  // in nm.

                const float area_um2 = (trackWidth/1000.0f) * (d/1000.0f);
                const float cap_pf = (3.8e-17 * area_um2) / 1.0e-12;

                os << "// capacitance in pf: " << cap_pf << "\n";
                os << resCounter << " " << srcName << " " << dstName << " ";
                os << (RperSqInOhms*static_cast<float>(d/trackWidth)) << "\n";
                resCounter++;
                connIter++;
            }
        }

        os << "*END\n\n";
    }

#if 0
    os << "# Routing positions to center of the cells. Generated by LunaPnR " LUNAVERSIONSTRING "\n";
    os << "# Format: <number of points> <point1 x> <point1 y> ... <pointN x> <pointN y> <newline>\n";

    for(auto const netKeyPair : netlist->m_nets)
    {
        // write number of points in the net
        os << netKeyPair->numberOfConnections() << " ";

        // write locations of all the terminals
        for(auto netConnect : *netKeyPair)
        {
            auto ins = netlist->lookupInstance(netConnect.m_instanceKey);
            if (!ins->isPlaced())
            {
                Logging::doLog(Logging::LogType::WARNING,"TXT writer: instance %s has no location / is not placed.!\n", ins->name().c_str());
                return false;
            }

            os << ins->m_pos.m_x << " " << ins->m_pos.m_y << "  ";
        }
        os << "\n";
    }
#endif
    return true;
}

bool LunaCore::SPEF::write(const std::string &filename, const std::shared_ptr<ChipDB::Module> module)
{
    std::ofstream ofile(filename);
    if (!ofile.good())
    {
        Logging::doLog(Logging::LogType::ERROR,"SPEF writer: cannot open file %s for writing!\n", filename.c_str());
        return false;
    }

    return LunaCore::SPEF::write(ofile, module);
}
