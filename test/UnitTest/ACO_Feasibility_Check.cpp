#include "doctest.h"
#include <random>
#include <QKP/QKP.hpp>
#include <iostream>

TEST_CASE("ACO_Feasibility_Check"){
    int n = 20, m =3;
    std::vector<double> mat(n*n);
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(-5, 15);
    std::uniform_real_distribution<double> dist01(0, 1);
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            mat[i*n+j] = dist(mt);
        }
    }

    std::vector<double> constraints(m*(n+1));
    std::vector<CorcaORBack::QKP::ConstraintOperation> operators(m);
    for(int i=0;i<m;i++){
        constraints[i*(n+1) + i*3] = constraints[i*(n+1)+i*3+1] = constraints[i*(n+1)+i*3+2] = 1;
        constraints[i*(n+1)+n] = rand()%4;
        if(rand()%2){
            operators[i] = CorcaORBack::QKP::ConstraintOperation::LEQ;
        }
        else {
            operators[i] = CorcaORBack::QKP::ConstraintOperation::L;
        }
    }

    CorcaORBack::QKP::QuadraticProgram qp(std::move(mat), std::move(constraints), std::move(operators));
    CorcaORBack::QKP::ACOSolver solver(qp);
    solver.set_verbose_iterations(-1);
    solver.solve();
    CHECK(solver.is_feasible());
}
