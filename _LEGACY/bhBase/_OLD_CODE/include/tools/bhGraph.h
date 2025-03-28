#pragma once
#include <vector>

class bhGraph
{
public:
    ////////////////////////////////////////////////////////////////////////////////
    class Node
    {
    public:
        Node();
        virtual ~Node();
        bool ResolvePath();

    protected:
        virtual bool Resolve() = 0;

        std::vector<Node*> inputs_v;
        Node* output = nullptr;
    private:
    };

    ////////////////////////////////////////////////////////////////////////////////
    bhGraph();
    virtual ~bhGraph();
    bool Resolve();

protected:
private:
    Node* result = nullptr;
};
