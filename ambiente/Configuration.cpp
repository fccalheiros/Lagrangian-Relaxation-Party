#include "Configuration.h"
#include <iostream>
#include <fstream>
#include <sstream>

Configuration::Configuration() {
	_pointer = 0;
	Initialization();
}

void Configuration::Initialization() {

	InsertContent("MAX_ITERATIONS", std::to_string(MAX_ITERATIONS));
	InsertContent("INI_ALFA", std::to_string(INI_ALFA));
	InsertContent("ITERATIONS_CHANGE_ALFA",std::to_string(ITERATIONS_CHANGE_ALFA));	
	InsertContent("STOP_GAP", std::to_string(STOP_GAP));
	InsertContent("MAX_DEPTH", std::to_string(MAX_DEPTH));		
	InsertContent("RESTART", std::to_string(RESTART));
	InsertContent("RESTART_ITERATIONS", std::to_string(RESTART_ITERATIONS));
	InsertContent("LAMBDA", std::to_string(LAMBDA));
	InsertContent("RATIO_TRY_FIX", std::to_string(RATIO_TRY_FIX));							
	InsertContent("STEP_TRY_FIX", std::to_string(STEP_TRY_FIX));
	InsertContent("MAX_TRY_FIX", std::to_string(MAX_TRY_FIX));									
	InsertContent("PLUS00", std::to_string(PLUS00));
	InsertContent("MINUS00", std::to_string(MINUS00));
	InsertContent("CLEANFACTOR", std::to_string(CLEANFACTOR));
	InsertContent("CUT_GENERATION", std::to_string(CUT_GENERATION));
	InsertContent("BRANCHSTRATEGY", BRANCHSTRATEGY);
	InsertContent("VARIABLESTRATEGY", VARIABLESTRATEGY);
	InsertContent("PRICEOUTRATIO", std::to_string(PRICEOUTRATIO));
	InsertContent("ITERATIONS_COLUMN_GENERATION", std::to_string(ITERATIONS_COLUMN_GENERATION));
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

	std::string token;
	std::string value;
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

bool Configuration::PARSE(std::string FileName) {

	std::string line;
	std::stringstream input;
	std::ifstream file(FileName.c_str());
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


void Configuration::InsertContent(std::string token, std::string value) {
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

std::string Configuration::getValue(std::string token) {
	
	std::vector <std::string>::iterator it;
	it = find(_token.begin(), _token.end(), token);
	if (it == _token.end())
		return "";

	return _value[std::distance(_token.begin(), it)];
}

int Configuration::getIntegerValue(std::string token) {
	std::string value = getValue(token);
	if (value.empty())
		return 0;
	try {
		return std::stoi(value);
	}
	catch (const std::exception&) {
		return 0;
	}

}

int Configuration::getTokenIndex(std::string token) {
	std::vector <std::string>::iterator it;
	it = find(_token.begin(), _token.end(), token);
	if (it == _token.end())
		return -1;
	return static_cast<int>(std::distance(_token.begin(), it));
}

void Configuration::TryChangeDefaultToken(std::string token, std::string value) {

	if (token.compare("MAX_ITERATIONS") == 0) { MAX_ITERATIONS = std::stoi(value); return; }
	if (token.compare("INI_ALFA") == 0) { INI_ALFA = std::stoi(value); return; }
	if (token.compare("ITERATIONS_CHANGE_ALFA") == 0) { ITERATIONS_CHANGE_ALFA = std::stoi(value); return; }
	if (token.compare("STOP_GAP") == 0) { STOP_GAP = std::stoi(value); return; }
	if (token.compare("MAX_DEPTH") == 0) { MAX_DEPTH = std::stoi(value); return; }
	if (token.compare("RESTART") == 0) { RESTART = ( value.compare("false") == 0 ? false : true ); return; }
	if (token.compare("RESTART_ITERATIONS") == 0) { RESTART_ITERATIONS = std::stoi(value); return; }
	if (token.compare("LAMBDA") == 0) { LAMBDA = std::stof(value); return; }
	if (token.compare("RATIO_TRY_FIX") == 0) { RATIO_TRY_FIX = std::stof(value); return; }
	if (token.compare("STEP_TRY_FIX") == 0) { STEP_TRY_FIX = std::stof(value); return; }
	if (token.compare("MAX_TRY_FIX") == 0) { MAX_TRY_FIX = std::stof(value); return; }
	if (token.compare("PLUS00") == 0) { PLUS00 = std::stof(value); return; }
	if (token.compare("MINUS00") == 0) { MINUS00 = std::stof(value); return; }
	if (token.compare("CLEANFACTOR") == 0) { CLEANFACTOR = std::stof(value); return; }
	if (token.compare("CUT_GENERATION") == 0) { CUT_GENERATION = (value.compare("false") == 0 ? false : true); return; }
	if (token.compare("BRANCHSTRATEGY") == 0) {	BRANCHSTRATEGY = (value.compare("BFS") == 0 ? "BFS" : (value.compare("DFS") == 0 ? "DFS" : BRANCHSTRATEGY)); return;}
	if (token.compare("VARIABLESTRATEGY") == 0) { VARIABLESTRATEGY = (value.compare("HIGHINCUMBENTCOST") == 0 ? "HIGHINCUMBENTCOST" : ((value.compare("LOWLAGRANGIAN") == 0 || value.compare("LOWLAGRANGEAN") == 0) ? "LOWLAGRANGIAN" : VARIABLESTRATEGY)); return; }
	if (token.compare("PRICEOUTRATIO") == 0) { PRICEOUTRATIO = std::stof(value); return; }
	if (token.compare("ITERATIONS_COLUMN_GENERATION") == 0) { ITERATIONS_COLUMN_GENERATION = std::stoi(value); return; }

}

std::string Configuration::Print() {

		std::stringstream work;

		work << "------------------------------------------------------" << std::endl;
		work << "    Parameters " << std::endl;
		for (unsigned int i = 0; i < _token.size(); i++)
			work << "        " << _token[i] << "=" << _value[i] << std::endl;
		work << "------------------------------------------------------" << std::endl;

		return work.str();

}




