 #include "typecheck.hpp"
 #include <cassert>

VariableTable* saved;
std::string currentSuperClass;

// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable:
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method:
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class:
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member:
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object:
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch:
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch:
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch:
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch:
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case do_while_predicate_type_mismatch:
      std::cerr << "Predicate of repeat loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch:
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch:
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch:
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type:
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class:
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present:
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method:
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature:
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

void TypeCheck::visitProgramNode(ProgramNode* node) {
  // WRITEME: Replace with code if necessary
  classTable = new ClassTable();
  node->visit_children(this);

  ClassTable::iterator ci = classTable->find("Main");
  if(ci == classTable->end()){
      typeError(no_main_class);
  }
}

void TypeCheck::visitClassNode(ClassNode* node) {
  // WRITEME: Replace with code if necessary
  currentMethodTable = new MethodTable();
  currentVariableTable = new VariableTable();
  currentMemberOffset = 0;
  currentClassName = node->identifier_1->name;

  ClassInfo c;
  if(node->identifier_2 != NULL){
    c.superClassName = node->identifier_2->name;
    currentSuperClass = node->identifier_2->name;
  } else {
    currentSuperClass = "";
    c.superClassName = "";
  }


  if(c.superClassName != ""){
      ClassTable::iterator ci = classTable->find(c.superClassName);
      if(ci == classTable->end()){
          typeError(undefined_class);
      }
  }


  if(c.superClassName != ""){
      std::string superName = c.superClassName;

      while(superName != ""){
          ClassTable::iterator ci = classTable->find(superName);
          assert(ci != classTable->end());
          for(VariableTable::iterator vi = (ci->second).members->begin(); vi != (ci->second).members->end(); vi++){
              VariableInfo v = vi->second;
              v.offset = currentMemberOffset;
              currentMemberOffset += 4;

              currentVariableTable->insert(std::make_pair(vi->first, v));
          }

          superName = (ci->second).superClassName;
      }
  }


  node->visit_children(this);



  c.methods = currentMethodTable;
  c.members = currentVariableTable;
  c.membersSize = currentMemberOffset;

  classTable->insert(std::make_pair(currentClassName, c));

  // if(c.superClassName != ""){
  //     ClassTable::iterator ci = classTable->find(c.superClassName);
  //     if(ci == classTable->end()){
  //         typeError(undefined_class);
  //     }
  // }

  if(currentClassName == "Main" && c.members->size() != 0){
      typeError(main_class_members_present);
  }

  if(currentClassName == "Main"){
      MethodTable::iterator mi= c.methods->find("main");
      if(mi == c.methods->end()){
          typeError(no_main_method);
      }
  }

}

void TypeCheck::visitMethodNode(MethodNode* node) {

  saved = currentVariableTable;

  currentVariableTable = new VariableTable();
  currentLocalOffset = -4;
  currentParameterOffset = 12;

  int temp = currentMemberOffset;
  currentMemberOffset = -1;

  MethodInfo m;

  std::list<CompoundType>* p = new std::list<CompoundType>();
  if(node->parameter_list){
      for(std::list<ParameterNode*>::iterator iter = node->parameter_list->begin(); iter != node->parameter_list->end(); iter++) {
          (*iter)->accept(this);
          CompoundType tmp;
          tmp.baseType = (*iter)->type->basetype;

          if(tmp.baseType == bt_object){
              ObjectTypeNode* t = dynamic_cast<ObjectTypeNode*>((*iter)->type);
              tmp.objectClassName = t->identifier->name;
          }

          VariableInfo v;
          v.type = tmp;
          v.size = 4;
          v.offset = currentParameterOffset;

          currentVariableTable->insert(std::make_pair((*iter)->identifier->name, v));

          p->push_back(tmp);
          currentParameterOffset += 4;
      }
  }


  node->visit_children(this);


  CompoundType ct;
  ct.baseType = node->type->basetype;
  if(ct.baseType == bt_object){
      ObjectTypeNode* t = dynamic_cast<ObjectTypeNode*>(node->type);
      ct.objectClassName = t->identifier->name;
  }

  m.returnType = ct;
  m.variables = currentVariableTable;
  m.parameters = p;
  m.localsSize = -4-currentLocalOffset;

  currentMethodTable->insert(std::make_pair(node->identifier->name, m));

  currentMemberOffset = temp;
  currentVariableTable = saved;


  // main method incorrect signature
  if(currentClassName == "Main" && node->methodbody != NULL && node->methodbody->returnstatement != NULL){
      typeError(main_method_incorrect_signature);
  }

  //constructor return type
  if(node->identifier->name == currentClassName && node->methodbody != NULL && node->methodbody->returnstatement != NULL){
      typeError(constructor_returns_type);
  }

  if(node->type->basetype != bt_none){
      if(node->methodbody == NULL){
          typeError(return_type_mismatch);
      }

      if(node->methodbody->returnstatement == NULL){
          typeError(return_type_mismatch);
      }
  }

  // return type mismatch
  if(node->type->basetype == bt_none && node->methodbody != NULL && node->methodbody->returnstatement != NULL){
      typeError(return_type_mismatch);
  }

  if(node->type->basetype != bt_none){
      if(node->type->basetype != node->methodbody->returnstatement->basetype){
          typeError(return_type_mismatch);
      }
  }

  //return type mismatch
  if(node->type->basetype != bt_none){
      if(node->type->basetype == node->methodbody->returnstatement->basetype && node->type->basetype == bt_object){
          ObjectTypeNode* t = dynamic_cast<ObjectTypeNode*>(node->type);
          std::string objectName = t->identifier->name;
          if(objectName != node->methodbody->returnstatement->objectClassName){
              typeError(return_type_mismatch);
          }
      }
  }
}




void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
  // WRITEME: Replace with code if necessary

      node->visit_children(this);

}

void TypeCheck::visitParameterNode(ParameterNode* node) {
  // WRITEME: Replace with code if necessary

  node->visit_children(this);

  if(node->type->basetype == bt_object){
      ObjectTypeNode* t = dynamic_cast<ObjectTypeNode*>(node->type);
      std::string className  = t->identifier->name;
      ClassTable::iterator ci = classTable->find(className);
      if(ci == classTable->end()){
          typeError(undefined_class);
      }
  }
}




void TypeCheck::visitDeclarationNode(DeclarationNode* node) {
  // WRITEME: Replace with code if necessary

  node->visit_children(this);

  if(currentMemberOffset == -1){
      VariableInfo v;
      CompoundType ct;
      ct.baseType = node->type->basetype;
      if(ct.baseType == bt_object){
          ObjectTypeNode* t = dynamic_cast<ObjectTypeNode*>(node->type);
          ct.objectClassName = t->identifier->name;
      }
      v.type = ct;
      v.size = 4;

      for(std::list<IdentifierNode*>::iterator iter = node->identifier_list->begin(); iter != node->identifier_list->end(); iter++) {
          v.offset = currentLocalOffset;
          currentVariableTable->insert(std::make_pair((*iter)->name, v));
          currentLocalOffset -= 4;
      }
  }
  else
  {
      VariableInfo v;
      CompoundType ct;
      ct.baseType = node->type->basetype;
      if(ct.baseType == bt_object){
          ObjectTypeNode* t = dynamic_cast<ObjectTypeNode*>(node->type);
          ct.objectClassName = t->identifier->name;
      }
      v.type = ct;
      v.size = 4;

      for(std::list<IdentifierNode*>::iterator iter = node->identifier_list->begin(); iter != node->identifier_list->end(); iter++) {
          v.offset = currentMemberOffset;
          currentVariableTable->insert(std::make_pair((*iter)->name, v));
          currentMemberOffset += 4;
      }
  }


  if(node->type->basetype == bt_object){
      ObjectTypeNode* t = dynamic_cast<ObjectTypeNode*>(node->type);
      std::string className  = t->identifier->name;
      ClassTable::iterator ci = classTable->find(className);
      if(ci == classTable->end()){
          typeError(undefined_class);
      }
  }
}



void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
  // WRITEME: Replace with code if necessary

  node->visit_children(this);

  node->basetype = node->expression->basetype;
  node->objectClassName = node->expression->objectClassName;
}







void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
    node->visit_children(this);

    BaseType bt;
    std::string objectName = "";

    // variable assignment
    if(node->identifier_2 == NULL){
        std::string variableName = node->identifier_1->name;

        // local variable or parameter
        VariableTable::iterator vi = currentVariableTable->find(variableName);

        if(vi != currentVariableTable->end()){
            CompoundType ct = (vi->second).type;
            bt = ct.baseType;
            objectName = ct.objectClassName;

            if(bt != node->expression->basetype || (bt == bt_object && objectName != node->expression->objectClassName)){
                typeError(assignment_type_mismatch);
            }

            return;
        }

        // class members
        vi = saved->find(variableName);
        if(vi != saved->end()){
            CompoundType ct = (vi->second).type;
            bt = ct.baseType;
            objectName = ct.objectClassName;

            if(bt != node->expression->basetype || (bt == bt_object && objectName != node->expression->objectClassName)){
                typeError(assignment_type_mismatch);
            }

            return;
        }


        // super class members
        std::string superName = currentSuperClass;

        while(true){
            if(superName == ""){
                typeError(undefined_variable);
            }

            ClassTable::iterator ci = classTable->find(superName);
            assert(ci != classTable->end());

            vi = (ci->second).members->find(variableName);

            if(vi != (ci->second).members->end()){
                CompoundType ct = (vi->second).type;
                bt = ct.baseType;
                objectName = ct.objectClassName;

                if(bt != node->expression->basetype || (bt == bt_object && objectName != node->expression->objectClassName)){
                    typeError(assignment_type_mismatch);
                }

                return;
            }

            superName = (ci->second).superClassName;
        }
    }



    // member assignment
    assert(node->identifier_2 != NULL);

    std::string variableName = node->identifier_1->name;
    std::string memberName = node ->identifier_2->name;


    VariableTable::iterator vi = currentVariableTable->find(variableName);

    // local variable
    if(vi != currentVariableTable->end()){
        CompoundType ct = (vi->second).type;
        if(ct.baseType != bt_object){
            typeError(not_object);
        }
        objectName = ct.objectClassName;
    }

    // class members
    if(objectName == ""){
        vi = saved->find(variableName);
        if(vi != saved->end()){
            CompoundType ct = (vi->second).type;
            if(ct.baseType != bt_object){
                typeError(not_object);
            }
            objectName = ct.objectClassName;
        }
    }


    // super class members
    if(objectName == ""){
        std::string superName = currentSuperClass;

        while(true){
            if(superName == ""){
                typeError(undefined_variable);
            }

            ClassTable::iterator ci = classTable->find(superName);
            assert(ci != classTable->end());

            vi = (ci->second).members->find(variableName);

            if(vi != (ci->second).members->end()){
                CompoundType ct = (vi->second).type;
                if(ct.baseType != bt_object){
                    typeError(not_object);
                }
                objectName = ct.objectClassName;
                break;
            }

            superName = (ci->second).superClassName;
        }
    }


    // find the member in the object class or its super class
    std::string superName = objectName;
    assert(superName != "");

    while(true){
        if(superName == ""){
            typeError(undefined_member);
        }

        ClassTable::iterator ci = classTable->find(superName);
        assert(ci != classTable->end());

        vi = (ci->second).members->find(memberName);

        if(vi != (ci->second).members->end()){
            CompoundType ct = (vi->second).type;
            bt = ct.baseType;
            objectName = ct.objectClassName;

            if(bt != node->expression->basetype || (bt == bt_object && objectName != node->expression->objectClassName)){
                typeError(assignment_type_mismatch);
            }

            break;
        }

        superName = (ci->second).superClassName;
    }

}








void TypeCheck::visitCallNode(CallNode* node) {
    node->visit_children(this);

    assert(node->methodcall != NULL);
    node->basetype = node->methodcall-> basetype;
    node->objectClassName = node->methodcall->objectClassName;
}





void TypeCheck::visitIfElseNode(IfElseNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if(node->expression->basetype != bt_boolean){
      typeError(if_predicate_type_mismatch);
  }
}

void TypeCheck::visitWhileNode(WhileNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if(node->expression->basetype != bt_boolean){
      typeError(while_predicate_type_mismatch);
  }
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
  node->visit_children(this);

  if(node->expression->basetype != bt_boolean){
      typeError(do_while_predicate_type_mismatch);
  }
}




void TypeCheck::visitPrintNode(PrintNode* node) {
  // WRITEME: Replace with code if necessary
    node->visit_children(this);
}





void TypeCheck::visitPlusNode(PlusNode* node) {
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
  }

  node->basetype = bt_integer;
}

void TypeCheck::visitMinusNode(MinusNode* node) {
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
  }

  node->basetype = bt_integer;
}

void TypeCheck::visitTimesNode(TimesNode* node) {
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
  }

  node->basetype = bt_integer;
}

void TypeCheck::visitDivideNode(DivideNode* node) {
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
  }

  node->basetype = bt_integer;
}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
    node->visit_children(this);
    if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
          typeError(expression_type_mismatch);
    }

    node->basetype = bt_boolean;
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
    node->visit_children(this);
    if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
          typeError(expression_type_mismatch);
    }

    node->basetype = bt_boolean;
}

void TypeCheck::visitEqualNode(EqualNode* node) {
    node->visit_children(this);
    if(node->expression_1->basetype != node->expression_2->basetype) {
          typeError(expression_type_mismatch);
    }

    node->basetype = bt_boolean;
}

void TypeCheck::visitAndNode(AndNode* node) {
    node->visit_children(this);
    if(node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
          typeError(expression_type_mismatch);
    }

    node->basetype = bt_boolean;
}

void TypeCheck::visitOrNode(OrNode* node) {
    node->visit_children(this);
    if(node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
          typeError(expression_type_mismatch);
    }

    node->basetype = bt_boolean;
}

void TypeCheck::visitNotNode(NotNode* node) {
    node->visit_children(this);
    if(node->expression->basetype != bt_boolean) {
          typeError(expression_type_mismatch);
    }

    node->basetype = bt_boolean;
}


void TypeCheck::visitNegationNode(NegationNode* node) {
    node->visit_children(this);
    if(node->expression->basetype != bt_integer) {
          typeError(expression_type_mismatch);
    }

    node->basetype = bt_integer;
}



void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
  node->visit_children(this);

  // method in current class or super class
  if(node->identifier_2 == NULL){
      std::string methodName = node->identifier_1->name;

      MethodTable::iterator mi = currentMethodTable->find(methodName);

      // current class method
      if(mi != currentMethodTable->end()){
          CompoundType ct = (mi->second).returnType;
          node->basetype = ct.baseType;
          node->objectClassName = ct.objectClassName;

          // check for argument number
          std::list<CompoundType>* p = (mi->second).parameters;
          if(node->expression_list->size() != p->size()){
              typeError(argument_number_mismatch);
          }

          // check for argument type match
          std::list<CompoundType>::iterator pi = p->begin();
          std::list<ExpressionNode*>::iterator ei = node->expression_list->begin();

          for(;pi != p->end(); pi++, ei++){
              CompoundType pct = (*pi);
              BaseType ebt = (*ei)->basetype;
              std::string eobject = (*ei)->objectClassName;
              if(pct.baseType != ebt){
                  typeError(argument_type_mismatch);
              }
              if(ebt == bt_object && eobject != pct.objectClassName){
                  typeError(argument_type_mismatch);
              }
          }


          return;
      }

      // super class method
      std::string superName = currentSuperClass;

      while(true){
          if(superName == ""){
              typeError(undefined_method);
          }

          ClassTable::iterator ci = classTable->find(superName);
          assert(ci != classTable->end());

          mi = (ci->second).methods->find(methodName);

          if(mi != (ci->second).methods->end()){
              CompoundType ct = (mi->second).returnType;
              node->basetype = ct.baseType;
              node->objectClassName = ct.objectClassName;

              // check for argument number
              std::list<CompoundType>* p = (mi->second).parameters;
              if(node->expression_list->size() != p->size()){
                  typeError(argument_number_mismatch);
              }

              // check for argument type match
              std::list<CompoundType>::iterator pi = p->begin();
              std::list<ExpressionNode*>::iterator ei = node->expression_list->begin();

              for(;pi != p->end(); pi++, ei++){
                  CompoundType pct = (*pi);
                  BaseType ebt = (*ei)->basetype;
                  std::string eobject = (*ei)->objectClassName;
                  if(pct.baseType != ebt){
                      typeError(argument_type_mismatch);
                  }
                  if(ebt == bt_object && eobject != pct.objectClassName){
                      typeError(argument_type_mismatch);
                  }
              }

              return;
          }

          superName = (ci->second).superClassName;
      }
  }


  // method in varible class and its super class
  assert(node->identifier_2 != NULL);

  std::string variableName = node->identifier_1->name;
  std::string methodName = node->identifier_2->name;

  std::string objectName = "";

  VariableTable::iterator vi = currentVariableTable->find(variableName);

  // local variable
  if(vi != currentVariableTable->end()){
      CompoundType ct = (vi->second).type;
      if(ct.baseType != bt_object){
          typeError(not_object);
      }
      objectName = ct.objectClassName;
  }

  // class members
  if(objectName == ""){
      vi = saved->find(variableName);
      if(vi != saved->end()){
          CompoundType ct = (vi->second).type;
          if(ct.baseType != bt_object){
              typeError(not_object);
          }
          objectName = ct.objectClassName;
      }
  }


  // super class members
  if(objectName == ""){
      std::string superName = currentSuperClass;

      while(true){
          if(superName == ""){
              typeError(undefined_variable);
          }

          ClassTable::iterator ci = classTable->find(superName);
          assert(ci != classTable->end());

          vi = (ci->second).members->find(variableName);

          if(vi != (ci->second).members->end()){
              CompoundType ct = (vi->second).type;
              if(ct.baseType != bt_object){
                  typeError(not_object);
              }
              objectName = ct.objectClassName;
              break;
          }

          superName = (ci->second).superClassName;
      }
  }


  std::string superName = objectName;
  assert(objectName != "");

  while(true){
      if(superName == ""){
          typeError(undefined_method);
      }

      ClassTable::iterator ci = classTable->find(superName);
      assert(ci != classTable->end());

      MethodTable::iterator mi = (ci->second).methods->find(methodName);

      if(mi != (ci->second).methods->end()){
          CompoundType ct = (mi->second).returnType;
          node->basetype = ct.baseType;
          node->objectClassName = ct.objectClassName;

          // check for argument number
          std::list<CompoundType>* p = (mi->second).parameters;
          if(node->expression_list->size() != p->size()){
              typeError(argument_number_mismatch);
          }

          // check for argument type match
          std::list<CompoundType>::iterator pi = p->begin();
          std::list<ExpressionNode*>::iterator ei = node->expression_list->begin();

          for(;pi != p->end(); pi++, ei++){
              CompoundType pct = (*pi);
              BaseType ebt = (*ei)->basetype;
              std::string eobject = (*ei)->objectClassName;
              if(pct.baseType != ebt){
                  typeError(argument_type_mismatch);
              }
              if(ebt == bt_object && eobject != pct.objectClassName){
                  typeError(argument_type_mismatch);
              }
          }

          return;
      }

      superName = (ci->second).superClassName;
  }

}




void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
    node->visit_children(this);

    std::string variableName = node->identifier_1->name;
    std::string memberName = node ->identifier_2->name;

    std::string objectName = "";

    VariableTable::iterator vi = currentVariableTable->find(variableName);

    // local variable
    if(vi != currentVariableTable->end()){
        CompoundType ct = (vi->second).type;
        if(ct.baseType != bt_object){
            typeError(not_object);
        }
        objectName = ct.objectClassName;
    }

    // class members
    if(objectName == ""){
        vi = saved->find(variableName);
        if(vi != saved->end()){
            CompoundType ct = (vi->second).type;
            if(ct.baseType != bt_object){
                typeError(not_object);
            }
            objectName = ct.objectClassName;
        }
    }


    // super class members
    if(objectName == ""){
        std::string superName = currentSuperClass;

        while(true){
            if(superName == ""){
                typeError(undefined_variable);
            }

            ClassTable::iterator ci = classTable->find(superName);
            assert(ci != classTable->end());

            vi = (ci->second).members->find(variableName);

            if(vi != (ci->second).members->end()){
                CompoundType ct = (vi->second).type;
                if(ct.baseType != bt_object){
                    typeError(not_object);
                }
                objectName = ct.objectClassName;
                break;
            }

            superName = (ci->second).superClassName;
        }
    }


    // find the member in the object class or its super class
    std::string superName = objectName;
    assert(superName != "");

    while(true){
        if(superName == ""){
            typeError(undefined_member);
        }

        ClassTable::iterator ci = classTable->find(superName);
        assert(ci != classTable->end());

        vi = (ci->second).members->find(memberName);

        if(vi != (ci->second).members->end()){
            CompoundType ct = (vi->second).type;
            node->basetype = ct.baseType;
            node->objectClassName = ct.objectClassName;
            break;
        }

        superName = (ci->second).superClassName;
    }

}


void TypeCheck::visitVariableNode(VariableNode* node) {
    node->visit_children(this);

    std::string variableName = node->identifier->name;

    // local variable or parameter
    VariableTable::iterator vi = currentVariableTable->find(variableName);

    if(vi != currentVariableTable->end()){
        CompoundType ct = (vi->second).type;
        node->basetype = ct.baseType;
        node->objectClassName = ct.objectClassName;
        return;
    }

    // class members
    vi = saved->find(variableName);
    if(vi != saved->end()){
        CompoundType ct = (vi->second).type;
        node->basetype = ct.baseType;
        node->objectClassName = ct.objectClassName;
        return;
    }


    // super class members
    std::string superName = currentSuperClass;

    while(true){

        if(superName == ""){
            typeError(undefined_variable);
        }

        ClassTable::iterator ci = classTable->find(superName);
        assert(ci != classTable->end());

        vi = (ci->second).members->find(variableName);

        if(vi != (ci->second).members->end()){
            CompoundType ct = (vi->second).type;
            node->basetype = ct.baseType;
            node->objectClassName = ct.objectClassName;
            break;
        }

        superName = (ci->second).superClassName;
    }
}






void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
  node->basetype = bt_integer;
  node->objectClassName= "";
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
  node->basetype = bt_boolean;
  node->objectClassName= "";
}

void TypeCheck::visitNewNode(NewNode* node) {
  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
  node->objectClassName= "";
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
  node->objectClassName= "";
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
}

void TypeCheck::visitNoneNode(NoneNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_none;
  node->objectClassName= "";
}





void TypeCheck::visitIdentifierNode(IdentifierNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
    node->basetype = bt_integer;
    node->objectClassName= "";
}


// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}
