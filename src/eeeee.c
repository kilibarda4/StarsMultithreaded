// MIT License
// 
// Copyright (c) 2023 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include "utility.h"
#include "star.h"
#include "float.h"

#define NUM_STARS 30000 
#define MAX_LINE 1024
#define DELIMITER " \t\n"

struct Star star_array[ NUM_STARS ];
uint8_t   (*distance_calculated)[NUM_STARS];
pthread_mutex_t mutex;
uint64_t count = 0;
double  min  = FLT_MAX;
double  max  = FLT_MIN;
int num_threads;
double mean = 0;
clock_t start, end;
double time_used;

void showHelp()
{
  printf("Use: findAngular [options]\n");
  printf("Where options are:\n");
  printf("-t          Number of threads to use\n");
  printf("-h          Show this help\n");
}

void* determineAverageAngularDistance(void* arg){
  int index = *((int*)arg);
  int portion = NUM_STARS / num_threads;
  int start = index * portion;
  int end = (((index+1) * portion) - 1);

  for ( int i = start ; i < end; i++)
  {
    for(int j = 0; j < NUM_STARS; j++)
    {
      if( i!=j && distance_calculated[i][j] == 0 )
      {
        double distance = calculateAngularDistance( star_array[i].RightAscension, star_array[i].Declination,
                                                    star_array[j].RightAscension, star_array[j].Declination ) ;
        distance_calculated[i][j] = 1;
        distance_calculated[j][i] = 1;       
        pthread_mutex_lock(&mutex);
        count++;
        if( min > distance )
        {
          min = distance;
        }
        if( max < distance )
        {
          max = distance;
        }
        mean = mean + (distance-mean)/count;
        pthread_mutex_unlock(&mutex);
      }
    }
  }
}

int main(int argc, char* argv[])
{
  FILE* fp;
  uint32_t star_count = 0;
  uint32_t n;
  pthread_mutex_init(&mutex, NULL);
  if (argc < 3)
  {
      perror("Please run with -t followed by # of threads desired\n");
      exit(1);
  }

  if (!strcmp(argv[1], "-t"))
  {
      num_threads = atoi(argv[2]);
  }

  distance_calculated = malloc(sizeof(uint8_t[NUM_STARS][NUM_STARS]));

  if (distance_calculated == NULL)
  {
      printf("Could not allocate memory for distance_calculated\n");
      exit(EXIT_FAILURE);
  }

  int i, j;

  memset(distance_calculated, 0, NUM_STARS * NUM_STARS * sizeof(uint8_t));

  for (n = 1; n < argc; n++)
  {
      if (strcmp(argv[n], "-help") == 0)
      {
          showHelp();
          exit(0);
      }
  }

  fp = fopen("data/tycho-trimmed.csv", "r");

  if (fp == NULL)
  {
      printf("ERROR: Unable to open the file data/tycho-trimmed.csv\n");
      exit(1);
  }

  char line[MAX_LINE];
  while (fgets(line, 1024, fp))
  {
      uint32_t column = 0;

      char* tok;
      for (tok = strtok(line, " "); tok && *tok; tok = strtok(NULL, " "))
      {
          switch (column)
          {
          case 0:
              star_array[star_count].ID = atoi(tok);
              break;

          case 1:
              star_array[star_count].RightAscension = atof(tok);
              break;

          case 2:
              star_array[star_count].Declination = atof(tok);
              break;

          default:
              printf("ERROR: line %d had more than 3 columns\n", star_count);
              exit(1);
              break;
          }
          column++;
      }
      star_count++;
  }
  printf("%d records read\n", star_count);

  int tid[num_threads];
  pthread_t th[num_threads];

  start = clock();
  for (i = 0; i < num_threads; i++)
  {
      tid[i] = i;
      pthread_create(&th[i], NULL, &determineAverageAngularDistance, &tid[i]);
  }

  for (i = 0; i < num_threads; i++)
  {
      pthread_join(th[i], NULL);
  }
  end = clock();

  time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Average distance found is %lf\n", mean);
  printf("Minimum distance found is %lf\n", min);
  printf("Maximum distance found is %lf\n", max);
  printf("Total time: %lf\n", time_used);
  pthread_mutex_destroy(&mutex);
  free(distance_calculated);
  return 0;
}

