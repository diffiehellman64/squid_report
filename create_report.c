#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define N_FIELDS 10

void parse_log(char *logfile);
char** split(char *str);

int main(int argc, char* argv[])
{
	if (argc > 1)
		parse_log(argv[1]);

	return 0;
}

char** split(char* str)
{
	int i = 0;
	char **result;
	char *cell;

	//вообще хорошей практикой является избавление кода от "магических" констант
	//Например можно было создать глобальную переменную или определить
	//#define N_FIELDS 10 чтобы код выглядел читабельно
	result = malloc(N_FIELDS * sizeof(char *));
	memset(result, 0, N_FIELDS * sizeof(char *));

	cell = strtok(str, " ");

	//надо вставлять проверки на выход из границ массива что бы не было мучительно больно
	while (cell != NULL && i < N_FIELDS) {
		//инкремент после присваивания
		result[i++] = cell;
		cell = strtok(NULL, " ");
	}

	return result;
}

void parse_log (char* logfile)
{
	FILE *file;
	int i, sz;
	char *result_string;
	//parsed это массив указателей на строку
	char** parsed;

	file = fopen(logfile,"r");
	result_string = NULL;
	//когда первый аргумент getline = NULL функция сама выделяет сколько нужно памяти.
	while(getline(&result_string, &sz, file) > 0) {
		parsed = split (result_string);
		for (i=0; i < N_FIELDS; ++i) {
			if (parsed[i])
				printf("%s ||", parsed[i]);
		}
		//\n можно пихать и в саму форматную строку, выглядит понятнее
		printf("%s", "\n\n");
		//обязательно освобождай память после использования
		free(result_string);
		free(parsed);
		result_string = NULL;
	}
	fclose(file);
}
