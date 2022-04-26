/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#include "common/logging.h"
#include "libreader.h"
#include "libreaderimpl.h"

using namespace ChipDB::Liberty;

bool Reader::load(Design &design, std::istream &source)
{
    try
    {
        std::stringstream src;
        src << source.rdbuf();

        ReaderImpl readerimpl(design);
        if (!readerimpl.parse(src.str()))
        {
            Logging::doLog(Logging::LogType::ERROR,"Liberty::Reader failed to load file.\n");
            return false;
        }
        
        return true;
    }
    catch(std::runtime_error &e)
    {
        Logging::doLog(Logging::LogType::ERROR,"%s\n", e.what());
    }
    
    return false;    
}
