//#include <iostream>
#include <vector>
#include <cstdio>

using namespace std;

char *Get_x_Name(int i)
{
	static  char aux[10];
	sprintf_s(aux, "x(%d)", i);
	return aux;
}

char *Get_y_Name(int i, int j)
{
	static char aux[20];
	sprintf_s(aux, "y(%d,%d)", i, j);
	return aux;
}

char *Get_z_Name(int i, int k)
{
	static char aux[20];
	sprintf_s(aux, "z(%d,%d)", i, k);
	return aux;
}


bool Read_Parameters(int argc, char *argv[])
{
	int data_format = 0;
	int p = 0;
	if (argc<2) return false;

	char *file_name;
	file_name = argv[1];

	if (strlen(file_name) >= 5)
	{
		printf("The file is:%s\n", file_name);
		if (strcmp(&file_name[strlen(file_name) - 4], ".tsp") == 0)
			data_format = 1;
		else if (strcmp(&file_name[strlen(file_name) - 7], ".dimacs") == 0)
		{
			data_format = 2;
		}
		else if (strcmp(&file_name[strlen(file_name) - 4], ".dat") == 0)
		{
			data_format = 3;
		}
		else
		{
			printf("ERROR: data extension not recognized.\n");
			printf("Valid extensions: .tsp, .dimacs\n");
			return false;
		}
	}
	int arg = 2;
	bool no_p = true;
	while (arg<argc)
	{
		if (strcmp(arg[argv], "-p") == 0)
		{
			++arg;
			if (arg == argc) return false;
			p = atoi(argv[arg]);
			if (p <= 0)
			{
				printf("ERROR: p must be strictly positive.\n");
				return false;
			}
			no_p = false;
			++arg;
			continue;
		}

		return false;
	}

	if (no_p)
	{
		printf("ERROR: No value for p has been given.\n");
		return false;
	}
	return true;
}