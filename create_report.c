#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void parse_log (char* logfile);
char* split (char* str);

int main (int argc, char* argv[])
{
	parse_log (argv[1]);
	return 0;
}

char* split (char* str)
{
	int i = 0;
	char* result;
	char* cell;
	result = (char*) malloc(10*sizeof(char));
	cell = strtok(str, " ");
	while (cell != NULL)
        {
        	++i;
	//	printf("%s\n", cell);
		result[i] = *cell;
                cell = strtok(NULL, " ");
        }
	return result;
}

void parse_log (char* logfile)
{
	FILE *file; 
	char result_sting[200]; 
	file = fopen(logfile,"r");
 	char* parsed;
	int i;
	while(fgets(result_sting, sizeof(result_sting), file))
	{
		parsed = split (result_sting);	
		for (i=0; i<=10; ++i){
			if (parsed[i])
				printf("%d\n", parsed[i]);
		}
		printf("%s", "\n\n");
//		split (result_sting);	
	}
	fclose(file);
}
