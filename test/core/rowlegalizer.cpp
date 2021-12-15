#include "lunacore.h"

#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(RowLegalizerTest)

BOOST_AUTO_TEST_CASE(check_interval_logic)
{
    std::cout << "--== CHECK INTERVAL LOGIC ==--\n";

    LunaCore::RowLegalizer::RowInfo rowInfo;

    // create one cluster and check for overlaps
    rowInfo.m_clusters.emplace_back();
    rowInfo.m_clusters.back().m_start = 10000;
    rowInfo.m_clusters.back().m_width = 555;

    BOOST_CHECK(rowInfo.hasOverlap(0, 100) == -1);      // interval completely to the left of the cluster
    BOOST_CHECK(rowInfo.hasOverlap(10556, 100) == -1);  // interval completely to the right of the cluster
    
    BOOST_CHECK(rowInfo.hasOverlap(10555, 100) == 0);  // interval touched the rigtht side of the cluster
    BOOST_CHECK(rowInfo.hasOverlap(9900, 100) == 0);   // interval touched the left side of the cluster

    BOOST_CHECK(rowInfo.hasOverlap(10010, 100) == 0);  // interval is completely inside the cluster

    // create a second cluster and check for overlaps
    rowInfo.m_clusters.emplace_back();
    rowInfo.m_clusters.back().m_start = 20000;
    rowInfo.m_clusters.back().m_width = 555;    

    BOOST_CHECK(rowInfo.hasOverlap(10556, 100) == -1);  // interval completely between the two clusters
    BOOST_CHECK(rowInfo.hasOverlap(20010, 100) == 1);   // interval completely in second cluster
    BOOST_CHECK(rowInfo.hasOverlap(30010, 100) == -1);  // interval completely to the right of the second cluster
}

BOOST_AUTO_TEST_CASE(check_create_cluster)
{
    std::cout << "--== CHECK CREATE CLUSTER ==--\n";

    LunaCore::RowLegalizer::RowInfo rowInfo;
    BOOST_CHECK(rowInfo.m_clusters.size() == 0);

    rowInfo.createCluster(10000, 555);
    BOOST_CHECK(rowInfo.m_clusters.size() == 1);

    rowInfo.createCluster(20000, 555);
    BOOST_CHECK(rowInfo.m_clusters.size() == 2);

    rowInfo.createCluster(100, 555);
    BOOST_CHECK(rowInfo.m_clusters.size() == 3);

    // check the order of the clusters
    BOOST_CHECK(rowInfo.hasOverlap(100, 100) == 0);
    BOOST_CHECK(rowInfo.hasOverlap(10000, 100) == 1);
    BOOST_CHECK(rowInfo.hasOverlap(20000, 100) == 2);
}

BOOST_AUTO_TEST_SUITE_END()
