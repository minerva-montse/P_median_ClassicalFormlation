//Pmedian problem (clasical formulation). Problem with m=4, n=2.
#include <fstream>
#include <iostream>
#include <ilcplex/ilocplex.h>
#include "Aux_Functions.h"

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

	//Read input parameters.
	if (Read_Parameters(argc, argv) == false)
	{
		printf("ERROR: wrong command line.\n");
		printf("The structure is: lineal.exe <data_file> -p p\n");
		printf("\n");
		return 0;
	}

	//Read data from pmed file
	char file_name[50] = "";
	strcat_s(file_name, "data/");
	strcat_s(file_name, argv[1]);

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

	//Close the file stream. Finish reading.
	in.close();

	IloEnv env;

	try {
		IloModel model(env);
		
		//env.setOut(env.getNullStream()); //Cancel output from CPLEX

		//Allocation variables
		//NumVarMatrix y(env, m); //Continuous variables
		NumBoolMatrix y(env, m); //Binary variables
		for (int i = 0; i < m; i++) {
			//y[i] = IloNumVarArray(env, m, 0, IloInfinity, ILOFLOAT); //Continuous variables
			y[i] = IloBoolVarArray(env, m); //Binary variables
			for (int j = 0; j < m; j++) {
				y[i][j].setName(Get_y_Name(i, j));
			}
		}

		//Location variables
		//IloNumVarArray X(env, m); //Continuous variables
		IloBoolVarArray X(env, m); //Binary variables
		for (int i = 0; i < m; i++) {
			//X[i] = IloNumVar(env, 0, IloInfinity, Get_x_Name(i)); //Continuous variables
			X[i] = IloBoolVar(env, Get_x_Name(i)); //Binary variables
		}

		//Objective Function
		IloExpr obj(env);
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < m; j++) {
				obj += dist[i][j] * y[i][j];
			}
		}
		model.add(IloMinimize(env, obj)); //Add objective to the model

		//Constraint 1: Each demand to be assigned to one facility
		for (int i = 0; i < m; i++) {
			IloExpr cons1(env);
			for (int j = 0; j < m; j++) {
				cons1 += y[i][j];
			} 
			model.add(cons1 == 1); //Add constraint 1 to the model
		}

		//Contraint 2: Locate P facilites
		IloExpr cons2(env);
		for (int i = 0; i < m; i++) {
			cons2 += X[i];
			if (i == m - 1) {
				model.add(cons2 == p); //Add constraint 2 to the model
			}
		}

		//Constraint 3: Link location and allocation variable.
		for (int i = 0; i < m; i++) {
			IloExpr cons3(env);
			for (int j = 0; j < m; j++) {
				cons3 = y[i][j] - X[j];
				model.add(cons3 <= 0); //Add constraint 3 to the model
			}
		}


		//Solve
		IloCplex cplex(model);
		cplex.setParam(IloCplex::ClockType, 1); // CPU time
		cplex.setParam(IloCplex::Threads, 1); // Number of threads
		cplex.setParam(IloCplex::TiLim, 1000); // Time limit
		//cplex.exportModel("pmedian_lineal.lp"); //Export model in LP format

		//Use CallBacks
		//cplex.use(MyBranch(env,X));
		//cplex.use(MyCallback(env));


		if (!cplex.solve()) {
			env.error() << "Failed to optimize LP." << endl;
			throw(-1);
		}

		

		//Output Solution
		Optimal_value = cplex.getObjValue();
		IloNumArray vals(env);
		IloNumArray valsy(env);
		env.out() << "Solution status = " << cplex.getStatus() << endl;
		env.out() << "Solution value = " << Optimal_value << endl;
		cplex.getValues(vals, X);
		//Output selected medians
		cout << "Medians x= ";
		for (size_t i = 0; i <m; i++)
		{
			if (vals[i]>0)
			{
				cout << i << ", ";
			}
		}
		cout << endl;
		//Print allocation
		for (int i = 0; i < m; i++) {
			cplex.getValues(valsy, y[i]);
			for (size_t j = 0; j < m; j++)
			{
				if (valsy[j]>0)
				{
					printf("Customer y(%d) to median X(%d)\n", i, j);
				}
			}
		}
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
	outfile.open("Results/Results_times.csv", std::fstream::app);
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