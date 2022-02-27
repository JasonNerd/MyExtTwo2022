#include <stdio.h>

#define YES 1
#define NO 0

///*判断函数，进行元素大小判断，increase判断大小比较*/
int compare(int a, int b, int increase) {
	if (increase > 0) {
		if (a > b)
			return YES;
		else
			return NO;
	} else {
		if (a < b)
			return YES;
		else
			return NO;
	}
}

/*冒泡排序进行数组排序*/
void OrderArr(int arry[], int(*cmp)(int, int, int), int length, int increase) {
	for (int i = 0; i < length - 1; i++) {
		for (int j = 0; j < length - i - 1; j++) {
			if (cmp(*(arry + j), *(arry + j + 1), increase)) {
				int temp = *(arry + j + 1);
				*(arry + j + 1) = *(arry + j);
				*(arry + j) = temp;
			}
		}
	}
}

/*输出函数*/
void Print(int a[], int length) {
	for (int i = 0; i < length; i++) {
		printf("%d ", *(a + i));
	}
	printf("\n");
}

int main() {
	int a[5] = { 1, 4, 2, 6, 3 };
	//增序排列数组
	OrderArr(a, compare, 5, 1);
	Print(a, 5);
	//降序排列数组
	OrderArr(a, compare, 5, -1);
	Print(a, 5);
}
