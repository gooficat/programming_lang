import sys
import time
import objprint

if len(sys.argv) != 3:
    print("Error! Correct usage is: \'<python executable> compiler.py <input file> <output file>\'")
    exit(1)

with open(sys.argv[1], "r", encoding="utf-8-sig") as input_file:
    input_content = input_file.read().replace('\r', '')

chunks = []

i = 0
while i < len(input_content):
    c = input_content[i]
    if c.isalpha():
        buf = ""
        while i < len(input_content) and input_content[i].isalpha():
            buf += input_content[i]
            i += 1
    elif c.isdigit():
        buf = ""
        while i < len(input_content) and input_content[i].isdigit():
            buf += input_content[i]
            i += 1
    elif not c in '\r \n':
        buf = c
        i += 1
    else:
        i += 1
        continue
    chunks.append(buf)

class TNode:
    def __init__(self):
        pass

class Constant(TNode):
    def __init__(self, type, val):
        self.type = type
        self.val = val
    

class Identifier(TNode):
    def __init__(self, name):
        self.name = name

class Assignment(TNode):
    def __init__(self, left, right):
        self.left = left
        self.right = right

class Operation(TNode):
    def __init__(self, operator, left, right):
        self.operator = operator
        self.left = left
        self.right = right

class Return(TNode):
    def __init__(self, value):
        self.value = value

class FuncDef(TNode):
    def __init__(self, name, body):
        self.name = name
        self.body = body

TREE = []

i = 0


def break_down(statement):
    if len(statement) > 2:
        operation = Operation(statement[1], statement[0], None)
        operation.right = break_down(statement[2:])
        return operation
    else:
        return Constant("int", statement[0])


while i < len(chunks):
    c = chunks[i]
    if c == "let":
        ident = Identifier(chunks[i + 1])
        TREE.append(Assignment(ident, None))
        i += 3
        statement = []
        while chunks[i] != ";":
            statement.append(chunks[i])
            i += 1
        TREE[-1].right = break_down(statement)
        i += 1
    else:
        i += 1

objprint.op(TREE)

# with open(sys.argv[2], "w", encoding="utf-8") as output_file:
#     output_file.flush()
#     for t in tokens:
#         # if t == 
#         pass