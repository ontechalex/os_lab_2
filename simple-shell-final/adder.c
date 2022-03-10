#include <stdio.h>
#include <stdlib.h>
// adds up the numbers that are passed as arguements
int main(int argc, char *argv[]){ 
	int total = 0;
	if (argc < 2) {
		printf("Invalid arguement amount, please input at least 1 number.\n");
		return -1;
	} else {
		for (int i = 1; i < argc; i++) {
			int num = atoi(argv[i]);
			
			if (num != 0 && argv[i] != "0") {
				total += num;
			} else if (num == 0 && argv[i] != "0") {
				printf("Invalid entry, '%s' must be of type int\n", argv[i]);
				return -1;
			}
			
		}
		
		printf("The total is: %d\n", total);
		
	}
	
	return total;
}

