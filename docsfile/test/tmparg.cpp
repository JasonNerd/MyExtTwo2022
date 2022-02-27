#include <stdio.h>

int main() {
	int a[3] = {9, 5, 4};
	int b = a[1];
	b = 3;
	int i;
	for (i = 0; i < 3; i++)
		printf("%d ", a[i]);
}