#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>

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
    ASSEMBLY,



    REGISTER
};

vector<char> Operators {
    '+',
    '-',
    '*',
    '/'
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
    string length;
    string value;
};

struct Named : Node {
    string name;
};

struct VarDef : Named {
    VarDef() {
        what_type = VARDEF;
    }
    string length;
    bool is_static;
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
    shared_ptr<Node> left;
    shared_ptr<Node> right;
    char operator_symbol;
};

struct Assignment : Node {
    Assignment() {
        what_type = ASSIGNMENT;
    }
    shared_ptr<VarRef> to_what;
    shared_ptr<Operation> value;
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
    inline static Scope *top_scope;
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

    for (auto& var : Scope::top_scope->identifiers) {
        if (var->name == val) {
            return var;
        }
    }
    for (auto& var : Scope::top_scope->function_identifiers) {
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
            n->length = "8";
            n->value = chunks.at(0);
            return n;
        }
    }
    else {
        for (auto& op : Operators) {
            for (size_t i = 0; i < chunks.size(); i++) {
                auto& c = chunks.at(i);
                if (c.at(0) == op) {
                    vector<string> left, right;
                    for (auto& chunk : chunks) {
                        if (chunk == c) {
                            break;
                        }
                        left.push_back(chunk);
                    }
                    for (size_t j = i+1; j < chunks.size(); j++) {
                        auto& chunk = chunks.at(j);
                        right.push_back(chunk);
                    }

                    Operation * oper = new Operation();
                    oper->operator_symbol = op;
                    oper->left = shared_ptr<Node>(parse_expression(left, scope));
                    oper->right = shared_ptr<Node>(parse_expression(right, scope));

                    return oper;
                }
            }
        }
    }
    
        cerr << "Expression does not fit any defined type. Chunk at beginning is " << chunks.at(0) << endl;
        exit(EXIT_FAILURE);
}


Scope *parse_chunks(const vector<string>& chunks, bool is_top_scope = false) {
    auto i = 0ULL;

    Scope *scope = new Scope();
    if (is_top_scope) {
        Scope::top_scope = scope;
        scope->function_identifiers.emplace_back(
            new Function()
        );
        scope->function_identifiers.at(0)->name = "ExitProcess";
        // scope->function_identifiers.at(0)->args = 
    }

    while (i < chunks.size()) {
        if (not chunks.at(i).compare("let")) {
            auto def = new VarDef();
            def->length = "8"; // FIXED FOR NOW CHANGE LATER
            def->is_static = false;
            def->name = chunks.at(++i);
            scope->identifiers.push_back(shared_ptr<VarDef>(def));
            scope->nodes.push_back(shared_ptr<VarDef>(def));
            if (chunks.at(++i).compare(";")) {
                cout << "Assignment to " << def->name << endl;
                auto ass = new Assignment();
                ass->to_what = make_shared<VarRef>();
                ass->to_what->to_what = shared_ptr<VarDef>(scope->identifiers.at(scope->identifiers.size() -1));

                vector<string> expr_chunks;

                while (chunks.at(i).compare(";")) {
                    expr_chunks.push_back(chunks.at(i++));
                }

                ass->value = static_pointer_cast<Operation>(shared_ptr<Node>(parse_expression(expr_chunks, scope)));

                scope->nodes.push_back(shared_ptr<Assignment>(ass));
            }
        }
        else if (not chunks.at(i).compare("static")) {
            auto def = new VarDef();
            def->length = "8"; // CHANGE LATER TO NON FIXED
            def->is_static = true;
            def->name = chunks.at(++i);
            Scope::top_scope->identifiers.push_back(shared_ptr<VarDef>(def));
            scope->nodes.push_back(shared_ptr<VarDef>(def));
            if (chunks.at(++i).compare(";")) {
                cout << "Assignment to " << def->name << endl;
                auto ass = new Assignment();
                ass->to_what = make_shared<VarRef>();
                ass->to_what->to_what = shared_ptr<VarDef>(Scope::top_scope->identifiers.at(Scope::top_scope->identifiers.size() -1));


                vector<string> expr_chunks;

                while (chunks.at(i).compare(";")) {
                    expr_chunks.push_back(chunks.at(i++));
                }
                for (auto& ch : expr_chunks) cout << "chunk of expr" << ch << endl;

                ass->value = static_pointer_cast<Operation>(shared_ptr<Node>(parse_expression(expr_chunks, scope)));

                scope->nodes.push_back(shared_ptr<Assignment>(ass));
            }
        }
        else if (not chunks.at(i).compare("func")) {
            cout << "Found a function" << endl;
            auto func = new Function();
            ++i;
            func->name = chunks.at(i);
            cout << "function name " << func->name << endl;
            i += 2;
            // while (chunks.at(i) != "begin") {
            //     i++; // skip over params
            // }
            cout << chunks.at(i) << endl;
            vector<string> body;
            i += 2;


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
            for (auto& ident : Scope::top_scope->function_identifiers) {
                if (not ident->name.compare(chunks.at(i))) {
                    call->of_what = shared_ptr<Function>(ident);
                }
            }
            for (auto& ident : scope->function_identifiers) {
                if (not ident->name.compare(chunks.at(i))) {
                    call->of_what = shared_ptr<Function>(ident);
                }
            }
            cout << "Argument for function CALL of type " << call->of_what->what_type << endl;
            ++(++i);
            vector<shared_ptr<Node>> arglist = {};
            while (chunks.at(i).compare(")")) {
                vector<string> exp;
                while (chunks.at(i).compare(",") and chunks.at(i).compare(")")) {
                    exp.push_back(chunks.at(i++));
                }
                arglist.push_back(unique_ptr<Node>(parse_expression(exp, scope))); // making it split by comma now // later, make it split by comma
                if (chunks.at(i) == ",") ++i;
            }
            ++i;
            call->args = arglist;
            cout << arglist.size() << " is the length of arglist" << endl;
            
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

    switch (node->what_type) {
        case CONSTANT : {
            auto c = static_pointer_cast<Constant>(node);
            output += prefix + " " + c->value;
            break;
        }
        case VARREF : {
            auto v = static_pointer_cast<VarRef>(node);
            if (v->to_what->is_static) {
                output += prefix + " " + "[" + v->to_what->name + "]";
            }
                break;
        }
        default :
            cerr << "Cannot currently pass non-constant, non-variable to a function" << endl;
            exit(EXIT_FAILURE);
            break;
    }
    return output;
}

struct Register : Named {
    Register() {
        what_type = REGISTER;
    }
};

string generate_operation(shared_ptr<Node> node, shared_ptr<Node> target) {
    string result = "";

    switch (node->what_type) {
        case OPERATION: {
            auto o = static_pointer_cast<Operation>(node);

            auto l = o->left;
            auto r = o->right;



            string to = "";
            switch (target->what_type) {
                case REGISTER:{
                        auto r = static_pointer_cast<Register>(node);

                        to = r->name;
                    }
                    break;
                case VARREF:{
                        auto r = static_pointer_cast<VarRef>(node);

                        to = r->to_what->is_static ? r->to_what->name : r->to_what->name; // URGENT REVISIT : make nonstatics work
                    }
                    break;
                default:
                    cerr << "Should not be attempting to assign to a non-register non-variable value!" << endl;
                    break;
            }
            result = "mov " + to + ", " + "";

        }
            break;
        case VARREF:
        case CONSTANT:
            result = generate_single(target, "mov ") + generate_single(node, ", ");
            //cout << "single of val " << result << endl; // IMMEDIATE REVISIT 
            break;
        default:
            cout << "Operation is failed! " << target->what_type << " " <<  node->what_type << endl;
            break;
    }

    return result;
}

string generate_assembly(Scope* scope) {
    static string section_bss = R"(DEFAULT REL
section .bss
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
            #define inc_cast(what) static_pointer_cast<what>(scope->nodes.at(i++))
            case FUNCDEF:
            {
                auto f = inc_cast(Function);
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
                auto c = inc_cast(Call);

                for (auto& arg : c->args) {
                    section_text.append(
                        generate_single(arg, "push") + "\n"
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
                    auto a = inc_cast(AssemblyBlock);

                    section_text.append(a->content + "\n");
                }
                break;
            case ASSIGNMENT:
                {
                    auto node = scope->nodes.at(i++);
                    auto as_ass = static_pointer_cast<Assignment>(node);
                    auto to_what = static_pointer_cast<Node>(as_ass->to_what);

                    string oper = generate_operation(
                            as_ass->value,
                            to_what
                        ) + "\n";

                    cout << oper << endl;

                    section_text.append(
                        oper
                    );

                }
                break;
            case VARDEF:
                {
                    auto v = inc_cast(VarDef);
                    if (v->is_static) {
                        section_bss.append(v->name + " resb " + v->length + "\n");
                    }
                    else {
                        cout << "nonstatic variables unimplemented!!!" << endl;
                    }
                }
                break;
            default:
                i++;
                break;
        }
    }
    return section_bss + section_data + section_text;
}

int main(int32_t argc, char *argv[]) {
    // cout << "Hello from " << LANGUAGE_NAME << "!" << endl;

    // assert(argc == 3 && "Incorrect arguments! Correct call is /path/to/compiler <input file> <output assembly file>");

    string def_in = "..\\z_meow\\test.meow";
    string def_out = "..\\z_meow\\test.asm";

    // cout << argv[1] << endl;

    ifstream input_file(argc == 3 ? argv[1] : def_in);
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

    std::ofstream out_file(argc == 3 ? argv[2] : def_out);
    out_file << assembly;
    out_file.close();

    return 0;
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