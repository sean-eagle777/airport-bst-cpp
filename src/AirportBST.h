#ifndef AIRPORTBST_H
#define AIRPORTBST_H

#include <string>
#include <vector>
#include "Airport.h"


struct RemoveResult
{
    bool found = false;
    bool wasRoot = false;
    std::string description; // e.g. "D2: only a left child, BOR"
};

class AirportBST
{
public:
    AirportBST();
    ~AirportBST();
    AirportBST(const AirportBST &) = delete;            // owns raw pointers
    AirportBST &operator=(const AirportBST &) = delete;

    bool insert(const Airport &a);

    const Airport *find(const std::string &code, std::vector<std::string> &path) const;
    const Airport *minimum() const;
    const Airport *maximum() const;

    RemoveResult remove(const std::string &code);

    bool validate() const;

    int size() const { return count; }
    bool empty() const { return root == nullptr; }
    int height() const;

    double averageDepth() const;

    std::vector<const Airport *> inorder() const;
    bool inorderIsSorted() const;

    bool contains(const std::string &code) const;


    std::vector<const Airport *> range(const std::string &from, const std::string &to,
                                       int &visited) const;

    std::vector<std::string> preorder() const;
    std::vector<std::string> levelOrder() const;

    void buildFromSorted(const std::vector<Airport> &sorted);
    void rebalance();
    std::vector<std::vector<std::string>> topLevels(int levels) const;
    void findExamples(std::string &leaf, std::string &onlyLeft,
                      std::string &onlyRight, std::string &twoChildren) const;

private:
    struct Node
    {
        Airport data;
        Node *left;
        Node *right;
        explicit Node(const Airport &a) : data(a), left(nullptr), right(nullptr) {}
    };

    Node *root;
    int count;

    void clear();
    Node *buildRec(const std::vector<Airport> &v, int lo, int hi);
};

#endif
