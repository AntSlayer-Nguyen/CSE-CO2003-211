#include "SymbolTable.h"






inline void SymbolTable::run(string filename)
{
    ifstream ifs;
    ifs.open(filename);
    int current_scope = 0;
    string line;
    container c(50);
    if(!ifs) {
        cout << "Can not read file \n";
        return;
    }
    while(getline(ifs, line)){
        string out[10];
        int words_extracted = tokenizer(line, ' ', out);
        if(words_extracted >= 4 && out[0] == "ASSIGN"){
            while(words_extracted != 3){
                out[words_extracted-2] += out[words_extracted-1];
                words_extracted--;
            }
        }
        if(out[words_extracted-1].back() == '\r') out[words_extracted].pop_back();
        if(out[0] == "INSERT") {
            if(!valid_Instruction(out[1])) throw InvalidInstruction(line);
            bool global;
            if(out[3] == "true") global = true;
            else if(out[3] == "false") global = false;
            else throw InvalidInstruction(line);
            if(out[2] != "number" && out[2] != "string"){
                string type_arr[10];
                int count = 0;
                bool res = extract_function(out[2], type_arr, count);
                if(!res) throw InvalidInstruction(line);
                if(current_scope != 0 && !global) throw InvalidDeclaration(line);
            }

            int num_comp = 0, num_splay = 0;


            bool result = this->insert(current_scope, out[1], out[2], global, num_comp, num_splay, c);
            if(!result) throw Redeclared(line);
            cout << num_comp << " " << num_splay << endl;
        }
        else if(out[0] == "ASSIGN"){
            if(!valid_Instruction(out[1])) throw InvalidInstruction(line);
            string type_arr[10];
            int num_comp = 0, num_splay = 0;
            int count = 0;
            string typ = value_type(out[2], type_arr, count, current_scope, num_comp, num_splay);
            if(typ == "-1") throw InvalidInstruction(line);
            if(typ == "1") throw TypeMismatch(line);
            if(typ == "2") throw Undeclared(line);
            if(typ == "number" || typ == "string"){
                if(count != 0) throw TypeMismatch(line);
                int res = assign(current_scope, out[1], typ, out[2], num_comp , num_splay);
                if(res == 0) throw TypeMismatch(line);
                if(res == -1) throw Undeclared(line);
                if(res == 1) cout << num_comp << " " << num_splay << endl;
            }
            else{
                string type_arr1[10];
                int count1 = 0;
                extract_function(typ, type_arr1, count1);
                if(count1 == count + 1){
                    for(int i = 0; i < count; i++)
                        if(type_arr[i] != type_arr1[i]) throw TypeMismatch(line);
                }
                else throw TypeMismatch(line);
                int res = assign(current_scope, out[1], typ, out[2], num_comp , num_splay);
                if(res == 0) throw TypeMismatch(line);
                if(res == -1) throw Undeclared(line);
                if(res == 1) cout << num_comp << " " << num_splay << endl;
            }
        }
        else if(out[0] == "BEGIN") {
            current_scope++;
        }
        else if(out[0] == "END") {
            if(current_scope == 0)
                throw UnknownBlock();
            else{
                deleteRecord(current_scope, c);
                current_scope--;
            }
        }
        else if(out[0] == "LOOKUP"){
            if(!valid_Instruction(out[1])) {
                throw InvalidInstruction(line);
            }
            int res = lookUp(out[1]);
            if(res == -1) throw Undeclared(line);
            else cout << res << endl;
        }
        else if(out[0] == "PRINT"){
            print();
        }
        else throw InvalidInstruction(line);
    }
    if(current_scope != 0)
        throw UnclosedBlock(current_scope);
    ifs.close();
}