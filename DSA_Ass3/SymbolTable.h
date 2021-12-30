#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "main.h"

inline long long int encoder(string id, int sc_level){
    int len = int(id.length());
    int tmp = -99;
    string result;
    result += to_string(sc_level);
    for(int i = 0; i < len; i++){
        tmp = id[i] - 48;
        result += to_string(tmp);
    }
    return stoll(result);
}

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

inline int tokenizer(string str, char delim, string* out)
{
    int count = 0;
    size_t start;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != string::npos)
    {
        end = str.find(delim, start);
        out[count++]=(str.substr(start, end - start));
        if(out[0] == "ASSIGN" && count == 2){
            out[count++] = str.substr(end + 1, str.length() - end);
            break;
        }
        else if(out[0] == "CALL" && count == 1){
            out[count++] = str.substr(end + 1, str.length() - end);
            break;
        }
    }
    return count;
}

// return false: void function
// return true : non-void function
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
    if(temp != "(,)->" && temp != "()->" && temp != "()" && temp != "(,)") return false;
    count = tokenizer(func.substr(1, first), ',', type_arr);
    if(temp == "()" || temp == "(,)") return false;
    type_arr[count++] = func.substr(second, func.length() - second);
    for(int i = 0; i < count; i++)
        if (type_arr[i] != "number" && type_arr[i] != "string") return false;
    return true;
}


class SymbolTable
{
    class Value{
    public:
        string id;
        int sc_level;
        int num_param;
        string type;
        string value;
        Value():id(""), sc_level(0), num_param(-1), type(""), value(""){};
        Value(string id, int sc_lv, int num_param, string typ, string val):id(id), sc_level(sc_lv),
        num_param(num_param), type(typ), value(val){};
        void clear(){
            this->id = "";
            this->sc_level = 0;
            this->num_param = -1;
            this->type = "";
            this->value = "";
        }
    };
    Value    *table;
    int size;
    int c, c2;
    string method;
public:
    SymbolTable() {
        table = nullptr;
        size = 0;
        c = 0;
        c2 = 0;
        method = "";
    }
    ~SymbolTable(){
        delete [] table;
        size = 0;
        c = 0;
        c2 = 0;
        method = "";
    }
    /*
     * return code:
     * -2 : overflow
     * -1 : redeclared
     * >= 0 : OK
     */
    int insert(string id, int sc_level, int num_param){
        long long int key = encoder(id, sc_level);
        int num_pass = 0;
        if(table[key % size].id == id && table[key % size].sc_level == sc_level) {
            if(num_param != -1) throw InvalidDeclaration(id);
            else return -1;
        }
        if(num_param != -1 && sc_level != 0) throw InvalidDeclaration(id);
        if(method == "LINEAR"){
            int hkey = key % this->size;
            int i = 0;
            int hp = hkey;
            while(!table[hp].id.empty() && i < this->size){
                hp = (hkey + this->c * i) % this->size;
                i++;
            }
            if(i == this->size) return -2;
            table[hp] = Value(id, sc_level, num_param, "", "");
            num_pass = (i == 0)? 0 : i - 1;
        }
        else if(method == "QUADRATIC"){
            int hkey = key % this->size;
            int i = 0;
            int hp = hkey;
            while(!table[hp].id.empty() && i < this->size){
                hp = (hkey + this->c * i + this->c2 * i * i) % this->size;
                i++;
            }
            if(i == this->size) return -2;
            table[hp] = Value(id, sc_level, num_param, "", "");
            num_pass = (i == 0)? 0 : i - 1;
        }
        else if(method == "DOUBLE"){
            int hkey1 = key % this->size;
            int hkey2 = 1 + (key % (this->size - 2));
            int i = 0;
            int hp = hkey1;
            while(!table[hp].id.empty() && i < this->size){
                hp = (hkey1 + this->c * i * hkey2) % this->size;
                i++;
            }
            if(i == this->size) return -2;
            table[hp] = Value(id, sc_level, num_param, "", "");
            num_pass = (i == 0)? 0 : i - 1;
        }
        return num_pass;
    }


    /*
     * return type of the previous identifier
     * if not found, return -1
     */
    int declared_id(string id, int sc_level, int& num_pass){
        long long int key = encoder(id, sc_level);
        if(method == "LINEAR"){
            int hkey = key % this->size;
            int i = 0;
            int hp = hkey;
            while(table[hp].id != id && i < this->size){
                hp = (hkey + this->c * i) % this->size;
                i++;
            }
            if(i == this->size) return -1;
            num_pass += (i == 0)? 0 : i - 1;
            return hp;
        }
        else if(method == "QUADRATIC"){
            int hkey = key % this->size;
            int i = 0;
            int hp = hkey;
            while(table[hp].id != id && i < this->size){
                hp = (hkey + this->c * i + this->c2 * i * i) % this->size;
                i++;
            }
            if(i == this->size) return -1;
            num_pass += (i == 0)? 0 : i - 1;
            return hp;
        }
        else if(method == "DOUBLE"){
            int hkey1 = key % this->size;
            int hkey2 = 1 + (key % (this->size - 2));
            int i = 0;
            int hp = hkey1;
            while(table[hp].id != id && i < this->size){
                hp = (hkey1 + this->c * i * hkey2) % this->size;
                i++;
            }
            if(i == this->size) return -1;
            num_pass += (i == 0)? 0 : i - 1;
            return hp;
        }
        return -1;
    }

    /*
     * -1: invalid
     * -2: undeclared
     * -3: mismatch
     * -4: type can not be inferred
     */
    int extract_RHS(string ID, string value, int& num_pass, int sc_level){
        string temp;
        string type_arr[10];
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
        if(temp != "(,)" && temp != "()") return -1; // invalid
        int count = tokenizer(value.substr(first, value.length() - first - 1), ',', type_arr);
        string id = value.substr(0, first - 1);
        int pos = -1;
        int test = sc_level;
        int dummy;
        while(test >= 0){
            dummy = num_pass;
            pos = declared_id(id, test, dummy);
            if(pos != -1) {
                if(test == 0 || table[pos].num_param == -1)
                    break;
            }
            test--;
        }
        num_pass = dummy;
        if(pos == -1) throw Undeclared(id);                   // undeclared
        if(table[pos].num_param == -1)             // mismatch
            return -3;
        else if(table[pos].type == ""){
            table[pos].type += "(";
            for(int i = 0; i < count; i++){
                if(table[pos].type.size() != 1) table[pos].type += ",";
                if(!isString(type_arr[i]) && !isNumber(type_arr[i])){
                    int tmp = sc_level;
                    int param_pos;
                    int dump;
                    while(tmp >= 0) {
                        dump = num_pass;
                        param_pos = declared_id(type_arr[i], tmp, dump);
                        if(param_pos != -1) break;
                        tmp--;
                    }
                    num_pass = dump;
                    if(param_pos == -1) throw Undeclared(type_arr[i]);    // undeclared
                    if(table[param_pos].num_param != -1) return -3;    // mismatch
                    if(table[param_pos].type == "") return -4;
                    type_arr[i] = table[param_pos].type;
                    table[pos].type += type_arr[i];
                }
                else if(isString(type_arr[i])) {
                    type_arr[i] = "string";
                    table[pos].type += "string";
                }
                else if(isNumber(type_arr[i])) {
                    type_arr[i] = "number";
                    table[pos].type += "number";
                }
            }
            if(table[pos].num_param != count) return -3;
            table[pos].type += ")->";
            int tmp = sc_level;
            int ID_pos;
            int dump;
            while(tmp >= 0) {
                dump = num_pass;
                ID_pos = declared_id(ID, tmp, dump);
                if(ID_pos != -1) break;
                tmp--;
            }
            num_pass = dump;
            if(ID_pos == -1) throw Undeclared(ID);
            if(table[ID_pos].num_param != -1) return -3; // mismatch
            if(table[ID_pos].type == "") return -4; // TypeCannotBeInferred
            table[pos].type += table[ID_pos].type;
            return ID_pos;
        }
        else {
            string type_arr1[10];
            int count1 = 0;
            bool non_void = extract_function(table[pos].type, type_arr1, count1);
            for (int i = 0; i < min(count, count1); i++) {
                if (!isString(type_arr[i]) && !isNumber(type_arr[i])) {
                    int tmp = sc_level;
                    int param_pos;
                    int dump;
                    while(tmp >= 0) {
                        dump = num_pass;
                        param_pos = declared_id(type_arr[i], tmp, dump);
                        if(param_pos != -1) break;
                        tmp--;
                    }
                    num_pass = dump;
                    if (param_pos == -1) throw Undeclared(type_arr[i]);    // undeclared
                    if (table[param_pos].num_param != -1) return -3;
                    if(table[param_pos].type == "") table[param_pos].type = type_arr1[i];
                    else if(table[param_pos].type != type_arr1[i]) return -3;
                    type_arr[i] = table[param_pos].type;
                } else if (isString(type_arr[i])) {
                    type_arr[i] = "string";
                    if (type_arr1[i] != "string") return -3;
                } else if (isNumber(type_arr[i])) {
                    type_arr[i] = "number";
                    if (type_arr1[i] != "number") return -3;
                }
            }
            if(count + 1 != count1 && non_void) return -3;
            if(!non_void) return -3;
            int tmp = sc_level;
            int ID_pos;
            int dump;
            while(tmp >= 0) {
                dump = num_pass;
                ID_pos = declared_id(ID, tmp, num_pass);
                if(ID_pos != -1) break;
                tmp--;
            }
            num_pass = dump;
            if(ID_pos == -1) throw Undeclared(ID);
            if(table[ID_pos].num_param != -1) return -3; // mismatch

            if(table[ID_pos].type == "") table[ID_pos].type = type_arr1[count1-1];
            else if(table[ID_pos].type != type_arr1[count1 - 1]) return -3;
            return ID_pos;
        }
        return -1;
    }
    int assignment_processing(string ID, string value, int sc_level,int& num_pass){
        if(isNumber(value)) {
            int tmp = sc_level;
            int ID_pos;
            int dump;
            while(tmp >= 0) {
                dump = num_pass;
                ID_pos = declared_id(ID, tmp, dump);
                if(ID_pos != -1) break;
                tmp--;
            }
            num_pass = dump;
            if(ID_pos == -1) throw Undeclared(ID);
            if(table[ID_pos].num_param != -1) return -3;
            if(table[ID_pos].type == "") table[ID_pos].type = "number";
            return ID_pos;
        }
        else if(isString(value)) {
            int tmp = sc_level;
            int ID_pos;
            int dump;
            while(tmp >= 0) {
                dump = num_pass;
                ID_pos = declared_id(ID, tmp, dump);
                if(ID_pos != -1) break;
                tmp--;
            }
            num_pass = dump;
            if(ID_pos == -1) throw Undeclared(ID);
            if(table[ID_pos].num_param != -1) return -3;
            if(table[ID_pos].type == "") table[ID_pos].type = "string";
            return ID_pos;
        }
        else if(value.back() == ')'){
            int res = extract_RHS(ID, value, num_pass, sc_level);
            if(res == -1) return -1;                       // invalid instruction
            //if(res == -2) return -2;                       // undeclared
            if(res == -3) return -3;                      // type mismatch
            if(res == -4) return -4;                      // TypeCannotBeInferred
            return res;
        }
        else if(value.back() != ')'){
            int tmp = sc_level;
            int rightside_pos;
            int dump;
            while(tmp >= 0) {  // y = x
                dump = num_pass;
                rightside_pos = declared_id(value, tmp, dump);
                if(rightside_pos != -1 && tmp != 0 && table[rightside_pos].num_param == -1 && table[rightside_pos].sc_level
                == tmp) break;
                tmp--;
            }
            num_pass = dump;
            if(rightside_pos == -1) throw Undeclared(value);                  // undeclared
            else{
                if(table[rightside_pos].num_param != -1) return -3;  // mismatch
                int temp = sc_level;
                int leftside_pos;
                int dummy;
                while(temp >= 0) {
                    dummy = num_pass;
                    leftside_pos = declared_id(ID, temp, dummy);
                    if(leftside_pos != -1 && temp != 0 && table[leftside_pos].num_param == -1 && table[leftside_pos].sc_level
                    == temp) break;
                    temp--;
                }
                num_pass = dummy;
                if(leftside_pos == -1) throw Undeclared(ID);                  // undeclared
                if(table[leftside_pos].num_param != -1) return -3; // mismatch
                if(table[leftside_pos].type == ""){
                    if(table[rightside_pos].type == "") return -4; // typecantbeinferred
                    else table[leftside_pos].type = table[rightside_pos].type;
                }
                else{
                    if(table[rightside_pos].type == "")
                        table[rightside_pos].type = table[leftside_pos].type;
                    else{
                        if(table[leftside_pos].type != table[rightside_pos].type)
                            return -3;                           // mismatch
                    }
                }
                return leftside_pos;
            }
        }
        return -1;
    }
    int extract_exp(string value ,int& num_pass, int sc_level){
        string temp;
        string type_arr[10];
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
        if(temp != "(,)" && temp != "()") return -1; // invalid
        int count = tokenizer(value.substr(first, value.length() - first - 1), ',', type_arr);
        string id = value.substr(0, first - 1);
        int pos = -1;
        int test = sc_level;
        int dummy;
        while(test >= 0){
            dummy = num_pass;
            pos = declared_id(id, test, dummy);
            if(pos != -1) {
                if(test == 0 || table[pos].num_param == -1)
                    break;
            }
            test--;
        }
        num_pass = dummy;
        if(pos == -1) throw Undeclared(id);                   // undeclared
        if(table[pos].num_param == -1)            // mismatch
            return -3;
        else if(table[pos].type == ""){
            table[pos].type += "(";
            for(int i = 0; i < count; i++){
                if(table[pos].type.size() != 1) table[pos].type += ",";
                if(!isString(type_arr[i]) && !isNumber(type_arr[i])){
                    int tmp = sc_level;
                    int param_pos;
                    int dummy;
                    while(tmp >= 0) {
                        dummy = num_pass;
                        param_pos = declared_id(type_arr[i], tmp, dummy);
                        if(param_pos != -1 && table[param_pos].sc_level == tmp) break;
                        tmp--;
                    }
                    num_pass = dummy;
                    if(param_pos == -1) throw Undeclared(type_arr[i]);    // undeclared
                    if(table[param_pos].num_param != -1) return -3;    //mismatch
                    if(table[param_pos].type == "") return -4;
                    type_arr[i] = table[param_pos].type;
                    table[pos].type += type_arr[i];
                }
                else if(isString(type_arr[i])) {
                    type_arr[i] = "string";
                    table[pos].type += "string";
                }
                else if(isNumber(type_arr[i])) {
                    type_arr[i] = "number";
                    table[pos].type += "number";
                }
            }
            if(table[pos].num_param != count) return -3;
            table[pos].type += ")";
            return pos;
        }
        else {
            string type_arr1[10];
            int count1 = 0;
            extract_function(table[pos].type, type_arr1, count1);
            for (int i = 0; i < min(count, count1); i++) {
                if (!isString(type_arr[i]) && !isNumber(type_arr[i])) {
                    int tmp = sc_level;
                    int param_pos;
                    int dummy;
                    while(tmp >= 0) {
                        dummy = num_pass;
                        param_pos = declared_id(type_arr[i], tmp, dummy);
                        if(param_pos != -1 && table[param_pos].sc_level == tmp) break;
                        tmp--;
                    }
                    num_pass = dummy;
                    if (param_pos == -1) throw Undeclared(type_arr[i]);    // undeclared
                    if (table[param_pos].num_param != -1) return -3; // mismatch
                    if(table[param_pos].type == "") table[param_pos].type = type_arr1[i];
                    else if(table[param_pos].type != type_arr1[i]) return -3;   // mismatch
                    type_arr[i] = table[param_pos].type;
                } else if (isString(type_arr[i])) {
                    type_arr[i] = "string";
                    if (type_arr1[i] != "string") return -3;   // mismatch
                } else if (isNumber(type_arr[i])) {
                    type_arr[i] = "number";
                    if (type_arr1[i] != "number") return -3;   // mismatch
                }
            }
            if(count != count1) return -3;             // mismatch
            return pos;
        }
    }
    int exp_processing(string value, int sc_level,int& num_pass){
        if(value.back() == ')'){
            int res = extract_exp(value, num_pass, sc_level);
            if(res == -1) return -1;                       // invalid instruction
            //if(res == -2) return -2;                       // undeclared
            if(res == -3) return -3;                      // type mismatch
            if(res == -4) return -4;                      // TypeCannotBeInferred
            return res;
        }
        else return -1;
    }




    void assign(int position, int sc_level, string value){
        table[position].value = value;
    }

    void deleteRecord(int sc_level){
        for(int i = 0; i < this->size; i++){
            if(table[i].id != "" && table[i].sc_level == sc_level)
                table[i].clear();
        }
    }

    int lookup(string id, int sc_level) {
        int tmp = sc_level;
        while (tmp >= 0) {
            for (int i = 0; i < this->size; i++){
                if(table[i].id == id && table[i].sc_level == tmp) return i;
            }
            tmp--;
        }
        return -1;
    }
    string print(){
        string s;
        for(int i = 0; i < this->size; i++){
            if(table[i].id == "") continue;
            s += to_string(i) + " " + table[i].id + "//" + to_string(table[i].sc_level) + ";";
        }
        if(s != "") s.pop_back();
        return s;
    }
    void printTable(){
        for(int i = 0; i < this->size; i++){
            if(i%5 == 0) cout <<"\n----------------------------------- \n";
            cout << table[i].id <<"//"<<table[i].sc_level<<"   ";
        }
    }
    void run(string filename);
};
#endif