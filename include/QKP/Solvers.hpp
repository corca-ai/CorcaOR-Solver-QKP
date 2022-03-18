#pragma once
#include <utility>
#include <vector>

#include <QKP/Types.hpp>


namespace CorcaORBack::QKP{
    class QuadraticSolver{
    protected:
        QuadraticProgram& program;
        explicit QuadraticSolver(QuadraticProgram& program): program(program){};

        bool solved = false;
        bool solved_feasibility = false;
        double solved_reward = 0.0;

        std::vector<double> sol;

        double get_reward();
    public:

        virtual void solve()=0;
        inline bool is_solved() const{
            return solved;
        }
        inline bool is_feasible() const{
            return solved_feasibility;
        }
        inline double get_solved_reward() const{
            return solved_reward;
        }

        inline std::vector<double> get_sol(){
            return sol;
        }
    };

    class TSTSSolver : public QuadraticSolver{
    private:
        static const int HASH_L = 1e8;
        static const double gammas[];
        double lambda;
        std::vector<double> lhs;
        std::vector<long long> hash_values;
        std::vector<bool> hash_tables;

        double time_limit_stage1;
        double time_limit_stage2;
        int no_improve_threshold;

        bool check_flip(int i);
        bool check_swap(int i, int j);
        double get_flip_value(int i);
        double get_swap_value(int i, int j);
        double get_flip_penalty(int i);
        double get_swap_penalty(int i, int j);

        double get_total_penalty();

        void set_initial_solution();
        void set_initial_hash_values();

        double run_first_stage();
        std::pair<bool, double> run_second_stage(double best_value);
    public:
        
        inline TSTSSolver(QuadraticProgram& program, double lambda, double time_limit_stage1, double time_limit_stage2, int no_improve_threshold): QuadraticSolver(program), time_limit_stage1(time_limit_stage1), time_limit_stage2(time_limit_stage2), lambda(lambda), no_improve_threshold(no_improve_threshold){
            lhs.resize(program.constraint_size, 0.0);
        };
        void solve() override;
    };

    class AcoGraph {
        public:
            AcoGraph(int num_vertices); // Used in 2D grid
            void initialize_pheromone_matrix(int min_tau, int max_tau);
            std::vector<int> explore (int initial_city, int max_vertices_in_path, double alpha, const std::vector<double>& constraints, const std::vector<ConstraintOperation>& operators) ;
            void evaporate(std::vector<std::vector<int>>& ants, std::vector<double>& score, double best_ant_score, double rho, double min_tau, double max_tau);

        private:
            int num_vertices;
            std::vector<std::vector<std::pair<int, double>>> pheromone_matrix;
            std::vector<int> get_candidate_vertices (const std::vector<int> &ant, const std::vector<double>& constraints, const std::vector<ConstraintOperation>& operators) const;
            int select_next_vertex (std::vector<int>& ant, const std::vector<int>& candidate_vertices, double alpha) const;
    };

    class ACO : public QuadraticSolver{ // Ant Colony Optimization
        public:
            ACO(
                QuadraticProgram& program,
                int num_ants=30,
                double alpha=1.,
                double beta=0.8,
                double rho=0.995,
                int stopping_condition=1e9,
                double min_tau=0.1,
                double max_tau=1.0,
                int max_iterations=100,
                int initial_city=0,
                int max_vertices_in_path=1000,
                int verbose_iterations=10
                );
            void solve() override;
            double evaluate(std::vector<int> ant) const;
            bool is_all_ants_same();
            void sort_ants();
            void set_num_vetices(int num_vertices);
            void set_max_iterations(int max_iterations);
            void set_max_vertices_in_path(int max_vertices_in_path);
            void set_verbose_iterations(int verbose_iterations);
            int get_num_vertices() const;

        private:
            int num_vertices;
            const int num_ants;
            const double alpha;
            const double beta;
            const double rho;
            const int stopping_condition;
            const double min_tau;
            const double max_tau; // the maximum value of pheromone, in MMAS tau_0 = max_tau
            int max_iterations;
            const int initial_city;
            int max_vertices_in_path;
            int verbose_iterations;

            std::vector<std::vector<int>> ants;
            std::vector<AcoGraph> graph;
    };
}