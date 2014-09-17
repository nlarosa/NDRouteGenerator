// Nicholas LaRosa
// Eric Krakowiak
// 
// NDroutes.h

// establish structure of node

struct node
{
	int number;
	double length;
};

typedef struct node Node;

// file and input functions

double getUserInput(void);

int countNodes (char *fileName);

void createNodes (char *fileName, Node ourNodes[], int branches[], int coordinates[]);

// path functions

double calculateDistance (int x1, int y1, int x2, int y2);

double straightDistance (int coordinates[], int startNode, int endNode);

double findCloser (Node ourNodes[], int branches[], int nodePath[], int coordinates[], int nodeCount, int endNode);

double findNextNode( int nodePath[] , int nodeCount , Node ourNodes[] , int branches[] );

int notRepeat( int nodePath[] , int nodeCount );

int findPath( int nodePath[] , Node ourNodes[] , int startNode , int endNode , double maxDistance , int branches[] , int coordinates[] );

int checkDistance( double travelDistance , double maxDistance );

int checkLastNode( int nodePath[] , int nodeCount , int endNode );

int findCorrectPath( int nodePath[] , Node ourNodes[] , int startNode , int endNode , double maxDistance , int branches[] , int coordinates[] );

// graphics functions

void drawCurve (double x, double y, double radius, double angleStart, double angleEnd);

void drawBranches( Node ourNodes[], int coordinates[], int branches[], int previous, int startNode, int totalNodes, int count );

void fillCircle ( double x, double y, double radius);

void drawNodes (int coordinates[], int totalNodes);

int selectStart (int coordinates[], int totalNodes, int mouseX, int mouseY);

int selectEnd (int coordinates[], int totalNodes, int mouseX, int mouseY, int startNode);

void printPath (int nodePath[], int coordinates[], int totalNodes);

void drawPath (int nodePath[], int coordinates[], int totalNodes);

// number drawing functions

void draw_0 (double x, double y, double height);

void draw_1 ( double x, double y, double height);

void draw_2 ( double x, double y, double height);

void draw_3 ( double x, double y, double height);

void draw_4 ( double x, double y, double height);

void draw_5 ( double x, double y, double height);

void draw_6 ( double x, double y, double height);

void draw_7 ( double x, double y, double height);

void draw_8 ( double x, double y, double height);

void draw_9 ( double x, double y, double height);

void draw_digit ( double x, double y, double height, int number);

void draw_number ( double x, double y, double height, int number);

void labelNode( int coordinates[], int nodeNumber);
