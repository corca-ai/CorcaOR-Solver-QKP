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

    class PyACOSolver : public ACOSolver{
        public:
            using ACOSolver::ACOSolver;
            void solve() override { PYBIND11_OVERRIDE(void, ACOSolver, solve,); 
        }
    };


    PYBIND11_MODULE(bindings, m) {
        py::enum_<ConstraintOperation>(m, "ConstraintOperation")
            .value("EQ", ConstraintOperation::EQ)
            .value("L", ConstraintOperation::L)
            .value("LEQ", ConstraintOperation::LEQ)
            .value("G", ConstraintOperation::G)
            .value("GEQ", ConstraintOperation::GEQ);

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

        py::class_<ACOSolver, QuadraticSolver, PyACOSolver>(m, "ACOSolver")
                .def(py::init<QuadraticProgram&, int, double, double, double, int, double, double, int, int, int>(),
                                py::arg("program"),
                                py::arg("num_ants") = 30, 
                                py::arg("alpha") = static_cast<double>(1.2), 
                                py::arg("beta") = static_cast<double>(0.8),
                                py::arg("rho") = static_cast<double>(0.995),
                                py::arg("shopping_condition") = static_cast<int>(1e9),
                                py::arg("min_tau") = static_cast<double>(0.1),
                                py::arg("max_tau") = static_cast<double>(1.0),
                                py::arg("max_iterations") = 100,
                                py::arg("max_vertices_in_path") = 1000,
                                py::arg("verbose_iterations") = 10)
                .def("solve", &ACOSolver::solve);
        
        
    }
}