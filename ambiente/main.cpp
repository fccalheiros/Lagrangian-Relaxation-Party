#include <fstream>

#include "main.h"
#include "Configuration.h"

#include "RGPManager.h"
#include "RGPLagrangeanRelaxation.h"
#include "grafo.h"
#include "BBTree.h"

bool perturbacao;
bool linear;

long int tempoInicio;
clock_t  tempoInicioCpu;

void StartStats() {
    tempoInicio = TempoAtual();
    tempoInicioCpu = TempoCpuAtual();
    cout << "------------------------------------------------------------------------------- " << endl;
    cout << "Branch & Bound starting stats" << endl;
    cout << "   Starting time: " << tempoInicio << endl;
    cout << "   CPU starting time: " << tempoInicioCpu << endl;
    cout << "   CLOCKS_PER_SEC: " << CLOCKS_PER_SEC << endl;
    cout << "------------------------------------------------------------------------------- " << endl;
}
void EndStats() {

    cout << endl;
    cout << "------------------------------------------------------------------------------- " << endl;
    cout << "Branch & Bound finish stats" << endl;
    cout << "   Execution time: " << TempoPassado(tempoInicio) << endl;
    cout << "   CPU time: " << TempoCpuPassado(tempoInicioCpu) << endl;
    cout << "   Current CPU time: " << TempoCpuAtual() << endl;
    cout << "------------------------------------------------------------------------------- " << endl;
 
}

bool CheckUsage(int argc, char * argv[])
{
    bool ok = true;
    ok =  ok && ( (argc == 3) || (argc == 5) );
    if (argc == 5) {
        ok = ok && (strcmp(argv[3], "l") or strcmp(argv[3], "c"));
    }
    if (! ok) {
        cout << "Usage: " << endl;
        cout << argv[0] << " InstanceFile ConfigurationFile [l|c] [OutputFile]" << endl;
        cout << "   Optional l to genarate linear programming file OutputFile" << endl;
        cout << "   Optional c to genarate cuts to file OutputFile --- whose usefulness I don't remember " << endl;
    }

    return ok;
}

// 
// 
//

int main2 (int argc, char * argv[]) {

  if  (! CheckUsage(argc,argv) ) return 1;

    srand(13);

    string configFile = argv[2];
    Configuration * config = new Configuration();
    try {
        if (!config->PARSE(configFile)) {
            cout << "Unable to Parse File: " << configFile << endl;
            cout << "Using default values: " << endl;
        }
    }
    catch (exception& e) {
        cout << e.what() << endl;
        cout << "Unable to Parse File: " << configFile << endl;
        cout << "Using default values: " << endl;
    }
    
    cout << config->Print();

    RGPLagrangeanRelaxation* algorithm = new RGPLagrangeanRelaxation(config);
    RGPManager *manager = new RGPManager(config, algorithm);
    SearchAlgorithm sa = SearchAlgorithm::BFS;
    if (config->getValue("BRANCHSTRATEGY").compare("DFS") == 0)
        sa = SearchAlgorithm::DFS;

    algorithm->setLagrangeanManager(manager);
    manager->GenerateProblem(argv[1]);
 
    BBTree bbTree(manager, algorithm, sa, config);

    if ( argc == 3 ) { 

        StartStats();
        bbTree.GO();

        string filename = manager->DefaultFilePrefix() + "." + (sa == SearchAlgorithm::BFS ? "BFS" : "DFS") + ".txt";
        bbTree.Print(filename);

        EndStats();
        cout << std::flush;
        return 0;
    }

    if (strcmp(argv[3], "l") == 0) {
        ofstream file;
        file.open(argv[4]);
        file << manager->PrintLP();
        file.close();
        return 0;
    }
    
    if (strcmp(argv[3], "c") == 0) {
        FILE* saida = stdout;
        saida = fopen(argv[4], "w");
        cortes(manager, saida);
        return 0;
    }
 
    return 0;

}


void cortes(LagrangeanManager* prob, FILE* saida) {
    unsigned int i, j;
    int num = 0;
    vector <int> vars;

    prob->_constraints.erase(prob->_constraints.begin(), prob->_constraints.end());
    cout << "li sol" << endl;
    FILE* fp = fopen("sol", "r");
    std::ignore = fscanf(fp, "%d", &num);
    while (num != -1) {
        vars.push_back(num);
        cout << num << " ";
        //    ((RGPVariable *)prob->_variables[num])->ImprimeRetangulo();
        cout << endl;
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
    prob->para_cada_corte(Imprime <Constraint*>(saida));
    cout << prob->_cuts.size() << endl;
}

float Psi(bool perturbacao) {
    if (!perturbacao) return (float)0;
    float res = (((float)rand() / (float)1048756) - (float)1024) / (float)1E+10;
    return (float)res;
}

float Custo(int var) {
    // return (float)gerente->_variables[var]->retCusto() + (float)Psi();
    return  0; /// apagar
}
