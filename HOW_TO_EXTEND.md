# How to Extend the Framework

This project was designed so that its generic Lagrangian Relaxation and Branch-and-Bound framework can be reused for other combinatorial optimization problems.

In practice, adapting the code to a new problem usually means defining the problem-specific objects and plugging them into the generic solution flow already implemented in the repository.

## Core idea

The framework separates the reusable optimization engine from the modeling details of each problem.

Besides the standard Lagrangian Relaxation plus Branch-and-Bound workflow, the framework is also prepared to support relax-and-cut strategies. In other words, cuts can be generated dynamically from relaxed solutions and injected back into the model during the iterative process, instead of being defined only once at the beginning.

The reusable part already provides:

- storage and management of variables and constraints
- Lagrangian multipliers and subgradient updates
- dynamic cut generation and cut management during the relaxation process
- primal solution storage
- variable fixing and cleanup
- Branch-and-Bound node management
- tree exploration with BFS or DFS

The problem-specific part is responsible for describing what a variable means, how the instance is read, how the model is built, and how the Lagrangian subproblem is handled.

## What you usually need to create

To model a new problem, you will normally create four artifacts.

### 1. A problem-specific variable class

Start from `Variable`.

Use the base class if a variable only needs a cost, an identifier, and the list of constraints it covers.

Create a derived class if each variable needs extra data, such as:

- geometry
- adjacency information
- custom feasibility checks
- fast conflict detection for heuristics or branching

In this repository, `RGPVariable` is the example of a problem-specific variable class.

### 2. A problem-specific constraint representation

Start from `Constraint`.

For many binary formulations, the base class is enough. This is especially true when each coefficient is either `0` or `1` and the main need is to know whether a variable belongs to a constraint.

Create a derived constraint class only if your problem needs custom coefficients, special comparison rules, or extra metadata.

### 3. A manager class that builds the model

Derive a class from `LagrangeanManager`.

This class is the central place where the problem instance is transformed into variables and constraints.

The most important methods to implement are:

`ReadProblem`
Reads and parses the instance file.

`CreateProblem`
Creates all variables and all constraints of the model.

`InsertVariableIntoConstraint`
Connects each variable to the constraints it satisfies or covers.

You may also customize:

`PrintSolution`
Writes the incumbent solution in a problem-specific format.

`CustomProcessing`
Runs optional logic during the solution process.

`SetVariableForBranch`
Defines what branching on a variable means for your model.

In this repository, `RGPManager` is the example of a specialized manager.

### 4. An algorithm class

If your problem fits the same general structure, derive from `LagrangeanRelaxation`.

If you need more freedom, derive directly from `Algoritmo`.

The most important methods are:

`Relaxacao`
Solves the Lagrangian subproblem and returns a relaxed solution.

`Heuristica`
Tries to transform the relaxed solution into a feasible primal solution.

Optional customization points include:

`FixaVariaveis`
Uses reduced-cost style information to fix variables.

`GeraCortes`
Adds cuts to strengthen the formulation during the Lagrangian solution process.

`Price`
Is a possible extension point for future pricing strategies, but the framework should not currently be considered ready for generic pricing or column generation.

`ChooseBranchVariable`
Defines the branching rule used by Branch-and-Bound.

This is also the place where relax-and-cut strategies can be implemented. In this framework, the method `GeraCortes` allows the algorithm to inspect the current relaxed solution, identify violated inequalities, and add new cuts dynamically. This makes it possible to combine Lagrangian Relaxation with cut generation inside the same iterative solution loop.

By contrast, pricing support is not yet mature as a general framework feature. Although there is a `Price` hook in the algorithm interface, it should be seen as experimental or problem-specific, not as a ready-to-use generic pricing or column generation component.

In this repository, `RGPLagrangeanRelaxation` is the example of a specialized algorithm.

## Suggested step-by-step process

1. Identify what one decision variable means in your formulation.
2. Decide which constraints will be dualized and which ones, if any, should remain explicit.
3. Create a variable class if the base `Variable` class does not carry enough information.
4. Create a manager derived from `LagrangeanManager`.
5. Implement `ReadProblem` to load your instance data.
6. Implement `CreateProblem` to enumerate variables and create the constraints.
7. Implement `InsertVariableIntoConstraint` so each variable knows the constraints it covers.
8. Create an algorithm derived from `LagrangeanRelaxation` or `Algoritmo`.
9. Implement `Relaxacao` for your Lagrangian subproblem.
10. Implement `Heuristica` to recover feasible solutions.
11. Optionally customize cuts, branching, and variable fixing. Pricing can also be explored, but it is not yet a mature generic feature of the framework.
12. Instantiate your manager and algorithm in `main` and run the tree through `BBTree`.

## Execution flow

At a high level, the execution flow is:

1. Read the instance.
2. Build variables and constraints.
3. Initialize the algorithm with a manager.
4. Solve the Lagrangian Relaxation iteratively.
5. Try to build primal solutions with a heuristic.
6. Update bounds.
7. Optionally generate cuts from the current relaxed solution or fix variables.
8. Branch when needed and continue the search in the tree.

Because cuts may be added during the iterative process, the framework can be used not only as a plain Lagrangian Relaxation engine, but also as a basis for relax-and-cut approaches.

## Important modeling assumption

At the moment, the code is most naturally suited to binary problems with unit coefficients, especially formulations that look like set partitioning, set covering, or packing models. Adapting it to more general integer models may require extending the base classes.

This happens because several parts of the implementation assume:

- binary variables
- coefficients equal to `1`
- constraints represented mainly by membership of variables in sets

So the framework is reusable, but it is not completely problem-agnostic in its current form.

The same caution applies to pricing: the code contains a hook for it, but the framework is not yet ready to present pricing or column generation as a general reusable capability.

## Minimal integration pattern

Once the problem-specific classes exist, the integration pattern is simple:

1. Load a `Configuration`.
2. Instantiate your specialized algorithm.
3. Instantiate your specialized manager.
4. Link the algorithm to the manager.
5. Call `GenerateProblem`.
6. Create a `BBTree`.
7. Run `GO`.

That is the same integration pattern used by the rectangular partition implementation already present in this repository.
