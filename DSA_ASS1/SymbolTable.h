#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "main.h"


bool valid_Instruction(string &identifier){
  if(identifier[0] < 97 || identifier[0] > 122) return false;
  for(unsigned int i = 1; i < identifier.length();i++){
      if((identifier[i] >= 65 && identifier[i] <= 90)||(identifier[i] >= 97 && identifier[i] <= 122) || identifier[i]
      == 95 || (identifier[i] >= 48 && identifier[i] <= 57) ) continue;
      else return false;
    }
  return true;
}
bool isNumber(string& str)
{
  for (char const &c : str) {
      if (std::isdigit(c) == 0) return false;
    }
  return true;
}
bool isString(string& str){
  if(str[0] != 39 || str[str.length()-1] != 39) return false;
  for(unsigned int i = 1; i < str.length() - 1;i++){
      if((str[i] >= 65 && str[i] <= 90)||(str[i] >= 97 && str[i] <= 122) || (str[i] >= 48 && str[i] <= 57) || str[i]
                                                                                                              == 32)
        continue;
      else return false;
    }
  return true;
}



void tokenize(string str, char delim, string out[])
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
    if(count == 2) {
      out[count] = str.substr(j, size - j);
      break;
    }
  }
}

class SymbolTable
{
 public:
  class Node{
   public:
    string identifier_name, type, value;
    int scope_level;
    bool available;
    Node* next;
    Node():next(nullptr){};
    Node(string i_name, string typ, string val, int sc_level, Node* pNext, bool avail)
    {
      this->identifier_name = std::move(i_name);
      this->type = std::move(typ);
      this->value = std::move(val);
      this->scope_level = sc_level;
      this->next = pNext;
      this->available = avail;
    }
    ~Node(){
      identifier_name.clear();
      type.clear();
      value.clear();
      scope_level = 0;
      next = nullptr;
      available = false;
    }
  };
 protected:
  Node* head;
  int size;
 public:
  SymbolTable(): head(nullptr), size(0){};
  ~SymbolTable(){
    this->clear();
  }
  bool insert(string& i_name, string& typ, int sc_level){
    Node* newNode = new Node(i_name, typ, "", sc_level, nullptr, true);
    Node* pt = head;
    if(size == 0) {
      head = newNode;
      size++;
      return true;
    }
    while(pt->next){
        if(pt->identifier_name == newNode->identifier_name){
          if(pt->scope_level == newNode->scope_level) {
              delete newNode;
              return false;
          }
          else pt->available = false;
        }
        pt = pt->next;
      }
    if(pt->identifier_name == newNode->identifier_name){
        if(pt->scope_level == newNode->scope_level) {
            delete newNode;
            return false;
        }
        else pt->available = false;
    }
    pt->next = newNode;
    size++;
    return true;
  }
  void dumpFunc(int sc_level){
    Node* pt = head;
    while(pt){
      if(pt->scope_level == sc_level) pt->available = true;
      pt = pt->next;
    }
  }


  /* return code:
   * 0 = successful
   * 1 = undeclared
   * 2 = type mismatched
   * */
  int assign(string& i_name, string& typ, string& value, int sc_level){
    Node* pt = head;
    if(!pt) return 1;
    Node* temp = nullptr;
    while(pt){
      if(pt->identifier_name == i_name && pt->scope_level <= sc_level){
        temp = pt;
      }
      pt = pt->next;
    }
    if(temp){
      if(temp->type == typ){
        temp->value = value; // ASSIGN successful
        return 0;
      }
      else return 2;      //  Type mismatched
    }
    return 1;               // undeclared
  }
  /* return code:
   * 0 = string type
   * 1 = number type
   * 2 = idf assign
   * 3 = invalid
   * */
  string identifier_assignment(string &value){
    Node* temp = head;
    if(!temp) return "-1";
    while(temp){
        if(value == temp->identifier_name && temp->available) return temp->type;
        temp = temp->next;
      }
    return "-1";
  }
  string value_type(string value){
    if(isString(value)) return "string";
    else if(isNumber(value)) return "number";
    else return identifier_assignment(value);
  }

  void deleteRecord(int sc_level){
    while (head && head->scope_level == sc_level){
      Node* temp = head;
      head = head->next;
      retrieve_var(sc_level, temp->identifier_name);
      delete temp;
      size--;
    }
    if(!head) return;
    Node *tmp = head;
    while (tmp->next)
      if (tmp->next->scope_level == sc_level){
        Node* t = tmp->next;
        tmp->next = tmp->next->next;
        retrieve_var(sc_level, t->identifier_name);
        delete t;
        size--;
      }
      else tmp = tmp->next;
  }
  void retrieve_var(int sc_level, string i_name){
      Node* pt = head;
      if(!pt) return;
      Node* res = nullptr;
      int nearest = INT8_MAX;
      while(pt) {
          if(pt->identifier_name == i_name && (sc_level - pt->scope_level) < nearest){
              res = pt;
              nearest = sc_level - pt->scope_level;
          }
          pt = pt->next;
      }
      if(res) res->available = true;
  }


  /* return code:
   * -1: undeclared
   * >=0 : scope_level found
   * */
  int look_up(string& i_name, int sc_level){
    Node* pt = head;
    if(!pt) return -1;
    Node* temp = nullptr;
    while(pt){
        if(pt->identifier_name == i_name && pt->scope_level <= sc_level){
          temp = pt;
        }
        pt = pt->next;
    }
    if(temp) return temp->scope_level;
    else return -1;
  }
  void print(int sc_level){
    Node* pt = head;
    string res;
    if(!pt) return;
    while(pt){
      if(pt->available) res += pt->identifier_name + "//" + to_string(pt->scope_level) + " ";
      pt = pt->next;
    }
    res.pop_back();
    cout << res << endl;
  }
  void Rprint(int sc_level){
    Node* pt = head;
    string res;
    if(!pt) return;

    while(pt){
        if(pt->available) res.insert(0, pt->identifier_name + "//" + to_string(pt->scope_level) + " ");
        pt = pt->next;
      }
    res.pop_back();
    cout << res << endl;
  }
  void clear(){
    Node* pt = head;
    while(pt){
        Node* temp = pt;
        pt = pt->next;
        delete temp;
    }
    head = nullptr;
    size = 0;
  }

  void run(string filename);
};
#endif


