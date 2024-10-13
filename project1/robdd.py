import sys

class Robddnode:
    """
    Class representing a node in the ROBDD (Reduced Ordered Binary Decision Diagram).
    
    Attributes:
        var: The Boolean variable assigned to this node (e.g., 'a', 'b', 'c').
        index: The unique index assigned to this node.
        then_edge: Represents the "then" (1) edge.
        else_edge: Represents the "else" (0) edge.
        left: Pointer to the left child node (for the "else" edge).
        right: Pointer to the right child node (for the "then" edge).
        is_redundant: A flag indicating whether this node is redundant and should be removed.
    """
    def __init__(self, var, index) -> None:
        self.var = var
        self.then_edge = 1
        self.else_edge = 0
        self.left = None
        self.right = None
        self.index = index
        self.is_redundant = False


class Robdd:
    """
    Class representing the overall ROBDD structure.

    Attributes:
        input_file: The input PLA file containing Boolean function.
        output_file: The file where the DOT format of the ROBDD will be written.
        output_node: The index for the output node.
        input_node: The index for the input node.
        vars: The list of Boolean variables in the function.
        func: The list of product terms (in PLA format).
        tree: The root node of the ROBDD.
        tree_dict: A dictionary that maps node indices to their respective nodes.
    """
    def __init__(self, input_file, output_file) -> None:
        self.input_file = input_file
        self.output_file = output_file
        self.output_node = 1
        self.input_node = -1
        self.vars = []
        self.func = []
        self.tree = None
        self.tree_dict = dict()

    def readfile(self):
        """
        Reads the input PLA file and extracts the variables and product terms. 
        It processes the file line by line to build the function representation.
        """
        start = False
        with open(self.input_file, 'r') as f:
            for line in f:
                if '.i' in line and self.input_node == -1: self.input_node = line[2:].strip() 
                if '.ilb' in line: self.vars = line[4:].split() 
                if '.e' in line: start = False 
                if start:
                    self.func.append(line[:-1].strip())
                if '.p' in line: start = True

        # proccess func
        self.func = dict((term.split()) for term in self.func)

    def build_completed_tree(self):
        """
        Builds the complete binary decision tree using the variables and function from the PLA file.
        Connects nodes and creates a structure for the ROBDD with pointers to left and right children.
        """
        # root
        self.tree = Robddnode(self.vars[0], index=1)
        self.tree_dict[1] = self.tree
        cur_i = 1

        # build tree
        for i in range(1, len(self.vars)):
            for j in range(2**i, 2**i*2):
                node = Robddnode(self.vars[i], index=j)
                self.tree_dict[j] = node
                if j % 2 == 0:
                    self.tree_dict[cur_i].left = node
                else:
                    self.tree_dict[cur_i].right = node
                    cur_i += 1  # move to the next paraent node
        
        # add terminal nodes (boolean 0 and 1)
        bool_zero = Robddnode(var='zero', index=0)  
        bool_one = Robddnode(var='one', index=2**len(self.vars))  
        self.tree_dict[0] = bool_zero
        self.tree_dict[2**len(self.vars)] = bool_one

        # proccess each product term based on provided function
        func = ""
        t = self.traversal(self.tree, func)

    def traversal(self, cur, func):
        """
        Recursively traverses the binary decision tree to check if the current path leads to a terminal node.
        
        Args:
            cur: The current node being traversed.
            func: The binary string representing the current path.
        
        Returns:
            The terminal node index (0 or 1) if the path leads to a terminal node.
        """
        # something wrong
        if cur is None:
            # return 0/1 for terminal node's leaf
            ans = self.proccess_term(func)
            return ans

        # left node (boolean 0)
        terminal = self.traversal(cur.left, func+"0")
        if terminal is not None:
            cur.left = self.tree_dict[int(terminal)]

        # right node (boolean 1)
        terminal = self.traversal(cur.right, func+"1")
        if terminal is not None:
            cur.right = self.tree_dict[terminal]

    def proccess_term(self, f):
        """
        Compares the given binary path to the product terms in the function and returns 1 if it matches a product term.
        
        Args:
            f: The binary string representing the current path.
        
        Returns:
            1 if the path matches a product term, else 0.
        """
        output = True

        for term, _ in self.func.items():   # check every term in self.func until there's one completely the same
            for c1, c2 in zip(f, term):
                if c1 != c2 and c2 != '-':
                    output = False
                    break
            if output:
                return 2**len(self.vars)    # there's at least one term in dict that is the same with f -> 1 
            output = True                   # initialize for the next term
        return 0                            # the index for boolean 1 is 2**len(self.vars) -> key in self.tree_dict
    
    # reduce isomorphic nodes, start from the bottom
    def reduce(self):
        """
        Reduces the ROBDD by removing isomorphic and redundant nodes.
        Starts from the bottom and checks for nodes with the same left and right children or identical nodes.
        """
        reserve, isomorphic, redundant = dict(), [], dict()

        for ch in reversed(self.vars):  # [a, b, c] -> c, b, a
            self.delete_node(reserve, isomorphic, redundant, ch)
            redundant = self.find_redundant(redundant, ch)
            reserve, isomorphic = self.find_iso(reserve, isomorphic, redundant, ch)
            self.print_tree_table()

    def delete_node(self, reserve, iso, redundant, ch):
        """
        Removes redundant nodes based on their left and right children. If a node is isomorphic, it's replaced by the appropriate node.
        
        Args:
            reserve: Dictionary to store nodes to reserve.
            iso: List to track isomorphic nodes.
            redundant: Dictionary of redundant nodes.
            ch: The variable for the current node.
        """
        # all the three params args are empty
        if not reserve:  
            return
        
        for i in range(self.vars.index(ch)+1, 2**(self.vars.index(ch)+1)):
            li = self.tree_dict[i].left.index  # 4
            ri = self.tree_dict[i].right.index  # 5

            # check redundant
            if li in redundant:
                self.tree_dict[i].left = redundant[li]
            if ri in redundant:
                self.tree_dict[i].right = redundant[ri]

            # check iso
            if li in iso:
                new_node = self.tree_dict[reserve[(self.tree_dict[li].left.index, self.tree_dict[li].right.index)]]
                self.tree_dict[i].left = new_node
            if ri in iso:
                new_node = self.tree_dict[reserve[(self.tree_dict[ri].left.index, self.tree_dict[ri].right.index)]]
                self.tree_dict[i].right = new_node

    def find_redundant(self, redundant, ch):
        """
        Identifies nodes where the left and right edges point to the same child, marking them as redundant.
        
        Args:
            redundant: The dictionary to store redundant nodes.
            ch: The current variable to check.
        
        Returns:
            The updated dictionary of redundant nodes.
        """
        redundant.clear()
        index = self.vars.index(ch)

        for i in range(2**index, 2**index*2):
            if self.tree_dict[i].left.index == self.tree_dict[i].right.index:
                redundant[i] = self.tree_dict[i].left
                self.tree_dict[i].is_redundant = True
        return redundant
    
    def find_iso(self, reserve, iso, redundant, ch):
        """
        Identifies isomorphic nodes and stores them in the reserve or isomorphic list.
        
        Args:
            reserve: Dictionary to store nodes to reserve.
            iso: List to track isomorphic nodes.
            redundant: Dictionary of redundant nodes.
            ch: The variable for the current node.
        
        Returns:
            The updated reserve and isomorphic nodes as a tuple.
        """
        reserve.clear()
        iso = []
        index = self.vars.index(ch)

        for i in range(2**index, 2**index*2):
            child = (self.tree_dict[i].left.index, self.tree_dict[i].right.index)
            if i not in redundant:
                if child not in reserve:
                    reserve[child] = i
                else:
                    iso.append(i)
                    self.tree_dict[i].is_redundant = True
        return reserve, iso
    
    def print_tree_table(self):
        """
        Prints the tree structure with indices, variable names, and edges to the console.
        This function is primarily for debugging and verification.
        """
        print(f"index\tvariables\telse-edge\tthen-edge\n")

        for key, value in self.tree_dict.items():
            if value.left is not None and not value.is_redundant:
                print(f"{key}\t{value.var}\t{value.left.index}\t{value.right.index}\n")
        print("="*20) 

        # for boolean 1
        bool_one = Robddnode(var='one', index=2**len(self.vars))  
        self.tree_dict[2**len(self.vars)] = bool_one

    def get_tree(self):
        """
        Returns the tree structure (dictionary of nodes).
        
        Returns:
            The dictionary mapping indices to nodes.
        """
        return self.tree_dict   

    def get_vars(self):
        """
        Returns the list of Boolean variables.
        
        Returns:
            The list of variables.
        """
        return self.vars  


def main():
    """
    Main function to read the input PLA file, build the ROBDD, and reduce the ROBDD.
    It then writes the final ROBDD structure to a DOT file for visualization.
    """
    input_file = sys.argv[1]
    output_file = sys.argv[2]

    robdd = Robdd(input_file, output_file)
    robdd.readfile()
    robdd.build_completed_tree()

    robdd.reduce()
    robdd.print_tree_table()

    write_dot_file(output_file, robdd.get_tree(), robdd.get_vars())


def write_dot_file(filename, tree, vars):
    """
    Writes a .dot file with the provided graph data.
    
    Args:
        filename: The name of the .dot file to write.
        tree: A dictionary where the key is the node index, and the value is the node object.
        vars: The list of variables in the ROBDD.
    """
    ranks = dict()
    with open(filename, 'w') as f:
        f.write("digraph G {\n")  # 'digraph' specifies a directed graph
        for index, node in tree.items():
            if not node.is_redundant and node.left:
                ranks[index] = (node.left.index, node.right.index)
                f.write("{rank=same " + str(index) + "}\n")
        f.write("\n0 [label=0, shape=box]\n")
        for index, _ in ranks.items():
            f.write(str(index) + " [label=\"" + tree[index].var + "\"]\n")
        f.write(str(2**len(vars)) + " [label=1, shape=box]\n\n")
        for index, child in ranks.items():
            f.write(str(index) + " -> " + str(child[0]) + " [label=\"0\", style=dotted]\n")
            f.write(str(index) + " -> " + str(child[1]) + " [label=\"1\", style=solid]\n")
        
        f.write("}\n")


if __name__ == "__main__":
    main()