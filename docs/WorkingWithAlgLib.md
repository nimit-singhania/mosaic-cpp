# Working with AlgLib

AlgLib provides the base solvers for linear constraints and regression problems that are generated as part of the model training. We leverage two types of solvers:
* Linear regression solver: this is a standard machine learning procedure to learn a linear function that interpolates given data and minimizes the error on input data points.
* Linear Programming: we leverage linear programming to find predicates that separate a set of positive and negative points.

We further leverage the random number generation from the library to generate splits of data-sets to improve predicate generation. More details can be found in the EMSOFT'13 paper.

We faced multiple challenges while implementing the tool:
* The split function that relies on random number generation from the library often returned infeasible or narrow splits. We added simple univariate (bivariate) heuristic and a global splitting heuristic to improve the splitting for common cases. The random number generation is only called when basic heuristics do not work.

* The genPredicate method that constructs a linear predicate that separates positive points from a negative group points returned incorrect predicate at times. The additional checking for generated predicate helps catch such this issue. The linear constraints solver seems to get stuck in local minima, and the solver was updated to use scaling to improve search efficiency. We further add solutions for simple predicates of the form x >= n (univariate) and x + y >= n (bivariate). These help scale the synthesis of predicates.

