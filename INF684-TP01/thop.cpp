#include "thop.h"

bool compItemWeight(item a, item b)
{
	return (a.weight < b.weight);
}

bool compItemProfit(item a, item b)
{
	return (a.profit < b.profit);
}

bool compItemProfitInv(item a, item b)
{
	return (a.profit > b.profit);
}

thop::thop(string fileName)
{
	this->FO = -1;
	this->sol.time = -1;

	// le o arquivo
	fstream arq;
	arq.open(fileName, ios::in);

	int cont = 1;
	int coordsRead = 0;
	int itemsRead = 0;
	string line;
	while (getline(arq, line)) {
		vector<string> tokens = split(line, ':');
		//cout << line << endl;
		if (cont == 1) { // PROBLEM NAME
			this->name = tokens.at(tokens.size() - 1);
		}
		else if (cont == 2) { // KNAPSACK DATA TYPE
			this->knapsackDataType = tokens.at(tokens.size() - 1);
		}			
		else if (cont == 3) // DIMENSION
			this->dimension = stoi(tokens.at(tokens.size() - 1));
		else if (cont == 4)
			this->numItems = stoi(tokens.at(tokens.size() - 1));
		else if (cont == 5)
			this->capacity = stoi(tokens.at(tokens.size() - 1));
		else if (cont == 6) // MAX TIME
			this->maxTime = stod(tokens.at(tokens.size() - 1));
		else if (cont == 7) // MIN SPEED
			this->minSpeed = stod(tokens.at(tokens.size() - 1));
		else if (cont == 8) // MAX SPEED
			this->maxSpeed = stod(tokens.at(tokens.size() - 1));
		else if (cont == 9)
			this->edgeWeightType = tokens.at(tokens.size() - 1);
		else if (cont > 10 && cont <= 10 + this->dimension) { // le os pontos
			//vector<string> comps = split();
			vector<string> tokens = split(line, ' ');
			coord c;
			c.index = stoi(tokens.at(0));// -1;
			c.x = stod(tokens.at(1));
			c.y = stod(tokens.at(2));
			this->coords.push_back(c);
			coordsRead++;
		}
		else if (cont > 10 + this->dimension + 1 && cont <= 10 + this->dimension + 1 + this->numItems) { // le os itens
			//vector<string> tokens = split2(line, ' ', '\t');
			vector<string> tokens = split(line, ' ');
			
			if (tokens.size() < 4) {
				cout << "Erro!" << endl;
				return;
			}

			item i;
			i.index = stoi(tokens[0]);
			i.profit = stoi(tokens[1]);
			i.weight = stoi(tokens[2]);
			i.assingnedCityId = stoi(tokens[3]);// -1;

			items.push_back(i);
			itemsRead++;
		}
		cont++;
	}

	if (this->items.size() != this->numItems) {
		cout << "Erro 1..." << endl;
	}
	if (this->coords.size() != this->dimension) {
		cout << "Erro 2..." << endl;
	}

	// preprocessamento

	// aloca a variavel de decisao y
	this->y.resize(numItems, false);

	// aloca e calcula a matriz de distancia
	dist.resize(this->dimension);
	for (int i = 0; i < this->dimension; i++) {
		dist[i].resize(this->dimension);
	}

	for (int i = 0; i < dimension; i++) {
		for (int j = 0; j < dimension; j++) {
			dist[i][j] = ceil(sqrt( pow(coords[i].x - coords[j].x, 2) + pow(coords[i].y - coords[j].y, 2)));
		}
	}

	sol.r1 = false;
	sol.r2 = false;
	sol.r3 = false;
	sol.feasible = false;

	// permutacao
	sol.permutation.resize(numItems);
	for (int i = 0; i < sol.permutation.size(); i++) {
		sol.permutation[i] = i+1;
	}



}

void thop::readSolution(string fileName)
{
	this->sol.route.push_back(0); // coloca a ponto incial na rota

	fstream arq;
	arq.open(fileName, ios::in);

	string line;
	int cont = 1;

	while (getline(arq, line)) {
		//vector<string> tokens = split2(line, '[', ',');
		line.erase(0, 1);
		line.erase(line.end()-1, line.end());
		vector<string> tokens = split(line, ',');

		if (cont == 1) { // le rota
			for (int i = 0; i < tokens.size(); i++) {
				this->sol.route.push_back(stoi(tokens.at(i)) -1 );
			}
		}
		else if (cont == 2) { // le item
			for (int i = 0; i < tokens.size(); i++) {
				this->sol.items.push_back(stoi(tokens.at(i)));
			}
		}		
		cont++;
	}

	this->sol.route.push_back(this->dimension - 1);
	//cout << "oi" << endl;
	//cout << this->sol.items.size() << endl;
	// seta as variaveis de decisao com base na solucao lida
	for (int i = 0; i < this->sol.items.size(); i++) {
		//cout << sol.items[i] - 1 << endl;
		y[sol.items[i]-1] = true;
	}
}

double thop::calcFO()
{
	FO = 0;
	for (int i = 0; i < this->sol.y.size(); i++) {
		//cout << items[i].weight << endl;
		FO += this->sol.y[i] * items[i].profit;
	}

	return FO;
}

int thop::eval()
{
	processSol();

	sol.feasible = true;
	sol.r1 = false;
	sol.r2 = false;
	sol.r3 = false;
	// restricao  1
	// um item só pode ser coletado se o ponto de controle onde est´a localizado faz parte da rota:
	for (int i = 0; i < this->sol.y.size(); i++) {
		if (this->sol.y[i] == 1 && inRoute(items[i].assingnedCityId) == false) {
			sol.feasible = false;
			sol.r1 = true;
		}
	}

	// restricao 2
	// o peso total dos itens coletados nao pode exceder a capacidade da mochila:
	double sum = 0;
	for (int i = 0; i < this->sol.y.size(); i++) {
		sum += items[i].weight * this->sol.y[i];
	}
	sol.weight = sum;
	if (sum > this->capacity) {
		sol.feasible = false;;
		sol.r2 = true;
	}

	// restricao 3
	// o tempo total da rota nao pode exceder o tempo limite
	double sum2 = 0;
	double PW = 0; // peso parcial
	double v = (maxSpeed - minSpeed) / capacity;
	for (int i = 0; i < this->sol.route.size() - 1; i++) {
		// calcula o peso parcial da mochila W ao se percorrer i na rota
		vector<item> it = getItemByCity(sol.route[i]);
		for (auto a : it) {
			if (this->sol.y[a.index - 1] == true) {
				PW += a.weight;
			}
		}
		double speed = (maxSpeed - (v * PW));
		if (speed < 0.1) { // nao deixa a velocidade ficar menor do que o minimo
			speed = 0.1;
		}
		sum2 += dist[sol.route[i]][sol.route[i + 1]] / speed;
	}
	if (sum2 > maxTime) {
		sol.feasible = false;
		sol.time = sum2;
		sol.r3 = true;

	}
	sol.time = sum2;

	sol.FO = calcFO();

	return 0;
}

int thop::evalFast() // igual eval mas encerra assim que a solucao nao for viavel
{
	processSol();

	sol.feasible = true;
	sol.r1 = false;
	sol.r2 = false;
	sol.r3 = false;
	// restricao  1
	// um item só pode ser coletado se o ponto de controle onde est´a localizado faz parte da rota:
	for (int i = 0; i < this->sol.y.size(); i++) {
		if (this->sol.y[i] == 1 && inRoute(items[i].assingnedCityId) == false) {
			sol.feasible = false;
			sol.r1 = true;
			return 1;
		}
	}

	// restricao 2
	// o peso total dos itens coletados nao pode exceder a capacidade da mochila:
	double sum = 0;
	for (int i = 0; i < this->sol.y.size(); i++) {
		sum += items[i].weight * this->sol.y[i];

		if (sum > this->capacity) {
			sol.feasible = false;
			sol.r2 = true;
			return 2;
		}
	}
	sol.weight = sum;


	// restricao 3
	// o tempo total da rota nao pode exceder o tempo limite
	double sum2 = 0;
	double PW = 0; // peso parcial
	double v = (maxSpeed - minSpeed) / capacity;
	for (int i = 0; i < this->sol.route.size() - 1; i++) {
		// calcula o peso parcial da mochila W ao se percorrer i na rota
		vector<item> it = getItemByCity(sol.route[i]);
		for (auto a : it) {
			if (this->sol.y[a.index - 1] == true) {
				PW += a.weight;
			}
		}
		double speed = (maxSpeed - (v * PW));
		if (speed < 0.1) { // nao deixa a velocidade ficar menor do que o minimo
			speed = 0.1;
		}
		sum2 += dist[sol.route[i]][sol.route[i + 1]] / speed;

		if (sum2 > maxTime) {
			sol.feasible = false;
			sol.time = sum2;
			sol.r3 = true;
			return 3;
		}
	}

	sol.time = sum2;

	sol.FO = calcFO();

	return 0;
}

// verifica restrições
int thop::evalLento()
{
	processSol();

	sol.feasible = true;
	sol.r1 = false;
	sol.r2 = false;
	sol.r3 = false;
	// restricao  1
	// um item só pode ser coletado se o ponto de controle onde est´a localizado faz parte da rota:
	for (int i = 0; i < this->sol.y.size(); i++) {
		if (this->sol.y[i] == 1 && inRoute(items[i].assingnedCityId) == false) {
			sol.feasible = false;
			sol.r1 = true;
		}
	}

	// restricao 2
	// o peso total dos itens coletados nao pode exceder a capacidade da mochila:
	double sum = 0;
	for (int i = 0; i < this->sol.y.size(); i++) {
		sum += items[i].weight * this->sol.y[i];
	}
	sol.weight = sum;
	if (sum > this->capacity) {
		sol.feasible = false;;
		sol.r2 = true;
	}	

	// restricao 3
	// o tempo total da rota nao pode exceder o tempo limite
	double sum2 = 0;
	double PW = 0; // peso parcial
	double v = (maxSpeed - minSpeed) / capacity;
	for (int i = 0; i < this->sol.route.size() - 1; i++) {
		// calcula o peso parcial da mochila W ao se percorrer i na rota
		vector<item> it = getItemByCity(sol.route[i]);
		for (auto a : it) {
			if (this->sol.y[a.index - 1] == true) {
				PW += a.weight;
			}
		}
		double speed = (maxSpeed - (v * PW));
		if (speed < 0.1) { // nao deixa a velocidade ficar menor do que o minimo
			speed = 0.1;
		}
		sum2 += dist[sol.route[i]][sol.route[i + 1]] / speed;
	}
	if (sum2 > maxTime) {
		sol.feasible = false;
		sol.time = sum2;
		sol.r3 = true;

	}
	sol.time = sum2;

	sol.FO = calcFO();

	return 0;
}

bool thop::evalRest()
{
	processSol();

	sol.feasible = true;
	sol.r1 = false;
	sol.r2 = false;
	sol.r3 = false;
	// restricao  1
	// um item só pode ser coletado se o ponto de controle onde est´a localizado faz parte da rota:
	for (int i = 0; i < this->sol.y.size(); i++) {
		if (this->sol.y[i] == 1 && inRoute(items[i].assingnedCityId) == false) {
			sol.feasible = false;
			sol.r1 = true;
			return false;
		}
	}

	// restricao 2
	// o peso total dos itens coletados nao pode exceder a capacidade da mochila:
	double sum = 0;
	for (int i = 0; i < this->sol.y.size(); i++) {
		sum += items[i].weight * this->sol.y[i];
	}
	sol.weight = sum;
	if (sum > this->capacity) {
		sol.feasible = false;;
		sol.r2 = true;
		return false;
	}

	// restricao 3
	// o tempo total da rota nao pode exceder o tempo limite
	double sum2 = 0;
	double PW = 0; // peso parcial
	double v = (maxSpeed - minSpeed) / capacity;
	for (int i = 0; i < this->sol.route.size() - 1; i++) {
		// calcula o peso parcial da mochila W ao se percorrer i na rota
		vector<item> it = getItemByCity(sol.route[i]);
		for (auto a : it) {
			if (this->sol.y[a.index - 1] == true) {
				PW += a.weight;
			}
		}
		double speed = (maxSpeed - (v * PW));
		if (speed < 0.1) { // nao deixa a velocidade ficar menor do que o minimo
			speed = 0.1;
		}
		sum2 += dist[sol.route[i]][sol.route[i + 1]] / speed;
	}
	if (sum2 > maxTime) {
		sol.feasible = false;
		sol.time = sum2;
		sol.r3 = true;
		return false;
	}
	sol.time = sum2;

	sol.FO = calcFO();

	return true;
}

solution thop::eval(solution s)
{
	processSol();

	s.feasible = true;
	s.r1 = false;
	s.r2 = false;
	s.r3 = false;
	// restricao  1
	// um item só pode ser coletado se o ponto de controle onde est´a localizado faz parte da rota:
	for (int i = 0; i < s.y.size(); i++) {
		if (s.y[i] == 1 && inRoute(items[i].assingnedCityId) == false) {
			s.feasible = false;
			s.r1 = true;
		}
	}

	// restricao 2
	// o peso total dos itens coletados nao pode exceder a capacidade da mochila:
	double sum = 0;
	for (int i = 0; i < s.y.size(); i++) {
		sum += items[i].weight * s.y[i];
	}
	s.weight = sum;
	if (sum > this->capacity) {
		s.feasible = false;;
		s.r2 = true;
	}

	// restricao 3
	// o tempo total da rota nao pode exceder o tempo limite
	double sum2 = 0;
	double PW = 0; // peso parcial
	double v = (maxSpeed - minSpeed) / capacity;
	for (int i = 0; i < s.route.size() - 1; i++) {
		// calcula o peso parcial da mochila W ao se percorrer i na rota
		vector<item> it = getItemByCity(s.route[i]);
		for (auto a : it) {
			if (s.y[a.index - 1] == true) {
				PW += a.weight;
			}
		}
		double speed = (maxSpeed - (v * PW));
		if (speed < 0.1) { // nao deixa a velocidade ficar menor do que o minimo
			speed = 0.1;
		}
		sum2 += dist[s.route[i]][s.route[i + 1]] / speed;
	}
	if (sum2 > maxTime) {
		s.feasible = false;
		s.time = sum2;
		s.r3 = true;

	}
	s.time = sum2;

	s.FO = calcFO();

	return s;
}

int thop::r1()
{
	// restricao  1
	// um item s´o pode ser coletado se o ponto de controle onde est´a localizado faz parte da rota:	
	for (int i = 0; i < y.size(); i++) {
		if (y[i] == 1 && inRoute(items[i].assingnedCityId) == false) {
			sol.feasible = false;
			return 1;
		}
	}
}

int thop::r2()
{
	double sum = 0;
	for (int i = 0; i < this->y.size(); i++) {
		sum += items[i].weight * y[i];
	}
	if (sum > this->capacity) {
		sol.feasible = false;
		return 2;
	}
}

bool thop::r3(solution sol)
{
	// restricao 3
	// o tempo total da rota nao pode exceder o tempo limite
	double sum2 = 0;
	double PW = 0; // peso parcial
	double v = (maxSpeed - minSpeed) / capacity;
	for (int i = 0; i < sol.route.size() - 1; i++) {
		// calcula o peso parcial da mochila W ao se percorrer i na rota
		//PW = 0;
		vector<item> it = getItemByCity(sol.route[i]);
		for (auto a : it) {
			if (sol.y[a.index - 1] == true) {
				PW += a.weight;
			}
		}
		//cout << "peso parcial ao passar por " << i << ": " << PW << endl;
		// calcula tempo parcial
		//cout << sol.route[i] << " - " << sol.route[i+1] << endl;
		//cout << sum2 << " += " << dist[sol.route[i]][sol.route[i+1]] << " / " << (maxSpeed - (v * PW)) << endl;
		//cout << dist[sol.route[i]][sol.route[i + 1]] / (maxSpeed - (v * PW)) << endl;
		double speed = (maxSpeed - (v * PW));
		if (speed < 0.1) { // nao deixa a velocidade ficar menor do que o minimo
			speed = 0.1;
		}
		sum2 += dist[sol.route[i]][sol.route[i + 1]] / speed;
	}

	if (sum2 > maxTime) {
		sol.feasible = false;
		return false;
	}
	else {
		return true;
	}
	//this->sol.time = sum2;
}

void thop::greedy() // guloso pelo peso
{
	/*
	vector<item> it = this->items;

	sort(it.begin(), it.end(), compItemWeight); // ordena itens por peso
	double peso = 0;

	srand(time(NULL));

	for (int i = 0; i<it.size(); i++) {
		int qt = rand() % 5; // escolhe entre os 5 com menor peso para colocar na solução
		if (qt + i > it.size() - 1) {
			qt = 0;
		}
		int ls = qt + i; // tamanho da lista restrita de candidatos
		//ls = i;
		solution s_cand = this->sol;

		if (it[ls].weight + peso <= this->capacity) { // se couber
			peso += it[ls].weight;
			s_cand.route.push_back(it[ls].assingnedCityId); // adiciona o ponto na rota
			s_cand.items.push_back(it[ls].index); // adiciona o item no plano de coleta

			s_cand = processSol(s_cand);

			if (r3(s_cand)) { // nao violar a restrição 3 aceita a insercao
				this->sol = s_cand;
			}
		}
	}
	sort(this->sol.items.begin(), this->sol.items.end()); // ordena os items

	// seta as variaveis de decisao com base na solucao gerada
	processSol();
	*/
}

void thop::greedy2() // guloso pelo lucro
{
	vector<item> it = this->items;

	sort(it.begin(), it.end(), compItemProfitInv); // ordena itens por lucro
	double peso = 0;

	//srand(time(NULL));

	for (int i = 0; i<it.size(); i++) {
		//int qt = rand() % 5; // escolhe entre os 5 com menor peso para colocar na solução
		auto qt = Random::get(0, 5);
		if (qt + i > it.size() - 1) {
			qt = 0;
		}
		int ls = qt + i; // tamanho da lista restrita de candidatos
		ls = i;
		solution s_cand = this->sol;

		if (it[ls].weight + peso <= this->capacity) { // se couber
			peso += it[ls].weight;
			
			// isso evita da pessoa ir em um lugar e depois voltar para buscar outro item, isso faz a FO reduzir
			if (inRoute(it[i].assingnedCityId)) { // se estiver na rota adiciona so o item no plano de coleta
				s_cand.items.push_back(it[ls].index); // adiciona o item no plano de coleta
			}
			else { // se nao estiver na rota adiciona o local e o item
				s_cand.items.push_back(it[ls].index); // adiciona o item no plano de coleta
				s_cand.route.push_back(it[ls].assingnedCityId); // adiciona o ponto na rota
			}

			s_cand = processSol(s_cand); // processa as variaveis de decisao

			if (r3(s_cand)) { // nao violar a restrição 3 aceita a insercao
				this->sol = s_cand;
			}
		}
	}
	sort(this->sol.items.begin(), this->sol.items.end()); // ordena os items

	// seta as variaveis de decisao com base na solucao gerada
	processSol();
	processPerm();
}

void thop::greedy3() // guloso pelo lucro
{
	
}

void thop::greedy_weight()
{
	//for (int i = 0; i < this->capacity; i++) {
	vector<item> it = this->items;

	sort(it.begin(), it.end(), compItemWeight); // ordena itens por peso
	double peso = 0;

	srand(time(NULL));

	//for (auto item : it) {
	for (int i  =0; i<it.size(); i++){
		int qt = rand() % 5; // escolhe entre os 5 com menor peso para colocar na solução
		if (qt + i > it.size() - 1) { 
			qt = 0;
		}
		int ls = qt + i; // tamanho da lista restrita de candidatos

		if (it[ls].weight + peso <= this->capacity) { // se couber
			peso += it[ls].weight;
			this->sol.route.push_back(it[ls].assingnedCityId); // adiciona o ponto na rota
			this->sol.items.push_back(it[ls].index); // adiciona o item no plano de coleta
		}
	}
	sort(this->sol.items.begin(), this->sol.items.end()); // ordena os items

}

void thop::randomNeigh()
{
	srand(time(NULL));
	//double i, j;

	// sorteia duas posições diferentes da permutacao
	//i = (rand() % this->items.size());
	int n = items.size();

	auto i = Random::get(1, n - 1);

	//j = (rand() % this->items.size());
	auto j = Random::get(1, n - 1);

	//cout << i << " - " << j << endl;
	if (i == j) {
		while (i == j) {
			j = (rand() % this->items.size());
		}
	}
	// faz o swap dessas duas posicoes
	int aux = sol.permutation[i];
	sol.permutation[i] = sol.permutation[j];
	sol.permutation[j] = aux;

	criaSol();
	eval();
}

void thop::shake(int k)
{
	for (int i = 0; i < k; i++) {
		randomNeigh();
	}
}

// troca itens na permutação
void thop::lS()
{
	solution original = sol;
	solution best = sol;

	int cont = 0; // iteracoes sem melhora
	for (int i = 0; i < sol.permutation.size()-1; i++) {
		for (int j = i+1; j < sol.permutation.size(); j++) {

			int aux = sol.permutation[i];
			sol.permutation[i] = sol.permutation[j];
			sol.permutation[j] = aux;
			//cout << "a\n";
			criaSol();
			//cout << "b\n";
			processSol();
			//cout << "c\n";
			//processPerm();
			//cout << "d\n";
			calcFO();
			//evalFast();
			//cout << "e\n";

			if (sol.FO > best.FO) {
				best = sol;
				cont = 0;
				return; // first improvment
			}
			/*
			else {
				cont++;
				if (cont == ceil(sol.permutation.size() / 2)) {
					//cout << "Parou a ls\n";
					return;
				}
			}
			*/
		}
	}

	eval();
}

void thop::lS2()
{
	solution original = sol;
	solution best = sol;

	int cont = 0; // iteracoes sem melhora
	int n = sol.items[0]; // pega o primeiro item da solucao para mudar ele de posicao

	for (int i = 0; i < sol.permutation.size(); i++) {
		if (n == sol.permutation[i]) {
			n = i;
			break;
		}
	}

	for (int i = n; i < sol.permutation.size() - 1; i++) {

		int aux = sol.permutation[i + 1];
		sol.permutation[i + 1] = sol.permutation[i];
		sol.permutation[i] = aux;
		//cout << "a\n";
		criaSol();
		//cout << "b\n";
		processSol();
		//cout << "c\n";
		//processPerm();
		//cout << "d\n";
		this->calcFO();
		//evalFast();
		//cout << "e\n";

		if (sol.FO > best.FO) {
			best = sol;
			cont = 0;
			return; // first improvment
		}
		/*
		else {
			cont++;
			if (cont == ceil(sol.permutation.size() / 2)) {
				//cout << "Parou a ls\n";
				return;
			}
		}
		*/
	}

	eval();
}

void thop::initSA()
{
	paramSA.init = true;
	paramSA.taxa = 0.80;
	paramSA.tMax = 100;
	paramSA.tMin = 1;
}



void thop::sA()
{

	if (paramSA.init == false) {
		cout << "Parametros do SA nao iniciados...\n";
	}

	solution best = sol;
	int iter = 0;

	auto t1 = std::chrono::high_resolution_clock::now();
	bool timeout = false;


	double t = paramSA.tMax;
	while (t >= paramSA.tMin || timeout == false) { // enquanto a temperatura atual nao for menor do que a temperatura minima
		cout << "Temp: " << t << endl;
		iter = 0;
		while (iter < itMax) { // iteracoes sem melhora

			auto t2 = std::chrono::high_resolution_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
			if (duration > TLIMIT) {
				timeout = true;
				break;
			}


			iter++;
			// gere um vizinho qualquer
			solution original = sol;
			randomNeigh();
			lS2();
			solution viz = sol;
			sol = original;
			//cout << "cont: " << cont << endl;

			double delta = viz.FO - sol.FO;
			
			//cout << "n: " << n << endl;
			//cout << "x: " << x << endl;


			if (viz.FO > sol.FO) { // se a solucao vizinha for melhor do que a solucao corrente aceita
				//cout << "Melhorou\n";
				//printSol();
				//eval();
				
				sol = viz; // aceita viz como a melhor solucao corrente

				if (sol.FO > best.FO) {
					best = sol;
					iter = 0;
					cout << "FO: " << sol.FO << endl;
				}


			}
			else{ // se nao teve melhora
				double x = double(Random::get(0, 100)) / 100.0; // sorteia um numero de 0 a 1
				double n = pow(EC, delta / t);
				if (x < n) { // aceita solucao de piora
					sol = viz;
				}
			}
		}
		t = t * paramSA.taxa;
		//sol = best;
	}
	sol = best;
	
}


void thop::ILS()
{
	lS();
	int it = 0;
	solution best = sol;
	int k = 2;

	while (it < itMax) { // numero de iteracoes sem melhora
		//cout << it << endl;
		solution original = sol;
		shake(k);
		lS();

		//cout << sol.FO << endl;
		//printSol();

		if (sol.FO > best.FO) { // se melhorou
			best = sol;
			k = 2;
			it = 0;
		}
		else { // se nao melhorou
			k++; // aumenta o shake
			it++; // mais uma iteracao sem melhora
		}
	}
}

void thop::VNS()
{
	solution best = sol;
	int it = 0;

	auto t1 = std::chrono::high_resolution_clock::now();
	bool timeout = false;

	while (it < itMax || timeout == false) // numero de iteracaoes sem melhoras
	{
		int k = 1; // vizinhancas
			
		while (k <= 5) {

			auto t2 = std::chrono::high_resolution_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
			if (duration > TLIMIT) {
				timeout = true;
				break;
			}
			solution original = sol;

			// escolhe a vizinhanca
			if (k == 1) { // vizinhanca 1
				randomNeigh();
			}
			else if (k == 2) { // move
				MOVE();
			}
			else if (k == 3) { // 2 OPT
				INVERT();
			}
			else if (k == 4) { // INVERT
				OPT_2();
			}
			else if (k == 5) { // INVERT
				OPT_22();
			}

			// shake, intensifica quanto mais iteracoes sem melhoras
			shake(2);

			solution viz = sol;
			sol = original;			

			// busca local
			lS();
			//VND();

			if (viz.FO > best.FO) { // se for melhor do que a melhor, aceita
				//cout << "melhorou\n";
				eval();
				best = viz; // guarda a melhor
				sol = viz; // muda de solucao
				cout << "FO: " << sol.FO << endl;

				k = 1; // volta para a primeira vizinhaca 
				it = 0;
			}
			else {
				k++; // proxima vizinhanca
				it++;
			}
		}
		if (timeout == true) {
			break;
		}
	}
	sol = best;
}

void thop::MOVE()
{
	int n = sol.items.size() -1;
	int x = Random::get(0, n); // seleciona qual item vai ser movido
	int t = Random::get(0, n - 1); // determina quantas posicoes ele vai ser movido (circular)

	int cont = 0;
	while (cont <= t) {
		if (x == n) {
			// swap
			int aux = sol.permutation[0];
			sol.permutation[0] = sol.permutation[n];
			sol.permutation[n] = aux;
			x = 0;
		}
		else {
			// swap
			int aux = sol.permutation[x + 1];
			sol.permutation[x + 1] = sol.permutation[x];
			sol.permutation[x] = aux;
		}	

		cont++;
	}

	criaSol();
	//eval();
	this->calcFO();
}

void thop::MOVECHUNK()
{
	int n = sol.items.size() - 1;
	int x = Random::get(0, n); // seleciona qual item vai ser movido
	int qt = n / 10;// Random::get(1, n/10); // quantos itens serao movidos////////////////////////////////////////////
	int t = Random::get(0, n - 1); // determina quantas posicoes ele vai ser movido (circular)

	x = x + qt;
	if (x > n) {
		x = qt+1;
	}

	while (x > x - qt) {


		int cont = 0;
		while (cont <= t) {

			if (x == n) {
				// swap
				int aux = sol.permutation[0];
				sol.permutation[0] = sol.permutation[n];
				sol.permutation[n] = aux;
				x = 0;
			}
			else {
				// swap
				int aux = sol.permutation[x + 1];
				sol.permutation[x + 1] = sol.permutation[x];
				sol.permutation[x] = aux;
			}

			cont++;
		}
		x--;
	}
	criaSol();
	//eval();
	calcFO();
}

void thop::OPT_2()
{
	int n = sol.permutation.size() - 1;
	auto x = Random::get(0, n); // posicao inicial que vai ser trocada

	auto y = Random::get(5, int(n / 25)); // posicao até onde sera invertido a ordem dos itens
	if (y + x > n) { // verifica se a troca alcancaria posicoes alem do final da permutacao
		y = n - x; // nao deixa a trocar acessar posicoes invalidas
	}
	
	int i = x;
	int j = y;
	while (i < j) {//(i < ceil((x + y) / 2)) {
		int aux = sol.permutation[i];
		sol.permutation[i] = sol.permutation[j];
		sol.permutation[j] = aux;
		i++;
		j--;
	}

	criaSol();
	//eval();
	this->calcFO();
}

void thop::OPT_22()
{
	OPT_2();
	OPT_2();
}

void thop::OPT_3()
{
}

void thop::INVERT()
{
	int n = sol.permutation.size() - 1;
	int k = Random::get(0, (n/2)-1);
	
	int i = 0;
	int j = sol.permutation.size()-1;

	while (i < j) {		
		int aux = sol.permutation[i];
		sol.permutation[i] = sol.permutation[j];
		sol.permutation[j] = aux;
		i++;
		j--;
	}

	criaSol();
	//eval();
	calcFO();
}

void thop::VND()
{
	solution best = sol;
	
	int k = 1; // vizinhancas

	while (k < 5) {
		solution original = sol;

		// escolhe a vizinhanca
		if (k == 1) { // vizinhanca 1
			randomNeigh();
		}
		else if (k == 2) { // move
			MOVE();
		}
		else if (k == 3) { // 2 OPT
			INVERT();
		}
		else if (k == 4) { // INVERT

			OPT_2();
		}

		solution viz = sol;
		sol = original;

		// busca local
		lS();

		if (viz.FO > best.FO) { // se for melhor do que a melhor, aceita
			eval();
			best = viz; // guarda a melhor
			sol = viz; // muda de solucao

			k = 1; // volta para a primeira vizinhaca 
		}
		else {
			k++; // proxima vizinhanca
		}

	}
	
	sol = best;
}



void thop::localSearch() // vizinhanca que para cada no da rota muda para outro no
{
	solution original = this->sol;
	solution best = this->sol;

	bool melhora = false;

	srand(time(NULL));

	//while (melhora == false) {
		for (int i = 0; i < sol.route.size(); i++) { // percorre a rota

			for (int j = 0; j < numItems; j++) { // para cada posição da rota troca com todas as possiveis localidades
				// remove o nó i da rota
				//sol.route.erase(sol.route.begin() + i); // nao precisa dessa linha pq abaixo esse no vai ser sobescrito
				// insere o proximo nó na rota
				if (!inRoute(j)) { // se nao estiver na rota simplesmente insere
					removeItens(sol.route[i]); // antes de colocar o j na rota é necessário remover os itens do lugar que ele substitui

					sol.route[i] = j; // coloca j na rota

					vector<item> itensJ = getItemByCity(j); // pega todos os itens do nó j

					
					// escolhe um ou mais itens de J para colocar na rota		
					for (int k = 0; k < itensJ.size(); k++) {
						int item = itensJ[k].index;
						sol.items.push_back(item); // coloca o item na solução

						//for (auto c : itensJ) {
						//	cout << c.index << " - " << c.profit << " - " << c.weight << " - " << c.assingnedCityId << endl;
						//}

						//printSol();
						//cout << "FO: " << sol.FO << endl;
						
						eval(); // avalia
						//cout << "F: " << sol.feasible << endl;
						//getchar();

						if (sol.feasible == false) { // se for inviavel com esse novo item, remove
							// sol.items.erase(sol.items.end()); // remove
							for (int a = 0; a < sol.items.size(); a++) { // remove alternativo
								if (item == sol.items[a]) {
									sol.items.erase(sol.items.begin() + a);
									break;
								}
							}
							
						}
						else { // se for viavel verifica se é melhor
							if (sol.FO > best.FO) { // se a solução vizinha for melhor aceita ela como a melhor até o memento
								best.FO = sol.FO;
								melhora = true;
								// break; // first improvment
								// se for first improvment então simplemente para na primeira vez que encontrar um melhor
							}
						}
						

					}
				}
				else { // se ja estiver na rota coloca um item
					vector<item> itensJ = getItemByCity(j); // pega todos os itens do nó j
					// escolhe um ou mais itens de J para colocar na rota
					for (int k = 0; k < itensJ.size(); k++) {
						int item = itensJ[k].index;
						sol.items.push_back(itensJ[k].index - 1); // coloca o item na solução
						
						//processSol(); // coloquei no eval para sempre fazer esse procedimento
						eval(); // avalia
						
						if (sol.feasible == false) { // se for inviavel com esse novo item, remove
							// sol.items.erase(sol.items.end()); // remove
							for (int a = 0; a < sol.items.size(); a++) { // remove alternativo
								if (item == sol.items[a]) {
									sol.items.erase(sol.items.begin() + a);
									break;
								}
							}
						}
						else { // se for viavel verifica se é melhor
							if (sol.FO > best.FO) { // se a solução vizinha for melhor aceita ela como a melhor até o memento
								best.FO = sol.FO;
								melhora = true;
								// break; // first improvment
								// se for first improvment então simplemente para na primeira vez que encontrar um melhor
							}
						}						
					}
				}				
			}
			sol = original; // volta a solução para a inicial para continuar fazendo os movimentos em outros pontos da rota
		}
	//}	
}

void thop::localSearchPerm()
{
 
}

void thop::dropBestNode() // estrutura de vizinhança que remove um nó da rota. Como é mais provável que a FO piore, aqui aceitamos movimento de piora, no caso, o que piora menos
{
	if (sol.route.size() == 0) { // se a rota nao tiver nada alem do ponto inicial e final nao da para remover
		return;
	}

	solution original = sol;
	
	dropNode(0); // remove o primeiro ponto para ele ser a referencia de melhor piora

	solution best = sol; // utiliza a rota sem o primeiro ponto como referencia
	sol = original; 

	
	for (int i = 0; i < original.route.size(); i++) { // percorre a rota
		
		dropNode(i);
		
		if (sol.feasible == true && sol.FO > best.FO) { // se for viavel e melhor aceita
			best = sol;
			sol = original; // desfaz op movimento
		}
		else { // senao retorna para a solucao antiga
			sol = original; // desfaz op movimento
		}
	}
	
	sol = best;
}

void thop::dropNode(int indice) // estrutura de vizinhança que remove um nó da rota. Como é mais provável que a FO piore, aqui aceitamos movimento de piora, no caso, o que piora menos
{
	if (indice >= sol.route.size()) {
		cout << "ERRO! dropNode\n";
		printSol();
		cout << indice << endl;
		return;
	}
	else if (sol.route.size() <= 0) {
		cout << "ERRO! dropNode\n Rota vazia!";
		return;
	}
	removePonto(sol.route[indice]); // na posicao indice da rota remove o ponto e ve no que da
	eval();
}

void thop::addBestNode()
{
	solution original = sol;
	solution best = sol;


	for (int i = 0; i < sol.route.size(); i++) { // para cada posição da rota	
		for (int j = 2; j < this->coords.size(); j++) { // para cada cidade
			addNode(i, j); // faz todas as inserções possiveis

			if (sol.FO > best.FO && sol.feasible == true) { // se a nova solucao for melhor e viavel
				cout << "melhoria" << endl;
				best = sol; // atualiza a melhor
				sol = original; // desfaz o movimento
			}
			else {
				sol = original; // desfaz o movimento
			}
		}
	}

	sol = best;
}

void thop::addNode(int pos, int no)
{
	solution best;
	bool adicionou = false;
	solution original = this->sol; // guarda a ultima solucao valida
	sol.route.insert(sol.route.begin() + pos, no); // insere o no na rota na posicao pos
	//printSol();
	// adiciona itens de forma gulosa
	vector<item> itens = getItemByCity(no);
	
	sort(itens.begin(), itens.end(), compItemWeight); // ordenas os itens por peso

	
	for (int i = 0; i < itens.size(); i++) {
		//cout << "cond: " << sol.weight << "+" << itens[i].weight << "<=" << capacity << endl;
		if (sol.weight + itens[i].weight > capacity) { // adiciona enquanto tiver itens e enquanto couber
			return;
		}
		sol.items.push_back(itens[i].index); // adiciona 
		//printSol();
		//eval();
		//printInv();
		//cout << sol.feasible << endl;

		if (sol.feasible == true && adicionou == false) { // verifica a primeira soluçao viavel que aparecer
			adicionou = true;
			best = sol;
		}
		else if (sol.feasible == true && sol.FO > best.FO && adicionou == true) { // se for viavel, melhor e nao for a primeira
			best = sol;
		}
		// MELHORAR ESSA PARTE
		//else if (sol.feasible == true && sol.FO <= best.FO && adicionou == true) { // se for viavel, pior e não a primeira
		//	sol = 
		//}
		else {
			//cout << "Nao gerou solucao viavel\n";
			//cout << "Tempo: " << sol.time << endl;
			sol = original; // desfaz o movimento
			//return;
		}		
	}
}

void thop::criaSol() // cria solucao no formato de saida (antigo) a partir da permutacao
{
	solution original = sol;
	float peso = 0;
		
	sol.route.clear();
	sol.items.clear();
	for (int k = 0; k < sol.y.size(); k++) // zera a var y
		sol.y[k] = false;


	for (int i = 0; i < sol.permutation.size()-1; i++) {
		int itn = sol.permutation[i]; // item corrente
		float pesoItn = items[itn-1].weight;
		int cityInt = items[itn-1].assingnedCityId;
		
		solution aux = sol; // faz um backup da solucao para depois de modificar e nao for viavel desfazer o movimento

		int pos = inRoute2(cityInt);
		
		if (pos != -1) { // se o no ja esta na rota adiciona o item e coloca ele no final
				
			// move o no para o final
				
			for (int j = pos; j < sol.route.size()-1; j++) {
				// faz swaps sucessivos ate o no chegar no final, preserva a ordem dos itens posteriores
				//cout << sol.route.size() << " - " << i << " - " << j << endl;
				int aux = sol.route[j];
				sol.route[j] = sol.route[j + 1];
				sol.route[j + 1] = aux;
			}
			sol.items.push_back(itn); // acidiona o item
		}
		else {
			sol.route.push_back(cityInt);
			sol.items.push_back(itn);
		}

		eval();
		

		// printVar();
		//printSol();

		if (sol.feasible == false) { // se nao for viavel desfaz o movimento
			sol = aux;
		}
		//}
	}

	if (sol.feasible == false) { // se por acaso nao conseguiu gerar solucao viavel retorna a mesma que entrou
		sol = original;
	}

}

// pega uma solucao pronta e cria a permutacao dela
void thop::processPerm() 
{
	/*
	// transforma a solucao em uma permutacao
	sol.permutation.resize(items.size());
	for (int i = 0; i < sol.permutation.size(); i++) {
		sol.permutation[i] = i + 1;
	}
	*/
	int ult = 0; // ultima posicao da permutacao modificada
	for (int i = 0; i < sol.route.size(); i++) { // para cada no da rota
		vector<item> it = getItemInSolutionByCity(sol.route[i]);
		for (int j = 0; j < it.size(); j++) { // para cada item em it

			for (int l = ult; l < sol.permutation.size(); l++) { // pesquisa o item na permutacao
				if (sol.permutation[l] == it[j].index) {
					int aux = sol.permutation[ult];
					sol.permutation[ult] = sol.permutation[l];
					sol.permutation[l] = aux;
					ult++;
					break;
				}
			}
		}
	}
}

void thop::processSol()
{
	this->sol.y.resize(numItems, false);
	// seta as variaveis de decisao com base na solucao gerada
	for (int i = 0; i < this->sol.items.size(); i++) {
		//cout << sol.items[i] - 1 << endl;
		sol.y[sol.items[i] - 1] = true;
	}
}

solution thop::processSol(solution s)
{
	s.y.resize(numItems, false);
	// seta as variaveis de decisao com base na solucao gerada
	//cout << s.items.size() << endl;
	for (int i = 0; i < s.items.size(); i++) {
		s.y[s.items[i] - 1] = true;
	}

	return s;
}

void thop::printInst()
{
	cout << "PROBLEM NAME: " << this->name << endl;
	cout << "KNAPSACK DATA TYPE: " << this->knapsackDataType << endl;
	cout << "DIMENSION: " << this->dimension << endl;
	cout << "NUMBER OF ITEMS: " << this->numItems << endl;
	cout << "CAPACITY OF KNAPSACK: " << this->capacity << endl;
	cout << "MAX TIME: " << this->maxTime << endl;
	cout << "MIN SPEED: " << this->minSpeed << endl;
	cout << "MAX SPEED: " << this->maxSpeed << endl;
	cout << "EDGE_WEIGHT_TYPE: " << this->edgeWeightType << endl;
	cout << "NODE_COORD_SECTION (INDEX, X, Y): " << endl;
	for (auto i : this->coords) 
		cout << i.index << " " << i.x << " " << i.y << endl;
	cout << "ITEMS SECTION	(INDEX, PROFIT, WEIGHT, ASSIGNED_CITY_ID): " << endl;
	for (auto i : this->items)
		cout << i.index << " "  << i.profit << " " << i.weight << " " << i.assingnedCityId << endl;

	// imprime a matriz de distancia
	cout << "Matriz de distancia: " << endl;
	for (int i = 0; i < dist.size(); i++) {
		for (int j = 0; j < dist[i].size(); j++) {
			cout << dist[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;

}

void thop::printSol()
{
	cout << "FO: " << sol.FO << endl;
	cout << "Solucao: \n[";
	for (int i = 0; i < this->sol.route.size() - 1; i++) {
		cout << this->sol.route[i] << ",";
	}
	cout << this->sol.route[this->sol.route.size() - 1] << "]\n";

	cout << "[";
	for (int i = 0; i < this->sol.items.size() - 1; i++) {
		cout << this->sol.items[i] << ",";
	}
	cout << this->sol.items[this->sol.items.size() - 1] << "]\n";

}

void thop::printVar()
{

	//for (int i = 0; i < coords.size(); i++) {
	//	for (int j = 0; j < coords.size(); j++) {
	//		cout << sol.x[i][j] << endl;
	//	}
	//	cout << endl;
	//}

	for (auto i : sol.y) {
		cout << i << " ";
	}
	cout << endl;

	for (auto i : sol.permutation) {
		cout << i << " ";
	}
	cout << endl;
}

void thop::printInv()
{
	cout << "Inv=" << sol.r1 << sol.r2 << sol.r3 << endl;
	/*
	cout << "Inv = ";
	for (auto i : sol.inv) {
		cout << i;
	}
	cout << endl;
	*/
}

// verifica se um local ja esta na rota
bool thop::inRoute(int p)
{
	for (auto i : this->sol.route) {
		if (i == p) {
			return true;
		}
	}
	return false;
}

int thop::inRoute2(int p)
{
	int pos = 0;
	for (auto i : this->sol.route) {
		if (i == p) {
			return pos;
		}
		pos++;
	}
	return -1;
}

bool thop::itemInSolution(item i)
{
	for (auto it : this->sol.items) {
		if (i.index == it)
			return true;
	}
	return false;
}

void thop::removeItens(int no) 
{
	
	for (int i = 0; i < sol.items.size(); i++) {
		
		if (items[sol.items[i]].assingnedCityId - 1 == no) {
			//cout << sol.items[i] << endl;
			//cout << "size: " << sol.y.size() << endl;
			sol.y[sol.items[i]-1] = false; // remove da var y
			
			sol.items.erase(sol.items.begin() + i);

		}
	}
}

void thop::removePonto(int no)
{
	for (int i = 0; i < sol.route.size(); i++) {
		if (sol.route[i] == no) { // procura o ponto
			sol.route.erase(sol.route.begin() + i); // remove o ponto
			removeItens(no); // remove os itens referentes ao ponto
			break;
		}
	}
	processSol();
}

vector<item> thop::getItemByCity(int city)
{
	vector<item> result;
	for (auto i : items) {
		if (i.assingnedCityId == city) {
			result.push_back(i);
		}
	}
	return result;
}

vector<item> thop::getItemInSolutionByCity(int city) // pega todos os itens do ponto city que estao na solucao corrente
{
	vector<item> result;

	for (auto i : items) {

		if (i.assingnedCityId == city && itemInSolution(i)) {
			result.push_back(i);
		}
	}

	return result;
}

double thop::getFO()
{
	return this->FO;
}

double thop::getTime()
{
	return this->sol.time;
}
double thop::getweight()
{
	return this->sol.weight;
}


solution thop::getSolution()
{
	return this->sol;
}

void thop::setSolution(solution s)
{
	this->sol = s;
}

bool thop::compIT(int a, int b)
{
	return (items[a - 1].weight > items[b - 1].weight);
}

string thop::getSol()
{
	string res = "";
	res += "[";
	for (int i = 0; i < this->sol.route.size() - 1; i++) {
		res += to_string(this->sol.route[i]) + " ";
	}
	res += to_string(this->sol.route[this->sol.route.size() - 1]) += "]";

	res += "[";
	for (int i = 0; i < this->sol.items.size() - 1; i++) {
		res += to_string(this->sol.items[i]) += " ";
	}
	res += to_string(this->sol.items[this->sol.items.size() - 1]) += "]";

	return res;
}


/*
bool thop::compItemWeight(item a, item b)
{
	return (a.weight < b.weight);
}

bool thop::compItemProfit(item a, item b)
{
	return (a.profit < b.profit);
}
*/
thop::~thop()
{
}

void thop::greedyItems(solution & sol)
{
	

	vector<item> its;
	//its = getItemByCity();

	//for (int i = 0; i < gntc.popSize; i++) {
		
	//}
}

// genetic

genetic::genetic(int popSize)
{
	this->popSize = popSize;
	


}

void genetic::initPop()
{
	population.resize(this->popSize);

	for (int i = 0; i < popSize; i++) { // para cada membro da população

	}
}

genetic::~genetic()
{
}
