#include <string>
#include <map>
#include <vector>
#include <memory>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <omp.h>
#include <QKP/QKP.hpp>

using namespace CorcaORBack::QKP;

template <class T> double get_score(std::string file_name, T func){

    std::vector<int> sol;
    std::vector<ConstraintOperation> operators;
    std::vector<double> mat, constraint;

    std::ifstream fin(file_name);
    int n, m, l;
    fin >> n >> m >> l;
    mat.clear();
    sol.resize(n, 0);
    operators.resize(m+l);
    mat.resize(n*n, 0);
    constraint.resize((m+l)*(n+1), 0);
    for(int i=0;i<n;i++) sol[i] = 0;

    for (int i = 0; i < m + l; i++) {
        operators[i] = (i < m) ? ConstraintOperation::LEQ : ConstraintOperation::GEQ;
    }
    for (int i = 0; i < n; i++) {
        fin >> mat[i * n + i];
    }
    for(int i=0;i<n;i++) for(int j=0;j<n;j++){
        if(i!=j) mat[i*n+j] = 0;
    }
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            fin >> constraint[i * (n + 1) + j];
        }
    }
    for (int i = 0; i < m; i++) {
        fin >> constraint[i * (n + 1) + n];
    }
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < n; j++) {
            fin >> constraint[(i + m) * (n + 1) + j];
        }
    }
    for (int i = 0; i < l; i++) {
        fin >> constraint[(i + m) * (n + 1) + n];
    }

    auto QP = QuadraticProgram(mat, constraint, operators);
    auto solver = func(std::move(QP));
    solver->solve();
    if(!solver->is_feasible()){
        std::cout<<"Not Feasible"<<std::endl;
    }
    double ret = solver->get_solved_reward();
    delete solver;
    return ret;
}

template <class T>
void print_benchmark_result(T func){
    std::map<std::string, int> scores;
    std::string dir_path = "../dataset/mdmkp";
    std::vector<std::pair<std::string, std::string>> names;
    for(const auto& entry: std::filesystem::directory_iterator(dir_path)){
        names.emplace_back(entry.path().filename().string(), entry.path().string());
    }
    sort(names.begin(), names.end());
    for(const auto& name: names){
        std::string filename = name.first;
        if(filename.length() < 4) continue;
        std::cout<<"Currently Running "<<name.second<<std::endl;
        if(filename.substr(filename.length()-4, 4) == ".txt") {
            std::string bench_name = filename.substr(0, filename.length() - 4);
            scores[bench_name] = get_score(name.second, func);
            std::cout<<bench_name<<' '<<scores[bench_name]<<std::endl;
        }
    }

    for(auto& p: scores){
        std::cout<<p.first<<' '<<p.second<<std::endl;
    }
}

int main(int argc, char* argv[]){
    // if argv[1] is mdmkqp_solver, run_benchmark_mdmkqp_solver is called.
    // else, run_benchmark_aco is called.
    omp_set_num_threads(16);

    auto get_TSTS_solver = [](QuadraticProgram&& qp){
        return dynamic_cast<QuadraticSolver*>(new TSTSSolver(qp, 100, 30.0, 30.0, 10000));
    };
    auto get_ACO_solver = [](QuadraticProgram&& qp){
        return dynamic_cast<QuadraticSolver*>(new ACOSolver(qp));
    };

    if(argc == 2 && std::string(argv[1]) == "TSTS"){
        std::cout<<"Now Running TSTS"<<std::endl;
        print_benchmark_result(get_TSTS_solver);
    }
    else if (argc == 2 && std::string(argv[1]) == "aco") {
        //std::cout<<"Now Running ACO"<<std::endl;
        //print_benchmark_result(get_ACO_solver);
        std::cout<<"ACO is not supported for MDMKP and we'll soon release dataset"<<std::endl;
    }
    else if (argc == 2 and std::string(argv[1]) == "all"){
        std::cout<<"Now Running TSTS"<<std::endl;
        print_benchmark_result(get_TSTS_solver);

        //std::cout<<"Now Running ACO"<<std::endl;
        //print_benchmark_result(get_ACO_solver);

        std::cout<<"ACO is not supported for MDMKP and we'll soon release dataset"<<std::endl;
    }
    return 0;
}