#pragma once
#include <vector>
#include <exception>
#include <stdexcept>
#include <string>
#include <cmath>

namespace CorcaORBack::QKP{
    enum class ConstraintOperation{
        LEQ = 0,
        L = 1,
        EQ = 2,
        G = 3,
        GEQ = 4
    };

    enum class OptimizeType{
        MAX = 0,
        MIN = 1
    };

    class QuadraticProgram{
    public:

        const std::vector<double> mat;
        const std::vector<double> constraints;
        const std::vector<ConstraintOperation> operators;

        const int n;
        const int constraint_size;

        inline QuadraticProgram(std::vector<double>& mat, std::vector<double>& constraints, std::vector<ConstraintOperation>& operators):mat(mat), n(static_cast<int>(sqrt(mat.size()))), constraints(constraints), constraint_size(operators.size()), operators(operators){
            if(mat.size() != n*n){
                throw std::out_of_range("Matrix size doesn't fit with parameter n, it should be " + std::to_string(n*n) + ", but mat.size(): " + std::to_string(mat.size()));
            }
            if(constraint_size * (n+1) != constraints.size()){
                throw std::out_of_range("Constraints size doesn't fit with parameter n and constraint_size, it should be " + std::to_string((n+1)*(constraint_size)) + ", but constraints.size(): " + std::to_string(constraints.size()));
            }
            if(operators.size() != constraint_size){
                throw std::out_of_range("Operators size doesn't fit with parameter constraint_size, it should be " + std::to_string(constraint_size) + ", but operators.size(): " + std::to_string(operators.size()));
            }
        }

        inline QuadraticProgram(std::vector<double>&& mat, std::vector<double>&& constraints, std::vector<ConstraintOperation>&& operators):mat(mat), n(static_cast<int>(sqrt(mat.size()))), constraints(constraints), constraint_size(operators.size()), operators(operators){
            if(mat.size() != n*n){
                throw std::out_of_range("Matrix size doesn't fit with parameter n, it should be " + std::to_string(n*n) + ", but mat.size(): " + std::to_string(mat.size()));
            }
            if(constraint_size * (n+1) != constraints.size()){
                throw std::out_of_range("Constraints size doesn't fit with parameter n and constraint_size, it should be " + std::to_string((n+1)*(constraint_size)) + ", but constraints.size(): " + std::to_string(constraints.size()));
            }
            if(operators.size() != constraint_size){
                throw std::out_of_range("Operators size doesn't fit with parameter constraint_size, it should be " + std::to_string(constraint_size) + ", but operators.size(): " + std::to_string(operators.size()));
            }
        }
        inline QuadraticProgram(QuadraticProgram& problem) = default;

        inline const std::vector<double>& get_mat(){
            return mat;
        }

        inline const std::vector<double>& get_constraints(){
            return constraints;
        }
        inline const std::vector<ConstraintOperation>& get_operators(){
            return operators;
        }
    };

}