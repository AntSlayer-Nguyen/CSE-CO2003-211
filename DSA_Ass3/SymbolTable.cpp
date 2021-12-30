#include "SymbolTable.h"





inline bool valid_Instruction(string identifier){
    if(identifier[0] >= 97 && identifier[0] <= 122) {
        for (unsigned int i = 1; i < identifier.length(); i++) {
            if ((identifier[i] >= 65 && identifier[i] <= 90) || (identifier[i] >= 97 && identifier[i] <= 122) ||
                identifier[i]
                == 95 || (identifier[i] >= 48 && identifier[i] <= 57))
                continue;
            else return false;
        }
        return true;
    }
    else return false;
}



inline void SymbolTable::run(string filename)
{
    ifstream ifs;
    ifs.open(filename);
    int current_scope = 0;
    string line;
    if(!ifs) {
        cout << "Can not read file \n";
        return;
    }
    while(getline(ifs, line)){
        //printTable();
        string out[10];
        int no_segment = tokenizer(line, ' ', out);
        if(line.back() == ' ') throw InvalidDeclaration(line);
        if(out[0] == "LINEAR"){
            if(no_segment != 3) throw InvalidInstruction(line);
            if(this->method.empty()) this->method = "LINEAR";
            else throw InvalidInstruction(line);
            long long int m = stoll(out[1]);
            long long int param = stoll(out[2]);
            if(m <= 0 || m > 999999) throw InvalidInstruction(line);
            if(param > 999999) throw InvalidInstruction(line);
            this->table = new Value[m];
            this->size = m;
            this->c = param;
        }
        else if(out[0] == "QUADRATIC"){
            if(no_segment != 4) throw InvalidInstruction(line);
            if(this->method.empty()) this->method = "QUADRATIC";
            else throw InvalidInstruction(line);
            long long int m = stoll(out[1]);
            long long int param1 = stoll(out[2]);
            long long param2 = stoll(out[3]);
            if(m <= 0 || m > 999999) throw InvalidInstruction(line);
            if(param1 > 999999) throw InvalidInstruction(line);
            if(param2 > 999999) throw InvalidInstruction(line);
            this->table = new Value[m];
            this->size = m;
            this->c = param1;
            this->c2 = param2;
        }
        else if(out[0] == "DOUBLE"){
            if(no_segment != 3) throw InvalidInstruction(line);
            if(this->method.empty()) this->method = "DOUBLE";
            else throw InvalidInstruction(line);
            long long int m = stoll(out[1]);
            long long int param = stoll(out[2]);
            if(m <= 0 || m > 999999) throw InvalidInstruction(line);
            if(param > 999999) throw InvalidInstruction(line);
            this->table = new Value[m];
            this->size = m;
            this->c = param;
        }
        else if(out[0] == "INSERT"){
            string id = out[1];
            int num_param = -1;
            if(!valid_Instruction(id)) throw InvalidInstruction(line);
            if(no_segment > 3 || no_segment < 2) throw InvalidInstruction(line);
            if(no_segment == 3) num_param = stoi(out[2]);
            int res = insert(id, current_scope, num_param);
            if(res == -2) throw Overflow(line);
            if(res == -1) throw Redeclared(id);
            cout << res << endl;
        }
        else if(out[0] == "ASSIGN"){
            if(no_segment != 3) throw InvalidInstruction(line);
            string id = out[1];
            //cout << print();
            if(!valid_Instruction(id)) throw InvalidInstruction(line);
            string value = out[2];
            int num_pass = 0;
            int res = this->assignment_processing(id, value, current_scope, num_pass);
            if(res == -1) throw InvalidInstruction(line);
            if(res == -3) throw TypeMismatch(line);
            //if(res == -2) throw Undeclared(line);
            if(res == -4) throw TypeCannotBeInferred(line);
            this->assign(res,current_scope,value);
            cout << num_pass << endl;
        }
        else if(out[0] == "CALL"){
            if(no_segment != 2) throw InvalidInstruction(line);
            string exp = out[1];
            int num_pass = 0;
            //cout << print() << endl;
            int res = this->exp_processing(exp, current_scope, num_pass);
            if(res == -1) throw InvalidInstruction(line);
            if(res == -3) throw TypeMismatch(line);
            //if(res == -2) throw Undeclared(line);
            if(res == -4) throw TypeCannotBeInferred(line);
            cout << num_pass << endl;
        }
        else if(out[0] == "BEGIN"){
            current_scope++;
        }
        else if(out[0] == "END"){
            if(current_scope == 0)
                throw UnknownBlock();
            else{
                deleteRecord(current_scope);
                current_scope--;
            }
        }
        else if(out[0] == "LOOKUP"){
            string id = out[1];
            if(no_segment != 2) throw InvalidInstruction(line);
            if(!valid_Instruction(id)) throw InvalidInstruction(line);
            int res = this->lookup(id, current_scope);
            if(res == -1) throw Undeclared(id);
            else cout << res << endl;
        }
        else if(out[0] == "PRINT"){
            if(no_segment != 1) throw InvalidInstruction(line);
            string result = print();
            if(result != "") cout << result << endl;
        }

    }
    if(current_scope != 0)
        throw UnclosedBlock(current_scope);

    ifs.close();
}