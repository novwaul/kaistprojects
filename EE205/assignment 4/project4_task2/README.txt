####################### Graph.txt ########################

# List of node name

Node
DFW
DEN
ORD
STL
PIT
DCA
ATL

# List of edges (node1, node2, edge value)
# You can set edge name same as edge value

Edge
DFW DEN 4
DEN ORD 1
ORD STL 6
DFW STL 8
DEN STL 9
ORD PIT 10
STL DCA 3
STL ATL 5
ATL DCA 2
PIT DCA 7

####################### shortest_path_task.txt ########################

# List of shortest path task (start node, destication node)

DFW ORD
ATL PIT

####################### ans_SP.txt ########################

# List of answer to shortest_path_task.txt (all visited node sequence, total length)

DFW DEN ORD 5
ATL DCA PIT 9

####################### ans_MSP.txt ########################

# MST, short link first, alphabetical previous node first

DEN ORD 1
ATL DCA 2
DCA STL 3
DEN DFW 4
ORD STL 6
DCA PIT 7
