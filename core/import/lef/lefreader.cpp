#include "common/logging.h"
#include "lefreader.h"
#include "lefreaderimpl.h"

using namespace ChipDB::LEF;

bool Reader::load(Design &design, std::istream &source)
{
    try
    {
        std::stringstream src;
        src << source.rdbuf();

        ReaderImpl readerimpl(design);
        if (!readerimpl.parse(src.str()))
        {
            doLog(LOG_ERROR,"LEF::Reader failed to load file.\n");
            return false;
        }
        
        return true;
    }
    catch(std::runtime_error &e)
    {
        doLog(LOG_ERROR,"%s\n", e.what());
    }
    
    return false;    
}
