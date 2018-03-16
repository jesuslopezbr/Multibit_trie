#include "io.h"

int error;
short *f_table;
short *s_table;
unsigned short ip_ext;
long ip_index;
uint32_t *ip_addr;
int *prefixLength;
int *outInterface;
struct timespec initialTime, finalTime;
int *processedPackets;
double *totalTableAccesses;
double *totalPacketProcessingTime;

void usage(int argc)
{
  if(argc != 3){
    fprintf(stderr, "\nUsage: <./my_routing_lookup> <FIB> <InputPacketFile>\n\n");
    exit(-1);
  }
}

//FILL FIRST TABLE & SECOND TABLE
void fill_FIB()
{
  ip_addr = calloc(1,sizeof(int));
	prefixLength = calloc(1,sizeof(int));
	outInterface = calloc(1,sizeof(int));
  long int hosts = 0;
  ip_ext = 0;
  ip_index = 0;

  error = readFIBLine(ip_addr, prefixLength, outInterface);

  while(error == OK)
  {
    if(*prefixLength <= 24)
    {
      hosts = pow(2, 24 - *prefixLength);

      //Fill the interface field of the first table for each IP
      for(ip_index = 0; ip_index < hosts; ip_index++)
			{
				f_table[(*ip_addr>>8) + ip_index] = *outInterface;
			}
    }
    else
    {
      hosts = pow(2, 32 - *prefixLength);

      if(f_table[*ip_addr>>8]>>15 == 0)
      {
        s_table = realloc(s_table, 256*(ip_ext + 1)*2);

        for(ip_index = 0; ip_index <= 255; ip_index++)
        {
          s_table[ip_ext*256 + ip_index] = f_table[*ip_addr>>8];
        }

        f_table[*ip_addr>>8] = ip_ext | 0x8000;

        for(ip_index = (*ip_addr & 0xFF); ip_index < hosts + (*ip_addr & 0xFF); ip_index++)
				{
					s_table[ip_ext*256 + ip_index] = *outInterface;
				}
				ip_ext++;
      }
      else
      {
        for(ip_index = (*ip_addr & 0xFF); ip_index < hosts + (*ip_addr & 0xFF); ip_index++)
				{
					s_table[(f_table[*ip_addr>>8] & 0x7FFF)*256 + ip_index] = *outInterface;
				}
      }
    }
    //READ ANOTHER LINE
    error = readFIBLine(ip_addr, prefixLength, outInterface);
  }
  //FREE RESOURCES
  free(ip_addr);
  free(prefixLength);
  free(outInterface);
}

//LOOK FOR AN INTERFACE AND A TABLE FOR AN SPECIFIC IP ADDRESS
void lookup(uint32_t *ip_lookup, short int *numberOfTableAccesses, unsigned short *out_Interface)
{
  *out_Interface = f_table[*ip_lookup>>8];
  if(*out_Interface>>15 == 0)
	{
		*numberOfTableAccesses = 1;
		return;
	}
	else
	{
		*numberOfTableAccesses = 2;
		*out_Interface = s_table[(*out_Interface & 0x7FFF)*256 + (*ip_lookup & 0x000000FF)];
		// 0x7fff = 0b0111111111111111 to adquire just the address to the 2nd table
		return;
	}
	return;
}

//LOOK FOR THE BEST INTERFACE FOR EACH IP ADDRESS (ROUTES)
void routing()
{
  uint32_t *ip_lookup = calloc(1,sizeof(uint32_t));
	unsigned short *out_Interface = calloc(1,sizeof(unsigned short));
	double *searchingTime = calloc(1,sizeof(double));
	short int *numberOfTableAccesses = calloc(1,sizeof(short int));

	error = readInputPacketFileLine(ip_lookup);

  while(error == OK)
  {
    //START TIME (INITIAL TIME)
    clock_gettime(CLOCK_REALTIME, &initialTime);
    lookup(ip_lookup, numberOfTableAccesses, out_Interface);
    //END TIME (FINAL TIME)
    clock_gettime(CLOCK_REALTIME, &finalTime);
    printOutputLine(*ip_lookup, *out_Interface, &initialTime, &finalTime,
                    searchingTime, *numberOfTableAccesses);
    *processedPackets += 1;
    *totalTableAccesses  += *numberOfTableAccesses;
    *totalPacketProcessingTime  += *searchingTime;

    //READ ANOTHER LINE
    error = readInputPacketFileLine(ip_lookup);
  }
  //FREE RESOURCES
  free(ip_lookup);
  free(out_Interface);
  free(searchingTime);
  free(numberOfTableAccesses);
}

int main(int argc, char *argv[])
{
  double averageTableAccesses, averagePacketProcessingTime;

  usage(argc);

  //RESERVE DINAMIC MEMORY
  f_table = calloc(F_TABLE_ENTRIES, sizeof(short));
  processedPackets  = calloc(1, sizeof(int));
	totalTableAccesses  = calloc(1, sizeof(double));
	totalPacketProcessingTime  = calloc(1, sizeof(double));
  error = 0;

  //INITIALIZE FILES
  error = initializeIO(argv[1], argv[2]);
  if(error != 0)
  {
    printIOExplanationError(error);
    exit(-1);
  }

  fill_FIB();

  routing();

  averageTableAccesses = (*totalTableAccesses / *processedPackets);
  averagePacketProcessingTime = (*totalPacketProcessingTime / *processedPackets);

  printSummary(*processedPackets, averageTableAccesses, averagePacketProcessingTime);

  freeIO();
  free(f_table);
  free(s_table);
  free(processedPackets);
  free(totalTableAccesses);
  free(totalPacketProcessingTime);

  return 0;
}
