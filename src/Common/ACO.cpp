#include <QKP/Types.hpp>
#include <QKP/Solvers.hpp>
#include <cmath>
#include <iostream>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace CorcaORBack::QKP {
    ACOSolver::ACOSolver(
        QuadraticProgram& program,
        int num_ants,
        double alpha,
        double beta,
        double rho,
        int stopping_condition,
        double min_tau,
        double max_tau,
        int max_iterations,
        int max_vertices_in_path,
        int verbose_iterations
        ) :
        QuadraticSolver(program),
        num_ants(num_ants),
        alpha(alpha),
        beta(beta),
        stopping_condition(stopping_condition),
        rho(rho),
        min_tau(min_tau),
        max_tau(max_tau),
        max_iterations(max_iterations),
        max_vertices_in_path(max_vertices_in_path),
        verbose_iterations(verbose_iterations) {}

    void ACOSolver::solve() {
        if (verbose_iterations != -1) {
            std::cout << "run ACOSolve::solve()..." << std::endl;
        }
        // initialize the pheromone matrix
        initialize_pheromone_matrix();
        // initialize the ants
        ants = std::vector<std::vector<int>>(num_ants);
        std::vector<int> best_ant;
        double best_ant_score = 0.;
        // until max iterations
        for (int iteration = 0; iteration < max_iterations; iteration++) {
            std::vector<double> score(num_ants, 0);
            // for each ant
            #pragma omp parallel for
            for (int ant_idx = 0; ant_idx < num_ants; ant_idx++) {
                // pheromone trail
                std::vector<int> ant = explore();

                score[ant_idx] = evaluate(ant);
                // update the best ant
                #pragma omp critical
                {
                    if (score[ant_idx] > best_ant_score) {
                        best_ant = ant;
                        best_ant_score = score[ant_idx];
                    }
                }
                ants[ant_idx] = ant;
            }
            // print best ant score
            if (verbose_iterations != -1 && iteration % verbose_iterations == 0) {
                std::cout << "iteration: " << iteration << " best ant score: " << best_ant_score << std::endl;
            }
            // todo: local search
            // update pheromone trails
            evaporate(score, best_ant_score);

            int condition = 0;
            sort_ants();
            if(is_all_ants_same()) {
                std::cout<<"all ants are same"<<std::endl;
                condition = 1;
            }
            if(condition == 1) {
                break;
            }
        }
        // sort best_ant
        sort(best_ant.begin(), best_ant.end());
        // update program
        solved = true;
        solved_feasibility = true; // todo
        solved_reward = evaluate(best_ant);
        sol.resize(best_ant.size());
        copy(best_ant.begin(), best_ant.end(), sol.begin());
    }
    double ACOSolver::evaluate (std::vector<int> ant) const {
        // evaluate solution
        double score = 0.;
        for (int i = 0; i < ant.size(); i++) {
            for (int j = 0; j < ant.size(); j++) {
                score += program.mat[ant[i] * program.n + ant[j]];
            }
        }
        return score;
    }
    void ACOSolver::sort_ants() {
        for (int i = 0; i < ants.size(); i++) {
            std::sort(ants[i].begin(), ants[i].end());
        }
    }
    std::vector<int> ACOSolver::explore () {
        // chose random number between 0 and num_vertices using mt19937_64
        std::random_device rd;
        std::mt19937_64 mt(rd());
        std::uniform_int_distribution<int> dist(0, program.n - 1);
        int initial_city = dist(mt);

        // knapsack
        std::vector<int> ant;
        ant.push_back(initial_city);
        while(ant.size() != max_vertices_in_path) { // stopping condition of path
            // calculate candidate vertices
            std::vector<int> candidate_vertices = get_candidate_vertices(ant);
            if (candidate_vertices.size() == 0) {
                break;
            }
            // select next vertex
            int next_vertex = select_next_vertex(ant, candidate_vertices);
            // add next vertex to knapsack
            ant.push_back(next_vertex);
        }
        return ant;
    }
    std::vector<int> ACOSolver::get_candidate_vertices (const std::vector<int> &ant) const {
        int constraint_len = program.n + 1;
        // get candidate vertices
        std::vector<int> candidate_vertices;
        std::vector<bool> ant_set(program.n, false);
        for (int i = 0; i < ant.size(); i++) {
            ant_set[ant[i]] = true;
        }
        // check each vertex is valid candidate or not
        for (int vertex=0; vertex < program.n; vertex++) {
            if (!ant_set[vertex]) {
                // check it satisfies constraints
                bool is_satisfied = true;
                for (int i = 0; i < program.operators.size(); i++) {
                    int constraint_idx = i * constraint_len;
                    if (is_satisfied == false) continue;
                    double total = 0.;
                    for (int j: ant) {
                        total += program.constraints[constraint_idx + j];
                    }
                    total += program.constraints[constraint_idx + vertex];
                    switch(program.operators[i]){
                        case ConstraintOperation::L:
                            if (total >= program.constraints[constraint_idx + constraint_len - 1]) {
                                is_satisfied = false;
                            }
                        case ConstraintOperation::LEQ:
                            if (total > program.constraints[constraint_idx + constraint_len - 1]) {
                                is_satisfied = false;
                            }
                            break;
                        default:
                            std::cout <<"Warning: Invalid constraint operation"<<std::endl;
                            break;
                    }
                }
                if (is_satisfied) candidate_vertices.push_back(vertex);
            }
        }
        return candidate_vertices;
    }
    int ACOSolver::select_next_vertex (std::vector<int>& ant, const std::vector<int>& candidate_vertices) const {
        // select next vertex
        std::vector<double> probability(candidate_vertices.size(), 0.);
        for (int i = 0; i < candidate_vertices.size(); i++) {
            double tau_factor = 0.;
            double heuristic_factor = 1.;
            for (int j: ant) {
                tau_factor += pheromone_matrix[candidate_vertices[i]][j];
            }
            heuristic_factor += program.mat[candidate_vertices[i] * program.n + i];
            for (int j: ant) {
                heuristic_factor += program.mat[candidate_vertices[i] * program.n + j];
            }
            probability[i] = std::pow(tau_factor, alpha) * std::pow(heuristic_factor, beta);
        }
        // normalize probability
        double sum = 0.;
        for (int i = 0; i < probability.size(); i++) {
            sum += probability[i];
        }
        for (int i = 0; i < probability.size(); i++) {
            probability[i] /= sum;
        }
        // random using mt
        std::random_device rd;
        std::mt19937 mt(rd());
        std::discrete_distribution<int> distribution (std::begin(probability), std::end(probability));
        int next_vertex = candidate_vertices[distribution(mt)];
        return next_vertex;
    }
    void ACOSolver::initialize_pheromone_matrix() {
        pheromone_matrix = std::vector<std::vector<double>>(program.n, std::vector<double>(program.n, max_tau));
    }

    void ACOSolver::evaporate(std::vector<double>& score, double best_ant_score) {
        // pheromone evaporate rate
        for (int i = 0; i < program.n; i++) {
            for (int j = 0; j < program.n; j++) {
                pheromone_matrix[i][j] *= rho;
            }
        }
        // choose local_best ant
        int local_best_ant_idx = 0;
        int local_best_ant_score = 0;
        for (int ant_idx = 0; ant_idx < ants.size(); ant_idx++) {
            if (score[ant_idx] > local_best_ant_score) {
                local_best_ant_idx = ant_idx;
                local_best_ant_score = score[ant_idx];
            }
        }
        // for local best ant
        { 
            int ant_idx = local_best_ant_idx;
            std::vector<int> ant = ants[ant_idx];
            for (int v1: ant)
                for (int v2: ant) {
                    if (v1 == v2) continue;
                    pheromone_matrix[v1][v2] += 1. / (1. + (double) (std::abs(best_ant_score) - std::abs(score[ant_idx])) / (double) std::abs(best_ant_score));
                    pheromone_matrix[v1][v2] = std::max(min_tau, pheromone_matrix[v1][v2]);
                    pheromone_matrix[v1][v2] = std::min(max_tau, pheromone_matrix[v1][v2]);
                }
        }
    }
    bool ACOSolver::is_all_ants_same() {
        // compare each ant
        for (int i = 0; i < ants.size()-1; i++) {
            if (ants[i].size() != ants[i+1].size()) {
                return false;
            }
            for (int j=0; j < ants[i].size(); j++) {
                if(ants[i][j] != ants[i+1][j]) {
                    return false;
                }
            }
        }
        return true;
    }
    void ACOSolver::set_max_iterations(int max_iterations) {
        this->max_iterations = max_iterations;
    }
    void ACOSolver::set_max_vertices_in_path(int max_vertices_in_path) {
        this->max_vertices_in_path = max_vertices_in_path;
    }
    void ACOSolver::set_verbose_iterations(int verbose_iterations) {
        this->verbose_iterations = verbose_iterations;
    }
    void ACOSolver::set_alpha(double alpha) {
        this->alpha = alpha;
    }
    void ACOSolver::set_beta(double beta) {
        this->beta = beta;
    }
    void ACOSolver::set_rho(double rho) {
        this->rho = rho;
    }
    void ACOSolver::set_min_tau(double min_tau) {
        this->min_tau = min_tau;
    }
    void ACOSolver::set_max_tau(double max_tau) {
        this->max_tau = max_tau;
    }
}