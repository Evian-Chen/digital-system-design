# digital-system-design
This is a required course for second-year CS students at NTUST (113-1).

Proffesor: Yi-yu Liu

##  Project 1 – ROBDD Generation 
Binary Decision Diagram (BDD) is a directed acyclic graph useful to represent 
the Boolean function. Each BDD node is associated with one Boolean variable. Two 
outgoing edges exist in each internal node: the then-edge and the else-edge. The 
then-edge indicates that the Boolean variable is "1,” and the else-edge indicates that 
the Boolean variable is "0". The true/false decision is made by evaluating variables 
from the root node to the leaf node. Given a fixed BDD variable ordering, we can 
generate a unique ordered BDD (OBDD) for every distinct Boolean function. An 
OBDD is said to be a reduced OBDD (ROBDD) when the following properties are 
satisfied: (1) all isomorphic nodes (i.e., Boolean variable, then-edge, and else-edge are 
the same) are merged into one single unique node and (2) The redundant node (i.e., 
both of its then-edge and else-edge point to the same node) is removed. In this project, 
your C/C++ program reads a single-output Boolean function in PLA format and 
outputs the corresponding ROBDD in pictorial DOT format. 

## Project 2 – Exact Boolean Minimization
The sum of products (SOP) is a standard form for 2-level Boolean function 
representation. The corresponding literals are ANDed in product terms and then ORed 
together as the output. The SOP representation can be realized in either AND-OR 
gates or NAND-NAND gates. To reduce the circuit cost, it is important to minimize 
the numbers of product terms and literals. Dr. Maurice Karnaugh proposed a pictorial 
form of a truth table, known as the Karnaugh map, and combined adjacent squares 
recursively for prime implicant (PI) generations. However, the Karnaugh map is 
difficult to scale up when the number of Boolean variables is greater than 6. Dr. Quine 
and Dr. McCluskey developed a computer-based tabular form to identify all PIs 
efficiently. This is the well-known Quine–McCluskey algorithm. Since the minimum 
SOP representation is the subset of PIs, Dr. Petrick proposed a technique to determine 
all minimum SOP solutions from the PI candidates. In this project, your C/C++ 
program reads a single-output Boolean function (with don't cares) in PLA format and 
outputs its minimum SOP representation in PLA format using the Qunie-McCluskey 
algorithm followed by the Petrick Algorithm. Report the number of product terms and 
literals. 

## Digital System Design Project 3 – State Minimization 
In the sequential circuit, the time sequence of inputs, states, and outputs can be 
graphically represented in a state diagram, a.k.a. state transition graph (STG). In an STG, 
states are represented by circles/vertices, and the transitions between two states are indicated 
by directed lines/edges. According to the input condition, each directed line/edge originates at 
a "present state" and terminates at a "next state". Since the number of states is correlated with 
the number of required state flip-flops in a sequential circuit, minimizing the number of states 
could reduce the number of state flip-flops as well as hardware costs. In this project, your 
program reads a completely specified STG (i.e., STG without don't care conditions) in KISS 
format and outputs its minimum STG in KISS format and in pictorial DOT format. 
