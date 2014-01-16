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
	char* result[10];
	char* cell;
	cell = strtok(str, " ");
	while (cell != NULL)
        {
        	++i;
		result[i] = cell;
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
	while(fgets(result_sting, sizeof(result_sting), file))
	{
		parsed = split (result_sting);	
		split (result_sting);	
	}
	fclose(file);
}
