#include "Configuration.h"
#include <iostream>
#include <fstream>
#include <sstream>

Configuration::Configuration() {
	_pointer = 0;
	Initialization();
}

void Configuration::Initialization() {

	InsertContent("MAX_ITERATIONS", to_string(MAX_ITERATIONS));
	InsertContent("INI_ALFA", to_string(INI_ALFA));
	InsertContent("ITERATIONS_CHANGE_ALFA",to_string(ITERATIONS_CHANGE_ALFA));	
	InsertContent("STOP_GAP", to_string(STOP_GAP));
	InsertContent("MAX_DEPTH", to_string(MAX_DEPTH));		
	InsertContent("RESTART", to_string(RESTART));
	InsertContent("RESTART_ITERATIONS", to_string(RESTART_ITERATIONS));
	InsertContent("LAMBDA", to_string(LAMBDA));
	InsertContent("RATIO_TRY_FIX", to_string(RATIO_TRY_FIX));							
	InsertContent("STEP_TRY_FIX", to_string(STEP_TRY_FIX));
	InsertContent("MAX_TRY_FIX", to_string(MAX_TRY_FIX));									
	InsertContent("PLUS00", to_string(PLUS00));
	InsertContent("MINUS00", to_string(MINUS00));
	InsertContent("CLEANFACTOR", to_string(CLEANFACTOR));
	InsertContent("CUT_GENERATION", to_string(CUT_GENERATION));
	InsertContent("BRANCHSTRATEGY", BRANCHSTRATEGY);
	InsertContent("VARIABLESTRATEGY", VARIABLESTRATEGY);
	InsertContent("PRICEOUTRATIO", to_string(PRICEOUTRATIO));

}

bool Configuration::AdvanceNext() {
	while (_fileText[_pointer] == ' ') {
		_pointer++;
		if (_pointer == _fileText.size()) {
			_pointer--;
			return false;
		}
	}
	return true;
}

bool Configuration::OpenBracket() {
	AdvanceNext();
	return _fileText[_pointer] == '{';
};


bool Configuration::CloseBracket() {
	AdvanceNext();
	return _fileText[_pointer] == '}';
};

bool Configuration::ParseContent() {
	_pointer++;
	AdvanceNext();

	if (!ParseElement())
		return false;
	AdvanceNext();
	while ( _fileText[_pointer] == ',') {
		_pointer++;
		if (!ParseElement())
			return false;
		AdvanceNext();
	}
	return true;
}

bool Configuration::ParseElement() {

	string token;
	string value;
	_pointer++;
	size_t ptr = _pointer;

	while (_fileText[_pointer] != '\"') {
		_pointer++;
		if (_pointer == _fileText.size())
			return false;
	}
	token = _fileText.substr(ptr, _pointer - ptr);

	_pointer++;

	AdvanceNext();
	if (_fileText[_pointer] != ':')
		return false;
	
	_pointer++;
	AdvanceNext();
	if (_fileText[_pointer] != '\"')
		return false;
	
	_pointer++;
	AdvanceNext();

	ptr = _pointer;
	while (_fileText[_pointer] != '\"') {
		_pointer++;
		if (_pointer == _fileText.size())
			return false;
	}
	value = _fileText.substr(ptr, _pointer - ptr);
	_pointer++;

	InsertContent(token, value);

	return true;
}

bool Configuration::PARSE(string FileName) {

	string line;
	stringstream input;
	ifstream file(FileName.c_str());
	bool result = ! file.fail();


	if (result) {
		// Use a while loop together with the getline() function to read the file line by line
		while (getline(file, line)) {
			input << line;
		}

		file.close();

		_fileText = input.str();
		_fileText.erase(std::remove(_fileText.begin(), _fileText.end(), '\n'), _fileText.end());
		_fileText.erase(std::remove(_fileText.begin(), _fileText.end(), '\t'), _fileText.end());
		_pointer = 0;


		if (!OpenBracket()) return false;
		if (!ParseContent()) return false;
		if (!CloseBracket()) return false;
	}

	return (result);
}


void Configuration::InsertContent(string token, string value) {
	trim(token);
	trim(value);

	int index = getTokenIndex(token);
	if (index < 0) {
		_token.push_back(token);
		_value.push_back(value);
	}
	else
		_value[index] = value;
	TryChangeDefaultToken(token, value);
}

string Configuration::getValue(string token) {
	
	vector <string>::iterator it;
	it = find(_token.begin(), _token.end(), token);
	if (it == _token.end())
		return "";

	return _value[distance(_token.begin(), it)];
}

int Configuration::getTokenIndex(string token) {
	vector <string>::iterator it;
	it = find(_token.begin(), _token.end(), token);
	if (it == _token.end())
		return -1;
	return static_cast<int>(distance(_token.begin(), it));
}

void Configuration::TryChangeDefaultToken(string token, string value) {

	if (token.compare("MAX_ITERATIONS") == 0) { MAX_ITERATIONS = stoi(value); return; }
	if (token.compare("INI_ALFA") == 0) { INI_ALFA = stoi(value); return; }
	if (token.compare("ITERATIONS_CHANGE_ALFA") == 0) { ITERATIONS_CHANGE_ALFA = stoi(value); return; }
	if (token.compare("STOP_GAP") == 0) { STOP_GAP = stoi(value); return; }
	if (token.compare("MAX_DEPTH") == 0) { MAX_DEPTH = stoi(value); return; }
	if (token.compare("RESTART") == 0) { RESTART = ( value.compare("false") == 0 ? false : true ); return; }
	if (token.compare("RESTART_ITERATIONS") == 0) { RESTART_ITERATIONS = stoi(value); return; }
	if (token.compare("LAMBDA") == 0) { LAMBDA = stof(value); return; }
	if (token.compare("RATIO_TRY_FIX") == 0) { RATIO_TRY_FIX = stof(value); return; }
	if (token.compare("STEP_TRY_FIX") == 0) { STEP_TRY_FIX = stof(value); return; }
	if (token.compare("MAX_TRY_FIX") == 0) { MAX_TRY_FIX = stof(value); return; }
	if (token.compare("PLUS00") == 0) { PLUS00 = stof(value); return; }
	if (token.compare("MINUS00") == 0) { MINUS00 = stof(value); return; }
	if (token.compare("CLEANFACTOR") == 0) { CLEANFACTOR = stof(value); return; }
	if (token.compare("CUT_GENERATION") == 0) { CUT_GENERATION = (value.compare("false") == 0 ? false : true); return; }
	if (token.compare("BRANCHSTRATEGY") == 0) {	BRANCHSTRATEGY = (value.compare("BFS") == 0 ? "BFS" : (value.compare("DFS") == 0 ? "DFS" : BRANCHSTRATEGY)); return;}
	if (token.compare("VARIABLESTRATEGY") == 0) { VARIABLESTRATEGY = (value.compare("HIGHINCUMBENTCOST") == 0 ? "HIGHINCUMBENTCOST" : (value.compare("LOWLAGRANGEAN") == 0 ? "LOWLAGRANGEAN" : VARIABLESTRATEGY)); return; }
	if (token.compare("PRICEOUTRATIO") == 0) { PRICEOUTRATIO = stof(value); return; }

}

string Configuration::Print() {

		stringstream work;

		work << "------------------------------------------------------" << endl;
		work << "    Parameters " << endl;
		for (unsigned int i = 0; i < _token.size(); i++)
			work << "        " << _token[i] << "=" << _value[i] << endl;
		work << "------------------------------------------------------" << endl;

		return work.str();

}