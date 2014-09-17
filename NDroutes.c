// Eric Krakowiak
// Nicholas LaRosa
//
// FINAL PROJECT
//
// NDroutes.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "NDroutes.h"		// contains our prototypes
#include "gfx.h"

#define RATIO 	.001444444	// this is the ratio of miles per pixel
#define RADIUS 	5		// radius (in pixels) of our nodes on the map
#define PI	3.14159
#define ERROR 	.10		// the maximum deviation a path is allowed from the requested distance
#define PERCENT .6		// the point at which the path turns around

int
main (int argc, char *argv[])
{
  if (argc != 2)		// make sure we have the correct argument structure
    {
      printf ("Usage: %s <node_file>\n", argv[0]);
      return 1;
    }

  char nodeFileName[100];
  strcpy (nodeFileName, argv[1]);	// put user argument into nodeFileName string

  // allocate memory for the provided node file

  int numberNodes = countNodes (nodeFileName);	// by counting nodes, we can establish sizes of ourNodes, branches, and coordinates
  Node *ourNodes = malloc (10 * numberNodes * sizeof (Node));	// this acts like multidimensional array - store the nodes in our file
  int *branches = malloc (numberNodes * sizeof (int));	// store the number of branches at each node
  int *coordinates = malloc (2 * numberNodes * sizeof (int));	// this acts like multidimensional array - sore the coordinates of each file

  // variables

  int nodePath[100];		// will hold the nodes that the current route passes
  int totalNodesPath;		// number of nodes in final path

  double userDistance;		// ask the user for their desired distance
  int userMouseX;
  int userMouseY;
  char userInput;
  int userStart;		// user's starting and ending points
  int userEnd;
  int gotFirst = 0;
  int gotSecond = 0;

  createNodes (nodeFileName, ourNodes, branches, coordinates);

  srand (time (NULL));

  // create menu

  printf
    ("\nWelcome to the Official Running App of Notre Dame Cross Country!\n");
  printf
    ("\nInstructions:\n(1) Give desired route distance\n(2) Select your route's starting and ending points\n(3) Enjoy the provided route!\n\n");

  userDistance = getUserInput ();

  // prepare graphics window for user start/end selection

  gfx_open (900, 800, "ND Routes");

  while ( !gotSecond )
    {

      drawBranches (ourNodes, coordinates, branches, 0, 0, numberNodes, 0);
	drawNodes (coordinates, numberNodes);

      if (gfx_event_waiting ())
	{

	  userInput = gfx_wait ();
	  if (userInput == 1 && !gotFirst)
	    {
	      userMouseX = gfx_xpos ();
	      userMouseY = gfx_ypos ();
	      userStart = selectStart (coordinates, numberNodes, userMouseX, userMouseY);	// wait for first mouse click
	      gotFirst = 1;
	    }
	  else if (userInput == 1 && gotFirst)
	    {
	      if (userInput == 1 && !gotSecond)
		{
		  userMouseX = gfx_xpos ();
		  userMouseY = gfx_ypos ();
		  userEnd = selectEnd (coordinates, numberNodes, userMouseX, userMouseY, userStart);	// wait for second mouse click
		  gotSecond = 1;

		  totalNodesPath =
		    findCorrectPath (nodePath, ourNodes, userStart, userEnd,
				     userDistance, branches, coordinates);

		  if (totalNodesPath == 0)	// given 100 attempts to find route, exit if unable
		    {
		      printf
			("\nA route could not be found. Please input a longer path distance.\n\n");
		      return;
		    }
		}
	    }

	  else if (userInput == 'q')
	    {
	      break;
	    }
	}
    }

  printPath (nodePath, coordinates, totalNodesPath + 1);
	usleep(10000);

  while (1)
    {

      drawNodes (coordinates, numberNodes);
      drawPath (nodePath, coordinates, totalNodesPath + 1);

      userInput = gfx_wait ();

      if (userInput == 'q')
	{
	  break;
	}


    }
}

int
nodeIndex (int node, int branch)	// converts indexes to seem like multidimensional arrays
{
  return (10 * node) + branch;
}

int
coordIndex (int node, int coordinate)	// converts indexes to seem like multidimensional arrays
{
  return (2 * node) + coordinate;
}

double
getUserInput (void)		// returns the path 
{
  int n;
  double userInput;

  printf
    ("Please enter the distance you would like to travel, between 0 and 5 miles: ");
  n = scanf ("%lf", &userInput);

  if (n != 1 || userInput <= 0 || userInput > 5)
    {
      printf ("Please try again.\n");
      return getUserInput ();
    }

  return userInput;
}

int
countNodes (char *fileName)	// this function will return the size of array necessary to hold our node information
{
  int count = 0;		// keep count of our nodes
  int n;
  char buffer[4096];		// buffer to hold current string

  FILE *ourFile = fopen (fileName, "r");	// each line should contain start and end nodes, distance (double)

  if (ourFile == NULL)		// error - check
    {
      printf ("File '%s' not found.\n", fileName);
      return 0;
    }

  while (fgets (buffer, sizeof (buffer), ourFile) != NULL)
    {
      n = sscanf (buffer, "%d", &count);	// we only care about the first digit, which corresponds to the node index

      if (n != 1)
	{
	  printf ("Incorrect file format.\n");
	}
    }

  fclose (ourFile);

  return count + 1;		// zero - indexed, add one
}

void
createNodes (char *fileName, Node ourNodes[], int branches[], int coordinates[])	// this function will fill in the node array with the file's data
{
  char buffer[4096];
  int n;

  int currentX;			// each line contains start, end node, length between the two, and (x,y) of start
  int currentY;

  int currentNode;		// these are the variables that we must search the string for
  int endNode;
  double currentLength;

  int previousNode = 0;
  int branchIndex = -1;		// keep track of the number of branches from each node

  FILE *ourFile = fopen (fileName, "r");

  if (ourFile == NULL)		// error - check
    {
      printf ("File '%s' not found.\n", fileName);
      return;
    }

  while (fgets (buffer, 4096 * sizeof (char), ourFile) != NULL)
    {
      n =
	sscanf (buffer, "%d %d %lf %d %d", &currentNode, &endNode,
		&currentLength, &currentX, &currentY);

      if (n != 5)
	{
	  printf ("Incorrect file format.\n");	// make sure each line is correctly formatted
	  return;
	}

      if (previousNode == currentNode)
	branchIndex++;		// check to see if we have reached a new node   
      else
	{
	  branches[previousNode] = branchIndex + 1;	// if we have, make sure to add branches array
	  branchIndex = 0;	// which keeps track of the number of end nodes each current node has
	}

      ourNodes[nodeIndex (currentNode, branchIndex)].number = endNode;	// set node structure values
      ourNodes[nodeIndex (currentNode, branchIndex)].length = currentLength;

      coordinates[coordIndex (currentNode, 0)] = currentX;	// store coordinates of start node
      coordinates[coordIndex (currentNode, 1)] = currentY;

      previousNode = currentNode;
    }

  branches[previousNode] = branchIndex + 1;

  fclose (ourFile);
}

double
calculateDistance (int x1, int y1, int x2, int y2)	// calculates the distance between two points
{
  double distance;

  distance =
    sqrt ((((double) x2 - (double) x1) * ((double) x2 - (double) x1)) +
	  (((double) y2 - (double) y1) * ((double) y2 - (double) y1)));

  return distance * RATIO;	// returns a distance in miles
}

double
straightDistance (int coordinates[], int startNode, int endNode)	// using ratio of miles/pixels, this function returns the distance between two nodes
{
  int startX = coordinates[coordIndex (startNode, 0)];
  int startY = coordinates[coordIndex (startNode, 1)];

  int endX = coordinates[coordIndex (endNode, 0)];
  int endY = coordinates[coordIndex (endNode, 1)];

  double mileDistance = calculateDistance (startX, startY, endX, endY);	// calculates distance in miles

  return mileDistance;
}

double
findCloser (Node ourNodes[], int branches[], int nodePath[],
	    int coordinates[], int nodeCount, int endNode)
{				// out of all the nodes branching from the current, finds the node that brings the path
  double currentDistance;	// closest to the final node in the desired path and assigns it in the node path
  double minDistance;

  int currentNode = nodePath[nodeCount - 1];
  int branchCount = branches[currentNode];	// get the number of branches to the current node (nodeCount is index to be assigned)
  int checkNode;

  int minNode1;			// this is the node that is closest to the end node
  int minIndex1;		// this is the index of this particular node

  int minNode2;			// this is the node and its index that is the 2nd closest to the end node
  int minIndex2;

  int i;

  for (i = 0; i < branchCount; i++)
    {
      checkNode = ourNodes[nodeIndex (currentNode, i)].number;	// find a node number that the current node branches to 

      currentDistance = straightDistance (coordinates, checkNode, endNode);	// find the distance from this node to the end node

      if (i == 0)
	{
	  minDistance = currentDistance;	// set the minimum distance as the distance between the first branch and the end
	  minNode1 = checkNode;
	  minNode2 = checkNode;
	  minIndex1 = i;
	  minIndex2 = i;
	}
      else
	{
	  if (currentDistance < minDistance)	// find the minimum distance from the branching node to the end node
	    {
	      minNode2 = minNode1;	// we need to keep track of the second shortest distanced node also
	      minIndex2 = minIndex1;

	      minDistance = currentDistance;
	      minNode1 = checkNode;
	      minIndex1 = i;
	    }
	}
    }

  nodePath[nodeCount] = minNode1;

  if (notRepeat (nodePath, nodeCount))
    {
      return ourNodes[nodeIndex (currentNode, minIndex1)].length;	// returns the length between the current node and the node that is closest to end
    }
  else
    {
      nodePath[nodeCount] = minNode2;
      return ourNodes[nodeIndex (currentNode, minIndex2)].length;
    }				// add the node that is closest to the ending node to our node path
  // if the closest node causes the path to go back to a previous travelled node,
  // then we choose the second closest node as a way of avoiding infinite loops
}

double
findNextNode (int nodePath[], int nodeCount, Node ourNodes[], int branches[])
{
  while (1)
    {
      int currentNodeNumber = nodePath[nodeCount - 1];	//find the node the route is at
      int currentBranches = branches[currentNodeNumber];	//get the number of branches leaving the node
      int nextNodeIndex = (rand () % (currentBranches));	//pick a random number branch to follow
      Node nextNode = ourNodes[nodeIndex (currentNodeNumber, nextNodeIndex)];	//find the node at the end of that branch
      nodePath[nodeCount] = nextNode.number;	//add that node to the node path
      if (nodeCount < 2 || notRepeat (nodePath, nodeCount))
	return nextNode.length;	// check to make sure it doesnt retrace steps
    }
}

int
notRepeat (int nodePath[], int nodeCount)
{
  if (nodeCount < 2)
    return 1;
  if (nodePath[nodeCount - 2] == nodePath[nodeCount])
    return 0;
  else
    return 1;
}

int
findPath (int nodePath[], Node ourNodes[], int startNode, int endNode,
	  double maxDistance, int branches[], int coordinates[])
{
  nodePath[0] = startNode;
  int nodeCount = 0;
  double travelDistance = 0;
  while (travelDistance < PERCENT * maxDistance)
    {				//while the distace is less than a certain percent, find a random path
      travelDistance +=
	findNextNode (nodePath, ++nodeCount, ourNodes, branches);
    }
  while (checkDistance (travelDistance, maxDistance) != -1)
    {				//while the current node is not the end node
      travelDistance +=
	findCloser (ourNodes, branches, nodePath, coordinates, ++nodeCount,
		    endNode);

      if (checkLastNode (nodePath, nodeCount, endNode) == 1
	  && checkDistance (travelDistance, maxDistance) == 1)
	{
	  printf ("\nRoute Details:\n\nTotal distance travelled: %lf\n", travelDistance);
	  return nodeCount;
	}
    }

  return 0;
}

int
checkDistance (double travelDistance, double maxDistance)
{
  if (travelDistance < (1 - ERROR) * maxDistance)	// still too short
    return 0;
  if (travelDistance > (1 + ERROR) * maxDistance)	// gone too far
    return -1;
  else				// in correct range
    return 1;
}

int
checkLastNode (int nodePath[], int nodeCount, int endNode)
{
  if (nodePath[nodeCount] == endNode)
    return 1;
  else
    return 0;
}

int
findCorrectPath (int nodePath[], Node ourNodes[], int startNode,
		 int endNode, double maxDistance, int branches[],
		 int coordinates[])
{
  int attempt = 0;		// count attempt number

  while (1)
    {
      int nodeCount =
	findPath (nodePath, ourNodes, startNode, endNode, maxDistance,
		  branches, coordinates);
      attempt++;
      if (nodeCount != 0)
	return nodeCount;
      if (attempt > 100)
	return 0;
    }
}

void
drawCurve (double x, double y, double radius, double angleStart, double angleEnd)	// draws a circular curve within the angle range specified, with center at (x,y)
{
  double theta;

  for (theta = angleStart; theta <= angleEnd; theta += .01)
    {
      gfx_line ((x + radius * cos (theta)), (y - radius * sin (theta)),
		(x + radius * cos (theta + .01)),
		(y - radius * sin (theta + .01)));
    }
}

void
fillCircle (double x, double y, double radius)	// draws a solid circle with center {x,y) and specified radius
{
  double theta;
  int i;

  for (i = 0; i < radius; i++)	// we will draw a circle at every point along the circle radius
    {
      drawCurve (x, y, i, 0, 2 * PI);
    }

  gfx_flush ();
}

void
drawNodes (int coordinates[], int totalNodes)	// highlights nodes that user can select (with green circle)
{
  int i;

  gfx_color (0, 255, 0);

  for (i = 0; i < totalNodes; i++)	// for every node, draw a green circle around it
    {
      drawCurve (coordinates[coordIndex (i, 0)],
		 coordinates[coordIndex (i, 1)], RADIUS, 0, 2 * PI);
    }
}

void
drawBranches (Node ourNodes[], int coordinates[], int branches[], int previous, int startNode, int totalNodes, int count)	// recursive function to print all paths between nodes
{
	if( count > 24 ){
		return;
	}

  int i;

  int startX = coordinates[coordIndex (startNode, 0)];
  int startY = coordinates[coordIndex (startNode, 1)];

  int endNode;			// index of the target node

  int endX;			// coordinates of the target node
  int endY;

  for (i = 0; i < branches[startNode]; i++)
    {
      endNode = ourNodes[nodeIndex (startNode, i)].number;

	count++;

      if (previous != endNode)
	{
	  endX = coordinates[coordIndex (endNode, 0)];
	  endY = coordinates[coordIndex (endNode, 1)];

		gfx_color(255, 255, 255);
	  gfx_line (startX, startY, endX, endY);

	  drawBranches (ourNodes, coordinates, branches, startNode, endNode,
			totalNodes, count);
	}
    }
}

int
selectStart (int coordinates[], int totalNodes, int mouseX, int mouseY)	// when the user clicks on a node circle, fill it in green
{
  int i;

  int checkX;
  int checkY;

  gfx_color (0, 255, 0);	// set color to green

  for (i = 0; i < totalNodes; i++)
    {
      checkX = coordinates[coordIndex (i, 0)];
      checkY = coordinates[coordIndex (i, 1)];

      if (mouseX <= (checkX + (2 * RADIUS)) && mouseX >= (checkX - (2 * RADIUS)))	// the user does not have to click exactly on the node
	{
	  if (mouseY <= (checkY + (2 * RADIUS))
	      && mouseY >= (checkY - (2 * RADIUS)))
	    {
	      fillCircle (checkX, checkY, RADIUS);
	      gfx_flush ();
	      break;
	    }
	}
    }

  return i;			// returns the node number that was selected
}

int
selectEnd (int coordinates[], int totalNodes, int mouseX, int mouseY, int startNode)	// when the user clicks a second circle, fill it with red
{
  int i;

  int checkX;
  int checkY;

  gfx_color (255, 0, 0);	// set color to red

  for (i = 0; i < totalNodes; i++)
    {
      checkX = coordinates[coordIndex (i, 0)];
      checkY = coordinates[coordIndex (i, 1)];

      if (mouseX <= (checkX + 2 * RADIUS) && mouseX >= (checkX - 2 * RADIUS))	// the user does not have to click exactly on the node
	{
	  if (mouseY <= (checkY + 2 * RADIUS)
	      && mouseY >= (checkY - 2 * RADIUS))
	    {
	      if (i == startNode)	// if they click the same node, create larger red circle with green inside
		{
		  fillCircle (checkX, checkY, 2 * RADIUS);
		  gfx_color (0, 255, 0);
		  fillCircle (checkX, checkY, RADIUS);
		  gfx_flush ();
		  break;
		}
	      else
		{
		  fillCircle (checkX, checkY, RADIUS);
		  gfx_flush ();
		  break;
		}
	    }
	}
    }

  return i;
}

void
printPath (int nodePath[], int coordinates[], int totalNodes)	// prints the final path in blue, animating along the way
{
  int i;

  int previousX;
  int previousY;

  int currentNode;		// we will draw a path from the previous node to the current node coordinates
  int currentX;
  int currentY;

  for (i = 0; i < totalNodes-1; i++)
    {
      currentNode = nodePath[i];
      currentX = coordinates[coordIndex (currentNode, 0)];
      currentY = coordinates[coordIndex (currentNode, 1)];

      if (i % 2 == 0)
	{
	 	printf ("\n");
	}

      printf ("Node: %d ->\t", currentNode);
	
      previousX = currentX;
      previousY = currentY;
    }

	printf("Node: %d\n\nEnjoy the run!\n\n", nodePath[totalNodes-1]);
}

void
drawPath (int nodePath[], int coordinates[], int totalNodes)	// draws path on graphics window
{
  int i;

  int previousX;
  int previousY;

  int currentNode;		// we will draw a path from the previous node to the current node coordinates
  int currentX;
  int currentY;

  for (i = 0; i < totalNodes; i++)
    {
      currentNode = nodePath[i];
      currentX = coordinates[coordIndex (currentNode, 0)];
      currentY = coordinates[coordIndex (currentNode, 1)];

      labelNode (coordinates, currentNode);	// label each node involved in the path

      if (i != 0)
	{
		gfx_color(0, 0, 255);

	  gfx_line (previousX, previousY, currentX, currentY);
	  gfx_flush ();		// draw out one segment at a time to give appearance of animation
	  usleep (10000);
	}

      previousX = currentX;
      previousY = currentY;
    }
}

void
labelNode (int coordinates[], int nodeNumber)	// prints a label of the node specified on the graphics window
{
  int currentX = coordinates[coordIndex (nodeNumber, 0)];
  int currentY = coordinates[coordIndex (nodeNumber, 1)];

	gfx_color(255, 255, 0);

  draw_number (currentX, currentY + RADIUS, 2 * RADIUS, nodeNumber);	// draws the number below the node

  gfx_flush ();
}

// functions to print numbers
// each function draws a number with height=height and width=.5*height
void
draw_0 (double x, double y, double height)
{
  gfx_line (x + .25 * height, y + .25 * height, x + .25 * height,
	    y + .75 * height);
  gfx_line (x + .75 * height, y + .25 * height, x + .75 * height,
	    y + .75 * height);
  drawCurve (x + .5 * height, y + .25 * height, .25 * height, 0, PI);
  drawCurve (x + .5 * height, y + .75 * height, .25 * height, PI, 2 * PI);
}

void
draw_1 (double x, double y, double height)
{
  gfx_line (x + .5 * height, y, x + .5 * height, y + height);
  gfx_line (x + .25 * height, y + height, x + .75 * height, y + height);
  gfx_line (x + .5 * height, y, x + .25 * height, y + .25 * height);
}

void
draw_2 (double x, double y, double height)
{
  drawCurve (x + .5 * height, y + .25 * height, .25 * height, 0, PI);
  gfx_line (x + .75 * height, y + .25 * height, x + .25 * height, y + height);
  gfx_line (x + .25 * height, y + height, x + .75 * height, y + height);
}

void
draw_3 (double x, double y, double height)
{
  drawCurve (x + .5 * height, y + .25 * height, .25 * height, -.5 * PI, PI);
  drawCurve (x + .5 * height, y + .75 * height, .25 * height, -PI, .5 * PI);
}

void
draw_4 (double x, double y, double height)
{
  gfx_line (x + .25 * height, y, x + .25 * height, y + .5 * height);
  gfx_line (x + .75 * height, y, x + .75 * height, y + height);
  gfx_line (x + .25 * height, y + .5 * height,
	    x + .75 * height, y + .5 * height);
}

void
draw_5 (double x, double y, double height)
{
  gfx_line (x + .25 * height, y, x + .75 * height, y);
  gfx_line (x + .25 * height, y, x + .25 * height, y + .5 * height);
  gfx_line (x + .25 * height, y + .5 * height,
	    x + .5 * height, y + .5 * height);
  gfx_line (x + .25 * height, y + height, x + .5 * height, y + height);
  drawCurve (x + .5 * height, y + .75 * height, .25 * height, -.5 * PI,
	     .5 * PI);
}

void
draw_6 (double x, double y, double height)
{
  gfx_line (x + .5 * height, y, x + .75 * height, y);
  gfx_line (x + .25 * height, y + .25 * height,
	    x + .25 * height, y + .75 * height);
  drawCurve (x + .5 * height, y + .75 * height, .25 * height, 0, 2 * PI);
  drawCurve (x + .5 * height, y + .25 * height, .25 * height, .5 * PI, PI);
}

void
draw_7 (double x, double y, double height)
{
  gfx_line (x + .25 * height, y, x + .75 * height, y);
  gfx_line (x + .75 * height, y, x + .25 * height, y + height);
}

void
draw_8 (double x, double y, double height)
{
  drawCurve (x + .5 * height, y + .25 * height, .25 * height, 0, 2 * PI);
  drawCurve (x + .5 * height, y + .75 * height, .25 * height, 0, 2 * PI);
}

void
draw_9 (double x, double y, double height)
{
  drawCurve (x + .5 * height, y + .25 * height, .25 * height, 0, 2 * PI);
  gfx_line (x + .75 * height, y + .25 * height, x + .75 * height, y + height);
}

//prdoubles a number 0 thru 9 with height=height and width=.5*height at x,y
void
draw_digit (double x, double y, double height, int number)
{
  switch (number)
    {
    case 0:
      draw_0 (x, y, height);
      break;
    case 1:
      draw_1 (x, y, height);
      break;
    case 2:
      draw_2 (x, y, height);
      break;
    case 3:
      draw_3 (x, y, height);
      break;
    case 4:
      draw_4 (x, y, height);
      break;
    case 5:
      draw_5 (x, y, height);
      break;
    case 6:
      draw_6 (x, y, height);
      break;
    case 7:
      draw_7 (x, y, height);
      break;
    case 8:
      draw_8 (x, y, height);
      break;
    case 9:
      draw_9 (x, y, height);
      break;
    }
}

// draws a number 0 thru 999
void
draw_number (double x, double y, double height, int number)
{
  if ((((number % 1000) - (number % 100)) / 100) == 0)
    {				//hundreds
      if ((((number % 100) - (number % 10)) / 10) == 0)
	{			//tens
	  draw_digit (x + 2 * height, y, height, number % 10);	// ones
	}
      else
	{
	  draw_digit (x + height, y, height, ((number % 100) - (number % 10)) / 10);	// tens
	  draw_digit (x + 2 * height, y, height, number % 10);	// ones
	}
    }
  else
    {
      draw_digit (x, y, height, ((number % 1000) - (number % 100)) / 100);	// hundreds
      draw_digit (x + height, y, height, ((number % 100) - (number % 10)) / 10);	// tens
      draw_digit (x + 2 * height, y, height, number % 10);	// ones
    }
  gfx_flush ();
}
