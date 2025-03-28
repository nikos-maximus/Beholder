#include "tools/bhGraph.h"

////////////////////////////////////////////////////////////////////////////////
bhGraph::Node::Node()
{
    inputs_v.reserve(2);
}

bhGraph::Node::~Node()
{}

bool bhGraph::Node::ResolvePath()
{
    bool result = true;
    for (size_t i = 0; i < inputs_v.size(); ++i)
    {
        result &= inputs_v[i]->ResolvePath();
    }
    if (result)
    {
        return Resolve();
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bhGraph::bhGraph()
{}

bhGraph::~bhGraph()
{
    delete result;
}

bool bhGraph::Resolve()
{
    return result->ResolvePath();
}
