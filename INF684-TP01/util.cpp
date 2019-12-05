#include "util.h"

vector<string> split(string line, char token) {
	vector<string> tokens;

	string aux = "";
	//cout << line.length() << endl;
	for (unsigned int i = 0; i < line.length(); i++) {
		if (line.at(i) != token) {
			aux += line.at(i);
		}
		else {
			tokens.push_back(aux);
			aux = "";
		}
	}
	tokens.push_back(aux);


	//printVectorString(tokens);

	return tokens;
}

vector<string> split2(string line, char token1, char token2) {
	vector<string> tokens;
	string aux = "";
	//cout << line.length() << endl;
	for (unsigned int i = 0; i < line.length(); i++) {

		if (line.at(i) != token1 && line.at(i) != token2) {
			aux += line.at(i);
		}
		else if (aux != to_string(token1) && aux != to_string(token2) && aux != ""){
			tokens.push_back(aux);
			aux = "";
		}
		
	}
	tokens.push_back(aux);


	//printVectorString(tokens);

	return tokens;
}

vector<string> split3(string line, char token1, char token2, char token3) {
	vector<string> tokens;
	string aux = "";
	//cout << line.length() << endl;
	for (unsigned int i = 0; i < line.length(); i++) {

		if (line.at(i) != token1 && line.at(i) != token2 && line.at(i) != token3) {
			aux += line.at(i);
		}
		else if (aux != to_string(token1) && aux != to_string(token2) && aux != to_string(token3) && aux != "") {
			tokens.push_back(aux);
			aux = "";
		}

	}
	tokens.push_back(aux);


	//printVectorString(tokens);

	return tokens;
}