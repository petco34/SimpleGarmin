#ifndef SIMPLEGARMIN_H_INCLUDED
#define SIMPLEGARMIN_H_INCLUDED

#include <string.h>
#include <stdlib.h>


#define White 'w'
#define Gray 'g'
#define Black 'b'
#define Infinity 99999

// What an edge looks like in the database
typedef struct gEdge {
    int child;                      // child vertex index
    MapDatabase weight;             // database for the edge 
    unsigned int weightToUse;       // the weight the Garmin uses
    struct gEdge *nextEdge;         // link to the next edge for this vertex
} GEdge, *GEdgePtr;

// What a vertex looks like in the database
typedef struct {
    char id[MaxVertexCharacters+1]; // vertex name
    char colour;                    // flag used when traversing
    int parent;                     // index of the parent to this vertex
    unsigned int cost;              // accumulated cost from this vertex
    GEdgePtr firstEdge;             // pointer to the vertex start edge list
} GVertex;

// What the graph itself looks like
typedef struct graph {
    int numV;                       // the number of vertices in the graph
    GVertex vertex[MaxNumOfVertices+1]; // array of vertices (see GVertex)
} *Graph;

// function prototypes
void buildGraph(FILE * in, Graph G, MapDatabase (*ReadFileData)(FILE*));
void initSingleSource(Graph, int, unsigned int (*GetWeight) (MapDatabase WD), char[]);
void siftDown(Graph, int, int[], int, int, int[]);
void siftUp(Graph, int[], int, int[]);
void printCostPath(Graph);
void MySimpleGarmin(Graph, char[], unsigned int (*SetWeight) (MapDatabase WD), char[]);
Graph newGraph(int);
void followPath(Graph, int);
int findStartIndexOrExit (Graph, char[]);
GEdgePtr newGEdge(int, MapDatabase);
GVertex newGVertex(char[]);
void addEdge(char[], char[], MapDatabase, Graph);
Graph BuildGarminDataBase( char DataBaseFile[], MapDatabase (*ReadFileData)(FILE*));

// MySimpleGarmin is an implementation of Dijkstra's algorithm
// It is based on the Kalicharan implementation, with
//      modifications for reading in data and printing it
void MySimpleGarmin(Graph G, char startVertex[], unsigned int (*GetWeight) (MapDatabase WD), char msg[])
{
    // internally, processing is done with respect to a
    // source vertex.  Locate the index in the
    // graph, exiting if it does not exist
    int s = findStartIndexOrExit(G, startVertex);
    int heap[MaxNumOfVertices + 1], heapLoc[MaxNumOfVertices + 1];
    //heapLoc[i] gives the position in heap of vertex i
    //if heapLoc[i] = k, then heap[k] contains i

    initSingleSource(G, s, GetWeight, msg);
    int i;
    for (i = 1; i <= G -> numV; i++) heap[i] = heapLoc[i] = i;
    heap[1] = s;
    heap[s] = 1;
    heapLoc[s] = 1;
    heapLoc[1] = s;
    int heapSize = G -> numV;
    while (heapSize > 0) {
        int u = heap[1];
        if (G -> vertex[u].cost == Infinity) break; //no paths to other vertices
        //reorganize heap after removing top item
        siftDown(G, heap[heapSize], heap, 1, heapSize-1, heapLoc);
        GEdgePtr p = G -> vertex[u].firstEdge;
        while (p != NULL) {
            if (G -> vertex[u].cost + p ->weightToUse < G -> vertex[p -> child].cost) {
                G -> vertex[p -> child].cost = G -> vertex[u].cost + p -> weightToUse;
                G -> vertex[p -> child].parent = u;
                siftUp(G, heap, heapLoc[p -> child], heapLoc);
            }
            p = p -> nextEdge;
        }
        --heapSize;
    } //end while

    printCostPath(G);
} //end MySimpleGarmin


// the database builder
// It receives a filename and verifies it can be opened.
// If so, it reads the first line (number of vertices) and passes
//      it on to buildGraph to construct the entire database from
//      the file
// The routine will force a program exit if it cannot open the
// file.  Otherwise, very limited handling of a bad database will be done.
Graph BuildGarminDataBase( char DataBaseFile[], MapDatabase (*ReadFileData)(FILE *))
{
    // open the file and exit if it fails
    int numVertices = 0;
    Graph tempGraph;
    // open the data file and build the graph
    FILE * in = fopen(DataBaseFile, "r");
    if (in == NULL) {
        printf ("Unable to open file \"%s\"", DataBaseFile);
        exit(0);
    }
    // get the first piece of data (the number of vertices),
    // exiting if the value is less than 1.
    // otherwise, allocate the graph using the size and
    //      call buildGraph to read in the database for
    //      the number of vertices
    fscanf(in, "%d", &numVertices);
    if (numVertices < 1)
    {
        printf ("File \"%s\" has too few vertices!  Exiting", DataBaseFile);
        exit(0);
    }
    // Good enough to start, so allocate the graph
    tempGraph = newGraph(numVertices);
    // Build the database into the allocated graph
    buildGraph(in, tempGraph, ReadFileData);
    // Done with the file, so close it
    fclose(in);
    // return the graph just built
    return tempGraph;
}

//Find the index that is startVertex, if it doesn't find it, it will exit the program
int findStartIndexOrExit (Graph G, char startVertex[])
{
    int s = 1;
    while (s <= G->numV) {
        if (!strcmp (startVertex, G->vertex[s].id))
            return s;
        else
            s++;
    }
    printf ("Unable to find \"%s\" in the graph.  Exiting.\n", startVertex);
    exit(0);
}
void initSingleSource(Graph G, int s, unsigned int (*GetWeight) (MapDatabase WD), char msg[])
{
    int i;
    for (i = 1; i <= G -> numV; i++) {
        G -> vertex[i].cost = Infinity;
        G -> vertex[i].parent = 0;
        GEdgePtr ep = G -> vertex[i].firstEdge;
        while (ep != NULL)
        {
             ep->weightToUse = GetWeight(ep->weight);
             ep = ep-> nextEdge;
        }
     }
    G -> vertex[s].cost = 0;
    printf ("From %s %s to...\n", G->vertex[s].id, msg);
} //end initSingleSource

void printCostPath(Graph G)
{
    int i;
    for (i = 1; i <= G -> numV; i++) {
        printf("   %-15s is %5d  \050", G -> vertex[i].id, G -> vertex[i].cost);
        followPath(G, i);
        printf("\051\n");
    }
} //end printCostPath


void followPath(Graph G, int c)
{
    if (c != 0) {
        followPath(G, G -> vertex[c].parent);
        if (G -> vertex[c].parent != 0) printf("->"); //do not print -> for source
        printf("%s", G -> vertex[c].id);
    }
} //end followPath

void siftUp(Graph G, int heap[], int n, int heapLoc[])
{
    //sifts up heap[n] so that heap[1..n] contains a heap based on cost
    int siftItem = heap[n];
    int child = n;
    int parent = child / 2;
    while (parent > 0) {
        if (G->vertex[siftItem].cost >= G->vertex[heap[parent]].cost) break;
        heap[child] = heap[parent]; //move down parent
        heapLoc[heap[parent]] = child;
        child = parent;
        parent = child / 2;
    }
    heap[child] = siftItem;
    heapLoc[siftItem] = child;
} //end siftUp

void siftDown(Graph G, int key, int heap[], int root, int last, int heapLoc[])
{
    int smaller = 2 * root;
    while (smaller <= last) { //while there is at least one child
        if (smaller < last) //there is a right child as well; find the smaller
            if (G->vertex[heap[smaller+1]].cost < G->vertex[heap[smaller]].cost)
                smaller++;
        //'smaller' holds the index of the smaller child
        if (G -> vertex[key].cost <= G -> vertex[heap[smaller]].cost) break;
        //cost[key] is bigger; promote heap[smaller]
        heap[root] = heap[smaller];
        heapLoc[heap[smaller]] = root;
        root = smaller;
        smaller = 2 * root;
    } //end while
    heap[root] = key;
    heapLoc[key] = root;
} //end siftDown

Graph newGraph(int n)
{
    if (n > MaxNumOfVertices) {
        printf("\nToo big. Only %d vertices allowed.\n", MaxNumOfVertices);
        exit(1);
    }
    Graph p = (Graph) malloc(sizeof(struct graph));
    p -> numV = n;
    return p;
} //end newGraph

// buildGraph is in charge of creating the Gramin database.
// It receives the open file handle, the graph to fill and
// the user's file reader function.
//
// processing consists of;
//  1. Reading in all of the verticex names
//  2. For each vertex, reading in the number of edges
//  3  For each edge,
//  3.1  Read in the destination vertex index
//  3.2  Call the user's file reader to read in the user's edge information
//  3.3  Call addEdge to add the edge data in the linked list
//       of the vertex being processed (see step 2)
void buildGraph(FILE * in, Graph G, MapDatabase (*ReadFileData)(FILE*) )
{
    int numEdges;
    char nodeID[MaxVertexCharacters+1], adjID[MaxVertexCharacters+1];
    int h;
    for (h = 1; h <= G -> numV; h++) {
        G -> vertex[h] = newGVertex("");      //create a vertex node
        fscanf(in, "%s", G -> vertex[h].id);   //read the name into id
    }
    for (h = 1; h <= G -> numV; h++) {
        fscanf(in, "%s %d", nodeID, &numEdges); //parent id and numEdges
        int k;

        for (k = 1; k <= numEdges; k++) {
            fscanf (in, "%s", adjID);           // get the child id
            // call the user back to load the database
            MapDatabase tempMDB = ReadFileData(in);
            // add the edge to the linked list of edges for a vertex
            addEdge(nodeID, adjID, tempMDB, G);
        }
    }
    return;
} //end buildGraph

GVertex newGVertex(char name[])
{
    GVertex temp;
    strcpy(temp.id, name);
    temp.firstEdge = NULL;
    return temp;
}

void addEdge(char X[], char Y[], MapDatabase MDB, Graph G)
{
    //add an edge X -> Y with a given weight
    int h, k;
    //find X in the list of nodes; its location is h
    for (h = 1; h <= G -> numV; h++) if (strcmp(X, G -> vertex[h].id) == 0) break;

    //find Y in the list of nodes; its location is k
    for (k = 1; k <= G-> numV; k++) if (strcmp(Y, G -> vertex[k].id) == 0) break;

    if (h > G -> numV || k > G -> numV) {
        printf("No such edge: %s -> %s\n", X, Y);
        exit(1);
    }
    GEdgePtr ep = newGEdge(k, MDB); //create edge vertex
    // add it to the list of edges, possible empty, from X;
    // it is added so that the list is in order by vertex id
    GEdgePtr prev, curr;
    prev = curr = G -> vertex[h].firstEdge;
    while (curr != NULL && strcmp(Y, G -> vertex[curr -> child].id) > 0) {
        prev = curr;
        curr = curr -> nextEdge;
    }

    if (prev == curr) {
        ep -> nextEdge = G -> vertex[h].firstEdge;
        G -> vertex[h].firstEdge = ep;
    } else {
        ep -> nextEdge = curr;
        prev -> nextEdge = ep;
    }
} //end addEdge

GEdgePtr newGEdge(int c, MapDatabase MDB)
{
    //return a pointer to a new GEdge node
    GEdgePtr p = (GEdgePtr) malloc(sizeof (GEdge));
    p -> child = c;
    p ->weight = MDB;
    p -> nextEdge = NULL;
    return p;
}

#endif // SIMPLEGARMIN_H_INCLUDED
