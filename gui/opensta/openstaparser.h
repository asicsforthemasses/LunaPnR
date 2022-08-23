// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <string>
#include <vector>
#include <regex>
#include <sstream>

/* Example output from OpenSTA:

Startpoint: b_in[1] (input port clocked by clk)
Endpoint: data_out[5] (output port clocked by clk)
Path Group: clk
Path Type: max

  Delay    Time   Description
---------------------------------------------------------
   0.00    0.00   clock clk (rise edge)
   0.00    0.00   clock network delay (ideal)
   0.00    0.00 v input external delay
   0.00    0.00 v b_in[1] (in)
   0.11    0.11 ^ _074_/Y (NAND2X1)
   0.16    0.27 ^ _090_/Y (XOR2X1)
   0.19    0.45 ^ _092_/Y (XNOR2X1)
   0.12    0.57 v _094_/Y (XOR2X1)
   0.14    0.71 ^ _095_/Y (INVX1)
   0.17    0.88 v _097_/Y (XNOR2X1)
   0.12    1.00 ^ _099_/Y (OAI21X1)
   0.08    1.09 v _108_/Y (XNOR2X1)
   0.00    1.09 v data_out[5] (out)
           1.09   data arrival time

  10.00   10.00   clock clk (rise edge)
   0.00   10.00   clock network delay (ideal)
   0.00   10.00   clock reconvergence pessimism
  -1.20    8.80   output external delay
           8.80   data required time
---------------------------------------------------------
           8.80   data required time
          -1.09   data arrival time
---------------------------------------------------------
           7.71   slack (MET)

*/

namespace GUI
{

/** parses the opensta output to check for errors */
class OpenSTAParser
{
public:

    /** reset the parser and clear all internal variables */
    void reset();

    /** submit one line of the OpenSTA console response */
    bool submitLine(const std::string &line);

    /** return the complete OpenSTA console response in a string */
    std::string getResponse() const
    {
        return m_response.str();
    }

    auto beginPaths() const
    {
        return m_paths.cbegin();
    }

    auto endPaths() const
    {
        return m_paths.cend();
    }

    struct PathInfo
    {
        std::string m_source;
        std::string m_destination;
        float       m_slack = {0.0f};

        bool isValid() const
        {
            return (!m_source.empty()) && (!m_destination.empty());
        }

        void clear()
        {
            m_source.clear();
            m_destination.clear();
            m_slack = 0.0f;
        }
    };

protected:
    enum class ParseState
    {
        NONE,
        UNITS,
        CHECKSETUP,
        REPORTCHECKS
    };

    ParseState m_state{ParseState::NONE};

    std::vector<PathInfo> m_paths;

    std::regex m_reSourcePath{R"(Startpoint: (\S*))"};
    std::regex m_reDestPath{R"(Endpoint: (\S*))"};
    std::regex m_reSlack{R"(\s*([0-9.]*)\s*slack)"};

    PathInfo m_pathInfo;

    std::ostringstream m_response;
};

};
