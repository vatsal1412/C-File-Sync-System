#include <stdio.h>
#include <stdlib.h>
#include "hash.h"

// Hash manipulation helper functions
char* hash(FILE *f)
{
  // initialize all the elements of hash_val to '\0'
  fseek(f,0,SEEK_END);
  rewind(f);
  int i;
  char* hash_val = malloc(sizeof(char) * BLOCK_SIZE);
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    hash_val[i] = '\0';
  }
  //declare and initialize input inp to '\0' and counting variable count to 0
  char inp ='\0';
  int count = 0;
  //loop until EOF i.e. input from the user
  while (fread(&inp,1,1,f) != 0)
  {
    //if the last element in hash_val is reached then xor the input with the
    // last element in the array and reset the counter to 0
    if(count == BLOCK_SIZE-1)
    {
      hash_val[count] = inp ^ hash_val[count];
      count = 0;
    }
    //else xor the input with the last element in the array
    // and increment the counter by 1
    else
    {
      hash_val[count] = inp ^ hash_val[count];
      count++;
    }
  }
  fclose(f);
  return hash_val;
}
