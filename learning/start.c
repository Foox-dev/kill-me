#include <stdio.h>
#include <stdbool.h>

char caca[] = "caca poo poo";
char *pCaca = caca;
float a = 0.1;
float b = 0.2;
bool isTrue = true;

int main()
{
  printf("Blah blah blah\n\n");
  printf("Hello, World!\n\n");
  printf("%s\n", caca);
  printf("%s\n", pCaca);
  printf("%f\n", a + b);
  printf("%lu\n", sizeof(caca));

  int kill = 10;
  int me = 20;
  if (kill > me) {
    printf("LAWRD ALL MIGHTY");
  } else {
    printf("ew\n");
  }

  int time = 20;
  (time < 18) ? printf("Good day.") : printf("Good evening."); 

  printf("\n type 1 to continue, 0 to exit\n");
  int input;
  scanf("%i", &input);

  return 0;
}