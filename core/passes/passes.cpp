// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <span>

#include "common/strutils.hpp"
#include "passes.hpp"
#include "floorplanpass.hpp"
#include "readpass.hpp"
#include "writepass.hpp"
#include "infopass.hpp"
#include "scriptpass.hpp"
#include "filesystempass.hpp"
#include "checkpass.hpp"
#include "placepass.hpp"
#include "flattenpass.hpp"
#include "ctspass.hpp"
#include "padringpass.hpp"

namespace LunaCore::Passes
{

bool run(Database &database, const std::string &cmdstring)
{
    auto chunks = LunaCore::split(cmdstring, ' ');
    if (!chunks.empty())
    {
        return runPass(database,chunks.front(), std::span(chunks.begin()+1, chunks.end()));
    }

    return false;
}

void registerAllPasses()
{
    registerPass(new Floorplan());
    registerPass(new ReadPass());
    registerPass(new WritePass());
    registerPass(new InfoPass());
    registerPass(new ScriptPass());
    registerPass(new LSPass());
    registerPass(new CDPass());
    registerPass(new CWDPass());
    registerPass(new CheckPass());
    registerPass(new PlacePass());
    registerPass(new FlattenPass());
    registerPass(new CTSPass());
    registerPass(new PadringPass());
}

};
