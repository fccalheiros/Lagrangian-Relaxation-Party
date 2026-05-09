#include "main.h"

#include "Configuration.h"
#include "RGPManager.h"
#include "RGPLagrangianRelaxation.h"
#include "grafo.h"

#include <fstream>
#include <iomanip>
#include <tuple>
#include <cctype>

Timer globalTimer;

int originalMain (int argc, char * argv[]) {

  if  (! CheckUsage(argc,argv) ) return 1;

    srand(13);

	Configuration* config = LoadConfig(argv[2]);
    RGPLagrangianRelaxation* algorithm = new RGPLagrangianRelaxation(config);
    RGPManager *manager = new RGPManager(config, algorithm);
    algorithm->setLagrangianManager(manager);
    manager->GenerateProblem(argv[1]);

    SearchAlgorithm sa = ParseBranchStrategy(config->getValue("BRANCHSTRATEGY"));
    BBTree bbTree(manager, algorithm, sa, config);

    if ( argc == 3 ) { 
        StartStats();
        bbTree.GO();
        EndStats();
		std::string prefix = manager->DefaultFilePrefix() + "." + config->getValue("BRANCHSTRATEGY");
        std::string filename = prefix + ".txt";
        bbTree.Print(filename);
		filename = prefix + ".sol";
		manager->PrintSolution(filename);
        return 0;
    }

    if (strcmp(argv[3], "l") == 0) {
        std::ofstream file;
        file.open(argv[4]);
        file << manager->PrintLP();
        file.close();
        return 0;
    }
    
    if (strcmp(argv[3], "c") == 0) {
        FILE* saida = stdout;
        saida = fopen(argv[4], "w");
        Cuts(manager, saida);
        return 0;
    }
 
    return 0;

}

static void StartStats() {
	globalTimer.Reset();    
    std::cout << "------------------------------------------------------------------------------- " << std::endl;
    std::cout << "Branch & Bound starting " << std::endl;
    std::cout << "------------------------------------------------------------------------------- " << std::endl;
}
static void EndStats() {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << std::endl;
    std::cout << "------------------------------------------------------------------------------- " << std::endl;
    std::cout << "Branch & Bound finish stats" << std::endl;
    std::cout << "   Execution time: " << globalTimer.ElapsedSeconds()  << std::endl;
    std::cout << "   CPU time: " << globalTimer.CpuElapsedSeconds() << std::endl;
    std::cout << "------------------------------------------------------------------------------- " << std::endl;
    std::cout << std::flush;
}

static bool CheckUsage(int argc, char* argv[])
{
    bool ok = true;
    ok = ok && ((argc == 3) || (argc == 5));
    if (argc == 5) {
        ok = ok && (strcmp(argv[3], "l") or strcmp(argv[3], "c"));
    }
    if (!ok) {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0] << " InstanceFile ConfigurationFile [l|c] [OutputFile]" << std::endl;
        std::cout << "   Optional l to genarate linear programming file OutputFile" << std::endl;
        std::cout << "   Optional c to genarate cuts to file OutputFile --- whose usefulness I don't remember " << std::endl;
    }

    return ok;
}

SearchAlgorithm ParseBranchStrategy(const std::string& strategy) {
    std::string upperStrategy = strategy;
    for (char& c : upperStrategy) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
	if (upperStrategy == "BFS")   return SearchAlgorithm::BFS;
    if (upperStrategy == "DFS")   return SearchAlgorithm::DFS;
	if (upperStrategy == "NONE")  return SearchAlgorithm::NONE;
    return SearchAlgorithm::NONE; // default
}

Configuration* LoadConfig(const char* configFile) {

    Configuration* config = new Configuration();

    try {
        if (!config->PARSE(configFile)) {
            std::cout << "Unable to Parse File: " << configFile << std::endl;
            std::cout << "Using default values: " << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        std::cout << "Unable to Parse File: " << configFile << std::endl;
        std::cout << "Using default values: " << std::endl;
    }

    std::cout << config->Print();
    return config;
}

void Cuts(LagrangianManager* prob, FILE* output) {
    unsigned int i, j;
    int num = 0;
    std::vector <int> vars;

    prob->_constraints.erase(prob->_constraints.begin(), prob->_constraints.end());
    std::cout << "li sol" << std::endl;
    FILE* fp = fopen("sol", "r");
    std::ignore = fscanf(fp, "%d", &num);
    while (num != -1) {
        vars.push_back(num);
        std::cout << num << " ";
        //    ((RGPVariable *)prob->_variables[num])->ImprimeRetangulo();
        std::cout << std::endl;
        std::ignore = fscanf(fp, "%d", &num);
    }
    fclose(fp);

    Grafo g;
    for (i = 0; i < vars.size(); i++)
        g.InsereNo(prob->_variables[vars[i]]);


    for (i = 0; i < vars.size() - 1; i++) {
        for (j = i + 1; j < vars.size(); j++) {
            if ((prob->_variables[vars[i]])->Intercepta((prob->_variables[vars[j]]))) {
                g.InsereAresta(prob->_variables[vars[i]], prob->_variables[vars[j]]);
            }
        }
    }
    g.Imprime();
    g.Clique(prob, 2);
    g.CicloImpar(prob);
    prob->for_each_cut(Imprime <Constraint*> (output) );
    std::cout << prob->_cuts.size() << std::endl;
}


