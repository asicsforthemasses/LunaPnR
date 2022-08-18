#include "openstaparser.h"

using namespace GUI;

void OpenSTAParser::reset()
{
    m_state = ParseState::NONE;
    m_response.str("");
    m_response.clear(); // clear any error flags
}

bool OpenSTAParser::submitLine(const std::string &line)
{
    if (line == "#UNITS\n")
    {
        m_state = ParseState::UNITS;
        return true;
    }
    else if (line == "#CHECKSETUP")
    {
        m_state = ParseState::CHECKSETUP;
        return true;
    }
    else if (line == "#REPORTCHECKS")
    {
        m_state = ParseState::REPORTCHECKS;
        return true;
    }

    m_response << line;

    std::smatch matches;

    switch(m_state)
    {
    case ParseState::UNITS:
        break;
    case ParseState::CHECKSETUP:
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
