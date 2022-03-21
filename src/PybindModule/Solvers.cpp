#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "QKP/QKP.hpp"

namespace py = pybind11;

namespace CorcaORBack::QKP{

    class PyQuadraticSolver: public QuadraticSolver{
        public:
            void solve() override {
                PYBIND11_OVERRIDE_PURE(void, QuadraticSolver, solve, );
            };
        };

        class PyTSTSSolver : public TSTSSolver{
        public:
            using TSTSSolver::TSTSSolver;
            void solve() override { PYBIND11_OVERRIDE(void, TSTSSolver, solve,); 
        }
    };

    PYBIND11_MODULE(bindings, m) {
    py::enum_<ConstraintOperation>(m, "ConstraintOperation")
            .value("EQ", ConstraintOperation::EQ)
            .value("L", ConstraintOperation::L)
            .value("LEQ", ConstraintOperation::LEQ)
            .value("G", ConstraintOperation::G)
            .value("GEQ", ConstraintOperation::GEQ)
            .export_values();
    py::class_<QuadraticProgram>(m, "QuadraticProgram")
            .def(py::init<std::vector<double> &, std::vector<double> &, std::vector<ConstraintOperation> &> ())
            .def("get_mat", &QuadraticProgram::get_mat)
            .def("get_constraints", &QuadraticProgram::get_constraints)
            .def("get_operators", &QuadraticProgram::get_operators);

    py::class_<QuadraticSolver, PyQuadraticSolver>(m, "QuadraticSolver")
            .def("solve", &QuadraticSolver::solve)
            .def("get_solved_reward", &QuadraticSolver::get_solved_reward);

    py::class_<TSTSSolver, QuadraticSolver, PyTSTSSolver>(m, "TSTSSolver")
            .def(py::init<QuadraticProgram& , double , double, double , int >())
            .def("solve", &TSTSSolver::solve);
    }
}