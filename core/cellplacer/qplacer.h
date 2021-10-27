#pragma once

#include "common/dbtypes.h"
#include <vector>
#include <string>
#include <iostream>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

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

using PlacerNodeId = ssize_t;
using PlacerNetId  = ssize_t;

enum class PlacerNodeType
{
    Undefined,
    MovableNode,
    FixedNode,
    StarNode
};

enum class PlacerNetType
{
    Undefined,
    StarNet,
    TwoNet,
    Ignore
};

std::string toString(const PlacerNodeType &t);
std::string toString(const PlacerNetType &t);

struct PlacerNode
{
    PlacerNode() : m_type(PlacerNodeType::Undefined), m_weight(1.0f) {}

    ChipDB::Coord64 m_pos;      ///< center position of node
    PlacerNodeType  m_type;     ///< node type
    float           m_weight;   ///< default = 1.0, and 10.0 for fixed pins

    constexpr bool isFixed() const noexcept
    {
        return m_type == PlacerNodeType::FixedNode;
    }

    void dump(std::ostream &os) const
    {
        for(auto const& netId : m_connections)
        {
            os << " " << netId;
        }
        os << "\n";
    }

    std::vector<PlacerNetId> m_connections;    
};

struct PlacerNet
{
    PlacerNet() : m_type(PlacerNetType::Undefined), m_weight(1.0f) {}

    bool isValid() const noexcept
    {
        return m_nodes.size() >= 2;
    }

    void dump(std::ostream &os) const
    {
        for(auto const& nodeId : m_nodes)
        {
            os << " " << nodeId;
        }
        os << "\n";
    }

    std::vector<PlacerNodeId>   m_nodes;
    PlacerNetType               m_type;
    float m_weight;
};

struct XAxisAccessor
{
    static constexpr int64_t get(const ChipDB::Coord64 &pos) noexcept
    {
        return pos.m_x;
    }
};

struct YAxisAccessor
{
    static constexpr int64_t get(const ChipDB::Coord64 &pos) noexcept
    {
        return pos.m_y;
    }
};


/** analytical placer based on quadratic cost function */
class Placer
{
public:
    Placer() {};

    void solve(std::vector<PlacerNode> &nodes, std::vector<PlacerNet> &nets,
        Eigen::VectorXd &xpos, Eigen::VectorXd &ypos);

protected:

    void addStarNodes(std::vector<PlacerNode> &nodes, std::vector<PlacerNet> &nets);
    void checkAndSetUnmovableNet(PlacerNet &net, const std::vector<PlacerNode> &nodes);

    template<class AxisAccessor>
    void buildEquations(const std::vector<PlacerNode> &nodes, const std::vector<PlacerNet> &nets,
        Eigen::SparseMatrix<double> &Amat, Eigen::VectorXd &Bvec)
    {   
        Amat.resize(nodes.size(), nodes.size());

        Amat.reserve(2*nodes.size());   // rough estimate of size
        Bvec = Eigen::VectorXd::Zero(nodes.size());

        // the last node on every valid net should be a star node
        // connect every node on the net to the star node
        // and make a matrix entry at (i,j) and (j,i)
        for(auto const& net : nets)
        {
            if (net.m_type == PlacerNetType::StarNet)
            {
                auto starNodeId = net.m_nodes.back();

                //
                // https://www.researchgate.net/publication/220915658_FastPlace_Efficient_analytical_placement_using_cell_shifting_iterative_local_refinement_and_a_hybrid_net_model
                // recommends 1/(k-1) for star nets
                //

                size_t netSize = net.m_nodes.size();    // -1 because we discard the star node
                auto effectiveWeight = net.m_weight / static_cast<double>(netSize-1);

                for(auto const netNodeId : net.m_nodes)
                {
                    auto const& netNode = nodes.at(netNodeId);
                    if (!netNode.isFixed())
                    {
                        Amat.coeffRef(netNodeId, netNodeId)  += effectiveWeight;
                        Amat.coeffRef(starNodeId, starNodeId)+= effectiveWeight;
                        Amat.coeffRef(netNodeId, starNodeId) -= effectiveWeight;
                        Amat.coeffRef(starNodeId, netNodeId) -= effectiveWeight;
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

                if (nodes.at(node1Idx).isFixed())
                {
                    std::swap(node1Idx, node2Idx);
                }

                auto const& node1 = nodes.at(node1Idx); // movable node
                assert(!node1.isFixed());

                auto const& node2 = nodes.at(node2Idx); // maybe movable node
                
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

};  // namespace