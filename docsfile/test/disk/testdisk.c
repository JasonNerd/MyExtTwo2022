#include <stdio.h>


int main() {
	unsigned int bm = 0xa6c37b25;
	// Ҫ��Ӹ����ͣ�����ĳһ��
	int j, i = 1;
	for (j = 0; j < 32; j++) {
		int bit = (bm & 1 << (31 - j)) >> (31 - j);
		if (bit == 0) {
			printf("%d", 32 * i + j);
			break;
		}
//		printf("%d", bit);
//		if ((j + 1) % 4 == 0)
//			printf(" ");
	}
}