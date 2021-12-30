#include "SymbolTable.h"





void SymbolTable::run(string filename)
{
    ifstream ifs;
    ifs.open(filename);
    string line;
    int current_scope = 0;
    if(!ifs) {
      cout << "Can not read file \n";
      return;
    }
    while(getline(ifs, line)){
      string out[3];
      tokenize(line, ' ', out);
      if(out[0] == "INSERT"){
        if(!valid_Instruction(out[1])) {
          throw InvalidInstruction(line);
        }
        bool res = this->insert (out[1], out[2], current_scope);
        if(res) cout << "success\n";
        else throw Redeclared(line);
      }
      else if(out[0] == "ASSIGN"){
        if(!valid_Instruction(out[1])) {
          throw InvalidInstruction(line);
        }
        string type = this->value_type(out[2]);
        if(type == "-1"){
          throw Undeclared(line);
        }
        int res = this->assign(out[1], type, out[2],  current_scope);
        if(res == 0) cout << "success\n";
        if(res == 1){
          throw Undeclared(line);
        }
        if(res == 2){
          throw TypeMismatch(line);
        }
      }
      else if(out[0] == "BEGIN") {
        current_scope++;
      }
      else if(out[0] == "END") {
        if(current_scope == 0) {
          throw UnknownBlock();
        }
        else {
          deleteRecord(current_scope);
          current_scope--;
          dumpFunc (current_scope);
        }
      }
      else if(out[0] == "LOOKUP"){
        if(!valid_Instruction(out[1])) {
          throw InvalidInstruction(line);
        }
        int res = this->look_up(out[1], current_scope);
        if(res == -1) {
          throw Undeclared(line);
        }
        else cout << res << endl;
      }
      else if(out[0] == "PRINT"){
        this->print(current_scope);
      }
      else if(out[0] == "RPRINT"){
        this->Rprint(current_scope);
      }
      else {
        throw InvalidInstruction(line);
      }
    }
    if(current_scope != 0 ) {
      throw UnclosedBlock(current_scope);
    }
    ifs.close();
}