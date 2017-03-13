#include "definiciones.h"
#include <stdlib.h>

void swap(float *a, float *b) {
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

float smallest_float(const float first, const float second, const float third) {
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

float greatest_float(const float first, const float second, const float third) {
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

struct vector *get_vector(int p, struct vector *vertexes) {
  for (int i = 0; i < p; i++)
    vertexes = vertexes->next;
  return vertexes;
}
