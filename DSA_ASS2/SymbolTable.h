#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "main.h"


inline bool isNumber(string& str)
{
    for (char const &c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

inline bool isString(string& str){
    if(str[0] != 39 || str[str.length()-1] != 39) return false;
    for(unsigned int i = 1; i < str.length() - 1;i++){
        if((str[i] >= 65 && str[i] <= 90)||(str[i] >= 97 && str[i] <= 122) || (str[i] >= 48 && str[i] <= 57) || str[i]
                                                                                                                == 32)
            continue;
        else return false;
    }
    return true;
}
inline int tokenize(string str, char delim, string out[])
{
    int size = str.length();
    int j = 0;
    int count = 0;
    for(int i = 0; i < size;i++){
        if(i == size - 1){
            out[count++] = str.substr(j, i - j + 1);
            break;
        }
        if(str[i] == delim){
            out[count++] = str.substr(j, i - j);
            j = i + 1;
        }
        if(count == 3) {
            out[count++] = str.substr(j, size - j);
            break;
        }
    }
    return count - 1;
}
inline bool valid_Instruction(string &identifier){
    if(identifier[0] < 97 || identifier[0] > 122) return false;
    for(unsigned int i = 1; i < identifier.length();i++){
        if((identifier[i] >= 65 && identifier[i] <= 90)||(identifier[i] >= 97 && identifier[i] <= 122) || identifier[i]
        == 95 || (identifier[i] >= 48 && identifier[i] <= 57) ) continue;
        else return false;
    }
    return true;
}
inline int tokenizer(string str, char delim, string* out)
{
    int count = 0;
    size_t start;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != string::npos)
    {
        end = str.find(delim, start);
        out[count++]=(str.substr(start, end - start));
    }
    return count;
}
inline bool extract_function(string func, string type_arr[], int& count){
    if(func[0] != '(') return false;
    int first = 0, second = 0;
    string temp;
    for(int i = 0; i < (int)func.length(); i++){
        if(func[i] == '(') temp.push_back('(');
        else if(func[i] == ','){
            if(temp.back() == '(' && i != 1) temp.push_back(',');
        }
        else if(func[i] == ')') {
            temp.push_back(')');
            first = i-1;
        }
        else if(func[i] == '-') temp.push_back('-');
        else if(func[i] == '>') {
            temp.push_back('>');
            second = i+1;
        }
    }
    if(temp != "(,)->" && temp != "()->") return false;
    count = tokenizer(func.substr(1, first), ',', type_arr);
    type_arr[count++] = func.substr(second, func.length() - second);
    for(int i = 0; i < count; i++)
        if (type_arr[i] != "number" && type_arr[i] != "string") return false;

    return true;

}

class SymbolTable
{
    class Node{
    public:
        string identifier_name, type, value;
        bool global;
        int scope_level;
        Node* left, *right, *parent;
        Node():left(nullptr), right(nullptr), parent(nullptr){};
        Node(string i_name, string typ, string val, int sc_level, bool glob, Node* left, Node* right, Node* parent)
        {
            this->identifier_name = std::move(i_name);
            this->type = std::move(typ);
            this->value = std::move(val);
            this->scope_level = sc_level;
            this->left = left;
            this->right = right;
            this->parent = parent;
            this->global = glob;
        }
        ~Node(){
            identifier_name.clear();
            type.clear();
            value.clear();
            scope_level = 0;
            left = right = parent =  nullptr;
        }
    };  // Splay Node
    Node *root;

    void rotR(Node *p)
    {
        Node* pt = p->left;
        p->left = pt->right;
        if (pt->right) pt->right->parent = p;
        pt->parent = p->parent;
        if (!p->parent) root = pt;
        else if (p == p->parent->right) p->parent->right = pt;
        else p->parent->left = pt;
        pt->right = p;
        p->parent = pt;
    }
    void rotL(Node *p)
    {
        Node* pt = p->right;
        p->right = pt->left;
        if (pt->left) pt->left->parent = p;
        pt->parent = p->parent;
        if (!p->parent) root = pt;
        else if (p == p->parent->left) p->parent->left = pt;
        else p->parent->right = pt;
        pt->left = p;
        p->parent = pt;
    }
    int splay(Node* p){
        int count = 0;
        while(p->parent){
            if(!p->parent->parent){
                if(p == p->parent->left) rotR(p->parent);   // ZIG
                else rotL(p->parent);  // ZAG
            }
            else if(p == p->parent->left && p->parent == p->parent->parent->left){ // ZIG-ZIG
                rotR(p->parent->parent);
                rotR(p->parent);
            }
            else if(p == p->parent->right && p->parent == p->parent->parent->right){ // ZAG-ZAG
                rotL(p->parent->parent);
                rotL(p->parent);
            }
            else if(p == p->parent->right && p->parent == p->parent->parent->left){ // ZIG-ZAG
                rotL(p->parent);
                rotR(p->parent);
            }
            else{                                                                   // ZAG-ZIG
                rotR(p->parent);
                rotL(p->parent);
            }
            count++;
        }
        return (count > 0)? 1 : 0; 
    }
public:
    SymbolTable(): root(nullptr){};
    ~SymbolTable(){
        clear();
    }
    class container{
    public:
        Node** arr;
        int cap;
        int count;
    public:
        container(){
            cap = 0;
            count = 0;
            arr = nullptr;
        }
        container(int c){
            cap = c;
            count = 0;
            arr = new Node*[c];
        }
        ~container(){
            delete [] arr;
            cap = 0;
            count = 0;
        }
        void addlast(Node* temp){
            if(count == cap) return;
            else arr[count++] = temp;
        }
        int peek(int highest){
            for (int i = 0; i < count; i++) {
                if(highest == arr[i]->scope_level)
                    return i;
            }
            return -1;
        }
        void deletehighest(int sc_level){
            if(count == 0) return;
            else {
                int idx = peek(sc_level);
                if(idx == -1) return;
                for(int i = idx;i < count - 1; i++)
                    arr[i] = arr[i + 1];
                count--;
            }
        }
        bool empty(){return !count;}
    };
    bool insert(int sc_level, string identifier, string type, bool global, int& num_comp, int& num_splay, container &c){
        Node* newNode = new Node(identifier, type, "", sc_level, global, nullptr, nullptr, nullptr);
        if(global) newNode->scope_level = 0;
        Node* temp = nullptr;
        Node* pt = this->root;
        bool dummy = false;  // true: left, false: right
        while (pt != nullptr) {
            temp = pt;
            if (newNode->scope_level < pt->scope_level) {
                pt = pt->left;
                dummy = true;
            }
            else if(newNode->scope_level > pt->scope_level) {
                pt = pt->right;
                dummy = false;
            }
            else {
                int dump = newNode->identifier_name.compare(pt->identifier_name);
                if(dump == 0) {
                    delete newNode;
                    return false;
                }
                else if(dump > 0) {
                    pt = pt->right;
                    dummy = false;
                }
                else {
                    pt = pt->left;
                    dummy = true;
                }
            }
            num_comp++;
        }
        // temp is parent of newNode
        newNode->parent = temp;
        if (!temp) root = newNode;
        else if (dummy) temp->left = newNode;
        else temp->right = newNode;
        c.addlast(newNode);
        num_splay += splay(newNode);
        return true;
    }
    Node* search_helper(int sc_level, string identifier, int& num_comp, Node* p)
    {
        if(!p) return p;
        else if(sc_level > p->scope_level) {
            num_comp++;
            return search_helper(sc_level, identifier, num_comp, p->right);
        }
        else if(sc_level < p->scope_level) {
            num_comp++;
            return search_helper(sc_level, identifier, num_comp, p->left);
        }
        else{
            num_comp++;
            int dump = identifier.compare(p->identifier_name);
            if(dump == 0) return p;
            else if(dump > 0) return search_helper(sc_level, identifier, num_comp,  p->right);
            else return search_helper(sc_level, identifier, num_comp, p->left);
        }
    }

    int assign(int sc_level, string identifier, string type, string value, int& num_comp, int& num_splay){
        int temp_sc_level = sc_level;
        Node *p = nullptr;
        while(temp_sc_level > -1) {
           int temp_num_comp = num_comp;
           p = search_helper(temp_sc_level, identifier, temp_num_comp, root);
           if(p) {
               num_comp = temp_num_comp;
               break;
           }
           temp_sc_level--;
        }
        if(!p) return -1;   // Undeclared
        num_splay += splay(p);
        if(p->type == "string" || p->type == "number"){
            if(p->type != type) {
                if(type != "string" && type != "number"){
                    string type_arr[10];
                    int count = 0;
                    extract_function(type, type_arr, count);
                    if(p->type == type_arr[count-1]) {
                        p->value = std::move(value);
                        return 1;          // successful
                    }
                    else return 0;         // type mismatch
                }
                else return 0;
            }
            else return 1;
        }
        else return 0;
    }
    Node* minimum(Node* node) {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    // find the node with the maximum key
    Node* maximum(Node* node) {
        while (node->right != nullptr) {
            node = node->right;
        }
        return node;
    }

    Node* join(Node* s, Node* t){
        if (!s) return t;

        if (!t) return s;
        Node* x = maximum(s);
        splay(x);
        x->right = t;
        t->parent = x;
        return x;
    }

    // splits the tree into s and t
    void split(Node* &x, Node* &s, Node* &t) {
        splay(x);
        if (x->right) {
            t = x->right;
            t->parent = nullptr;
        } else {
            t = nullptr;
        }
        s = x;
        s->right = nullptr;
        x = nullptr;
    }

    bool deleteNodeHelper(Node* node, int sc_level, container &c) {
        Node* x = nullptr;
        Node* t, *s;
        int idx = c.peek(sc_level);
        if(idx == -1) return false;
        x = c.arr[idx];
        c.deletehighest(sc_level);
        if (x == nullptr) return false;
        split(x, s, t); // split the tree
        if (s->left){ // remove x
            s->left->parent = nullptr;
        }
        root = join(s->left, t);
        delete(s);
        s = nullptr;
        return true;
    }

    void deleteRecord(int sc_level, container &c){
        while(deleteNodeHelper(root, sc_level, c));
    }
    void LNR(string identifier, Node* p, int& sc_level, Node*& temp){
        if(!p) return;
        LNR(identifier, p->left, sc_level, temp);
        if(identifier == p->identifier_name)
        {
            sc_level = p->scope_level;
            temp = p;
        }
        LNR(identifier, p->right, sc_level, temp);
    }
    int lookUp(string identifier){
        int sc_level = -1;
        Node* temp = nullptr;
        LNR(identifier, root, sc_level, temp);
        if(temp) splay(temp);
        return sc_level;
    }

    void preOrderHelper(Node* node, string& s) {
        if (node != nullptr) {
            s = s + node->identifier_name + "//" + to_string(node->scope_level)+" ";
            preOrderHelper(node->left, s);
            preOrderHelper(node->right, s);
        }
    }
    void print(){
        string s;
        preOrderHelper(root, s);
        if(!s.empty()) s.pop_back();
        else return;
        cout<<s<<endl;
    }
    void printHelper(Node* root, string indent, bool last) {
        // print the tree structure on the screen
        if (root != nullptr) {
            cout<<indent;
            if (last) {
                cout<<"└────";
                indent += "     ";
            } else {
                cout<<"├────";
                indent += "|    ";
            }

            cout<<root->identifier_name<<"//"<<root->scope_level<<endl;

            printHelper(root->left, indent, false);
            printHelper(root->right, indent, true);
        }
    }
    void prettyPrint() {
        printHelper(this->root, "", true);
    }
    string extract_LHS(string value, string type_arr[], int& count, string& id, int& num_comp, int& num_splay, int sc_level){
        string temp;
        if(value.back() != ')') return "0";
        int first = 0;
        for(int i = 0; i < (int)value.length(); i++){
            if(value[i] == '(') {
                temp.push_back('(');
                first = i + 1;
            }
            else if(value[i] == ','){
                if(temp.back() == '(' && i != 1) temp.push_back(',');
            }
            else if(value[i] == ')') temp.push_back(')');
        }
        if(temp != "(,)" && temp != "()") return "0";
        count = tokenizer(value.substr(first, value.length() - first - 1), ',', type_arr);
        id = value.substr(0, first - 1);
        string other_res =  identifier_assignment( id, num_comp, num_splay, sc_level);
        if(other_res == "2") return "2";
        string type_arr1[10];
        int count1 = 0;
        extract_function(other_res, type_arr1, count1);
        for(int i = 0; i < count; i++){
            if(!isString(type_arr[i]) && !isNumber(type_arr[i])){
                string res = prev_assignment(type_arr[i], num_comp, num_splay, sc_level);
                if(res == "0") return "1";
                if(res == "-1") return "2";
                type_arr[i] = res;
                if(type_arr1[i] != res) return "1";
            }
            else if(isString(type_arr[i])) {
                type_arr[i] = "string";
                if(type_arr1[i] != "string") return "1";
            }
            else if(isNumber(type_arr[i])) {
                type_arr[i] = "number";
                if(type_arr1[i] != "number") return "1";
            }
        }
        return other_res;
    }

    string prev_assignment(string value, int& num_comp, int& num_splay, int sc_level){
        int temp_sc_level = sc_level;
        Node* temp = nullptr;
        while(temp_sc_level > -1) {
            int temp_num_comp = num_comp;
            temp = search_helper(temp_sc_level, value, temp_num_comp, root);
            if(temp) {
                if(temp->type != "string" && temp->type != "number") return "0";
                num_splay += splay(temp);
                num_comp = temp_num_comp;
                break;
            }
            temp_sc_level--;
        }
        if(!temp) return "-1";
        else return temp->type;
    }
    string identifier_assignment(string id, int& num_comp, int& num_splay, int sc_level){
        int temp_sc_level = sc_level;
        Node* temp = nullptr;
        while(temp_sc_level > -1) {
            int temp_num_comp = num_comp;
            temp = search_helper(temp_sc_level, id, temp_num_comp, root);
            if(temp) {
                num_splay += splay(temp);
                num_comp = temp_num_comp;
                break;
            }
            temp_sc_level--;
        }
        if(!temp) return "2";
        else return temp->type;
    }

    string value_type(string value, string type_arr[], int& count, int sc_level, int& num_comp, int& num_splay){
        if(isString(value)) return "string";
        else if(isNumber(value)) return "number";
        else if(value.back()==')'){
            string id;
            string res = extract_LHS(value, type_arr, count, id, num_comp, num_splay, sc_level);
            if(res == "0") return "-1";                       // invalid instruction
            if(res == "2") return "2";                       // undeclared
            if(res == "1") return "1";                      // type mismatch
            return res;
        }
        else if(value.back() != ')'){
            string result = prev_assignment(value, num_comp, num_splay, sc_level);   // int y = x;
            if(result == "-1") return "2";
            else if(result == "0") return "1";
            else return result;
        }
        return "-1";
    }
    void deleteTree(Node* node)
    {
        if (node == nullptr) return;
        /* first delete both subtrees */
        deleteTree(node->left);
        deleteTree(node->right);
        /* then delete the node */
        delete node;
    }
    void clear(){
        deleteTree(root);
    }


    void run(string filename);
};
#endif