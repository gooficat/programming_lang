#include <cassert>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

constexpr string LANGUAGE_NAME = "meowlang";

// TODO decide whether to follow windows calling convention (probably not)
const vector<string> REGS = {
    "rax",
    "rbx",
    "rcx",
    "rdx",
    "rsi",
    "rdi",
    "r8",
    "r9",
    "r10",
    "r11",
    "r12",
    "r13",
    "r14",
    "r15"
};
vector<string> split(const string& input);

enum NodeType {
    CONSTANT,
    VARDEF,
    VARREF,
    OPERATION,
    ASSIGNMENT,
    FUNCDEF,
    CALL,
    SCOPE,
    COMPARISON,
    CONDITIONAL,
    ASSEMBLY
};

map<string, string> MACROS = {
    {"exit", "ExitProcess"}
};

struct Node {
    NodeType what_type;
};

struct AssemblyBlock : Node {
    AssemblyBlock() {
        what_type = ASSEMBLY;
    }
    string content;
};

struct Constant : Node {
    Constant() {
        what_type = CONSTANT;
    }
    // enum ConstantType {} type; // add later
    uint8_t length;
    string value;
};

struct Named : Node {
    string name;
};
struct VarDef : Named {
    
};

struct VarRef : Node {
    VarRef() {
        what_type = VARREF;
    }
    shared_ptr<VarDef> to_what;
};

struct Operation : Node {
    Operation() {
        what_type = OPERATION;
    }
    vector<unique_ptr<Node>> Nodes;
};

struct Assignment : Node {
    Assignment() {
        what_type = ASSIGNMENT;
    }
    shared_ptr<VarDef> to_what;
    Operation value;
};

struct Function : Named {
    Function() {
        what_type = FUNCDEF;
    }
    unique_ptr<struct Scope> body;
    vector<VarDef> args;
};

struct Scope : Node {
    Scope() {
        what_type = SCOPE;
    }
    vector<shared_ptr<Node>> nodes;
    vector<shared_ptr<VarDef>> identifiers;
    vector<shared_ptr<Function>> function_identifiers;
    // add some storage of all of its definitions perhaps
};


struct Comparison : Node {
    Comparison() {
        what_type = COMPARISON;
    }
    vector<unique_ptr<Node>> left;
    vector<unique_ptr<Node>> right;
};

struct Call : Node {
    Call() {
        what_type = CALL;
    }
    shared_ptr<Function> of_what;
    vector<shared_ptr<Node>> args;
};

struct Conditional : Node {
    Conditional() {
        what_type = CONDITIONAL;
    }
    vector<unique_ptr<Comparison>> comparisons;
    unique_ptr<Node> outcome;
};

auto find_var_in_scope(string val, Scope *scope) -> shared_ptr<Node> {
    for (auto& var : scope->identifiers) {
        if (var->name == val) {
            return var;
        }
    }
    for (auto& var : scope->function_identifiers) {
        if (var->name == val) {
            return var;
        }
    }
    
    cerr << "Could not find var in scope!" << endl;
    exit(EXIT_FAILURE);
}

Node * parse_expression(const vector<string>& chunks, Scope* scope) {
    if (chunks.size() == 1) {
        if(isalpha(chunks.at(0).at(0))) {
            cout << chunks.at(0) << " is beginning with an alpha!" << endl;
            shared_ptr<Node> to_what = find_var_in_scope(chunks.at(0), scope);
            if (to_what->what_type == VARDEF) {
                auto n = new VarRef();
                n->to_what = static_pointer_cast<VarDef>(shared_ptr<Node>(to_what));
                return n;
            }
            else if (to_what->what_type == FUNCDEF) {
                auto n = new Call();
                n->of_what = static_pointer_cast<Function>(shared_ptr<Node>(to_what));
                return n;
            }
        }
        else {
            cout << chunks.at(0) << " is beginning with an digit!" << endl;
            auto n = new Constant();
            n->length = 4;
            n->value = chunks.at(0);
            return n;
        }
    }
    else {
        cerr << "Length of expression greater than 1! Not yet supported" << endl;
        exit(EXIT_FAILURE);
    }
}


Scope *parse_chunks(const vector<string>& chunks, bool is_topscope = false) {
    auto i = 0ULL;

    Scope *scope = new Scope();
    static Scope *topscope = scope;
    if (is_topscope) {
        scope->function_identifiers.emplace_back(
            new Function()
        );
        scope->function_identifiers.at(0)->name = "ExitProcess";
    }

    while (i < chunks.size()) {
        if (not chunks.at(i).compare("func")) {
            cout << "Found a function" << endl;
            auto func = new Function();
            ++i;
            func->name = chunks.at(i);
            cout << "function name " << func->name << endl;
            ++(++i);
            // while (chunks.at(i) != "begin") {
            //     i++; // skip over params
            // }
            cout << chunks.at(i) << endl;
            vector<string> body;
            ++(++i);


            cout << chunks.at(i) << endl;
            while (chunks.at(i).compare("}")) {
                body.push_back(chunks.at(i++));
            }
            cout << body.at(0) << " to " << body.at(body.size()-1) << endl;
            ++i;

            func->body = unique_ptr<Scope>(parse_chunks(body));
            
            scope->nodes.push_back(shared_ptr<Function>(func));
            scope->function_identifiers.push_back(shared_ptr<Function>(func));
        }

        else if (not chunks.at(i).compare("call")) {
            cout << "Found a function call of " << chunks.at(i+1) << endl;
            auto call = new Call();
            ++i;
            for (auto& ident : topscope->function_identifiers) {
                if (not ident->name.compare(chunks.at(i))) {
                    call->of_what = shared_ptr<Function>(ident);
                }
            }
            for (auto& ident : scope->function_identifiers) {
                if (not ident->name.compare(chunks.at(i))) {
                    call->of_what = shared_ptr<Function>(ident);
                }
            }
            cout << "Argument for function CALL of name" << call->of_what->what_type << endl;
            ++(++i);
            vector<shared_ptr<Node>> arglist = {};
            while (chunks.at(i).compare(")")) {
                arglist.push_back(unique_ptr<Node>(parse_expression({chunks.at(i++)}, scope))); // later, make it split by comma
            }
            ++i;
            call->args = arglist;
            cout << arglist.size() << " is the length of arglist" << endl;
            if (arglist.size() > 0) cout << arglist.at(0) << " " << call->args.at(0) << endl;
            
            scope->nodes.push_back(shared_ptr<Call>(call));
        }
        else if (not chunks.at(i).compare("asm")) {
            cout << "Found assembly block" << endl;
            [[maybe_unused]]
            auto block = new AssemblyBlock();
            
            ++i;

            if (chunks.at(i) == "{") {
                while (chunks.at(i).compare("}")) {
                    block->content.append(chunks.at(i) + " ");
                    i++;
                }
            }
            else {
                while (chunks.at(i).compare(";")) {
                    block->content.append(chunks.at(i) + " ");
                    i++;
                }
            }
            cout << block->content << endl;
            scope->nodes.push_back(shared_ptr<AssemblyBlock>(block));
        }
        else {
            cout << "Skipping over node " << chunks.at(i) << endl;
            i++;
        }
    }
    return scope;
}

string generate_single(const shared_ptr<Node>& node, const string& prefix) {
    string output = "";

    if (node->what_type == CONSTANT) {
        auto c = static_pointer_cast<Constant>(node);

        output += prefix + c->value;
    }
    else {
        cerr << "Cannot currently pass non-constant to a function" << endl;
        exit(EXIT_FAILURE);
    }

    return output;
}

string generate_assembly(Scope* scope) {
    static string section_bss = R"(section .bss
)";
    static string section_data = R"(section .data
)";
    static string section_text = R"(section .text
extern ExitProcess
global start
)";


    auto i = 0ULL;

    while (i < scope->nodes.size()) {
        switch (scope->nodes.at(i)->what_type) {
            #define node_as(what) static_pointer_cast<what>(scope->nodes.at(i++))
            case FUNCDEF:
            {
                auto f = node_as(Function);
                cout << "Found a funcdef named " << f->name << endl;

                section_text.append(
                    f->name +
                    ":\n"
                );
                generate_assembly(f->body.get());
                section_text.append("\n");
            }
                break;
            case CALL:
            {
                auto c = node_as(Call);

                for (auto& arg : c->args) {
                    section_text.append(
                        generate_single(arg, "push ") + "\n"
                    );
                }

                section_text.append(
                    "call " + c->of_what->name +
                    "\n"
                );
            }
                break;
            case ASSEMBLY:
                {
                    auto a = node_as(AssemblyBlock);

                    section_text.append("\n;user generated assembly starts here\n" + a->content + "\n;user generated assembly ends here\n");
                }
                break;
            default:
                i++;
                break;
        }
    }
    return section_bss + section_data + section_text;
}

auto main(int32_t argc, char *argv[]) -> int {
    cout << "Hello from " << LANGUAGE_NAME << "!" << endl;

    assert(argc == 3 && "Incorrect arguments! Correct call is /path/to/compiler <input file> <output assembly file>");

    cout << argv[1] << endl;

    ifstream input_file(argv[1]);
    stringstream input_stream;
    input_stream << input_file.rdbuf();
    input_file.close();

    string input_contents = input_stream.str();

    vector<string> chunks = split(input_contents);

    for (auto& chunk : chunks) {
        cout << "|" << chunk << "|" << endl;
    }

    Scope* nodes = parse_chunks(chunks, true);

    string assembly = generate_assembly(nodes);
    

    cout << "\n" << assembly << endl;

    std::ofstream out_file(argv[2]);
    out_file << assembly;
    out_file.close();

    return 2;
}

vector<string> split(const string& input) {
    auto i = 0llu;

    vector<string> output;

    while (i != input.length()) {
        string buffer = "";

        if (std ::isalpha(input.at(i))) {
           while (std ::isalpha(input.at(i))) {
              buffer += input.at(i);
              i += 1;
           }
        }

        else if (isdigit(input.at(i))) {
           while (isdigit(input.at(i))) {
              buffer += input.at(i);
              i += 1;
           }
        }

        else if (not isspace(input.at(i)))
        {
            buffer = input.at(i);
            i += 1;
        }

        else {
            i += 1;
            continue;
        }
        if (MACROS.find(buffer) != MACROS.end()) {
            buffer = MACROS.at(buffer);
        }
        output.emplace_back(buffer);
    }

    return output;
}