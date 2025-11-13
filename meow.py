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
    def __init__(self, type, value):
        self.type = type
        self.value = value
    

existing_identifiers = []

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
    def __init__(self, name : str, body):
        self.name = name
        self.body = body


ops = [
    '+',
    '-',
    '*',
    '/'
]

def parse_operation(inp):
    if not isinstance(inp, TNode):
        if len(inp) == 1:
            return Constant('int', inp[0])
        for o in ops:
            if o in inp:
                ol = inp.index(o)
                return Operation(
                    o,
                    parse_operation(inp[:ol]),
                    parse_operation(inp[ol+1:])
                )
    else:
        print("that's not good")


TREE = []

i = 0


def break_down(statement):
    if len(statement) > 2:
        operation = parse_operation(statement)
        return operation
    else:
        if (statement[0][0].isalpha()):
            return Identifier(statement[0])
        else:
            return Constant("int", statement[0])


while i < len(chunks):
    c = chunks[i]
    if c == "let":
        ident = Identifier(chunks[i + 1])
        existing_identifiers.append(ident.name)
        TREE.append(Assignment(ident, None))
        i += 3
        statement = []
        while i < len(chunks) and chunks[i] not in ";\n":
            statement.append(chunks[i])
            i += 1
        TREE[-1].right = break_down(statement)
        i += 1
    elif c == "return":
        i += 1
        statement = []
        while i < len(chunks) and chunks[i] not in ";\n":
            statement.append(chunks[i])
            i += 1
        TREE.append(Return(break_down(statement)))
        i += 1
    else:
        i += 1

output_section_bss = """section .bss
"""

output_section_text = """
section .text
global _start

_start:
"""

for identifier in existing_identifiers:
    output_section_bss += identifier + " resb 4\n"

def generate_operation(operation : Operation) -> str:
    out = "NO OPERATOR FOR " + operation.operator
    match operation.operator:
        case "+":
            out = "add eax, ebx"
        case "-":
            out = "sub eax, ebx"
        case "*":
            out = "imul eax, ebx"
        case "/":
            out = "idiv eax, ebx"
    return out

def generate_statement(ident : TNode) -> str:
    if isinstance(ident, Constant):
        return "mov eax, " + str(ident.value) + " \n"
    
    elif isinstance(ident, Identifier):
        return "mov eax, [" + ident.name + "]\n"
    
    elif isinstance(ident, Operation):
        asm = generate_statement(ident.left)
        asm += "push eax\n"
        asm += generate_statement(ident.right)
        asm += "pop ebx\n"
        asm += generate_operation(ident) + "\n"
        return asm
    elif isinstance(ident, Assignment):
        asm = generate_statement(ident.right)
        asm += "mov [" + ident.left.name + "], eax\n"
        return asm
    else:
        exit(2)

objprint.op(TREE)

for node in TREE:
    if isinstance(node, Assignment) or isinstance(node, Operation):
        output_section_text += generate_statement(node)

    elif isinstance(node, Return):
        output_section_text += generate_statement(node.value)
        output_section_text += "ret\n"

with open(sys.argv[2], "w", encoding="utf-8") as output_file:
    output_file.flush()
    output_file.write(output_section_bss)
    output_file.write(output_section_text)