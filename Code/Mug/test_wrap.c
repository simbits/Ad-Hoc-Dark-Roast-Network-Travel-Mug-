#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int i, j, foundSpace;
	char *msg = argv[1];

	foundSpace = 0;


	printf("msg: %s\n", msg);
	for (i=0,j=0; i<strlen(msg); i++,j++) { 

		if (*(msg+i) == ' ') {
			foundSpace = i;
		}

		if (j==18) {
			if (*(msg+i) != ' ') {
				if (foundSpace) {
					*(msg + foundSpace) = '~';
					i = foundSpace + 1;
					foundSpace = 0;
				} else {
					*(msg+i) = '~';
				}
			}
			j = 0;
		}
	}
	printf("wra: %s\n", msg);

}
