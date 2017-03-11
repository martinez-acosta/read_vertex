#include "definiciones.h"
#include <stdlib.h>

static void swap(float *a, float *b) {
  float tmp = *a;
  *a = *b;
  *b = tmp;
}

void fatal(char *str, char *error) {
  printf("%s: %s\n", str, error);
  exit(1);
}

void error(char *str) {
  puts(str);
  exit(1);
}

static float smallest_float(float first, float second, float third) {
  float tmp;
  if (first < second && first < third) {
    tmp = first;
  } else {
    if (second < first && second < third) {
      tmp = second;
    } else {
      tmp = third;
    }
  }
  return tmp;
}

static float greatest_float(float first, float second, float third) {
  float tmp;
  if (first > second && first > third) {
    tmp = first;
  } else {
    if (second > first && second > third) {
      tmp = second;
    } else {
      tmp = third;
    }
  }
  return tmp;
}

static struct vector *get_vector(int p, struct vector *vertexes) {
  for (int i = 0; i < p; i++)
    vertexes = vertexes->next;
  return vertexes;
}
