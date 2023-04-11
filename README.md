# EC513</br>
Homework 3 Question 4: The Pin Assignment</br>

Implement a branch predictor with the Pin tool and upload your source codes and results to this repository. Follow the tutorial for a bimodal branch predictor here: https://github.com/bu-ece513-spring2023/pin-bimodal-tutorial

Reference recource:
https://csnlp.github.io/2019/11/03/branch-prediction/
https://people.eecs.berkeley.edu/~kubitron/courses/cs252-F03/handouts/papers/p52-evers.pdf
https://github.com/hibagus/TwoLevelBPSimulator

In the project, I am using two level predict. The first level predict history is BHR, the second level predict history is pattern history table with 2 bit saturating counter. In the my code, I used two pattern history tables.
The first level of history is usually a local history table, which stores information about the outcomes of recent branches that have the same address as the current branch. The second level of history is a global history register, which stores a sequence of recently executed branches across the entire program.
When a new branch is encountered, the processor uses the current program counter (PC) to look up the entry in the local history table corresponding to the current branch. This entry is used to index into a pattern history table (PHT), which contains information about the outcome of recent branches with the same history as the current branch. The PHT entry is then used to make a prediction for the current branch.
By incorporating global history information, two-level branch prediction can better capture the long-term behavior of branches and improve the accuracy of predictions.
I try to add another pattern table and selector. I think it will perform better. But there is still bug in it.
Below is the result I get:
Icount: 614471 Mispredicts: 2392 MPKI: 3.892779
References: 117412 Predicts: 115020
