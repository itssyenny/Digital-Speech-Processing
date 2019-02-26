#include <stdio.h>
#include<string.h>
#include <ctype.h>
#include "hmm.h"
int main(int argc, char const *argv[]){

	FILE *f1 = open_or_die(argv[1], "r");	//result
	FILE *f2 = open_or_die(argv[2], "r");	//testing_answer

	char str[5000], ans[5000];
	double count = 0, row = 0;
	while(fscanf(f1, "%s", str) != EOF) {
		if(isdigit(str[0])) continue;
		row++;
		fscanf(f2, "%s", ans);

		if(strcmp(ans, str) == 0) count++;
		
	}

	printf("Accuracy = %lf\n", count/row);

	return 0;
}