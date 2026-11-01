# Pseudocode (M1)

## Load and split
```
function splitCSV(line):
    fields = empty list; cur = ""; inQuotes = false
    for i = 0 to length(line) - 1:
        c = line[i]
        if inQuotes:
            if c == '"':
                if next char is '"': append '"' to cur; i = i + 1   // escaped quote
                else: inQuotes = false
            else: append c to cur
        else:
            if c == '"': inQuotes = true
            else if c == ',': add cur to fields; cur = ""
            else: append c to cur
    add cur to fields
    return fields

function loadAirports(file):
    for each line in file:
        f = splitCSV(line)
        if f[4] is not exactly 3 letters (e.g. \N): skipped++; continue
        make Airport from f[1..7]; add to array
```

## Build (by repeated insert, file order)
```
function build(array):
    tree = empty
    for i = 0 to n - 1:
        if not tree.insert(array[i]): report duplicate
```

## Insert (iterative)
```
function insert(a):
    if root is NULL: root = new node(a); return true
    cur = root
    while true:
        if a.code < cur.code:
            if cur.left is NULL: cur.left = new node(a); return true
            cur = cur.left
        else if a.code > cur.code:
            if cur.right is NULL: cur.right = new node(a); return true
            cur = cur.right
        else: return false          // duplicate (rule I4)
```

## Height and average depth (level order with a queue)
```
function height():
    if root is NULL: return -1
    queue = [root]; levels = 0
    while queue not empty:
        repeat size(queue) times: pop node, push its children
        levels = levels + 1
    return levels - 1

function averageDepth():
    queue = [(root, 0)]; sum = 0
    while queue not empty:
        (node, d) = pop; sum = sum + d
        push (left, d+1) and (right, d+1) if they exist
    return sum / n
```

# Pseudocode (M2)

## Search (iterative, records the path)
```
function find(code):
    path = empty list; cur = root
    while cur is not NULL:
        append cur.code to path
        if code < cur.code: cur = cur.left
        else if code > cur.code: cur = cur.right
        else: return cur            // found; comparisons = length(path)
    return NULL                     // not found
```

## Minimum / maximum
```
function minimum(): cur = root; while cur.left is not NULL: cur = cur.left; return cur
function maximum(): cur = root; while cur.right is not NULL: cur = cur.right; return cur
```

## Delete (link = the pointer that leads to the node: root or a parent's left/right)
```
function remove(code):
    link = address of root
    while *link is not NULL and (*link).code != code:
        if code < (*link).code: link = address of (*link).left
        else: link = address of (*link).right
    if *link is NULL: return not found
    node = *link
    if node has no children:                    // D1 leaf
        *link = NULL; delete node
    else if node has only a left child:         // D2
        *link = node.left; delete node
    else if node has only a right child:        // D2
        *link = node.right; delete node
    else:                                       // D3 two children
        sl = address of node.right
        while (*sl).left is not NULL: sl = address of (*sl).left
        succ = *sl                              // smallest key in right subtree
        copy succ's data into node
        *sl = succ.right                        // succ has no left child
        delete succ
    n = n - 1
```
The root needs no special case: when node is the root, `link` is the root pointer itself.

## Validate (range method)
```
function validate():
    stack = [(root, no lower bound, no upper bound)]
    while stack not empty:
        (node, lo, hi) = pop
        if lo exists and node.code <= lo: return false
        if hi exists and node.code >= hi: return false
        push (node.left,  lo,        node.code)
        push (node.right, node.code, hi)
    return true
```

# Pseudocode (M3)

## List (inorder, iterative)
```
function inorder():
    stack = empty; cur = root; out = empty list
    while cur is not NULL or stack not empty:
        while cur is not NULL: push cur; cur = cur.left
        cur = pop
        append cur to out
        cur = cur.right
    return out            // codes come out in alphabetical order
```
Sorted check: `for i = 1 to n - 1: if out[i-1].code >= out[i].code: return false`.

## Save / load (preorder)
```
function preorder():
    stack = [root]; out = empty
    while stack not empty:
        cur = pop; append cur.code to out
        push cur.right (if any)       // right first, so left is popped first
        push cur.left (if any)
    return out

function save(file):  write preorder() to file, one code per line
function load(file):
    tree = empty
    for each code in file (in file order):
        tree.insert(record of code)   // records come from the airport database
```
Why the shape is identical: in preorder every node comes before its descendants, and each
node is inserted into the same place because all its ancestors are already there.
Check: same height and same level order (a level order determines a BST uniquely).

## Level order
```
function levelOrder():
    queue = [root]
    while queue not empty:
        cur = pop front; append cur.code to out
        push cur.left, cur.right (if they exist)
```

# Pseudocode (M4)

## Range query FROM..TO (inorder with pruning)
```
function range(from, to):
    stack = empty; cur = root; visited = 0
    while cur is not NULL or stack not empty:
        while cur is not NULL:
            visited = visited + 1
            if cur.code < from:           // cur and its left subtree are all below FROM
                cur = cur.right
            else:
                push cur
                if cur.code > from: cur = cur.left   // left keys are smaller than cur
                else: cur = NULL                     // cur == from: nothing smaller is in range
        if stack empty: break
        cur = pop
        if cur.code > to: break           // inorder: everything after this is larger too
        append cur to result
        if cur.code < to: cur = cur.right else: cur = NULL
    return result, visited
```
Subtrees entirely below FROM or above TO are never entered, so the cost is O(h + k).

## Balanced build from the sorted array
```
function buildBalanced(a, lo, hi):
    if lo > hi: return NULL
    mid = (lo + hi) / 2
    node = new node(a[mid])
    node.left  = buildBalanced(a, lo, mid - 1)
    node.right = buildBalanced(a, mid + 1, hi)
    return node
rebalance(): a = inorder() copy; root = buildBalanced(a, 0, n - 1)
```
Recursion depth is only about log2(n) = 13, so it cannot overflow the stack.

## Shape experiment
```
queries = 10,000 random codes from the file (fixed seed)
for each build order in [file order, sorted, balanced, random shuffle]:
    build the tree; record height and average depth
    start timer; for each query: contains(query); stop timer
```
