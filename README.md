# Ecole

Ecole (pronounced [ekɔl]) stands for _Extensible Combinatorial Optimization Learning Environments_ and aims to expose a number
of combinatorial optimization tasks as Markov Decision Processes (_i.e._, Reinforcement Learning
environments).
Rather than trying to solve such tasks directly, the philosophy behind Ecole is to work in cooperation
with state-of-the-art Mixed Integer Linear Programming solver.

The underlying solver use is [SCIP](https://scip.zib.de/), and the user facing API is meant to mimic
the [OpenAi Gym](https://gym.openai.com/) API, as much as possible.

## User Documentation
Please refer to the (upcoming) documentation for tutorials, examples, and installation
instructions.

## Developer Notes
### Build Tools
  * C++14 compiler,
  * CMake >= 3.0.
  
### External dependencies
  * An installation of [SCIP](https://scip.zib.de/) >= 6.0, installed with CMake and `-D PARASCIP=true`,
  * Use `git clone --recursive` to resolve git submodules.
