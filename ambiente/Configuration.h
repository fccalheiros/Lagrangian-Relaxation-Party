#pragma once

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <vector>
#include <string>



class Configuration
{

public:

	int MAX_ITERATIONS = 10000;
	int INI_ALFA = 2;
	int ITERATIONS_CHANGE_ALFA = 400;
	int ITERATIONS_COLUMN_GENERATION = MAX_ITERATIONS + 1;
	int STOP_GAP = 2;

	int MAX_DEPTH = 10;

	bool RESTART = false;
	int RESTART_ITERATIONS = 1001;

	float LAMBDA = 0.5f;
	float RATIO_TRY_FIX = 0.9f;
	float STEP_TRY_FIX = 0.05f;
	float MAX_TRY_FIX = 0.5f;

	float PLUS00 = 1073610756.0f;
	float MINUS00 = -1073610755.0f;
	float CLEANFACTOR = 0.0000001f;

	bool CUT_GENERATION = true;

	float PRICEOUTRATIO = 0;

	std::string BRANCHSTRATEGY = "BFS";                  // BFS or DFS
	std::string VARIABLESTRATEGY = "HIGHINCUMBENTCOST"; // HIGHINCUMBENTCOST or LOWLAGRANGIAN




protected:

	std::vector <std::string> _token;
	std::vector <std::string> _value;

	std::string _fileText;

	bool AdvanceNext();

	bool OpenBracket();
	bool CloseBracket();
	bool ParseContent();
	bool ParseElement();

	void InsertContent(std::string token, std::string value);
	void TryChangeDefaultToken(std::string token, std::string value);
	void Initialization();

	size_t _pointer;

public:

	Configuration();

	bool PARSE(std::string file);
	std::string getValue(std::string token);
	int getIntegerValue(std::string token);
	int getTokenIndex(std::string token);

	std::string Print();


protected:

	inline void ltrim(std::string& s) {
		s.erase(s.begin(), 
			find_if(s.begin(), s.end(), [](unsigned char ch) { return !isspace(ch); })
		);
	}

	inline void rtrim(std::string& s) {
		s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !isspace(ch); }).base(), s.end());
	}

	inline void trim(std::string& s) {
		ltrim(s);
		rtrim(s);
	}


};

#endif
