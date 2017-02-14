#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
	if(argc < 2){ 
		printf("%s <num>\n",argv[0]);
		exit(1);
	}

	int a = atoi(argv[1]);
	int b = (a >= 16) ? 2 : 4; // if(a >= 16) { return 10; } else { return 20; }

	printf("%d\n",b);
}
