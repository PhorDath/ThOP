#ifndef THOP_H
#define THOP_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "util.h"
#include <cmath>
#include <chrono>
#include "effolkronium/random.hpp"

#define itMax 10
#define TLIMIT 300

using namespace std;
using Random = effolkronium::random_static;


const double EC = std::exp(1.0);

struct coord {
	int index;
	double x, y;
};

struct item {
	int		index;
	double	profit, weight, assingnedCityId;
};

struct ParamSa {
	double init = false;
	double tMax;
	double tMin;
	double taxa;


};

struct solution {
	vector<int> route;
	vector<int> items;
	vector<int> permutation;
	double		time;
	vector<int> inv; // guarda quais restricoes foram inviaveis durante a avaliacao
	double		weight;

	bool		feasible;
	bool		r1;
	bool		r2;
	bool		r3;
	double		FO;

	// variavel de decisao
	vector<int>		x; // rota
	vector<bool>	y; // plano de coleta
};

bool compItemWeight	(item a, item b);
bool compItemProfit	(item a, item b);

class genetic {
public:
	int popSize;
	vector<solution> population;

public:
	genetic(int popSize);
	void	initPop();
	~genetic();

};

class thop
{
private:
	ParamSa paramSA;

	// variavel de decisao
	vector<int>		x; // rota
	vector<bool>	y; // plano de coleta

	// parametros do arquivo
	string			name;
	string			knapsackDataType;
	
	int				numItems;
	int				capacity;
	double			maxTime;
	double			minSpeed;
	double			maxSpeed;
	string			edgeWeightType;
	vector<coord>	coords;
	vector<item>	items;

	// parametros de processamento
	double					FO;
	vector<vector<double>>	dist;
	solution				sol;
	//bool					feasible;

public:
	int				dimension;

					thop			(string fileName);	
	void			readSolution	(string fileName);
	double			calcFO			();
	int				eval			();
	int				evalFast		();
	int				evalLento();
	bool			evalRest		();
	solution		eval			(solution s);
	int				r1				();
	int				r2				();
	bool			r3				(solution sol);
	void			greedy			();
	void			greedy2			();
	void			greedy3			();
	void			greedy_weight	();
	void			randomNeigh		(); // troca dois itens de posicao na permutacao 
	void			shake			(int k); // troca dois itens de posicao na permutacao k vezes
	void			lS				(); // troca o i-esimo elemento de lugar
	void			lS2				(); // move o primeiro elemento da solucao pela permutacao ate o final
	void			initSA			();
	void			sA				();
	void			ILS				();
	void			VNS				();
	void			MOVE			(); // seleciona um item qualquer e desloca ele de lugar, obs: diferente de swap
	void			MOVECHUNK		(); //move um pedaco da solucao de lugar (circular)
	void			OPT_2			(); // faz uma vizinhanca 2 opt aleatoria invertendo parcialmente um pedaco da permutacao
	void			OPT_22			(); 
	void			OPT_3			();
	void			INVERT			(); // invert parcialmente a permutacao
	void			VND				();

	// funcoes referente a antiga representacao da solucao, elas ainda podem ser utilizadas porque a struct de solucao ainda contem os dados antigos
	void			localSearch		(); 
	void			localSearchPerm	();
	void			dropBestNode	(); // remove um nó da rota que calsa menor perda de FO
	void			dropNode		(int indice); // remove um nó a partir de um indice
	void			addBestNode		(); // adiciona um nó na rota na posicao pos
	void			addNode			(int pos, int no);
	//

	// funcoes auxiliares
	void			criaSol			(); // a prtir da permutacao cria a solucao no formato antigo
	void			processPerm		(); // pega uma solucao pronta e cria a permutacao dela
	void			processSol		(); // pega uma solucao pronta e cria a var Y dela
	solution		processSol		(solution s);
	void			printInst		();
	void			printSol		();
	void			printVar		();
	void			printInv		();
	bool			inRoute			(int p);
	int				inRoute2		(int p);
	bool			itemInSolution	(item i);
	void			removeItens		(int it); // remove da solução os itens da cidade i
	void			removePonto		(int no);
	vector<item>	getItemByCity	(int city);
	vector<item>	getItemInSolutionByCity(int city);
	double			getFO			();
	double			getTime			();
	double			getweight		();
	solution		getSolution		();
	void			setSolution		(solution s);
	bool			compIT			(int a, int b);
	string			getSol();
					~thop			();

	// genetico
	//genetic			gntc;
	void			genetico		();
	void			greedyItems		(solution &sol); // pega uma solução só com uma rota e coloca itens nela de forma gulosa
};



#endif // !THOP_H

