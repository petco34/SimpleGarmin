// This is the Simple Garmin demo
// For the database defined in "testfile", it will construct
// the database and then compute the best path to all places in
// the database from the starting point given.
//
// It uses Dijkstra's algorithm to apply non-negative weights
// to determine the shortest path
//
// Weight is dependent on the travel criteria, which is resolved by the
// function passed to the SimpleGarmin.  The function will be called
// once for each edge, being passed the edge structure.  It must return
// the weight to be used.
#include <stdio.h>
// A place "vertex" should have no spaces in it and should be less than the following
#define MaxVertexCharacters 20
// The maximum number of vertices that will be in the database
#define MaxNumOfVertices 50
// Definition of the data in the Garmin database for each edge
// The SimpleGarmin uses the function ReadEdgeData to read in this
// data, so this struct and ReadEdgeData MUST be kept in sync!!
typedef struct {
    unsigned int distance;      // distance data for the edge in miles from the data file
    } MapDatabase;
// Code to perform a Dijkstra analysis for the Simple Garmin
#include "SimpleGarmin.h"
// test files supported with the code
#define testfile "CountryDriving.txt"
// It is the user's responsibility to read in the data for an edge
// The data corresponds to the fields in a MapDatabase structure.
// The function is called by the Garmin to form a MapDatabase object
// and read in it's data from the open file provided.  on return, the
// Garmin will use the object to fill in the Garmin's database.
MapDatabase ReadEdgeData (FILE * in)
{
    // create a MapDatabase structure to fill with data
    MapDatabase MDB;
    // fill in the data
    fscanf (in, "%d", &MDB.distance);
    // return the filled in struct
    return (MDB);
}
////////
// Weight determination based on distance follows...
// This callback is a function that receives a copy of the database for
// an edge in the Garmin database and must return the weight to be
// used in the analysis for the edge.
//
// Some criteria can be more Simple than others, but here is a simple one
// that is used if you want the criteria to be distance.  It simple says "use the
// distance from the database".
//
// For a more Simple criteria (like avoiding tolls), the routine would have
// to return either the distance in the structure or a very large distance (99999)
// if the road should be avoided because it is a toll road.
///////////
unsigned int MinimumDistance (MapDatabase WD)
{
    return WD.distance;
}
///////
// The test consists of loading the Garmin Database and asking for distances
//      from a starting point
// Processing consists of:
//      1. Building the data base (it will terminate if it has difficulty)
//      2. Printing out the database contents so you can see if it looks other
//         than you expect (optional, by uncommenting the 2 lines shown)
//      3. Calling for lowest cost (distances) from one place to all other places
//
//////
int main()
{
    // Build the Garmin DataBase
    // Given the file name and the function to read in the user piece of the database,
    // BuildGarminDataBase will return its graph
    Graph G = BuildGarminDataBase(testfile, ReadEdgeData);
    // Compute distances from several starting points.
    // NOTE: MySimpleGarmin gets:
    //       the graph to use,
    //       the name of the place to use as the start point
    //       the routine that provides the weight for an edge, given the edge data structure
    //       a text message to accompany the output logged when MySimpleGarmin finishes
    MySimpleGarmin(G, "NewYork", MinimumDistance, "(mileage, using minimum distance)");
    MySimpleGarmin(G, "Fargo", MinimumDistance, "(mileage, using minimum distance)");
    MySimpleGarmin(G, "Orlando", MinimumDistance, "(mileage, using minimum distance)");
    return 0;
} // end main
