import sys
import time
import objprint

if len(sys.argv) != 3:
    print("Error! Correct usage is: \'<python executable> compiler.py <input file> <output file>\'")
    exit(1)

with open(sys.argv[1], "r", encoding="utf-8-sig") as input_file:
    input_content = input_file.read().replace('\r', '')

chunks = []

allowed_prefixes = [
    "_"
]

i = 0
while i < len(input_content):
    c = input_content[i]
    if c.isalpha() or c in allowed_prefixes:
        buf = ""
        if not c[0].isalpha():
            buf += input_content[i]
            i += 1
        while i < len(input_content) and input_content[i].isalpha():
            buf += input_content[i]
            i += 1
    elif c.isdigit():
        buf = ""
        while i < len(input_content) and input_content[i].isdigit():
            buf += input_content[i]
            i += 1
    elif not c in '\r ':
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

class Exit(TNode):
    def __init__(self, value):
        self.value = value

class Scope(TNode):
    def __init__(self, body : list):
        self.body = body

class FuncDef(Scope):
    def __init__(self, name : str, params : list, body : list):
        super().__init__(body)
        self.name = name
        self.params = []

class FuncCall(TNode):
    def __init__(self, name : str, params : list):
        super().__init__()
        self.name = name
        self.params = params

class AssemblyBlock(TNode):
    def __init__(self, content):
        self.content = content

class NewLine(TNode):
    def __init__(self):
        super().__init__()

ops = [ # strictly by order of priority
    '+',
    '-',
    '*',
    '/'
]

def parse_operation(inp):
    if not isinstance(inp, TNode):
        if len(inp) == 1:
            if inp[0][0].isalpha():
                return Identifier(inp[0])
            elif inp[0][0].isdigit():
                return Constant("int", inp[0])
        for o in ops:
            if o in inp:
                ol = inp.index(o)
                return Operation(
                    o,
                    parse_operation(inp[:ol]),
                    parse_operation(inp[ol+1:])
                )
    else:
        print("tried to parse an operation that is a tree node??? that's not good...")



def parse_chunks(chunks):
    tr = []
    i = 0
    while i < len(chunks):
        c = chunks[i]
        if c == "let":
            ident = Identifier(chunks[i + 1])
            existing_identifiers.append(ident.name)
            if chunks[i + 2] == "=":
                tr.append(Assignment(ident, None))
                i += 3
                statement = []
                while i < len(chunks) and chunks[i] not in ";\n":
                    statement.append(chunks[i])
                    i += 1
                tr[-1].right = parse_operation(statement)
                i += 1
            else:
                i += 3
        elif c == "exit":
            i += 1
            statement = []
            while i < len(chunks) and chunks[i] not in ";\n":
                statement.append(chunks[i])
                i += 1
            tr.append(Exit(parse_operation(statement)))
            i += 1
        elif c == "asm":
            i += 2
            block = "\n"
            while i < len(chunks) and chunks[i] != "}":
                while chunks[i] not in ";\n":
                    block += chunks[i] + " "
                    i += 1
                block += chunks[i]
                i += 1
            
            tr.append(AssemblyBlock(block))
        elif c == "func":
            i += 1
            name = chunks[i]
            existing_identifiers.append(name)
            i += 2
            params = []
            while chunks[i] != ")":
                params.append(chunks[i])
                i += 1
            i += 2
            body = []
            while chunks[i] != "}":
                body.append(chunks[i])
                i += 1
            tr.append(
                FuncDef(
                    name,
                    params,
                    parse_chunks(body)
                )
            )
        elif c in existing_identifiers:
            ident = Identifier(c)
            if chunks[i + 1] == "=":
                tr.append(Assignment(ident, None))
                i += 2
                statement = []
                while i < len(chunks) and chunks[i] not in ";\n":
                    statement.append(chunks[i])
                    i += 1
                tr[-1].right = parse_operation(statement)
                i += 1
            else:
                params = []
                i += 1
                while i < len(chunks) and chunks[i] not in ")":
                    params.append(chunks[i])
                    i += 1
                tr.append(FuncCall(ident.name, params))
                i += 1
        else:
            i += 1
    return tr


TREE = Scope(parse_chunks(chunks))


output_section_bss = """section .bss
"""
output_section_data = """section .data
"""
output_section_text = """
section .text
global _start

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
        return ""

objprint.op(TREE)

def parse_scope(scope) -> str:
    output = ""
    nodes = scope.body
    global output_section_data
    global output_section_bss
    for node in nodes:
        if isinstance(node, Assignment) or isinstance(node, Operation):
            output += generate_statement(node)

        elif isinstance(node, Exit):
            output += generate_statement(node.value)
            output += "ret\n"

        elif isinstance(node, AssemblyBlock):
            output += node.content

        elif isinstance(node, FuncDef):
            output += "_" + node.name + ":\n"
            output += parse_scope(node)

        elif isinstance(node, FuncCall):
            # !!!TODO!!! push the params from node.params to the parameter stack
            output += "call " + "_" + node.name + "\n"
        else:
            print("!!!!!!!! NODE NOT HANDLED BY COMPILER !!!!!!!!")
    return output


output_section_text += parse_scope(TREE)

with open(sys.argv[2], "w", encoding="utf-8") as output_file:
    output_file.flush()
    output_file.write(output_section_bss)
    output_file.write(output_section_data)
    output_file.write(output_section_text)