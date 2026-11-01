#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "Airport.h"
#include "AirportBST.h"
#include "Loader.h"

using namespace std;


static vector<Airport> fileOrder;     
static map<string, Airport> database; 
static AirportBST *tree = new AirportBST();


static void printAirport(const Airport &a)
{
    cout << a.iata << " " << a.name << ", " << a.city << ", " << a.country << '\n';
}

static void printLevels(const AirportBST &t, int levels)
{
    vector<vector<string>> rows = t.topLevels(levels);
    for (int lv = 0; lv < (int)rows.size(); lv++)
    {
        cout << "  Level " << lv << ":";
        for (int i = 0; i < (int)rows[lv].size(); i++)
            cout << ' ' << rows[lv][i];
        cout << '\n';
    }
}

static void printValidate(const AirportBST &t)
{
    cout << "validate: " << (t.validate() ? "OK" : "FAILED") << '\n';
}

static void printStats(const AirportBST &t)
{
    cout << "Height: " << t.height() << "  Average depth: " << t.averageDepth() << '\n';
}

static AirportBST *buildByInsert(const vector<Airport> &v, int &duplicates)
{
    AirportBST *t = new AirportBST();
    duplicates = 0;
    for (int i = 0; i < (int)v.size(); i++)
    {
        if (!t->insert(v[i]))
        {
            duplicates++;
            cout << "Duplicate code: " << v[i].iata << '\n';
        }
    }
    return t;
}

static void doLoadAirports(const string &file)
{
    vector<Airport> airports;
    int skipped = 0;
    if (!loadAirports(file, airports, skipped))
    {
        cout << "Error: cannot open " << file << '\n';
        return;
    }
    fileOrder = airports;
    database.clear();
    for (int i = 0; i < (int)airports.size(); i++)
        database[airports[i].iata] = airports[i];

    int dup = 0;
    delete tree;
    tree = buildByInsert(airports, dup);
    cout << "Loaded " << tree->size() << " airports (" << skipped
         << " lines skipped: no IATA code). Duplicates: " << dup << '\n';
    printStats(*tree);
}

static void doLoadPreorder(const string &file)
{
    ifstream in(file);
    if (!in)
    {
        cout << "Error: cannot open " << file << '\n';
        return;
    }
    AirportBST *t = new AirportBST();
    string code;
    int n = 0;
    while (getline(in, code))
    {
        if (!code.empty() && code.back() == '\r')
            code.pop_back();
        if (code.empty())
            continue;
        map<string, Airport>::iterator it = database.find(code);
        if (it == database.end())
        {
            cout << "Error: code " << code << " is not in the airport database "
                 << "(run 'load data/airports.dat' first)\n";
            delete t;
            return;
        }
        t->insert(it->second);
        n++;
    }
    delete tree;
    tree = t;
    cout << "Rebuilt tree from " << n << " codes in preorder.\n";
    printStats(*tree);
}

static bool isCodeList(const string &file)
{
    ifstream in(file);
    string first;
    if (!getline(in, first))
        return false;
    if (!first.empty() && first.back() == '\r')
        first.pop_back();
    return first.size() == 3 && first.find(',') == string::npos;
}

static bool doSave(const string &file)
{
    ofstream out(file);
    if (!out)
    {
        cout << "Error: cannot write " << file << '\n';
        return false;
    }
    vector<string> codes = tree->preorder();
    for (int i = 0; i < (int)codes.size(); i++)
        out << codes[i] << '\n';
    cout << "Saved " << codes.size() << " codes (preorder) to " << file << '\n';
    return true;
}

static bool doRemove(AirportBST &t, const string &code)
{
    RemoveResult r = t.remove(code);
    if (!r.found)
    {
        cout << code << " not found, nothing deleted\n";
        return false;
    }
    cout << "Deleted " << code << " (case " << r.description
         << (r.wasRoot ? "; it was the root" : "") << ") ";
    printValidate(t);
    return true;
}

static void doAdd(istringstream &in)
{
    Airport a;
    in >> a.iata;
    string rest;
    getline(in, rest);
    vector<string> parts;
    string cur;
    for (int i = 0; i < (int)rest.size(); i++)
    {
        if (rest[i] == ';')
        {
            parts.push_back(cur);
            cur.clear();
        }
        else
            cur += rest[i];
    }
    parts.push_back(cur);
    if (!parts[0].empty() && parts[0][0] == ' ')
        parts[0].erase(0, 1);
    a.name = parts[0];
    a.city = parts.size() > 1 ? parts[1] : "";
    a.country = parts.size() > 2 ? parts[2] : "";

    if (a.iata.size() != 3)
        cout << "Code must be exactly 3 letters\n";
    else if (tree->insert(a))
    {
        database[a.iata] = a;
        cout << "Added " << a.iata << " ";
        printValidate(*tree);
    }
    else
        cout << a.iata << " already exists, not added\n";
}

struct ShapeRow
{
    string name;
    int height;
    double avgDepth;
    double ms;
};

static ShapeRow measure(const string &name, const AirportBST &t, const vector<string> &queries)
{
    ShapeRow r;
    r.name = name;
    r.height = t.height();
    r.avgDepth = t.averageDepth();

    int found = 0;
    chrono::steady_clock::time_point t0 = chrono::steady_clock::now();
    for (int i = 0; i < (int)queries.size(); i++)
        if (t.contains(queries[i]))
            found++;
    chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
    r.ms = chrono::duration<double, milli>(t1 - t0).count();

    if (found != (int)queries.size())
        cout << "WARNING: " << name << " found only " << found << " of "
             << queries.size() << " queries\n";
    return r;
}

static void runExperiment()
{
    if (fileOrder.empty())
    {
        cout << "Load the data first.\n";
        return;
    }
    int n = (int)fileOrder.size();

    mt19937 rng(2560);
    vector<string> queries;
    for (int i = 0; i < 10000; i++)
        queries.push_back(fileOrder[rng() % n].iata);

    vector<Airport> sorted = fileOrder;
    sort(sorted.begin(), sorted.end(),
         [](const Airport &a, const Airport &b) { return a.iata < b.iata; });
    vector<Airport> shuffled = fileOrder;
    shuffle(shuffled.begin(), shuffled.end(), rng);

    int dup = 0;
    vector<ShapeRow> rows;

    AirportBST *t1 = buildByInsert(fileOrder, dup);
    rows.push_back(measure("File order", *t1, queries));
    delete t1;

    AirportBST *t2 = buildByInsert(sorted, dup);
    rows.push_back(measure("Sorted order", *t2, queries));
    delete t2;

    AirportBST *t3 = new AirportBST();
    t3->buildFromSorted(sorted);
    rows.push_back(measure("Balanced (middle rule)", *t3, queries));
    delete t3;

    AirportBST *t4 = buildByInsert(shuffled, dup);
    rows.push_back(measure("Random shuffle (bonus)", *t4, queries));
    delete t4;

    printf("\n%-24s %8s %14s %22s\n", "Build order", "Height", "Average depth", "10,000 searches (ms)");
    for (int i = 0; i < (int)rows.size(); i++)
        printf("%-24s %8d %14.2f %22.3f\n", rows[i].name.c_str(), rows[i].height,
               rows[i].avgDepth, rows[i].ms);
    printf("(floor(log2(%d)) = 12 is the best possible height)\n\n", n);
}

static int failures = 0;
static void check(bool ok, const string &what)
{
    cout << (ok ? "  PASS  " : "  FAIL  ") << what << '\n';
    if (!ok)
        failures++;
}

static void selfTest()
{
    failures = 0;
    cout << "== Task 1: load and build ==\n";
    doLoadAirports("data/airports.dat");
    check(tree->size() == 6072, "6072 airports inserted, 0 duplicates");
    check(tree->height() == 177, "height of file-order tree is 177");
    check(tree->inorderIsSorted(), "inorder is sorted");
    check(tree->validate(), "validate after build");

    cout << "== Task 2: search ==\n";
    vector<string> path;
    vector<vector<string>> top = tree->topLevels(1);
    string rootCode = top[0][0];
    string leaf, oneL, oneR, two;
    tree->findExamples(leaf, oneL, oneR, two);
    check(tree->find(rootCode, path) != nullptr && path.size() == 1, "root found with 1 comparison (" + rootCode + ")");
    bool leafFound = tree->find(leaf, path) != nullptr;
    check(leafFound, "leaf found (" + leaf + ", " + to_string(path.size()) + " comparisons)");
    check(tree->find("BOS", path) != nullptr && path.size() == 12, "deep code BOS found with 12 comparisons");
    check(tree->find("XYZ", path) == nullptr, "XYZ (does not exist) not found");
    check(tree->find("000", path) == nullptr, "000 (smaller than every key) not found");
    check(tree->find("ZZZ", path) == nullptr, "ZZZ (larger than every key) not found");
    check(tree->minimum()->iata == "AAA", "minimum is AAA");
    check(tree->maximum()->iata == "ZZV", "maximum is ZZV");

    cout << "== Task 3: delete (each case on a fresh tree) ==\n";
    struct Case
    {
        string label, code, prefix;
    };
    Case cases[] = {{"D1 leaf", leaf, "D1"},
                    {"D2 only left child", oneL, "D2: only a left"},
                    {"D2 only right child", oneR, "D2: only a right"},
                    {"D3 two children", two, "D3"},
                    {"root", rootCode, "D3"}};
    for (int i = 0; i < 5; i++)
    {
        int dup = 0;
        AirportBST *t = buildByInsert(fileOrder, dup);
        cout << " " << cases[i].label << ": " << cases[i].code << "  top 3 levels before:\n";
        printLevels(*t, 3);
        RemoveResult r = t->remove(cases[i].code);
        cout << "  top 3 levels after (" << r.description << "):\n";
        printLevels(*t, 3);
        check(r.found && r.description.compare(0, cases[i].prefix.size(), cases[i].prefix) == 0,
              cases[i].label + ": correct case reported");
        check(t->size() == 6071 && !t->contains(cases[i].code), cases[i].label + ": size 6071, code gone");
        check(t->validate() && t->inorderIsSorted(), cases[i].label + ": validate OK");
        delete t;
    }
    {
        int dup = 0;
        AirportBST *t = buildByInsert(fileOrder, dup);
        Airport q;
        q.iata = "QQQ";
        q.name = "Test Airport";
        q.city = "Boston";
        q.country = "United States";
        check(t->insert(q) && t->size() == 6073 && t->validate(), "add QQQ, validate OK");
        check(t->find("QQQ", path) != nullptr, "QQQ can be found");
        check(!t->insert(q), "adding QQQ again is rejected (duplicate)");
        check(t->remove("QQQ").found && t->size() == 6072 && t->validate(), "remove QQQ, validate OK");
        check(t->find("QQQ", path) == nullptr, "QQQ is gone");
        delete t;
    }

    cout << "== Task 4: traversals and save/load ==\n";
    {
        int dup = 0;
        AirportBST *t = buildByInsert(fileOrder, dup);
        vector<const Airport *> all = t->inorder();
        check(all.size() == 6072 && t->inorderIsSorted(), "list gives 6072 airports in alphabetical order");

        vector<string> pre = t->preorder();
        AirportBST *t2 = new AirportBST();
        for (int i = 0; i < (int)pre.size(); i++)
            t2->insert(database[pre[i]]);
        check(t2->height() == t->height(), "reloaded tree has the same height");
        check(t2->levelOrder() == t->levelOrder(), "reloaded tree has the same level order");
        check(t2->validate(), "reloaded tree validates");
        delete t2;
        delete t;
    }

    cout << "== Task 5: range ==\n";
    {
        int dup = 0, visited = 0;
        AirportBST *t = buildByInsert(fileOrder, dup);
        vector<const Airport *> r = t->range("BOS", "BWI", visited);
        check(r.size() == 113, "range BOS BWI returns 113 airports (" + to_string(visited) + " nodes visited)");
        delete t;
    }

    cout << "== Task 6: rebalance ==\n";
    {
        int dup = 0;
        AirportBST *t = buildByInsert(fileOrder, dup);
        t->rebalance();
        check(t->height() == 12 && t->size() == 6072 && t->validate(), "rebalance gives height 12");
        delete t;
    }

    cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
}


static void printHelp()
{
    cout << "Commands:\n"
            "  load FILE            FILE = airports.dat (build in file order)\n"
            "                       or a preorder list written by 'save' (rebuild the tree)\n"
            "  save FILE            write the codes in preorder\n"
            "  find CODE            search, show path and comparisons\n"
            "  min | max            smallest / largest code\n"
            "  list [N]             all airports in alphabetical order (or the first N)\n"
            "  range FROM TO        airports with FROM <= code <= TO\n"
            "  add CODE NAME;CITY;COUNTRY\n"
            "  remove CODE          delete an airport\n"
            "  testdelete CODE      top 3 levels, delete, top 3 levels again\n"
            "  cases                example code for each deletion case\n"
            "  levels [N]           top N levels (default 3)\n"
            "  validate             check the BST property\n"
            "  info                 size, height, average depth\n"
            "  rebalance            rebuild the tree balanced (middle rule)\n"
            "  experiment           Task 6 table (4 build orders, 10,000 searches)\n"
            "  selftest             run every test case of Tasks 1-6\n"
            "  quit\n";
}

int main()
{
    string line;
    cout << "Airport BST. Type 'help' for commands.\n";
    while (true)
    {
        cout << "> ";
        if (!getline(cin, line))
            break;
        istringstream in(line);
        string cmd;
        in >> cmd;
        if (cmd.empty())
            continue;

        if (cmd == "quit" || cmd == "exit")
            break;
        else if (cmd == "help")
            printHelp();
        else if (cmd == "load")
        {
            string file;
            in >> file;
            if (file.empty())
                file = "data/airports.dat";
            if (isCodeList(file))
                doLoadPreorder(file);
            else
                doLoadAirports(file);
        }
        else if (cmd == "save")
        {
            string file;
            in >> file;
            if (file.empty())
                file = "tree_preorder.txt";
            doSave(file);
        }
        else if (cmd == "find")
        {
            string code;
            in >> code;
            vector<string> path;
            const Airport *a = tree->find(code, path);
            cout << "Path:";
            for (int i = 0; i < (int)path.size(); i++)
                cout << (i == 0 ? " " : " -> ") << path[i];
            if (a == nullptr)
                cout << (path.empty() ? " " : " -> ") << "(NULL) not found (comparisons: "
                     << path.size() << ")\n";
            else
            {
                cout << " (comparisons: " << path.size() << ")\n";
                printAirport(*a);
            }
        }
        else if (cmd == "min" || cmd == "max")
        {
            const Airport *a = (cmd == "min") ? tree->minimum() : tree->maximum();
            if (a == nullptr)
                cout << "Tree is empty\n";
            else
                printAirport(*a);
        }
        else if (cmd == "list")
        {
            int limit = -1;
            in >> limit;
            vector<const Airport *> all = tree->inorder();
            int n = (int)all.size();
            if (limit >= 0 && limit < n)
                n = limit;
            for (int i = 0; i < n; i++)
                printAirport(*all[i]);
            cout << "(" << n << " of " << all.size() << " airports shown; sorted: "
                 << (tree->inorderIsSorted() ? "yes" : "NO") << ")\n";
        }
        else if (cmd == "range")
        {
            string from, to;
            in >> from >> to;
            int visited = 0;
            vector<const Airport *> r = tree->range(from, to, visited);
            for (int i = 0; i < (int)r.size(); i++)
                printAirport(*r[i]);
            cout << r.size() << " airports, " << visited << " nodes visited\n";
        }
        else if (cmd == "add")
            doAdd(in);
        else if (cmd == "remove")
        {
            string code;
            in >> code;
            doRemove(*tree, code);
        }
        else if (cmd == "testdelete")
        {
            string code;
            in >> code;
            cout << "Before:\n";
            printLevels(*tree, 3);
            doRemove(*tree, code);
            cout << "After:\n";
            printLevels(*tree, 3);
        }
        else if (cmd == "cases")
        {
            string leaf, onlyLeft, onlyRight, two;
            tree->findExamples(leaf, onlyLeft, onlyRight, two);
            vector<vector<string>> top = tree->topLevels(1);
            cout << "D1 leaf:             " << leaf << '\n';
            cout << "D2 only left child:  " << onlyLeft << '\n';
            cout << "D2 only right child: " << onlyRight << '\n';
            cout << "D3 two children:     " << two << '\n';
            if (!top.empty())
                cout << "Root:                " << top[0][0] << '\n';
        }
        else if (cmd == "levels")
        {
            int n = 3;
            in >> n;
            printLevels(*tree, n);
        }
        else if (cmd == "validate")
            printValidate(*tree);
        else if (cmd == "info")
            cout << "Size: " << tree->size() << "  ", printStats(*tree);
        else if (cmd == "rebalance")
        {
            tree->rebalance();
            printStats(*tree);
            printValidate(*tree);
        }
        else if (cmd == "experiment")
            runExperiment();
        else if (cmd == "selftest")
            selfTest();
        else
            cout << "Unknown command. Type 'help'.\n";
    }
    delete tree;
    return 0;
}
