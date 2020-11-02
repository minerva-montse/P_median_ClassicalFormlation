//Pmedian problem (clasical formulation). Problem with m=4, n=2.
#include <fstream>
#include <iostream>
#include <ilcplex/ilocplex.h>
#include "Aux_Functions.h"
#include "subrutines.h"

using namespace std;

ILOSTLBEGIN

typedef IloArray<IloNumArray>    NumMatrix;
typedef IloArray<IloNumVarArray> NumVarMatrix;
typedef vector <vector <int> > matrixInt;
typedef IloArray<IloBoolVarArray> NumBoolMatrix;


ILOSIMPLEXCALLBACK0(MyCallback) {
	cout << "Iteration " << getNiterations() << ": ";
	if (isFeasible()) {
		cout << "Objective = " << getObjValue() << endl;
	}
	else {
		cout << "Infeasibility measure = " << getInfeasibility()  << endl;
	}
}

ILOBRANCHCALLBACK1(MyBranch, IloBoolVarArray, vars) {
	if (getBranchType() != BranchOnVariable)
		return;

	//cout << "branch type:" << getBranchType() << ", branches: " << getNbranches() << endl;


	// Branch on var with largest objective coefficient
	// among those with largest infeasibility


	IloNumArray x;
	IloNumArray obj;
	IntegerFeasibilityArray feas;

	try {
		x = IloNumArray(getEnv());
		obj = IloNumArray(getEnv());
		feas = IntegerFeasibilityArray(getEnv());
		getValues(x, vars);
		getObjCoefs(obj, vars);
		getFeasibilities(feas, vars);

		IloCplex::BranchDirectionArray dirs_1;
		IloCplex::BranchDirectionArray dirs_2;

		dirs_1 = IloCplex::BranchDirectionArray(getEnv());
		dirs_2 = IloCplex::BranchDirectionArray(getEnv());

		for (size_t i = 0; i < vars.getSize(); i++)
		{
			dirs_1.add(IloCplex::BranchUp);
			dirs_2.add(IloCplex::BranchDown);
		}
		
		

		//makeBranch(vars, x, dirs_1, getObjValue());
		//makeBranch(vars, x, dirs_2, getObjValue());


		IloInt bestj = -1;
		IloNum maxinf = 0.0;
		IloNum maxobj = 0.0;
		IloInt cols = vars.getSize();
		for (IloInt j = 0; j < cols; j++) {
			if (feas[j] == Infeasible) {
				IloNum xj_inf = x[j] - IloFloor(x[j]);
				if (xj_inf > 0.5)
					xj_inf = 1.0 - xj_inf;
				//if (xj_inf >= maxinf && (xj_inf > maxinf || IloAbs(obj[j]) >= maxobj)) {
				//	bestj = j;
				//	maxinf = xj_inf;
				//	maxobj = IloAbs(obj[j]);
				if (xj_inf >= maxinf){
					bestj = j;
					maxinf = xj_inf;
				}
			}
		}

		if (bestj >= 0) {
			makeBranch(vars[bestj], x[bestj], IloCplex::BranchUp, getObjValue());
			makeBranch(vars[bestj], x[bestj], IloCplex::BranchDown, getObjValue());
		}
	}
	catch (...) {
		x.end();
		obj.end();
		feas.end();
		throw;
	}
	x.end();
	obj.end();
	feas.end();
}


int main(int argc, char ** argv)
{
	//Start time BB.
	clock_t clock_initial = clock();
	double total_time = 0;

	//solution
	double Optimal_value;
	if (Read_Parameters(argc, argv) == false)
	{
		printf("ERROR: wrong command line.\n");
		printf("The structure is: lineal.exe <data_file> -p p\n");
		printf("\n");
		return 0;
	}

	char file_name[50] = "";
	strcat_s(file_name, "data/");
	strcat_s(file_name, argv[1]);
	printf("The file is:%s\n", file_name);

	//Create an input file stream
	ifstream in(file_name, ios::in);

	//int number;  //Variable to hold each number as it is read
	double number;  //Variable to hold each number as it is read

	in >> number;
	int m = number;// size of the problem, number of costumers
	printf("The value of m: %d\n", m);

	in >> number;
	const int p = number;// facilite to locate
	printf("The value of p: %d\n", p);

	matrixInt dist;
	dist.resize(m, vector <int>(m));

	//Read number using the extraction (>>) operator
	for (int i = 0; i < m; i++) {
		if (i != m - 1) {
			for (int j = i + 1; j <= (m - 1); j++) {
				in >> number;
				dist[i][j] = number;
				dist[j][i] = number;
			}
			dist[i][i] = 0;
		}
		else {
			dist[i][i] = 0;
		}
	}


	//Close the file stream
	in.close();

	IloEnv env;

	try {
		IloModel model(env);
		
		//Cancel output from CPLEX
		//env.setOut(env.getNullStream());

		//Allocation variables
		//NumVarMatrix y(env, m);
		NumBoolMatrix y(env, m);
		for (int i = 0; i < m; i++) {
			//y[i] = IloNumVarArray(env, m, 0, IloInfinity, ILOFLOAT);
			y[i] = IloBoolVarArray(env, m);
			for (int j = 0; j < m; j++) {
				y[i][j].setName(Get_y_Name(i, j));
				//y[i][j].setName();
			}
		}
		//Location variables
		IloBoolVarArray X(env, m);
		//IloNumVarArray X(env, m);
		for (int i = 0; i < m; i++) {
			X[i] = IloBoolVar(env, Get_x_Name(i));
			//X[i] = IloNumVar(env, 0, IloInfinity, Get_x_Name(i));
		}

		//Objective
		IloExpr obj(env);
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < m; j++) {
				obj += dist[i][j] * y[i][j];
			}
		}
		model.add(IloMinimize(env, obj));

		//Constraint 1: Each demand to be assigned to one facility
		for (int i = 0; i < m; i++) {
			IloExpr cons1(env);
			for (int j = 0; j < m; j++) {
				cons1 += y[i][j];
			}
			model.add(cons1 == 1);
		}

		//Contraint 2: Locate P facilites
		IloExpr cons2(env);
		for (int i = 0; i < m; i++) {
			cons2 += X[i];
			if (i == m - 1) {
				model.add(cons2 == p);
			}
		}

		//Constraint 3: Link location and allocation variable.
		for (int i = 0; i < m; i++) {
			IloExpr cons3(env);
			for (int j = 0; j < m; j++) {
				cons3 = y[i][j] - X[j];
				model.add(cons3 <= 0);
			}
		}


		//Solve
		IloCplex cplex(model);
		cplex.setParam(IloCplex::ClockType, 1); // CPU time
		cplex.setParam(IloCplex::Threads, 1); // Number of threads
		cplex.setParam(IloCplex::TiLim, 1000); // Time limit
		//cplex.extract(model);
		cplex.exportModel("pmedian_lineal.lp");

		//cplex.use(MyBranch(env,X));
		//cplex.use(MyCallback(env));
		if (!cplex.solve()) {
			env.error() << "Failed to optimize LP." << endl;
			throw(-1);
		}

		Optimal_value = cplex.getObjValue();
		//Output Solution
		//IloNumArray vals(env);
		//IloNumArray valsy(env);
		//env.out() << "Solution status = " << cplex.getStatus() << endl;
		env.out() << "Solution value = " << Optimal_value << endl;
		//cplex.getValues(vals, X);
		//env.out() << "Values x= " << vals << endl;
		//for (int i = 0; i < m; i++) {
		//	cplex.getValues(valsy, y[i]);
		//	printf("Values y(%d)= ", i);
		//	env.out() << valsy << endl;
		//}
		//cplex.exportModel("pmedian_CF.lp");

	}
	catch (IloException& ex) {
		cerr << "Concert exception caught: " << ex << endl;
	}
	catch (...) {
		cerr << "Unknown exception caught" << endl;
	}

	env.end();

	total_time = (double)(clock() - clock_initial) / CLOCKS_PER_SEC;


	//Output solution

	ofstream outfile;
	outfile.open("Results/Results_times_1Thread_1000seconds.csv", std::fstream::app);
	puts("Writing to the file");

	outfile << "File," << char(9);
	outfile << "n," << char(9);
	outfile << "p," << char(9);
	outfile << "Optimal," << char(9);
	outfile << "Time," << endl;

	outfile << file_name << char(9);
	outfile << "," << m << char(9);
	outfile << "," << p << char(9);
	outfile << "," << Optimal_value << char(9);
	outfile << "," << total_time << endl;

	//close the opened file.
	outfile.close();

	return 0;
}