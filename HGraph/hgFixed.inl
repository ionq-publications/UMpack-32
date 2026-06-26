#include "ABKCommon/config.h"

#ifdef SIGNAL_DIRECTIONS
ABKINLINE void HGraphFixed::addSrc(const HGFNode& node, const HGFEdge& edge)
{ _srcs.push_back(std::make_pair(node.getIndex(), edge.getIndex())); }

ABKINLINE void HGraphFixed::addSnk(const HGFNode& node, const HGFEdge& edge)
{ _snks.push_back(std::make_pair(node.getIndex(), edge.getIndex())); }
#endif

ABKINLINE void HGraphFixed::addSrcSnk(const HGFNode& node, const HGFEdge& edge)
{ _srcSnks.push_back(std::make_pair(node.getIndex(), edge.getIndex())); }

ABKINLINE void HGraphFixed::fastAddSrcSnk(HGFNode* node, HGFEdge* edge)
{
    edge->_nodes.push_back(node);
    node->_edges.push_back(edge);
}

ABKINLINE HGFEdge* HGraphFixed::fastAddEdge(unsigned numPins, HGWeight weight) 
{
    abkassert(_addEdgeStyle != SLOW_ADDEDGE_USED, 
        "cannot mix fast and slow addEdge in one HGraph object");

    abkassert2(_param.makeAllSrcSnk, 
        "cannot use fast addEdge if graph has directionality",
        " (i.e., allSrcSnks is false)");

#ifdef DABKDEBUG
    _addEdgeStyle=FAST_ADDEDGE_USED;

    if (_param.netThreshold<numPins) 
    {
        abkfatal(_param.removeBigNets, 
            "fast addEdge requires that removeBigNets is set to true");
        return &mDummyEdge;
    }
#endif

    HGFEdge* edge=new HGFEdge(weight);
    _edges.push_back(edge);
    edge->_index=_edges.size()-1;
    edge->_nodes.reserve(numPins);
#ifdef SIGNAL_DIRECTIONS
    edge->_snksBegin = edge->_srcSnksBegin = edge->_nodes.begin();
#endif
    return edge;
}

ABKINLINE std::vector<std::string>& HGraphFixed::getNodeNames(void)
{
    abkfatal(_haveNames,"Names are gone, check if discardNames or clearNames was called");
    return _nodeNames;
}

ABKINLINE std::vector<std::string>& HGraphFixed::getNetNames(void)
{
    abkfatal(_haveNames,"Names are gone, check if discardNames or clearNames was called");
    return _netNames;
}

ABKINLINE HGNodeNamesMap& HGraphFixed::getNodeNamesMap(void)
{
    abkfatal(_haveNameMaps,"Name maps are gone, check if discardNames or clearNameMaps was called");
    return _nodeNamesMap;
}

ABKINLINE HGNetNamesMap& HGraphFixed::getNetNamesMap(void)
{
    abkfatal(_haveNameMaps,"Name maps are gone, check if discardNames or clearNameMaps was called");
    return _netNamesMap;
}

ABKINLINE unsigned HGraphFixed::maxNodeIndex() const { return _nodes.size()-1; }

ABKINLINE unsigned HGraphFixed::maxEdgeIndex() const { return _edges.size()-1; }

ABKINLINE const HGFNode& HGraphFixed::getNodeByIdx(unsigned nodeIndex) const
{
    abkassert3(nodeIndex<getNumNodes(), nodeIndex,
        " is an out-of-range index, max=", getNumNodes());
    return *_nodes[nodeIndex];
}

ABKINLINE bool HGraphFixed::haveSuchNode(const char* name) const
{
    abkfatal(_haveNameMaps, "Name maps no longer exist. Check when discardNames() or clearNameMaps() was called.");
    HGNodeNamesMap::const_iterator nameItr = _nodeNamesMap.find(name);
    return nameItr != _nodeNamesMap.end(); 
}

ABKINLINE bool HGraphFixed::haveSuchNet(const char* name) const
{
    abkfatal(_haveNameMaps, "Name maps no longer exist. Check when discardNames() or clearNameMaps() was called.");
    HGNetNamesMap::const_iterator nameItr = _netNamesMap.find(name);
    return nameItr != _netNamesMap.end(); 
}

ABKINLINE const HGFNode& HGraphFixed::getNodeByName(const char* name) const
{
    abkfatal(_haveNameMaps, "Name maps no longer exist. Check when discardNames() or clearNameMaps() was called.");
    HGNodeNamesMap::const_iterator nameItr = _nodeNamesMap.find(name);
    abkassert2(nameItr != _nodeNamesMap.end(), 
        "name not found in getNodeByName: ", name);
    return *_nodes[(*nameItr).second];
}

ABKINLINE const HGFEdge& HGraphFixed::getNetByName(const char* name) const
{
    abkfatal(_haveNameMaps, "Name maps no longer exist. Check when discardNames() or clearNameMaps() was called.");
    HGNetNamesMap::const_iterator nameItr = _netNamesMap.find(name);
    abkassert2(nameItr != _netNamesMap.end(), 
        "name not found in getNetByName: ", name);
    return *_edges[(*nameItr).second];
}

ABKINLINE HGFNode& HGraphFixed::getNodeByIdx(unsigned nodeIndex)
{
    abkassert3(nodeIndex<getNumNodes(), nodeIndex,
        " is an out-of-range index, max=", getNumNodes());
    return *_nodes[nodeIndex];
}

ABKINLINE HGFNode& HGraphFixed::getNodeByName(const char* name)
{
    abkfatal(_haveNameMaps, "Name maps no longer exist. Check when discardNames() or clearNameMaps() was called.");
    HGNodeNamesMap::const_iterator nameItr = _nodeNamesMap.find(name);
    abkassert2(nameItr != _nodeNamesMap.end(), 
        "name not found in getNodeByName: ", name);
    return *_nodes[(*nameItr).second];
}

ABKINLINE const HGFEdge& HGraphFixed::getEdgeByIdx(unsigned edgeIndex) const
{
    abkassert(edgeIndex<getNumEdges(), "edge index too big");
    return *_edges[edgeIndex];
}

ABKINLINE HGFEdge& HGraphFixed::getEdgeByIdx(unsigned edgeIndex)
{
    abkassert(edgeIndex<getNumEdges(), "edge index too big");
    return *_edges[edgeIndex];
}

ABKINLINE HGFEdge& HGraphFixed::getNetByName(const char* name)
{
    abkfatal(_haveNameMaps, "Name maps no longer exist. Check when discardNames() or clearNameMaps() was called.");
    HGNetNamesMap::const_iterator nameItr = _netNamesMap.find(name);
    abkassert2(nameItr != _netNamesMap.end(), 
        "name not found in getNetByName: ", name);
    return *_edges[(*nameItr).second];
}

ABKINLINE const Permutation& HGraphFixed::getNodesSortedByWeights() const
{
    if (_weightSort.getSize()==0)
        computeNodesSortedByWeights();
    return _weightSort;
}

ABKINLINE const Permutation& HGraphFixed::getNodesSortedByWeightsWShuffle() const
{
    if (_weightSort.getSize()==0)
        computeNodesSortedByWeightsWShuffle();
    return _weightSort;
}

ABKINLINE const Permutation& HGraphFixed::getNodesSortedByDegrees() const
{
    if (_degreeSort.getSize()==0)
        computeNodesSortedByDegrees();
    return _degreeSort;
}

ABKINLINE unsigned HGraphFixed::getNumNodes() const { return _nodes.size(); }

ABKINLINE unsigned HGraphFixed::getNumEdges() const { return _edges.size(); }

ABKINLINE double HGraphFixed::getAvgNodeDegree() const { return 1.0*getNumPins()/getNumNodes(); }

ABKINLINE double HGraphFixed::getAvgEdgeDegree() const { return 1.0*getNumPins()/getNumEdges(); }

ABKINLINE itHGFNodeGlobal HGraphFixed::nodesBegin() const { return _nodes.begin(); }

ABKINLINE itHGFNodeGlobal HGraphFixed::nodesEnd()   const { return _nodes.end();   }

ABKINLINE itHGFNodeGlobal HGraphFixed::terminalsBegin() const { return _nodes.begin(); }

ABKINLINE itHGFNodeGlobal HGraphFixed::terminalsEnd()   const { return _nodes.begin()+_numTerminals; }

ABKINLINE itHGFEdgeGlobal HGraphFixed::edgesBegin() const { return _edges.begin(); }

ABKINLINE itHGFEdgeGlobal HGraphFixed::edgesEnd()   const { return _edges.end();   }

ABKINLINE itHGFNodeGlobalMutable HGraphFixed::nodesBegin(){ return _nodes.begin(); }

ABKINLINE itHGFNodeGlobalMutable HGraphFixed::nodesEnd()  { return _nodes.end();   }

ABKINLINE itHGFEdgeGlobalMutable HGraphFixed::edgesBegin(){ return _edges.begin(); }

ABKINLINE itHGFEdgeGlobalMutable HGraphFixed::edgesEnd()  { return _edges.end();   }

ABKINLINE const std::string HGraphFixed::getNodeNameByIndex(unsigned nId) const
{ 
    //abkfatal(_haveNames,"Names are gone, check if discardNames or clearNames was called");
    if(_haveNames && nId < _nodeNames.size())
        return _nodeNames[nId];
    else
    {
        std::string rval;
        if(isTerminal(nId))
        {
            rval+="p";
            std::stringstream ss;
            ss<<(nId+1);
            std::string numstr;
            ss>>numstr;
            rval+=numstr;
        }
        else
        {
            rval+="a";
            abkfatal(nId>=getNumTerminals(),"BOOOM!");
            std::stringstream ss;
            ss<<nId-getNumTerminals();
            std::string numstr;
            ss>>numstr;
            rval+=numstr;
        }
        return rval;
    }
}

ABKINLINE const std::string HGraphFixed::getNetNameByIndex(unsigned nId) const
{
    //abkfatal(_haveNames,"Names are gone, check if discardNames or clearNames was called");
    if(_haveNames && nId < _netNames.size())
        return _netNames[nId];
    else
    {
        std::string rval("N");
        std::stringstream ss;
        ss<<nId;
        std::string numstr;
        ss>>numstr;
        rval+=numstr;
        return rval;
    }
}

ABKINLINE const std::vector<std::string>& HGraphFixed::getNetNames(void) const
{
    abkfatal(_haveNames,"Names are gone, check if discardNames or clearNames was called");
    return _netNames;
}

ABKINLINE const std::vector<std::string>& HGraphFixed::getNodeNames(void) const
{
    abkfatal(_haveNames,"Names are gone, check if discardNames or clearNames was called");
    return _nodeNames;
}
