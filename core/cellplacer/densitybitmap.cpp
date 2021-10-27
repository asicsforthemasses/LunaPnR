
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <deque>
#include "densitybitmap.h"

using namespace LunaCore;

constexpr bool isBigEndian() noexcept
{
    //FIXME: how do we do this at compile time,
    // C++20 has std::endian
    return false;
}


void DensityBitmap::writeToPGM(std::ostream &os)
{
    os << "P5 " << m_width << " " << m_height << " 255" << "\n";

    auto const* pixelPtr = &m_bitmap[0];
    for(int64_t y=0; y<m_height; y++)
    {
        for(int64_t x=0; x<m_width; x++)
        {
            auto pixel = *pixelPtr;
            uint8_t pixelValue = 0;
            if (pixel > 0)
            {
                // 100%   utilization -> 100
                // 10%    utilization -> 0
                // 1000%  utilization -> 200
                float dB = 100.0f*std::log10(pixel) + 100.0f;

                dB = std::min(dB,255.0f);
                dB = std::max(dB,0.0f);

                pixelValue = static_cast<uint8_t>(dB);
            }

            static_assert(sizeof(pixelValue)==1, "pixels must be 8 bits wide!");

            os.write((char*)&pixelValue, sizeof(pixelValue));
            pixelPtr++;
        }
    }
}

std::shared_ptr<DensityBitmap> LunaCore::createDensityBitmap(const ChipDB::Netlist *netlist, const ChipDB::Region *region,
    int64_t bitmapCellWidth /* nm */, int64_t bitmapCellHeight /* nm */)
{
    // The region is covered by the bitmap and divides into bitmap cells
    // Determine the bitmap size:

    auto regionSize   = region->m_rect.getSize();
    auto regionOffset = region->m_rect.getLL();

    auto xcells = static_cast<ssize_t>(1+std::floor(regionSize.m_x / static_cast<double>(bitmapCellWidth)));
    auto ycells = static_cast<ssize_t>(1+std::floor(regionSize.m_y / static_cast<double>(bitmapCellHeight)));

    std::shared_ptr<DensityBitmap> bitmap(new DensityBitmap(xcells, ycells));

    //TODO: add one fake instance just beyond the right outer edge
    //      to make sure the sweep gets to the right end of the grid.
    //      even when there are no instances there.

    // sweep from left to right
    // queue: instances that will become active.
    // active: instances that are active.

    // make local copy of instance pointers
    std::deque<ChipDB::InstanceBase*> queue(netlist->m_instances.begin(), netlist->m_instances.end());

    // remove all nullptr instances
    auto removeIter  = std::remove_if(queue.begin(), queue.end(), [](auto ptr){ return ptr==nullptr; } );
    queue.erase(removeIter, queue.end());

    // sort the instances according to x position
    // least x first
    std::sort(queue.begin(), queue.end(),
        [&](auto const ins1, auto const ins2)
        {
            return ins1->m_pos.m_x  < ins2->m_pos.m_x;
        }
    );

    std::deque<ChipDB::InstanceBase*> active;

    int64_t x = 0;  // sweep position    
    int64_t nextGridx = bitmapCellWidth;
    int64_t gridxIndex = 0;

    while(!queue.empty())
    {
        int64_t newx = queue.front()->m_pos.m_x - regionOffset.m_x;

        if (newx > nextGridx)
        {
            // new grid edge comes before the remaining
            // instances
            newx = nextGridx;
        }
        else
        {
            // transfer the next cell to the active queue
            // as it we move to the next sweep position
            active.push_back(queue.front());
            queue.pop_front();

            while(!queue.empty() && ((queue.front()->m_pos.m_x - regionOffset.m_x) <= newx))
            {
                active.push_back(queue.front());
                queue.pop_front();
            }
        }

        auto activeIter = active.begin();
        while(activeIter != active.end())
        {
            auto& activeIns = *activeIter;

            auto insSize   = activeIns->instanceSize();
            auto rightEdge = activeIns->m_pos.m_x+insSize.m_x - regionOffset.m_x;

            if (rightEdge < newx)
            {
                // chop the instance into blocks according to the
                // grid cell height

                int64_t gridyIndex = (activeIns->m_pos.m_y - regionOffset.m_y) / bitmapCellHeight;
                int64_t gridyStart = gridyIndex*bitmapCellHeight;

                auto insTopEdge = activeIns->m_pos.m_y + insSize.m_y - regionOffset.m_y;

                for(int64_t gridy=gridyStart; gridy < insTopEdge; gridy += bitmapCellHeight)
                {
                    auto ystart = std::max(gridy, activeIns->m_pos.m_y - regionOffset.m_y);
                    auto ystop  = std::min(gridy + bitmapCellHeight, insTopEdge);

                    auto insCellArea = (ystop-ystart)*(rightEdge-x);

                    bitmap->at(gridxIndex, gridyIndex) += static_cast<double>(insCellArea) / static_cast<double>(bitmapCellHeight*bitmapCellWidth);

                    gridyIndex++;
                }

                // no longer active -> remove it from the list
                activeIter = active.erase(activeIter);
            }
            else
            {                
                // chop the instance into blocks according to the
                // grid cell height

                int64_t gridyIndex = (activeIns->m_pos.m_y - regionOffset.m_y) / bitmapCellHeight;
                int64_t gridyStart = gridyIndex*bitmapCellHeight;

                auto insTopEdge = activeIns->m_pos.m_y + insSize.m_y - regionOffset.m_y;

                for(int64_t gridy=gridyStart; gridy < insTopEdge; gridy += bitmapCellHeight)
                {
                    auto ystart = std::max(gridy, activeIns->m_pos.m_y - regionOffset.m_y);
                    auto ystop  = std::min(gridy + bitmapCellHeight, insTopEdge);

                    auto insCellArea = (ystop-ystart)*(newx - x);

                    bitmap->at(gridxIndex, gridyIndex) += static_cast<double>(insCellArea) / static_cast<double>(bitmapCellHeight*bitmapCellWidth);

                    gridyIndex++;
                }

                activeIter++;
            }
        }   

        if (newx == nextGridx)
        {
            nextGridx += bitmapCellWidth;
            gridxIndex++;
        }

        x = newx;             
    }

    return bitmap;
}
