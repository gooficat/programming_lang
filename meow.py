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


ops = [ # strictly by order of priority
    '+',
    '-',
    '*',
    '/',
    '^'

]


i = 0
while i < len(input_content):
    c = input_content[i]
    buf = ""
    if c.isalpha() or c in allowed_prefixes:
        if not c[0].isalpha():
            buf += input_content[i]
            i += 1
        while i < len(input_content) and input_content[i].isalpha():
            buf += input_content[i]
            i += 1
    elif c.isdigit():
        while i < len(input_content) and input_content[i].isdigit():
            buf += input_content[i]
            i += 1
    elif not c in '\r \t\n':
        buf = input_content[i]
        i += 1
    else:
        i += 1
        continue

    # print(buf)
    chunks.append(buf)

class TNode:
    def __init__(self):
        pass

class Constant(TNode):
    def __init__(self, type, value):
        self.type = type
        self.value = value
    

# existing_identifiers = []
static_variable_identifiers = []
function_identifiers = []

class Identifier(TNode):
    def __init__(self, name):
        self.name = name

class Assignment(TNode):
    def __init__(self, left, right):
        self.left = left
        self.right = right

class IdentDef(TNode):
    def __init__(self, name : str):
        self.name : str = name
        self.static = True
        self.offset = 0

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
        self.variable_identifiers : list = []
        self.variable_identifier_offsets : list = []
    def __eq__(self, other):
        return True

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

rec_depth = 0
current_off = 0

def parse_operation(inp):
    global current_off
    global current_scope
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
                    inp[ol],
                    parse_operation(inp[:ol]),
                    parse_operation(inp[ol+len(o):])
                )
    else:
        print("tried to parse an operation that is a tree node??? that's not good...")


current_scope = None

stackvar_offset = 0

def parse_chunks(chunks):
    tr = []
    i = 0
    global stackvar_offset
    global current_scope
    while i < len(chunks):
        c = chunks[i]
        if c == "static":
            static_variable_identifiers.append(chunks[i+1])
            if chunks[i + 2] == "=":
                tr.append(Assignment(Identifier(chunks[i+1]), None))
                i += 3
                statement = []
                while i < len(chunks) and chunks[i] not in ";\n":
                    statement.append(chunks[i])
                    i += 1
                tr[-1].right = parse_operation(statement)
            else:
                i += 3
        if current_scope is not None and c == "let":
            ident = IdentDef(chunks[i + 1])
            ident.static = False
            ident.offset = stackvar_offset
            stackvar_offset += 4
            current_scope.variable_identifier_offsets.append(ident.offset)
            current_scope.variable_identifiers.append(ident.name)
            if chunks[i + 2] == "=":
                tr.append(Assignment(Identifier(chunks[i+1]), None))
                i += 3
                statement = []
                while i < len(chunks) and chunks[i] not in ";\n":
                    statement.append(chunks[i])
                    i += 1
                tr[-1].right = parse_operation(statement)
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
            function_identifiers.append(name)
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
            func = FuncDef(
                    name,
                    params,
                    []
                )
            current_scope = func
            func.body = parse_chunks(body)
            tr.append(
                func
            )
        elif current_scope is not None and c in current_scope.variable_identifiers:
            idnt = Identifier(c)
            if chunks[i + 1] == "=":
                tr.append(Assignment(idnt, None))
                i += 2
                statement = []
                while i < len(chunks) and chunks[i] not in ";\n":
                    statement.append(chunks[i])
                    i += 1
                tr[-1].right = parse_operation(statement)
                i += 1
            else:
                print("why is it not an assignment?")
                i += 1
        
        elif c in static_variable_identifiers:
            print("found a static var")
            idnt = Identifier(c)
            print("made an identifier instance")
            if chunks[i + 1] == "=":
                print("parsing a nonstatic assignment now")
                tr.append(Assignment(idnt, None))
                i += 2
                statement = []
                while i < len(chunks) and chunks[i] not in ";\n":
                    statement.append(chunks[i])
                    i += 1
                tr[-1].right = parse_operation(statement)
                i += 1
            else:
                print("static why is it not an assignment?")
                i += 1
        elif c in function_identifiers:
            ident = Identifier(c)
            params = []
            i += 2
            while i < len(chunks) and chunks[i] not in ")":
                params.append(chunks[i])
                i += 1
            tr.append(FuncCall(ident.name, params))
            i += 1
        else:
            i += 1
    return tr


TREE = Scope(parse_chunks(chunks))
print(chunks)

output_header = """extern _ExitProcess@4
"""

output_section_bss = """section .bss
"""
output_section_data = """section .data
"""
output_section_text = """
section .text
global _start

"""
internal_labels = 0

for identifier in static_variable_identifiers:
    output_section_bss += identifier + " resb 4\n"

def generate_operation(operation : Operation, r1 = "eax", r2="ebx") -> str:
    global internal_labels
    out = "NO OPERATOR FOR " + operation.operator
    match operation.operator:
        case "+":
            out = f"add {r1}, {r2}"
        case "-":
            out = f"sub {r1}, {r2}"
        case "*":
            out = f"imul {r1}, {r2}"
        case "/":
            out = f"idiv {r1}, {r2}"
        case "^":
            # print(operation.right.value)
            out = \
f"""
mov ecx, {r1}
mov edx, {r2}
label_{internal_labels}:
imul {r2}, edx
sub ecx, 1
cmp ecx, 1
jne label_{internal_labels}
mov {r1}, {r2}"""
            internal_labels += 1
    return out

regs = [ # whether it is reserved or not
    "eax",
    "ebx",
    "ecx",
    "edx",
    "esi",
    "edi"
]

regact = [
    False,
    False,
    False,
    False,
    False,
    False
]


def fal_reg(reg):
    global regs
    global regact
    regact[regs.index(reg)] = False
def tru_reg(reg):
    global regs
    global regact
    regact[regs.index(reg)] = True

def reset_regs():
    global regs
    for r in regs:
        fal_reg(r)


def generate_statement(ident : TNode, scope : Scope) -> str:
    global rec_depth
    global regs
    global regact
    rec_depth += 1
    global current_off

    r1 = ""
    r2 = ""
    for r in range(len(regs)):
        if regact[r]:
            r1 = regs[r]

    if isinstance(ident, Constant):
        return "mov eax, " + str(ident.value) + " \n"
    
    elif isinstance(ident, Identifier):
        if ident.name in static_variable_identifiers:
            return "mov eax, [" + ident.name + "]\n"
        elif current_scope is not None:
            return f"mov eax, [esp + {current_scope.variable_identifier_offsets[current_scope.variable_identifiers.index(ident.name)]} + {current_off}]\n"
        else:
            print("ERERRRRRORRRRRRRRRR")
            return ""
    elif isinstance(ident, Operation):
        asm = generate_statement(ident.left, scope)
        asm += "push eax\n"
        current_off += 4
        asm += generate_statement(ident.right, scope)
        asm += "pop ebx\n"
        current_off -= 4
        asm += generate_operation(ident) + "\n"
        return asm
    elif isinstance(ident, Assignment):
        if ident.left.name in scope.variable_identifiers:
            asm = generate_statement(ident.right, scope)
            asm += f"sub esp, 4\n"
            asm += f"mov [esp], eax\n" # make a nonstatic variable
        else:
            asm = generate_statement(ident.right, scope)
            asm += f"mov [" + ident.left.name + "], eax\n"
        return asm
    else:
        print("no identifier type check for")
        objprint.op(ident)
        return ""

objprint.op(TREE)

def parse_scope(scope) -> str:
    output = ""
    nodes = scope.body
    global output_section_data
    global output_section_bss
    for node in nodes:
        if isinstance(node, Assignment) or isinstance(node, Operation):
            rec_depth = 0
            reset_regs()
            output += generate_statement(node, scope)

        elif isinstance(node, Exit):
            if node.value in static_variable_identifiers:
                output += generate_statement(node.value, scope) + "\n" # must change to actually dereference for stack vars too
            else:
                pass
            output += "call _ExitProcess@4\n"

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
    output_file.write(output_header)
    output_file.write(output_section_bss)
    output_file.write(output_section_data)
    output_file.write(output_section_text)