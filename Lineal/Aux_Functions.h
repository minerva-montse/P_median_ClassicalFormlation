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

//// This is the 'matrix' class.
////template <typename Number>
struct matrix : vector <vector <int> >
{
	matrix(unsigned m, unsigned n)
	{
		resize(m, vector <int>(n, 0));
	}
	unsigned rows()    const { return this->size(); }
	unsigned columns() const { return this->front().size(); }
};
//
//// This is the output operator for the 'matrix' class. Yes, I know it is pretty simple...
////template <typename Number>
//ostream& operator << (ostream& outs, const matrix & m)
//{
//	for (unsigned row = 0; row < m.rows(); row++)
//	{
//		for (unsigned col = 0; col < m.columns(); col++)
//			outs << m[row][col] << " ";
//		outs << "\n";
//	}
//	return outs;
//}

bool Read_Parameters(int argc, char *argv[])
{
	int data_format = 0;
	int p = 0;
	if (argc<2) return false;

	char *file_name;
	file_name = argv[1];
	cout << "value: " << *file_name << endl;
	cout << "address: " << file_name << endl;
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

		/*if (strcmp(arg[argv], "-time") == 0)
		{
		++arg;
		if (arg == argc) return false;
		time_limit = atof(argv[arg]);
		++arg;
		continue;
		}

		if (strcmp(arg[argv], "-r") == 0)
		{
		++arg;
		if (arg == argc) return false;
		int roundedvalue = atoi(argv[arg]);
		if (roundedvalue == 1) roundedc = true;
		++arg;
		continue;
		}

		if (strcmp(arg[argv], "-b") == 0)
		{
		++arg;
		if (arg == argc) return false;
		incumbent_value = atof(argv[arg]);
		++arg;
		continue;
		}

		if (strcmp(arg[argv], "-h") == 0)
		{
		RW = true;
		++arg;
		if (arg == argc) return false;
		RWiter = atoi(argv[arg]);
		++arg;
		continue;
		}

		if (strcmp(arg[argv], "-s") == 0)
		{
		save_solution_info = true;
		++arg;
		if (arg == argc) return false;
		solution_file_name = argv[arg];
		++arg;
		continue;
		}

		if (strcmp(arg[argv], "-d") == 0)
		{
		++arg;
		if (arg == argc) return false;
		int d = atoi(argv[arg]);
		if (d == 1) allD = true;
		else if (d != 0) return false;
		++arg;
		continue;
		}*/

		return false;
	}

	if (no_p)
	{
		printf("ERROR: No value for p has been given.\n");
		return false;
	}
	return true;
}