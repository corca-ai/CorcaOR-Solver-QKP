#include <MDMQKP/Types.hpp>
#include <MDMQKP/Solvers.hpp>
#include <iostream>

#include <random>
#include <chrono>

namespace CorcaORBack::MDMQKP{
    const double TSTSSolver::gammas[] = {1.9, 2.1, 2.3};
    // No thinking Code without Future
    struct max_st{
        double val;
        std::pair<int, int> which;
        double penalty;
    };

    const max_st& max(const max_st& a, const max_st& b){
        return a.val > b.val ? a : b;
    }

    double TSTSSolver::get_total_penalty(){
        double ret = 0.0;
        //#pragma omp parallel for reduction(+:ret)
        for(int i=0;i<program.constraint_size;i++){
            double lhs_tmp = 0.0;
            double constant = program.constraints[i*(program.n+1)+program.n];
            for(int j=0;j<program.n;j++){
                lhs_tmp += sol[j] * program.constraints[i*(program.n+1)+j];
            }
            switch(program.operators[i]){
                case ConstraintOperation::L:
                case ConstraintOperation::LEQ:
                    ret += std::min(constant - lhs_tmp, 0.0);
                    break;
                case ConstraintOperation::EQ:
                    ret += std::min(constant - lhs_tmp, 0.0);
                    ret += std::min(lhs_tmp - constant, 0.0);
                    break;
                case ConstraintOperation::GEQ:
                case ConstraintOperation::G:
                    ret += std::min(lhs_tmp - constant, 0.0);
                    break;
            }
        }
        return lambda * ret;
    }

    void TSTSSolver::set_initial_solution(){
        std::vector<int> random_perm(program.n);
        for(int i=0;i<program.n;i++) random_perm[i]=i;
        sol.resize(program.n, 0);
        std::shuffle(random_perm.begin(), random_perm.end(), std::mt19937(std::random_device()()));
        for(int k: random_perm){
            bool assign0 = false, assign1 = false;
            for(int i=0;i<program.constraint_size;i++){
                bool flag = false;
                switch(program.operators[i]){
                    case ConstraintOperation::LEQ:
                    case ConstraintOperation::L:
                    case ConstraintOperation::EQ:
                        if(lhs[i] + program.constraints[i*(program.n+1)+k] > program.constraints[i*(program.n+1)+program.n]){
                            flag = true;
                        }
                        break;
                    default:
                        continue;
                }
                if(flag) {
                    assign0 = true;
                    break;
                }
            }

            for(int i=0;i<program.constraint_size;i++){
                bool flag = false;
                switch(program.operators[i]){
                    case ConstraintOperation::GEQ:
                    case ConstraintOperation::G:
                    case ConstraintOperation::EQ:
                        if(lhs[i] + program.constraints[i*(program.n+1)+k] < program.constraints[i*(program.n+1)+program.n]){
                            flag = true;
                        }
                        break;
                    default:
                        continue;
                }
                if(flag) {
                    assign1 = true;
                    break;
                }
            }
            if(assign1 or !assign0){
                sol[k] = 1;
                for(int i=0;i<program.constraint_size;i++) lhs[i] += program.constraints[i*(program.n+1)+k];
            }else{
                sol[k] = 0;
            }
        }
    }

    void TSTSSolver::set_initial_hash_values(){
        hash_values.resize(3, 0);
        for(int w=0;w<3;w++){
            for(int i=0;i<program.n;i++){
                hash_values[w] += static_cast<long long>(floor(pow(i+1, gammas[w])) * sol[i]);
                hash_values[w] %= HASH_L;
            }
        }
    }

    bool TSTSSolver::check_swap(int i, int j){
        if(sol[i]==1){
            std::swap(i, j);
        }
        //sol[j]가 1인 경우엔 저깄는걸 빼서 i에 더하기
        long long hs[3];
        for(int w=0;w<3;w++){
            hs[w] = (hash_values[w] - static_cast<long long>(pow(j+1, gammas[w])) +HASH_L) + static_cast<long long>(pow(i+1, gammas[w]));
            hs[w] %= HASH_L;
        }
        if(hash_tables[hs[0]] and hash_tables[HASH_L+hs[1]] and hash_tables[2*HASH_L+hs[2]]){
            return false;
        }
        return true;
    }

    bool TSTSSolver::check_flip(int i){
        long long mul = 1;
        if(sol[i]==1){
            mul *= -1;
        }
        long long hs[3];
        for(int w=0;w<3;w++){
            hs[w] = hash_values[w] + mul * static_cast<long long>(pow(i+1, gammas[w]));
            hs[w] += HASH_L;
            hs[w] %= HASH_L;
        }
        if(hash_tables[hs[0]] and hash_tables[HASH_L+hs[1]] and hash_tables[2*HASH_L+hs[2]]){
            return false;
        }
        return true;
    }

    double TSTSSolver::get_swap_value(int i, int j){
        if(sol[j] == 1) std::swap(i, j);
        double delta = 0.0;
        //i가 1이며 j가 1로 바뀔 예정
        for(int w=0;w<program.n;w++){
            if(w==j or w==i) continue;
            if(sol[w]==1){
                delta -= program.mat[w*program.n+i] + program.mat[i*program.n+w];
                delta += program.mat[w*program.n+j] + program.mat[j*program.n+w];
            }
        }
        delta -= program.mat[i*program.n+i];
        delta += program.mat[j*program.n+j];
        return delta;
    }

    double TSTSSolver::get_flip_value(int i){
        double delta = 0.0;
        double mul = 1;
        if(sol[i] == 1) mul = -1;
        for(int w=0;w<program.n;w++){
            if(w==i) continue;
            if(sol[w]==1){
                delta += mul * program.mat[i*program.n+w] + mul * program.mat[w*program.n+i];
            }
        }
        delta += mul * program.mat[i*program.n+i];
        return delta;
    }

    double TSTSSolver::get_swap_penalty(int i, int j){
        if(sol[j] == 1) std::swap(i, j);

        double ret = 0.0;

        for(int w=0;w<program.constraint_size;w++){
            double tmp_lhs = lhs[w] - program.constraints[w*(program.n+1)+i] + program.constraints[w*(program.n+1)+j];
            double new_val;
            double constant = program.constraints[w*(program.n+1)+program.n];
            switch(program.operators[w]){
                case ConstraintOperation::L:
                case ConstraintOperation::LEQ:
                    new_val = std::min(constant - tmp_lhs, 0.0);
                    break;
                case ConstraintOperation::EQ:
                    new_val = std::min(constant - tmp_lhs, 0.0);
                    new_val += std::min(tmp_lhs - constant, 0.0);
                    break;
                case ConstraintOperation::GEQ:
                case ConstraintOperation::G:
                    new_val = std::min(tmp_lhs - constant, 0.0);
                    break;
            }
            ret += new_val;
        }
        return lambda * ret;
    }
    double TSTSSolver::get_flip_penalty(int i){
        double mul = 1;
        if(sol[i] == 1) mul = -1;
        double ret = 0.0;
        //j가 1이 된다면?!
        for(int w=0;w<program.constraint_size;w++){
            double tmp_lhs = lhs[w] + mul * program.constraints[w*(program.n+1)+i];
            double new_val;
            double constant = program.constraints[w*(program.n+1)+program.n];
            switch(program.operators[w]){
                case ConstraintOperation::L:
                case ConstraintOperation::LEQ:
                    new_val = std::min(constant - tmp_lhs, 0.0);
                    break;
                case ConstraintOperation::EQ:
                    new_val = std::min(constant - tmp_lhs, 0.0);
                    new_val += std::min(tmp_lhs - constant, 0.0);
                    break;
                case ConstraintOperation::GEQ:
                case ConstraintOperation::G:
                    new_val = std::min(tmp_lhs - constant, 0.0);
                    break;
            }
            ret += new_val;
        }
        return lambda * ret;
    }

    double TSTSSolver::run_first_stage(){
        double org_value = get_reward();
        std::vector<max_st> tabus;
        std::vector<max_st> non_tabus;

#pragma omp parallel for collapse(2)
        for(int i=0;i<program.n;i++){
            for(int j=0;j<program.n;j++){
                if(j>=i) continue;
                if(sol[i] == sol[j]) continue;
                bool check_swapping = check_swap(i, j);
                bool check_non_tabu_empty = non_tabus.empty();
                if(!check_swapping and !check_non_tabu_empty) continue;
                double new_val = org_value + get_swap_value(i,j)+ get_swap_penalty(i, j);
#pragma omp critical
                {
                    if (check_swapping) {
                        // non tabu
                        if (non_tabus.empty() or non_tabus.begin()->val == new_val) {
                            non_tabus.push_back({new_val, {i, j}});
                        } else if (non_tabus.begin()->val < new_val) {
                            non_tabus.clear();
                            non_tabus.push_back({new_val, {i, j}});
                        }
                    } else {
                        if (tabus.empty() or tabus.begin()->val == new_val) {
                            tabus.push_back({new_val, {i, j}});
                        } else if (tabus.begin()->val < new_val) {
                            tabus.clear();
                            tabus.push_back({new_val, {i, j}});
                        }
                    }
                }
            }
        }
        #pragma omp parallel for
        for(int i=0;i<program.n;i++){
            bool check_flipping = check_flip(i);
            bool check_non_tabu_empty;
#pragma omp critical
            {
                check_non_tabu_empty = non_tabus.empty();
            };
            if(!check_flipping and !check_non_tabu_empty) continue;
            double new_val = org_value + get_flip_value(i) + get_flip_penalty(i);

            if(check_flipping){
                // non tabu
#pragma omp critical
{
                if (non_tabus.empty() or non_tabus.begin()->val == new_val) {
                    non_tabus.push_back({new_val, {i, i}});
                } else if (non_tabus.begin()->val < new_val) {
                    non_tabus.clear();
                    non_tabus.push_back({new_val, {i, i}});
                }
}
            }else{
#pragma omp critical
                {
                    if (tabus.empty() or tabus.begin()->val == new_val) {
                        tabus.push_back({new_val, {i, i}});
                    } else if (tabus.begin()->val < new_val) {
                        tabus.clear();
                        tabus.push_back({new_val, {i, i}});
                    }
                }
            }
        }
        double best_val;
        std::pair<int, int> best_loc;

        if(!non_tabus.empty()){
            int which = rand()%non_tabus.size();
            best_loc = non_tabus[which].which;
            best_val = non_tabus[which].val;
            non_tabus.clear();
        }else{
            int which = rand()%tabus.size();
            best_loc = tabus[which].which;
            best_val = tabus[which].val;
            tabus.clear();
        }

        if(best_loc.first == best_loc.second){
            sol[best_loc.first] = 1 - sol[best_loc.first];
            long long mul = -1;
            if(sol[best_loc.first] == 1){
                mul = 1;
            }
            for(int w=0;w<program.constraint_size;w++){
                lhs[w] += static_cast<double>(mul) * program.constraints[w*(program.n+1)+best_loc.first];
            }
            for(int w=0;w<3;w++){
                long long hs = hash_values[w] + mul * static_cast<long long>(pow(best_loc.first+1, gammas[w]));
                hs += HASH_L;
                hs %= HASH_L;
                hash_tables[w*HASH_L + hs] = true;
                hash_values[w] = hs;
            }
        }else{
            int i = best_loc.first, j = best_loc.second;
            double tmp = sol[i]; sol[i] = sol[j]; sol[j] = tmp;
            if(sol[i]==1) std::swap(i, j);
            for(int w=0;w<program.constraint_size;w++){
                lhs[w] += program.constraints[w*(program.n+1)+j] - program.constraints[w*(program.n+1)+i];
            }
            for(int w=0;w<3;w++){
                long long hs = (hash_values[w] + static_cast<long long>(pow(j+1, gammas[w]))+HASH_L) - static_cast<long long>(pow(i+1, gammas[w]));
                hs %= HASH_L;
                hash_tables[w*HASH_L + hs] = true;
                hash_values[w] = hs;
            }

        }
        return best_val;
    }

    std::pair<bool, double> TSTSSolver::run_second_stage(double best_value){
        //Get

        std::vector<max_st> tabus;
        std::vector<max_st> non_tabus;

        double org_small_f = get_reward();
        #pragma omp parallel for collapse(2)
        for(int i=0;i<program.n;i++){
            for(int j=0;j<program.n;j++){
                //i가 0 AND J가 1
                if(sol[i]==1 or sol[j]==0) continue;
                bool check_swapping = check_swap(i, j);
                bool check_non_tabu_empty = non_tabus.empty();
                if(!check_swapping and !check_non_tabu_empty) continue;
                double swap_delta = get_swap_value(i, j);

                if(org_small_f + swap_delta <= best_value) continue;
                double penalty = get_swap_penalty(i, j);
                double new_val = org_small_f + swap_delta + penalty;
                if(check_swapping){

#pragma omp critical
                    {
                        // non tabu
                        if (non_tabus.empty() or non_tabus.begin()->val == new_val) {
                            non_tabus.push_back({new_val, {i, j}, penalty});
                        } else if (non_tabus.begin()->val < new_val) {
                            non_tabus.clear();
                            non_tabus.push_back({new_val, {i, j}, penalty});
                        }
                    }
                }else{
#pragma omp critical
                    {
                        if (tabus.empty() or tabus.begin()->val == new_val) {
                            tabus.push_back({new_val, {i, j}, penalty});
                        } else if (tabus.begin()->val < new_val) {
                            tabus.clear();
                            tabus.push_back({new_val, {i, j}, penalty});
                        }
                    }
                }
            }
        }
        std::pair<int, int> best_loc;
        double sel_penalty;
        double best_val;
        if(non_tabus.empty() and tabus.empty()){
            double ret = 0.0;
            for(int w=0;w<program.constraint_size;w++){
                double new_val;
                double constant = program.constraints[w*(program.n+1)+program.n];
                switch(program.operators[w]){
                    case ConstraintOperation::L:
                    case ConstraintOperation::LEQ:
                        new_val = std::min(constant - lhs[w], 0.0);
                        break;
                    case ConstraintOperation::EQ:
                        new_val = std::min(constant - lhs[w], 0.0);
                        new_val += std::min(lhs[w] - constant, 0.0);
                        break;
                    case ConstraintOperation::GEQ:
                    case ConstraintOperation::G:
                        new_val = std::min(lhs[w] - constant, 0.0);
                        break;
                }
                ret += new_val;
            }
            return {ret==0.0, best_val};
        }

        if(!non_tabus.empty()){
            int which = rand()%non_tabus.size();
            best_loc = non_tabus[which].which;
            best_val = non_tabus[which].val;
            sel_penalty = non_tabus[which].penalty;
            non_tabus.clear();
        }else{
            int which = rand()%tabus.size();
            best_loc = tabus[which].which;
            best_val = tabus[which].val;
            sel_penalty = tabus[which].penalty;
            tabus.clear();
        }

        int i = best_loc.first, j = best_loc.second;
        double tmp = sol[i]; sol[i] = sol[j]; sol[j] = tmp;
        for(int w=0;w<program.constraint_size;w++){
            lhs[w] += program.constraints[w*(program.n+1)+i] - program.constraints[w*(program.n+1)+j];
        }
        for(int w=0;w<3;w++){
            long long hs = (hash_values[w] - static_cast<long long>(pow(j+1, gammas[w]))+HASH_L) + static_cast<long long>(pow(i+1, gammas[w]));
            hs %= HASH_L;
            hash_values[w] = hs;
            hash_tables[hs+w*HASH_L] = true;
        }
        return std::make_pair(sel_penalty == 0, best_val);
    }

    

    void TSTSSolver::solve(){
        std::vector<double> best_sol;
        lhs.clear();
        lhs.resize(program.constraint_size, 0.0);
        set_initial_solution();
        best_sol = sol;

        double best_val = get_reward() + get_total_penalty();
        int no_improve = 0;
        hash_tables.clear();
        hash_tables.resize(3 * (HASH_L), false);
        hash_values.clear();
        hash_values.resize(3, 0);
        set_initial_hash_values();

        auto begin = std::chrono::system_clock::now();
        while(static_cast<double>((std::chrono::system_clock::now() - begin).count())/1e9 < time_limit_stage1 and ((no_improve < no_improve_threshold) or (get_total_penalty()<0))){
            double non_tabu_val = run_first_stage();
            if(non_tabu_val>best_val) {
                best_sol = sol;
                best_val = non_tabu_val;
                no_improve = 0;
            }
            no_improve += 1;
        }
        sol = best_sol;
        for(int i=0;i<program.constraint_size;i++){
            lhs[i] = 0.0;
            for(int j=0;j<program.n;j++){
                lhs[i] += best_sol[j] * program.constraints[i*(program.n+1)+j];
            }
        }

        std::fill(hash_tables.begin(), hash_tables.end(), false);

        set_initial_hash_values();
        begin = std::chrono::system_clock::now();
        while(static_cast<double>((std::chrono::system_clock::now() - begin).count())/1e9 < time_limit_stage2){
            auto [feasible, non_tabu_val] = run_second_stage(best_val);

            if(feasible and non_tabu_val>best_val){
                best_sol = sol;
                best_val = non_tabu_val;
            }
        }

        sol = best_sol;
        solved = true;
        solved_feasibility = get_total_penalty() == 0;
        solved_reward = best_val;
    }
}