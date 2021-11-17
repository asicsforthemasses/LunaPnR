#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

#include "common/logging.h"
#include "common/dbtypes.h"
#include "qplacertypes.h"

// predeclarations
namespace ChipDB
{
    class Module;   
    class Region;
    class Design;
};

/** analytical placer namespace */
namespace LunaCore::QPlacer
{

std::string toString(const PlacerNodeType &t);
std::string toString(const PlacerNetType &t);

struct XAxisAccessor
{
    static constexpr ChipDB::CoordType get(const ChipDB::Coord64 &pos) noexcept
    {
        return pos.m_x;
    }

    static constexpr const char* m_name= "X";

};

struct YAxisAccessor
{
    static constexpr ChipDB::CoordType get(const ChipDB::Coord64 &pos) noexcept
    {
        return pos.m_y;
    }

    static constexpr const char* m_name = "Y";
};


/** analytical placer based on quadratic cost function */
class Placer
{
public:
    Placer() {};

    /** solve the quadratice placement, update xpos and ypos. */
    void solve(const PlacerNetlist &netlist,
        Eigen::VectorXd &xpos, Eigen::VectorXd &ypos);

protected:
    void addStarNodes(PlacerNetlist &netlist);
    void checkAndSetUnmovableNet(PlacerNet &net, const std::vector<PlacerNode> &nodes);

    template<class AxisAccessor>
    void buildEquations(const PlacerNetlist &netlist,
        Eigen::SparseMatrix<double> &Amat, Eigen::VectorXd &Bvec)
    {   
        Amat.resize(netlist.numberOfNodes(), netlist.numberOfNodes());

        Amat.reserve(2*netlist.numberOfNodes());    // rough estimate of size
        Bvec = Eigen::VectorXd::Zero(netlist.numberOfNodes());

        // the last node on every valid net should be a star node
        // connect every node on the net to the star node
        // and make a matrix entry at (i,j) and (j,i)
        for(auto const& net : netlist.m_nets)
        {
            if (net.m_type == PlacerNetType::StarNet)
            {
                auto starNodeId = net.m_nodes.back();

                //
                // https://www.researchgate.net/publication/220915658_FastPlace_Efficient_analytical_placement_using_cell_shifting_iterative_local_refinement_and_a_hybrid_net_model
                // recommends 1/(k-1) for star nets
                //

                size_t netSize = net.m_nodes.size();
                auto effectiveWeight = net.m_weight / static_cast<double>(netSize-1);

                for(auto const netNodeId : net.m_nodes)
                {
                    auto const& netNode = netlist.getNode(netNodeId);
                    if (!netNode.isFixed())
                    {
                        Amat.coeffRef(netNodeId, netNodeId)  += effectiveWeight;
                        Amat.coeffRef(starNodeId, starNodeId)+= effectiveWeight;
                        Amat.coeffRef(netNodeId, starNodeId) -= effectiveWeight;
                        Amat.coeffRef(starNodeId, netNodeId) -= effectiveWeight;

                        // Note: if we want to account for pin offset within an instance cell
                        //       Bvec should also be changed. See: https://d-nb.info/990084132/34 page 38.
                        //  
                    }
                    else
                    {
                        // fixed nodes can increase the net weight!
                        Amat.coeffRef(starNodeId, starNodeId)+= effectiveWeight * netNode.m_weight;
                        Bvec[starNodeId] += effectiveWeight * netNode.m_weight * AxisAccessor::get(netNode.m_pos);
                    }
                }
            }
            else if (net.m_type == PlacerNetType::TwoNet)
            {
                auto node1Idx = net.m_nodes.at(0);
                auto node2Idx = net.m_nodes.at(1);

                if (netlist.getNode(node1Idx).isFixed())
                {
                    std::swap(node1Idx, node2Idx);
                }

                auto const& node1 = netlist.getNode(node1Idx); // movable node
                assert(!node1.isFixed());

                auto const& node2 = netlist.getNode(node2Idx); // maybe movable node
                
                const double effectiveWeight = 1.0;
                if (!node2.isFixed())
                {
                    Amat.coeffRef(node1Idx, node1Idx)  += effectiveWeight;
                    Amat.coeffRef(node2Idx, node2Idx)  += effectiveWeight;
                    Amat.coeffRef(node1Idx, node2Idx)  -= effectiveWeight;
                    Amat.coeffRef(node2Idx, node1Idx)  -= effectiveWeight;
                }
                else
                {
                    // fixed nodes can increase the net weight!
                    Amat.coeffRef(node1Idx, node1Idx)  += effectiveWeight * node2.m_weight;
                    Bvec[node1Idx] += effectiveWeight * node2.m_weight * AxisAccessor::get(node2.m_pos);
                }
            }
        }
    };

    Eigen::SparseMatrix<double> m_Amat_x;
    Eigen::SparseMatrix<double> m_Amat_y;
    Eigen::VectorXd m_Bvec_x;
    Eigen::VectorXd m_Bvec_y;
};


bool placeModuleInRegion(const ChipDB::Design *design, ChipDB::Module *mod, const ChipDB::Region *region);
void doRecursivePartitioning(const PlacerNetlist &netlist, const ChipDB::Rect64 &partitionRect);

template<class AxisAccessor>
std::vector<PlacerNodeId> selectNodesByCenterOfMassPosition(const PlacerNetlist &netlist)
{
    std::vector<PlacerNodeId> sortedNodesIdx(netlist.m_nodes.size());
    std::generate(sortedNodesIdx.begin(), sortedNodesIdx.end(), [n = 0]() mutable { return n++; });

    auto &nodes = netlist.m_nodes;

    std::sort(sortedNodesIdx.begin(), sortedNodesIdx.end(), [nodes, sortedNodesIdx](const ssize_t &idx1, const ssize_t &idx2)
        {
            return AxisAccessor::get(nodes.at(idx1).m_pos) < AxisAccessor::get(nodes.at(idx2).m_pos);
        }
    );

    // calculate total mass
    double totalMass = 0.0;
    for(auto const& node : nodes)
    {
        if (node.m_type != LunaCore::QPlacer::PlacerNodeType::FixedNode)
        {
            totalMass += static_cast<double>(node.m_size.m_x * node.m_size.m_y);
        }
    }

    // walk through sorted nodes until
    // the running mass has exceeded half the total mass
    double runningMass = 0;
    ssize_t idx = 0;
    while(runningMass < (totalMass/2))
    {
        auto const& sortedNode = nodes.at(sortedNodesIdx.at(idx));
        if (sortedNode.m_type != LunaCore::QPlacer::PlacerNodeType::FixedNode)
        {
            runningMass += static_cast<double>(sortedNode.m_size.m_x * sortedNode.m_size.m_y);
        }
        idx++;
    }

    doLog(LOG_VERBOSE,"  Center of mass found at position %s=%lu\n", AxisAccessor::m_name, AxisAccessor::get(nodes.at(sortedNodesIdx.at(idx)).m_pos));

    // remove the node indexes not part of the selection
    sortedNodesIdx.erase(sortedNodesIdx.begin()+idx,sortedNodesIdx.end());

    doLog(LOG_VERBOSE,"  Number of nodes returned: %lu of %lu\n", sortedNodesIdx.size(), netlist.m_nodes.size());

    return sortedNodesIdx;
}

};  // namespace
