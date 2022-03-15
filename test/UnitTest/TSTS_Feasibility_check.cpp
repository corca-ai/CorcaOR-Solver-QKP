#include "doctest.h"
#include <random>
#include <MDMQKP/MDMQKP.hpp>
#include <iostream>

TEST_CASE("TSTS_Feasibility_Check"){
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
    std::vector<CorcaORBack::MDMQKP::ConstraintOperation> operators(m);
    for(int i=0;i<m;i++){
        constraints[i*(n+1) + i*3] = constraints[i*(n+1)+i*3+1] = constraints[i*(n+1)+i*3+2] = 1;
        constraints[i*(n+1)+n] = rand()%4;
        if(rand()%2){
            operators[i] = CorcaORBack::MDMQKP::ConstraintOperation::LEQ;
        }
        else {
            operators[i] = CorcaORBack::MDMQKP::ConstraintOperation::GEQ;
        }
    }

    CorcaORBack::MDMQKP::QuadraticProgram qp(std::move(mat), std::move(constraints), std::move(operators));
    CorcaORBack::MDMQKP::TSTSSolver solver(qp, 1000.0,3.0, 3.0, 1000);
    solver.solve();
    CHECK(solver.is_feasible());
}
