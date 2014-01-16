#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void parse_log (char* logfile);

int main (int argc, char* argv[])
{
	parse_log (argv[1]);
	return 0;
}

//char* split (char* str)
void split (char* str)
{
	int i;
//	char* result;
	char* cell;
	cell = strtok(str, " ");
	while (cell != NULL)
        {
        	++i;
        	printf("%s\n", cell);
                cell = strtok(NULL, " ");
        }
//	return result;
}

void parse_log (char* logfile)
{
	FILE *file; 
//	char *fname = logfile;
	char result_sting[200]; 
	file = fopen(logfile,"r");
	 
	if(file == 0)
	{
		printf("Not exist file '%s'", logfile);
	}
 
//	int i;
//	char* pch;
// 	char* parsed;
	while(fgets(result_sting, sizeof(result_sting), file))
	{
	
	//	parsed = split (result_sting);	
		split (result_sting);	
		
	//	printf("%s", result_sting);
	/*	i = 0;
		pch = strtok(result_sting, " ");
		while (pch != NULL)
  		{
    			++i;
			if (i == 7 || i == 8)
			{
				printf("%s\n", pch);
			}
    			pch = strtok(NULL, " ");
  		}*/
	}
	fclose(file);
}
