#include "io.h"
//hola hola

#define F_TABLE_ENTRIES 16777216 //2^24
#define S_TABLE_ENTRIES 6400000 //2^8 * 25000

int error;
short *f_table;
short *s_table;
uint32_t *prefix;
int *prefixLength;
int *outInterface;
struct timespec initialTime, finalTime;
int *processedPackets;
double *totalTableAccesses;
double *totalPacketProcessingTime;

void usage(int argc)
{
  if(argc != 3){
    fprintf(stderr, "\nUsage: <./my_routing_lookup> <FIB> <InputPacketFile>\n\n" );
    exit(-1);
  }
}

//FILL FIRST TABLE & SECOND TABLE
void fill_FIB()
{
  prefix = calloc(1,sizeof(int));
	prefixLength = calloc(1,sizeof(int));
	outInterface = calloc(1,sizeof(int));

  error = readFIBLine(prefix, prefixLength, outInterface);

  while(error == OK)
  {
    //FILLING HERE
    /*
        ....
    */

    //READ ANOTHER LINE
    error = readFIBLine(prefix, prefixLength, outInterface);
  }
  //FREE RESOURCES
  free(prefix);
  free(prefixLength);
  free(outInterface);
}

//LOOK FOR AN INTERFACE AND A TABLE FOR AN SPECIFIC IP ADDRESS
void lookup(uint32_t *IP_lookup, short int *numberOfTableAccesses, unsigned short *out_Interface)
{
  //LOOKUP
}

//LOOK FOR THE BEST INTERFACE FOR EACH IP ADDRESS (ROUTES)
void routing()
{
  uint32_t *IP_lookup = calloc(1,sizeof(uint32_t));
	unsigned short *out_Interface = calloc(1,sizeof(unsigned short));
	double *searchingTime = calloc(1,sizeof(double));
	short int *numberOfTableAccesses = calloc(1,sizeof(short int));

	error = readInputPacketFileLine(IP_lookup);

  while(error == OK)
  {
    //START TIME HERE (INITIAL TIME)
    lookup(IP_lookup, numberOfTableAccesses, out_Interface);
    //END TIME HERE (FINAL TIME)
    printOutputLine(*IP_lookup, *out_Interface, &initialTime, &finalTime,
                    searchingTime, *numberOfTableAccesses);
    /*
          ....
    */

    //READ ANOTHER LINE
    error = readInputPacketFileLine(IP_lookup);
  }
  //FREE RESOURCES
  free(IP_lookup);
  free(out_Interface);
  free(searchingTime);
  free(numberOfTableAccesses);
}

int main(int argc, char *argv[])
{

  usage(argc);

  //RESERVE DINAMIC MEMORY
  f_table = calloc(F_TABLE_ENTRIES, sizeof(short));
  s_table = malloc(S_TABLE_ENTRIES);
  processedPackets  = calloc(1,sizeof(int));
	totalTableAccesses  = calloc(1,sizeof(double));
	totalPacketProcessingTime  = calloc(1,sizeof(double));

  //INITIALIZE FILES
  error = initializeIO(argv[1], argv[2]);
  if(error != 0)
  {
    printIOExplanationError(error);
    exit(-1);
  }

  //FILL TABLES
  fill_FIB();

  //COMPUTE ROUTES
  routing();

  //SUMMARY
  printSummary(*processedPackets, (*totalTableAccesses / *processedPackets), (*totalPacketProcessingTime / *processedPackets));

  //FREE RESOURCES
  freeIO();
  free(f_table);
  free(s_table);
  free(processedPackets);
  free(totalTableAccesses);
  free(totalPacketProcessingTime);

  return 0;
}
