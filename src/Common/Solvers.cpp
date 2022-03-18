#include <QKP/Solvers.hpp>

namespace CorcaORBack::QKP {
    double QuadraticSolver::get_reward() {
        double ret = 0.0;
        for (int i = 0; i < program.n; i++) {
            for (int j = 0; j < program.n; j++) {
                ret += sol[i] * sol[j] * program.mat[i * program.n + j];
            }
        }
        return ret;
    }
}