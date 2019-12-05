#include <iostream>
#include <fstream>
#include <iomanip>
#include "thop.h"
#define N 300
using namespace std;

int main() {
	fstream inst("inst.txt", ios::in);
	
	string file;

	int i = 0;

	//saida << "Instancia" << ";" << "FO" << ";" << "Viabilidade" << ";" << "Tempo" << ";" << "Solucao" << ";" << "FO" << ";" << "Viabilidade" << ";" << "Tempo" << ";" << "Solucao" << ";" << "FO" << ";" << "Viabilidade" << ";" << "Tempo" << ";" << "Solucao" << "\n";
	//saida << "id ; inst  ;  tam  ;  itMax  ; FO ; viabilidade ; tempo ; alg \n";

	while (getline(inst, file)) {
		fstream saida("sol.csv", ios::out | ios::app);

		cout << endl << file << endl << endl;
		cout << i << endl;
		if (i < -1) {
			i++;
			continue;
		}
		if (i > N) {
			i++;
			break;
		}
		

		thop p("inst/" + file);

		cout << endl << "Solucao inicial: " << endl;
		p.greedy2();
		//p.lS();
		p.eval();
		
		cout << "Viabilidade: " << p.getSolution().feasible << endl;
		cout << "           : "; p.printInv();
		cout << setprecision(10) << "FO         : " << p.getFO() << endl;
		cout << "Tempo      : " << p.getTime() << endl;
		cout << "Peso       : " << p.getweight() << endl << endl;

		p.printVar();
		p.printSol();

		solution original = p.getSolution();

		// saida << setprecision(10) << file << ";" << p.getFO() << ";" << p.getSolution().feasible << ";" << p.getTime() << ";" << p.getSol() << ";";
		saida << setprecision(10) << i + 87 << " ; " << file << " ; " << p.dimension << " ; " << itMax << " ; " << p.getFO() << " ; " << p.getSolution().feasible << " ; " << p.getTime() << " ; " << " greed " << endl;

		cout << endl << "Busca local: " << endl;
		p.lS();
		p.eval();

		cout << "Viabilidade: " << p.getSolution().feasible << endl;
		cout << "           : "; p.printInv();
		cout << setprecision(10) << "FO         : " << p.getFO() << endl;
		cout << "Tempo      : " << p.getTime() << endl;
		cout << "Peso       : " << p.getweight() << endl << endl;

		p.printVar();
		p.printSol();
		
		
		/*
		cout << endl << "Random Neigh: " << endl;
		p.randomNeigh();
		p.lS();
		p.eval();

		cout << "Viabilidade: " << p.getSolution().feasible << endl;
		cout << "           : "; p.printInv();
		cout << setprecision(10) << "FO         : " << p.getFO() << endl;
		cout << "Tempo      : " << p.getTime() << endl;
		cout << "Peso       : " << p.getweight() << endl << endl;

		p.printVar();
		p.printSol();
		*/
		
		p.setSolution(original);
		cout << endl << "Simulated Annealing: " << endl;
		p.initSA();
		p.sA();
		p.eval();

		cout << "Viabilidade: " << p.getSolution().feasible << endl;
		cout << "           : "; p.printInv();
		cout << setprecision(10) << "FO         : " << p.getFO() << endl;
		cout << "Tempo      : " << p.getTime() << endl;
		cout << "Peso       : " << p.getweight() << endl << endl;

		p.printVar();
		p.printSol();
		
		// saida << setprecision(10) << p.getSolution().FO << ";" << p.getSolution().feasible << ";" << p.getTime() << ";" << p.getSol()	<< ";";
		saida << setprecision(10) << i + 87 << " ; " << file << " ; " << p.dimension << " ; " << itMax << " ; " << p.getFO() << " ; " << p.getSolution().feasible << " ; " << p.getTime() << " ; " << " SA " << endl;
		
		/*
		cout << endl << "ILS: " << endl;
		p.ILS();
		p.eval();

		cout << "Viabilidade: " << p.getSolution().feasible << endl;
		cout << "           : "; p.printInv();
		cout << setprecision(10) << "FO         : " << p.getFO() << endl;
		cout << "Tempo      : " << p.getTime() << endl;
		cout << "Peso       : " << p.getweight() << endl << endl;

		p.printVar();
		p.printSol();
		*/

		
		p.setSolution(original);
		cout << endl << "VNS: " << endl;
		p.VNS();
		p.eval();

		cout << "Viabilidade: " << p.getSolution().feasible << endl;
		cout << "           : "; p.printInv();
		cout << setprecision(10) << "FO         : " << p.getFO() << endl;
		cout << "Tempo      : " << p.getTime() << endl;
		cout << "Peso       : " << p.getweight() << endl << endl;

		p.printVar();
		p.printSol();
		
		//saida << setprecision(10) << p.getSolution().FO << ";" << p.getSolution().feasible << ";" << p.getTime() << ";" << p.getSol();
		saida << setprecision(10) << i + 87 << " ; " << file << " ; " << p.dimension << " ; " << itMax << " ; " << p.getFO() << " ; " << p.getSolution().feasible << " ; " << p.getTime() << " ; " << " VNS " << endl;

		//saida << "\n";
		
		cout << i << endl;
		i = i + 1;

		saida.close();
	}

	
	inst.close();
	return 0;
}