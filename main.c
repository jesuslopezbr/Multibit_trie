#include "io.h"

#define F_TABLE_ENTRIES 16777216 //2^24
#define S_TABLE_ENTRIES 6400000 //2^8 * 25000

int error;
short *f_table;
short *s_table;
unsigned short extended_IPs;
long ip_index;
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
  long int hosts = 0;
  extended_IPs = 0;

  error = readFIBLine(prefix, prefixLength, outInterface);

  while(error == OK)
  {
    if(*prefixLength <= 24)
    {
      hosts = pow(2, 24 - *prefixLength);

      //Fill the interface field of the first table for each IP
      for(ip_index = 0; ip_index < hosts; ip_index++)
			{
				f_table[(*prefix>>8) + ip_index] = *outInterface;
			}
    }
    else
    {
      hosts = pow(2, 32 - *prefixLength);

      if(f_table[*prefix>>8]>>15 == 0)
      {
        s_table = realloc(s_table, 256*(extended_IPs + 1)*2);

        for(ip_index = 0; ip_index <= 255; ip_index++)
        {
          s_table[extended_IPs*256 + ip_index] = f_table[*prefix>>8];
        }

        f_table[*prefix>>8] = extended_IPs | 0x8000;

        for(ip_index = (*prefix & 0xFF); ip_index < hosts + (*prefix & 0xFF); ip_index++)
				{
					s_table[extended_IPs*256 + ip_index] = *outInterface;
				}
				extended_IPs++;
      }
      else
      {
        for(ip_index = (*prefix & 0xFF); ip_index < hosts + (*prefix & 0xFF); ip_index++)
				{
					s_table[(f_table[*prefix>>8] & 0x7FFF)*256 + ip_index] = *outInterface;
				}
      }
    }

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
  *out_Interface = f_table[*IP_lookup>>8];
  if(*out_Interface>>15 == 0)
	{
		*numberOfTableAccesses = 1;
		return;
	}
	else
	{
		*numberOfTableAccesses = 2;
		*out_Interface = s_table[(*out_Interface & 0x7FFF)*256 + (*IP_lookup & 0x000000FF)];
		// 0x7fff = 0b0111111111111111 to adquire just the address to the 2nd table
		return;
	}
	return;
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
    //START TIME (INITIAL TIME)
    gettimeofday(&initialTime, NULL);
    lookup(IP_lookup, numberOfTableAccesses, out_Interface);
    //END TIME HERE (FINAL TIME)
    gettimeofday(&finalTime, NULL);
    printOutputLine(*IP_lookup, *out_Interface, &initialTime, &finalTime,
                    searchingTime, *numberOfTableAccesses);
    *processedPackets = *processedPackets + 1;
    *totalTableAccesses  = *totalTableAccesses + *numberOfTableAccesses;
    *totalPacketProcessingTime  = *totalPacketProcessingTime + *searchingTime;

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
  //s_table = malloc(S_TABLE_ENTRIES);
  processedPackets  = calloc(1,sizeof(int));
	totalTableAccesses  = calloc(1,sizeof(double));
	totalPacketProcessingTime  = calloc(1,sizeof(double));
  error = 0;

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
