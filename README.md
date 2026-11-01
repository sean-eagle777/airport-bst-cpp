# airport-bst-cpp: Airport Code Lookup Service with a Binary Search Tree

EECE 2560, Fundamentals of Engineering Algorithms, Mini Project.
Author: *shang-yun hsieh*

## The problem

Every booking, boarding pass and baggage tag identifies an airport by its three-letter IATA
code (BOS, JFK, DAC). This program is a lookup service: given a code it returns the airport's
name, city, country and location, it stays correct while airports are added and removed, and it
supports alphabetical listings and range queries ("every code from BOS to BWI").
It is built on my own binary search tree, keyed by IATA code, loaded with real airport data.
Every operation is iterative (while loops), so even a 6,071-deep chain cannot overflow the stack.

## Data

Airport data from [OpenFlights.org](https://openflights.org/data.php) (`airports.dat`),
published under the Open Database License. 7,698 lines; 6,072 have a 3-letter IATA code
(the other 1,626 contain `\N` and are skipped); all 6,072 codes are different.
The file is in `data/airports.dat`.

## Build and run

```
g++ -std=c++17 -O2 src/*.cpp -o airports
./airports                 # run from the project root (the data path is data/airports.dat)
```

Run it from the project root folder. On Windows use `.\airports.exe`.

## Files

| File | Purpose |
|---|---|
| `src/Airport.h` | `Airport` record class |
| `src/AirportBST.h/.cpp` | `AirportBST` class (root is private): insert, find, remove, validate, traversals, range, balanced build |
| `src/Loader.h/.cpp` | CSV line splitter (handles commas inside quotes) and file loader |
| `src/main.cpp` | command line interface, shape experiment, self test |
| `docs/pseudocode.md` | pseudocode for every operation |
| `docs/sample_session.txt` | output of `./airports < docs/demo_commands.txt` |

## Commands

```
load FILE          airports.dat (build in file order) or a preorder list made by 'save'
save FILE          write the codes in preorder
find CODE          search; prints the path and number of comparisons
min | max          smallest / largest code
list [N]           alphabetical listing (inorder), checks it is sorted
range FROM TO      airports with FROM <= code <= TO, and nodes visited
add CODE NAME;CITY;COUNTRY
remove CODE        delete (reports case D1, D2 or D3)
testdelete CODE    top 3 levels before and after a delete
cases              one example code for each deletion case
levels [N]         top N levels
validate           BST property check with the (lo, hi) range method
info               size, height, average depth
rebalance          rebuild balanced (middle element = root)
experiment         the Task 6 table
selftest           runs every test case below and prints PASS / FAIL
```

## Sample session

```
> load data/airports.dat
Loaded 6072 airports (1626 lines skipped: no IATA code). Duplicates: 0
Height: 177  Average depth: 18.3233
> find BOS
Path: GKA -> AEY -> EGS -> BUJ -> BJA -> BSK -> BOY -> BNI -> BOH -> BOO -> BOU -> BOS (comparisons: 12)
BOS General Edward Lawrence Logan International Airport, Boston, United States
> find XYZ
Path: GKA -> MAG -> POM -> ... -> XYA -> XYE -> (NULL) not found (comparisons: 14)
> range BOS BWI
113 airports, 127 nodes visited
> remove AEY
Deleted AEY (case D3: two children, replaced by successor AFA) validate: OK
> rebalance
Height: 12  Average depth: 10.6529
```

`selftest` runs every case from the assignment (each search case, each delete case on a fresh
tree with the top three levels printed before and after, add/remove of a made-up airport QQQ,
save/load shape check, `range BOS BWI` = 113, rebalance height 12) and calls `validate`
after every insert and delete. All tests pass.

## Task 6: the shape experiment

Same 6,072 codes, four ways of building the tree, then 10,000 random searches (fixed seed, so
all trees get the same queries). Times are from one machine at `-O2` and vary between runs.

| Build order | Height | Average depth | Time for 10,000 searches | Explanation |
|---|---|---|---|---|
| File order | 177 | 18.32 | about 2.2 ms | The file is not random: it is made of blocks that are partly sorted, which create long chains (see below). |
| Sorted order | 6071 | 3035.50 | about 250 ms | Every new key is the largest so far and goes to the right: the tree is a linked list (height n - 1), and a search is a linear scan. |
| Balanced (middle rule) | 12 | 10.65 | about 1.9 ms | The middle element is the root of every subtree, so both sides always have almost the same size: height floor(log2 6072) = 12, the minimum possible. |
| Random shuffle (bonus) | 28 | 14.40 | about 2.3 ms | A random order gives a tree that is balanced on average, height about 2 to 3 times log2 n. |

**Why is the file order (177) so much worse than a random shuffle (about 28)?**
The file is ordered by OpenFlights airport ID, not randomly. Airports were added in groups
(a country or region at a time), and inside a group they tend to follow the order of the
source list, which is roughly sorted by name or ICAO code. I measured this on the file:

* 57.8% of neighbouring lines are in increasing code order (a random order gives 50%).
* There are 11 stretches of 10 or more consecutive increasing codes; the longest has **162**.
* 81% of neighbouring lines are in increasing ICAO order, and IATA codes follow ICAO codes closely.

Inserting an increasing stretch builds a chain, the same thing that happens in the sorted case
but on a smaller scale. A random shuffle destroys these runs, so a long chain almost never
forms. The height is the length of the single longest chain, so a few of these stretches are
enough to push the height from about 28 to 177.

Notice also that the *average* depth is only 18.3 versus 14.4 for the shuffle, and the time for
10,000 searches is almost the same (about 2.2 ms vs 2.3 ms). Most nodes are not on the long
chains, so the typical search is fine; the height measures the worst search. The sorted tree is
different: its average depth is 3035, and it is about 100 times slower. Every BST operation costs
O(h), so what matters is how large h is, and the middle-element build gives the best h.

## Credit

Airport data: **OpenFlights.org**, [Open Database License](https://opendatacommons.org/licenses/odbl/1-0/).
