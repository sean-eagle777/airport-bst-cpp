#include "AirportBST.h"
#include <queue>
#include <stack>
#include <utility>

AirportBST::AirportBST() : root(nullptr), count(0) {}

AirportBST::~AirportBST() { clear(); }
void AirportBST::clear()
{
    std::stack<Node *> st;
    if (root)
        st.push(root);
    while (!st.empty())
    {
        Node *cur = st.top();
        st.pop();
        if (cur->left)
            st.push(cur->left);
        if (cur->right)
            st.push(cur->right);
        delete cur;
    }
    root = nullptr;
    count = 0;
}

bool AirportBST::insert(const Airport &a)
{
    if (root == nullptr)
    {
        root = new Node(a);
        count++;
        return true;
    }

    Node *cur = root;
    while (true)
    {
        if (a.iata < cur->data.iata)
        {
            if (cur->left == nullptr)
            {
                cur->left = new Node(a);
                count++;
                return true;
            }
            cur = cur->left;
        }
        else if (a.iata > cur->data.iata)
        {
            if (cur->right == nullptr)
            {
                cur->right = new Node(a);
                count++;
                return true;
            }
            cur = cur->right;
        }
        else
        {
            return false; // duplicate key
        }
    }
}

int AirportBST::height() const
{
    if (root == nullptr)
        return -1;

    std::queue<Node *> q;
    q.push(root);
    int levels = 0;
    while (!q.empty())
    {
        int n = (int)q.size();
        for (int i = 0; i < n; i++)
        {
            Node *cur = q.front();
            q.pop();
            if (cur->left)
                q.push(cur->left);
            if (cur->right)
                q.push(cur->right);
        }
        levels++;
    }
    return levels - 1;
}

double AirportBST::averageDepth() const
{
    if (root == nullptr)
        return 0.0;

    std::queue<std::pair<Node *, int>> q;
    q.push({root, 0});
    long long sum = 0;
    while (!q.empty())
    {
        Node *cur = q.front().first;
        int d = q.front().second;
        q.pop();
        sum += d;
        if (cur->left)
            q.push({cur->left, d + 1});
        if (cur->right)
            q.push({cur->right, d + 1});
    }
    return (double)sum / count;
}

std::vector<const Airport *> AirportBST::inorder() const
{
    std::vector<const Airport *> out;
    std::stack<Node *> st;
    Node *cur = root;
    while (cur != nullptr || !st.empty())
    {
        while (cur != nullptr)
        {
            st.push(cur);
            cur = cur->left;
        }
        cur = st.top();
        st.pop();
        out.push_back(&cur->data);
        cur = cur->right;
    }
    return out;
}

bool AirportBST::inorderIsSorted() const
{
    std::vector<const Airport *> v = inorder();
    for (int i = 1; i < (int)v.size(); i++)
    {
        if (!(v[i - 1]->iata < v[i]->iata))
            return false;
    }
    return true;
}

// ======================= M2 =======================

const Airport *AirportBST::find(const std::string &code, std::vector<std::string> &path) const
{
    path.clear();
    Node *cur = root;
    while (cur != nullptr)
    {
        path.push_back(cur->data.iata);
        if (code < cur->data.iata)
            cur = cur->left;
        else if (code > cur->data.iata)
            cur = cur->right;
        else
            return &cur->data;
    }
    return nullptr;
}

const Airport *AirportBST::minimum() const
{
    if (root == nullptr)
        return nullptr;
    Node *cur = root;
    while (cur->left != nullptr)
        cur = cur->left;
    return &cur->data;
}

const Airport *AirportBST::maximum() const
{
    if (root == nullptr)
        return nullptr;
    Node *cur = root;
    while (cur->right != nullptr)
        cur = cur->right;
    return &cur->data;
}

RemoveResult AirportBST::remove(const std::string &code)
{
    RemoveResult res;

    Node **link = &root;
    while (*link != nullptr && (*link)->data.iata != code)
    {
        if (code < (*link)->data.iata)
            link = &(*link)->left;
        else
            link = &(*link)->right;
    }
    if (*link == nullptr)
        return res; 

    Node *node = *link;
    res.found = true;
    res.wasRoot = (node == root);

    if (node->left == nullptr && node->right == nullptr)
    {
        res.description = "D1: leaf";
        *link = nullptr;
        delete node;
    }
    else if (node->right == nullptr)
    {
        res.description = "D2: only a left child, " + node->left->data.iata;
        *link = node->left;
        delete node;
    }
    else if (node->left == nullptr)
    {
        res.description = "D2: only a right child, " + node->right->data.iata;
        *link = node->right;
        delete node;
    }
    else
    {
        Node **sl = &node->right;
        while ((*sl)->left != nullptr)
            sl = &(*sl)->left;
        Node *succ = *sl;
        res.description = "D3: two children, replaced by successor " + succ->data.iata;
        node->data = succ->data;
        *sl = succ->right; // successor has no left child
        delete succ;
    }

    count--;
    return res;
}

bool AirportBST::validate() const
{
    struct Item
    {
        Node *node;
        const std::string *lo; // nullptr = no lower bound
        const std::string *hi; // nullptr = no upper bound
    };

    if (root == nullptr)
        return true;

    std::stack<Item> st;
    st.push({root, nullptr, nullptr});
    while (!st.empty())
    {
        Item it = st.top();
        st.pop();
        const std::string &key = it.node->data.iata;
        if (it.lo != nullptr && !(key > *it.lo))
            return false;
        if (it.hi != nullptr && !(key < *it.hi))
            return false;
        if (it.node->left)
            st.push({it.node->left, it.lo, &key});
        if (it.node->right)
            st.push({it.node->right, &key, it.hi});
    }
    return true;
}

std::vector<std::vector<std::string>> AirportBST::topLevels(int levels) const
{
    std::vector<std::vector<std::string>> out;
    if (root == nullptr)
        return out;

    std::queue<Node *> q;
    q.push(root);
    for (int lv = 0; lv < levels && !q.empty(); lv++)
    {
        std::vector<std::string> row;
        int n = (int)q.size();
        for (int i = 0; i < n; i++)
        {
            Node *cur = q.front();
            q.pop();
            row.push_back(cur->data.iata);
            if (cur->left)
                q.push(cur->left);
            if (cur->right)
                q.push(cur->right);
        }
        out.push_back(row);
    }
    return out;
}

void AirportBST::findExamples(std::string &leaf, std::string &onlyLeft,
                              std::string &onlyRight, std::string &twoChildren) const
{
    leaf = onlyLeft = onlyRight = twoChildren = "";
    if (root == nullptr)
        return;

    std::queue<Node *> q; // level order -> shallowest examples first
    q.push(root);
    while (!q.empty())
    {
        Node *cur = q.front();
        q.pop();
        if (cur->left == nullptr && cur->right == nullptr)
        {
            if (leaf.empty())
                leaf = cur->data.iata;
        }
        else if (cur->right == nullptr)
        {
            if (onlyLeft.empty())
                onlyLeft = cur->data.iata;
        }
        else if (cur->left == nullptr)
        {
            if (onlyRight.empty())
                onlyRight = cur->data.iata;
        }
        else if (twoChildren.empty() && cur != root) 
        {
            twoChildren = cur->data.iata;
        }
        if (cur->left)
            q.push(cur->left);
        if (cur->right)
            q.push(cur->right);
    }
}

// ======================= M3 / M4 =======================

bool AirportBST::contains(const std::string &code) const
{
    Node *cur = root;
    while (cur != nullptr)
    {
        if (code < cur->data.iata)
            cur = cur->left;
        else if (code > cur->data.iata)
            cur = cur->right;
        else
            return true;
    }
    return false;
}

std::vector<const Airport *> AirportBST::range(const std::string &from, const std::string &to,
                                               int &visited) const
{
    std::vector<const Airport *> out;
    visited = 0;
    std::stack<Node *> st;
    Node *cur = root;
    while (cur != nullptr || !st.empty())
    {
        while (cur != nullptr)
        {
            visited++;
            if (cur->data.iata < from)
            {
                cur = cur->right; // node and its whole left subtree are below FROM
            }
            else
            {
                st.push(cur);
                // keys in the left subtree are < cur's key, so only go there if key > FROM
                cur = (cur->data.iata > from) ? cur->left : nullptr;
            }
        }
        if (st.empty())
            break;
        cur = st.top();
        st.pop();
        if (cur->data.iata > to)
            break; // this node and everything after it (inorder) is above TO
        out.push_back(&cur->data);
        cur = (cur->data.iata < to) ? cur->right : nullptr;
    }
    return out;
}

std::vector<std::string> AirportBST::preorder() const
{
    std::vector<std::string> out;
    std::stack<Node *> st;
    if (root)
        st.push(root);
    while (!st.empty())
    {
        Node *cur = st.top();
        st.pop();
        out.push_back(cur->data.iata);
        if (cur->right)
            st.push(cur->right); // right first so left is processed first
        if (cur->left)
            st.push(cur->left);
    }
    return out;
}

std::vector<std::string> AirportBST::levelOrder() const
{
    std::vector<std::string> out;
    std::queue<Node *> q;
    if (root)
        q.push(root);
    while (!q.empty())
    {
        Node *cur = q.front();
        q.pop();
        out.push_back(cur->data.iata);
        if (cur->left)
            q.push(cur->left);
        if (cur->right)
            q.push(cur->right);
    }
    return out;
}

AirportBST::Node *AirportBST::buildRec(const std::vector<Airport> &v, int lo, int hi)
{
    if (lo > hi)
        return nullptr;
    int mid = (lo + hi) / 2; // middle element becomes the root of this subtree
    Node *n = new Node(v[mid]);
    n->left = buildRec(v, lo, mid - 1);
    n->right = buildRec(v, mid + 1, hi);
    return n;
}

void AirportBST::buildFromSorted(const std::vector<Airport> &sorted)
{
    clear();
    root = buildRec(sorted, 0, (int)sorted.size() - 1); // depth only ~log2(n), recursion is safe
    count = (int)sorted.size();
}

void AirportBST::rebalance()
{
    std::vector<Airport> v;
    std::vector<const Airport *> in = inorder();
    for (int i = 0; i < (int)in.size(); i++)
        v.push_back(*in[i]);
    buildFromSorted(v);
}
