# OR Solver for variants of QKP

![Ubuntu Unit Test With Docker](https://github.com/corca-ai/CorcaOR-Solver-QKP/workflows/Unit%20Test%20using%20docker/badge.svg)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ad04fb509a0d4e18a0726c32dc18207f)](https://www.codacy.com?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=corca-ai/CorcaOR-Solver-QKP&amp;utm_campaign=Badge_Grade)

Our CorcaOR-Solver-QKP is a tool for solving Quadratic Knapsack Problem in various constraints or environments. We offer two types of solvers for following constraints.

![Our Formulation](https://raw.githubusercontent.com/corca-ai/CorcaOR-Solver-QKP/develop/content/Formulation.svg)

## TSTS Quadratic Solver
The original paper[[1]](https://ideas.repec.org/a/eee/ejores/v274y2019i1p35-48.html) introduced linear MDMKP solver, which can have Multi-demand(2) and Multi-Dimensional(1) constraints. While reimplementing the algorithm, we accelerated the algorithm on paper using OpenMP and modified linear objective function to quadratic function. As a result, this solver can solve MDMQKP (Multi-Demand Multi-Dimensional Qudratic Knapsack Problem).

## ACO Solver
We also implemented ACO Solver. It can have Multi-Dimensional(1) constraints and also we modified it to have quadratic objective function. It can solve MDQKP(Multi-Dimensional Quadratic Knapsack Problem).

## Benchmark Results

There isn't any common metric that can evaluate our Algorithms. We'll soon release new dataset for our use case. We just used MDMKP dataset to check successful reimplementation of the TSTS.

Here's our benchmark result.




## TODOs
- Making Wrappers and More general solvers for many cases.

## References

1. Lai, Xiangjing & Hao, Jin-Kao & Yue, Dong, 2019. "Two-stage solution-based tabu search for the multidemand multidimensional knapsack problem," European Journal of Operational Research, Elsevier, vol. 274(1), pages 35-48. <https://ideas.repec.org/a/eee/ejores/v274y2019i1p35-48.html>
2. Solnon, Christine and Bridge, Derek, 2006. "An Ant Colony Optimization Meta-Heuristic for Subset Selection Problems," System Engineering using Particle Swarm Optimization, Nova Science publishers, pages 3-25. <https://hal.archives-ouvertes.fr/hal-01541555/file/PSO2006.pdf>
