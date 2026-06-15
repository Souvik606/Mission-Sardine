class Node:
    def __init__(self, value):
        self.value = value
        self.left = False
        self.right = False

class BST:
    def __init__(self):
        self.root = False
        
    def insert(self, value):
        if self.root is False:
            self.root = Node(value)
        else:
            self.insert_node(self.root, value)
            
    def insert_node(self, current, value):
        if value < current.value:
            if current.left is False:
                current.left = Node(value)
            else:
                self.insert_node(current.left, value)
        else:
            if current.right is False:
                current.right = Node(value)
            else:
                self.insert_node(current.right, value)
                
    def search(self, value):
        return self.search_node(self.root, value)
        
    def search_node(self, current, value):
        if current is False:
            return False
        if current.value == value:
            return True
        if value < current.value:
            return self.search_node(current.left, value)
        else:
            return self.search_node(current.right, value)

def run_bst(n):
    bst = BST()
    for i in range(1, n + 1):
        val = (i * 17) % n
        bst.insert(val)
        
    count = 0
    for i in range(1, n + 1):
        val = (i * 17) % n
        if bst.search(val):
            count += 1
    return count

print("bst count =", run_bst(1000))
