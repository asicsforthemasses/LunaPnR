// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "openstaparser.h"
#include "common/logging.h"

using namespace GUI;

void OpenSTAParser::reset()
{
    m_SPEFChecksOk=true;
    m_foundSPEFReport=false;
    m_state = ParseState::NONE;
    m_response.str("");
    m_response.clear(); // clear any error flags
}

bool OpenSTAParser::submitLine(const std::string &line)
{
    if (line.substr(0,6) == "#UNITS")
    {
        m_state = ParseState::UNITS;
        return true;
    }
    else if (line.substr(0,11) == "#CHECKSETUP")
    {
        m_state = ParseState::CHECKSETUP;
        return true;
    }
    else if (line.substr(0,10) == "#CHECKSPEF")
    {
        m_state = ParseState::CHECKSPEF;
        return true;
    }
    else if (line.substr(0,13) == "#REPORTCHECKS")
    {
        m_state = ParseState::REPORTCHECKS;
        return true;
    }

    m_response << line;

    std::smatch matches;

    switch(m_state)
    {
    case ParseState::UNITS:
        if (std::regex_search(line, matches, m_reTimeUnit))
        {
            m_timeMult = std::stod(matches.str(1));
            m_timeUnit = matches.str(2);
            Logging::doLog(Logging::LogType::VERBOSE,"OpenSTA time unit: %f %s\n", m_timeMult, m_timeUnit.c_str());
        }
        break;
    case ParseState::CHECKSETUP:
        if (std::regex_search(line, matches, m_reWarning))
        {
            m_setupWarnings.push_back(matches.str(1));
        }
        break;
    case ParseState::CHECKSPEF:
        if (std::regex_search(line, matches, m_spefCheck1))
        {
            m_foundSPEFReport = true;            
            auto netCount = std::stod(matches.str(1));
            if (netCount > 0)
            {
                m_SPEFChecksOk = false;
                m_setupWarnings.push_back(matches.str(1));
            }
            else
            {
                Logging::doLog(Logging::LogType::VERBOSE,"CheckTiming: all nets have SPEF annotations\n");
            }
        }
        else if (std::regex_search(line, matches, m_spefCheck2))
        {
            m_foundSPEFReport = true;
            auto netCount = std::stod(matches.str(1));
            if (netCount > 0)
            {
                m_SPEFChecksOk = false;
                m_setupWarnings.push_back(matches.str(1));
            }
            else
            {
                Logging::doLog(Logging::LogType::VERBOSE,"CheckTiming: all nets have SPEF annotations\n");
            }
        }
        break;        
    case ParseState::REPORTCHECKS:
        if (std::regex_search(line, matches, m_reSourcePath))
        {
            m_pathInfo.m_source = matches.str(1);
        }
        else if (std::regex_search(line, matches, m_reDestPath))
        {
            m_pathInfo.m_destination = matches.str(1);
        }
        else if (std::regex_search(line, matches, m_reSlack))
        {
            auto slackString = matches.str(1);
            m_pathInfo.m_slack = std::stof(slackString);
            if (!m_pathInfo.isValid())
            {
                // error: the source and destination should have been
                // set!
                m_pathInfo.clear();
                return false;
            }
            else
            {
                m_paths.push_back(m_pathInfo);
                m_pathInfo.clear();
                return true;
            }
        }
        break;
    default:
        break;
    }

    return true;
}
