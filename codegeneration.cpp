#include "codegeneration.hpp"
#include <iostream>
#include <string>
#include <cassert>

using namespace std;

// CodeGenerator Visitor Functions: These are the functions
// you will complete to generate the x86 assembly code. Not
// all functions must have code, many may be left empty.



void CodeGenerator::visitProgramNode(ProgramNode* node) {
    std::cout << "# Program" << std::endl;
    std::cout << ".data" << std::endl;
    std::cout << "printstr: .asciz \"%d\\n\"" << std::endl;
    std::cout << std::endl;
    std::cout << ".text" << std::endl;
    std::cout << ".global Main_main" << std::endl;

    node->visit_children(this);

}

void CodeGenerator::visitClassNode(ClassNode* node) {
    // set current class name and info
    std::cout << "# Class" << std::endl;
    std::string name = node->identifier_1->name;
    this->currentClassName = name;
    ClassInfo ci = this->classTable->find(currentClassName)->second;
    this->currentClassInfo = ci;

    node->visit_children(this);
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
    std::cout << "# Method" << std::endl;
    // set current method name and info
    std::string method = node->identifier->name;
    this->currentMethodName = method;
    MethodInfo mi = currentClassInfo.methods->find(method)->second;
    this->currentMethodInfo = mi;
    // get local size
    std::string localSize = std::to_string(mi.localsSize);

    std::string label = currentClassName + "_" + currentMethodName + ":";

    std::cout << label << std::endl;
    // save ebp, offset 0
    std::cout << "push %ebp" << std::endl;

    // set new %ebp
    std::cout << "movl %esp, %ebp" << std::endl;
    // allocate memory for local variables,  offset -4
    std::cout << "sub $" + localSize + ", " + "%esp" << std::endl;

    // save %ebx %esi % edi
    std::cout << "push %ebx" << std::endl;
    std::cout << "push %esi" << std::endl;
    std::cout << "push %edi" << std::endl;

    node->visit_children(this);

    // restore %ebx %esi % edi
    std::cout << "pop %edi" << std::endl;
    std::cout << "pop %esi" << std::endl;
    std::cout << "pop %ebx" << std::endl;


    // deallocate memory for local variables
    std::cout << "add $" + localSize + ", " + "%esp" << std::endl;

    // resotre %esp
    std::cout << "movl %ebp, %esp" << std::endl;

    // resotre %ebp
    std::cout << "pop %ebp" << std::endl;
    // call return
    std::cout << "ret" << std::endl;
}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {

    node->visit_children(this);
}

void CodeGenerator::visitParameterNode(ParameterNode* node) {
    // empty
}




void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {
    //  leave empty ???????????????????????
}




void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {

    // expression result on top of stack.
    node->visit_children(this);
    // store return value
    std::cout << "pop %eax" << std::endl;
}







void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
    // 1. basic type, direct assignment
    // 2. object type, assign each member
    std::cout << "# Assginment" << std::endl;
    // expression result on top of stack.
    node->visit_children(this);

    BaseType bt;
    std::string objectName = "";
    std::string offset = "";

    // local variable/member assignment
    if(node->identifier_2 == NULL){
        std::string variableName = node->identifier_1->name;

        // local variable or parameter
        VariableTable::iterator vi = currentMethodInfo.variables->find(variableName);
        if(vi != currentMethodInfo.variables->end()){
            CompoundType ct = (vi->second).type;
            bt = ct.baseType;
            objectName = ct.objectClassName;
            offset = std::to_string((vi->second).offset);

            // basic types
            if(bt != bt_object){
                std::cout << "pop %eax" << std::endl; // get the value on rhs
                std::cout << "movl %eax, " + offset + "(%ebp)" << std::endl; // store the value;
            } else {
            // object type
                // get the member size;
                assert(objectName != "");
                ClassTable::iterator ci = classTable->find(objectName);
                assert(ci != classTable->end());
                int size = (ci->second).membersSize;

                std::cout << "pop %eax" << std::endl; // get the address of rhs
                std::cout << "movl %eax, " + offset + "(%ebp)" << std::endl; // store the rhs address;

                // // set each member, target address in ebx, source in edx
                // for(int i = 0; i < size; i += 4){
                //     std::cout << "movl " + std::to_string(i) + "(%edx), %eax" << std::endl; // move one to eax
                //     std::cout << "movl  %eax, " + std::to_string(i) + "(%ebx)" << std::endl; // move to target
                // }
            }
        }

        // class members, super class included
        if(vi == currentMethodInfo.variables->end()){
            vi = currentClassInfo.members->find(variableName);
            if(vi != currentClassInfo.members->end()){
                CompoundType ct = (vi->second).type;
                bt = ct.baseType;
                objectName = ct.objectClassName;
                offset = std::to_string((vi->second).offset);

                // basic types
                if(bt != bt_object){
                    std::cout << "pop %eax" << std::endl; // get the value on rhs
                    std::cout << "movl 8(%ebp), %edx" << std::endl;
                    std::cout << "movl %eax, " + offset + "(%edx)" << std::endl; // store the value;
                } else {
                // object type
                    // get the member size;
                    assert(objectName != "");
                    ClassTable::iterator ci = classTable->find(objectName);
                    assert(ci != classTable->end());
                    int size = (ci->second).membersSize;

                    std::cout << "pop %eax" << std::endl; // get the address of rhs
                    // std::cout << "movl 8(%ebp), %eax" << std::endl;
                    // std::cout << "movl " + offset + "(%eax), %ebx" << std::endl; // address of lhs

                    std::cout << "movl 8(%ebp), %edx" << std::endl; // get self pointer
                    std::cout << "movl %eax, " + offset + "(%edx)" << std::endl; // move to the offset + self pointer


                    // // set each member, target address in ebx, source in edx
                    // for(int i = 0; i < size; i += 4){
                    //     std::cout << "movl " + std::to_string(i) + "(%edx), %eax" << std::endl; // move one to eax
                    //     std::cout << "movl  %eax, " + std::to_string(i) + "(%ebx)" << std::endl; // move to target
                    // }
                }
            }
        }
    }
    // variable.member assignment
    else {
        assert(node->identifier_2 != NULL);

        bool flag = false;  // local variable or class member

        std::string variableName = node->identifier_1->name;
        std::string memberName = node ->identifier_2->name;
        std::string variableOffset = "";

        // local variable
        VariableTable::iterator vi = currentMethodInfo.variables->find(variableName);
        if(vi != currentMethodInfo.variables->end()){
            CompoundType ct = (vi->second).type;
            objectName = ct.objectClassName;
            variableOffset = std::to_string((vi->second).offset);
        }

        // class member, super class included
        if(vi == currentMethodInfo.variables->end()){
            vi = currentClassInfo.members->find(variableName);
            assert(vi != currentClassInfo.members->end());
            CompoundType ct = (vi->second).type;
            objectName = ct.objectClassName;
            variableOffset = std::to_string((vi->second).offset);
            flag = true;
        }

        // find the member in the object class, super class included
        ClassTable::iterator ci = classTable->find(objectName);
        assert(ci != classTable->end());

        vi = (ci->second).members->find(memberName);
        assert(vi != (ci->second).members->end());

        CompoundType ct = (vi->second).type;
        bt = ct.baseType;
        objectName = ct.objectClassName;
        std::string offset = std::to_string((vi->second).offset);   // member offset

        // basic types
        if(bt != bt_object){
            std::cout << "pop %eax" << std::endl; // get the value on rhs

            if(flag){   // if class member
                std::cout << "movl 8(%ebp), %edx" << std::endl; // get self pointer
            } else {    // if local varibale
                std::cout << "movl %ebp, %edx" << std::endl;
            }

            assert(variableOffset != "");
            assert(offset != "");
            std::cout << "movl " + variableOffset + "(%edx), %ebx" << std::endl; // get the variable address
            std::cout << "movl %eax, " + offset + "(%ebx)" << std::endl; // store the value at variable address + offset;
        } else {
        // object type
            // get the member size;
            assert(objectName != "");
            ClassTable::iterator ci = classTable->find(objectName);
            assert(ci != classTable->end());
            int size = (ci->second).membersSize;

            std::cout << "pop %edx" << std::endl; // get the address of rhs

            if(flag){   // if class member
                std::cout << "movl 8(%ebp), %edx" << std::endl; // get self pointer
            } else {    // if local varibale
                std::cout << "movl %ebp, %edx" << std::endl;
            }

            assert(variableOffset != "");
            assert(offset != "");
            std::cout << "movl " + variableOffset + "(%ebx), %eax" << std::endl; // get the variable address
            std::cout << "movl %eax, " + offset + "(%ebx)" << std::endl; // store the rhs address;

            // std::cout << "movl " + offset + "(%eax), %ebx" << std::endl; // address of lhs
            //
            // // set each member, target address in ebx, source in edx
            // for(int i = 0; i < size; i += 4){
            //     std::cout << "movl " + std::to_string(i) + "(%edx), %eax" << std::endl; // move one to eax
            //     std::cout << "movl  %eax, " + std::to_string(i) + "(%ebx)" << std::endl; // move to target
            // }
        }
    }
}










void CodeGenerator::visitCallNode(CallNode* node) {
    node->visit_children(this);
    std::cout << "add $4, %esp" << std::endl;
}





void CodeGenerator::visitIfElseNode(IfElseNode* node) {

    std::string label1 = "L" + std::to_string(this->nextLabel());
    std::string label2 = "L" + std::to_string(this->nextLabel());

    std::cout << "# If Else" << std::endl;
    node->expression->accept(this);

    // evaluate the if expression
    std::cout << "pop %eax" << std::endl;
    std::cout << "cmp $1, %eax" << std::endl;
    std::cout << "je " + label1 << std::endl;

    // else block
    std::cout << "#Else" << std::endl;
    if (node->statement_list_2) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list_2->begin();
            iter != node->statement_list_2->end(); iter++) {
          (*iter)->accept(this);
        }
    }

    std::cout << "jmp " + label2 << std::endl;

    // if block
    std::cout << "#If" << std::endl;
    std::cout << label1 + ":" << std::endl;
    if (node->statement_list_1) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list_1->begin();
            iter != node->statement_list_1->end(); iter++) {
          (*iter)->accept(this);
        }
    }

    std::cout << label2 + ":" << std::endl;
}

void CodeGenerator::visitWhileNode(WhileNode* node) {
    std::string label1 = "L" + std::to_string(this->nextLabel());
    std::string label2 = "L" + std::to_string(this->nextLabel());

    std::cout << "# While" << std::endl;
    std::cout << label1 + ":" << std::endl;

    // evaluate the expression
    node->expression->accept(this);

    // evaluate the boolean expression
    std::cout << "pop %eax" << std::endl;
    std::cout << "cmp $1, %eax" << std::endl;
    std::cout << "jne " + label2 << std::endl;

    if (node->statement_list) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list->begin();
            iter != node->statement_list->end(); iter++) {
          (*iter)->accept(this);
        }
    }

    std::cout << "jmp " + label1 << std::endl;
    std::cout << label2 + ":" << std::endl;

}

void CodeGenerator::visitPrintNode(PrintNode* node) {
    std::cout << "# Print" << std::endl;
    node->visit_children(this);
    std::cout << "push $printstr" << std::endl;
    std::cout << "call printf" << std::endl;
    std::cout << "add $4, %esp" << std::endl;
    std::cout << "add $4, %esp" << std::endl;
}

void CodeGenerator::visitDoWhileNode(DoWhileNode* node) {
    std::string label1 = "L" + std::to_string(this->nextLabel());

    std::cout << "# Do While" << std::endl;
    std::cout << label1 + ":" << std::endl;

    if (node->statement_list) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list->begin();
            iter != node->statement_list->end(); iter++) {
          (*iter)->accept(this);
        }
    }

    node->expression->accept(this);
    // evaluate the boolean expression
    std::cout << "pop %eax" << std::endl;
    std::cout << "cmp $1, %eax" << std::endl;
    std::cout << "je " + label1 << std::endl;
}








void CodeGenerator::visitPlusNode(PlusNode* node) {

    node->visit_children(this);
    std::cout << "# PLUS" << std::endl;
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "add %edx, %eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitMinusNode(MinusNode* node) {

    node->visit_children(this);
    std::cout << "# MINUS" << std::endl;
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "sub %edx, %eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitTimesNode(TimesNode* node) {

    node->visit_children(this);
    std::cout << "# TIMES" << std::endl;
    std::cout << "pop %ebx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "movl $0, %edx" << std::endl;
    std::cout << "imul %ebx" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitDivideNode(DivideNode* node) {

    node->visit_children(this);
    std::cout << "# DIVIDE" << std::endl;
    std::cout << "pop %ebx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "movl $0, %edx" << std::endl;
    std::cout << "idiv %ebx" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitGreaterNode(GreaterNode* node) {

    node->visit_children(this);
    std::string label1 = "L" + std::to_string(nextLabel());
    std::string label2 = "L" + std::to_string(nextLabel());
    std::cout << "# Greater" << std::endl;
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "jg " + label1 << std::endl;
    std::cout << "movl $0, %eax" << std::endl;
    std::cout << "jmp " + label2 << std::endl;
    std::cout << label1 + ":" << std::endl;
    std::cout << "movl $1, %eax" << std::endl;
    std::cout << label2 + ":" << std::endl;
    std::cout << "push %eax" << std::endl;

}

void CodeGenerator::visitGreaterEqualNode(GreaterEqualNode* node) {

    node->visit_children(this);
    std::string label1 = "L" + std::to_string(nextLabel());
    std::string label2 = "L" + std::to_string(nextLabel());
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "jge " + label1 << std::endl;
    std::cout << "movl $0, %eax" << std::endl;
    std::cout << "jmp " + label2 << std::endl;
    std::cout << label1 + ":" << std::endl;
    std::cout << "movl $1, %eax" << std::endl;
    std::cout << label2 + ":" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitEqualNode(EqualNode* node) {

    node->visit_children(this);
    std::string label1 = "L" + std::to_string(nextLabel());
    std::string label2 = "L" + std::to_string(nextLabel());
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "je " + label1 << std::endl;
    std::cout << "movl $0, %eax" << std::endl;
    std::cout << "jmp " + label2 << std::endl;
    std::cout << label1 + ":" << std::endl;
    std::cout << "movl $1, %eax" << std::endl;
    std::cout << label2 + ":" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitAndNode(AndNode* node) {

    node->visit_children(this);
    std::cout << "# AND" << std::endl;
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "and %edx, %eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitOrNode(OrNode* node) {

    node->visit_children(this);
    std::cout << "# OR" << std::endl;
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "or %edx, %eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitNotNode(NotNode* node) {

    node->visit_children(this);
    std::cout << "# Not" << std::endl;
    std::cout << "movl $0xFFFFFFFE, %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "not %eax" << std::endl;
    std::cout << "sub %edx, %eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitNegationNode(NegationNode* node) {

    node->visit_children(this);
    std::cout << "# Negation" << std::endl;
    std::cout << "pop %edx" << std::endl;
    std::cout << "movl $0, %eax" << std::endl;
    std::cout << "sub %edx, %eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}





void CodeGenerator::visitMethodCallNode(MethodCallNode* node) {
    // add inheritance

    std::cout << "# Method Call" << std::endl;

    bool flag = false;

    std::string functionName = "";
    std::string variableOffset = "";
    if(node->identifier_2 == NULL){
        std::string methodName = node->identifier_1->name;
        MethodTable::iterator mi = currentClassInfo.methods->find(methodName);

        // method in current class
        if(mi != currentClassInfo.methods->end()){
            functionName = currentClassName + "_" + methodName;
        }

        // method in super class
        if(mi == currentClassInfo.methods->end()){
            std::string superName = currentClassInfo.superClassName;
            while(true){
                assert(superName != "");
                ClassTable::iterator ci = classTable->find(superName);
                assert(ci != classTable->end());

                mi = (ci->second).methods->find(methodName);
                if(mi != (ci->second).methods->end()){
                    functionName = superName + "_" + methodName;
                    break;
                }
            }
        }

    } else { // variable class and its super class
        std::string variableName = node->identifier_1->name;
        std::string methodName = node->identifier_2->name;
        std::string superName = "";
        VariableTable::iterator vi = currentMethodInfo.variables->find(variableName);
        // local variable
        if(vi != currentMethodInfo.variables->end()){
            CompoundType ct = (vi->second).type;
            if(ct.baseType != bt_object){
                typeError(not_object);
            }
            superName = ct.objectClassName;
            variableOffset = std::to_string((vi->second).offset);
        }
        // class members
        if(superName == ""){
            vi = currentClassInfo.members->find(variableName);
            if(vi != currentClassInfo.members->end()){
                CompoundType ct = (vi->second).type;
                if(ct.baseType != bt_object){
                    typeError(not_object);
                }
                superName = ct.objectClassName;
                variableOffset = std::to_string((vi->second).offset);
            }
            flag = true;
        }

        // find the class where the method is in
        assert(superName != "");
        while(true){
            assert(superName != "");

            ClassTable::iterator ci = classTable->find(superName);
            assert(ci != classTable->end());

            MethodTable::iterator mi = (ci->second).methods->find(methodName);

            if(mi != (ci->second).methods->end()){
                functionName = superName + "_" + methodName;
                break;
            }

            superName = (ci->second).superClassName;
        }
    }



    // save %eax %ecx %edx
    std::cout << "push %eax" << std::endl;
    std::cout << "push %ecx" << std::endl;
    std::cout << "push %edx" << std::endl;

    // push parameter in reverse order, offset 12
    if(node->expression_list) {
        for(std::list<ExpressionNode*>::reverse_iterator iter = node->expression_list->rbegin();
            iter != node->expression_list->rend(); iter++) {
          (*iter)->accept(this);
        }
    }

    // self pointer, offset 8
    if(variableOffset == "") {  // local or super method, inherit self pointer
        std::cout << "movl 8(%ebp), %eax" << std::endl;
        std::cout << "push %eax" << std::endl;
    } else {    // variable pointer
        if(flag){
            std::cout << "movl 8(%ebp), %edx" << std::endl;
            std::cout << "movl " + variableOffset + "(%edx), %eax" << std::endl;
            std::cout << "push %eax" << std::endl;
        } else {
            std::cout << "movl " + variableOffset + "(%ebp), %eax" << std::endl;
            std::cout << "push %eax" << std::endl;
        }
    }

    // return address, offset 4
    // save by call
    // jump to function
    std::cout << "call " + functionName << std::endl;

    // remove parameters
    std::cout << "add $4, %esp" << std::endl; // self pointer
    // parameters
    if(node->expression_list) {
        for(std::list<ExpressionNode*>::iterator iter = node->expression_list->begin();
            iter != node->expression_list->end(); iter++) {
          std::cout << "add $4, %esp" << std::endl;
        }
    }


    std::cout << "movl %eax, %esi" << std::endl;
    // restore %eax %ecx %edx
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %ecx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    // push the result to stack
    std::cout << "push %esi" << std::endl;
}




void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {
    std::string variableName = node->identifier_1->name;
    std::string memberName = node ->identifier_2->name;

    std::string variableOffset = "";
    std::string objectName = "";

    std::cout << "# Member Access" << std::endl;

    VariableTable::iterator vi = currentMethodInfo.variables->find(variableName);

    // local variable
    if(vi != currentMethodInfo.variables->end()){
        CompoundType ct = (vi->second).type;

        assert(ct.baseType == bt_object);
        objectName = ct.objectClassName;

        variableOffset = std::to_string((vi->second).offset);

        std::cout << "movl " + variableOffset + "(%ebp), %eax" << std::endl;
        std::cout << "push %eax" << std::endl;
    }

    // class members
    if(objectName == ""){
        vi = currentClassInfo.members->find(variableName);
        if(vi != currentClassInfo.members->end()){
            CompoundType ct = (vi->second).type;

            assert(ct.baseType == bt_object);

            objectName = ct.objectClassName;
            variableOffset = std::to_string((vi->second).offset);

            //  push member value onto stack
            std::cout << "movl 8(%ebp), %edx" << std::endl; //self pointer
            std::cout << "movl " + variableOffset + "(%edx), %eax" << std::endl; // get the value;
            std::cout << "push %eax" << std::endl;    // push the value
        }
    }

    // get offset
    ClassTable::iterator ci = classTable->find(objectName);
    assert(ci != classTable->end());

    vi = (ci->second).members->find(memberName);
    std::string memberOffset = std::to_string((vi->second).offset);

    assert(variableOffset != "");
    assert(memberOffset != "");


    std::cout << "pop %edx" << std::endl; // get the base address of variable
    std::cout << "movl " + memberOffset + "(%edx), %eax" << std::endl;

    // push result to the stack
    std::cout << "push %eax" << std::endl;

}




void CodeGenerator::visitVariableNode(VariableNode* node) {
    std::string variableName = node->identifier->name;

    std::cout << "# Variable" << std::endl;
    // get offset
    std::string offset = "";

    // local variable or parameter
    VariableTable::iterator vi = currentMethodInfo.variables->find(variableName);
    if(vi != currentMethodInfo.variables->end()){
        offset = std::to_string((vi->second).offset);
        // push variable value onto stack
        std::cout << "movl " + offset + "(%ebp), %eax" << std::endl;
        std::cout << "push %eax" << std::endl;
    } else {    // class members
        vi = currentClassInfo.members->find(variableName);
        assert(vi != currentClassInfo.members->end());

        // offset from object base address
        offset = std::to_string((vi->second).offset);

        //  push member value onto stack
        std::cout << "movl 8(%ebp), %edx" << std::endl; //self pointer
        std::cout << "movl " + offset + "(%edx), %eax" << std::endl; // get the value;
        // push result to the stack
        std::cout << "push %eax" << std::endl;    // push the value

    }
}





void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
    std::cout << "# Integer Literal" << std::endl;
    node->visit_children(this);
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
    std::cout << "# Boolean Literal" << std::endl;
    node->visit_children(this);
}



void CodeGenerator::visitNewNode(NewNode* node) {

    std::cout << "# New" << std::endl;

    std::string className = node->identifier->name;

    ClassInfo ci = classTable->find(className)->second;
    std::string size = std::to_string(ci.membersSize);

    // malloc space for object
    std::cout << "push $" + size << std::endl;
    std::cout << "call malloc" << std::endl;
    std::cout << "add $4, %esp" << std::endl;

    if(ci.methods->find(className) == ci.methods->end()){
        //push the address to stack
        std::cout << "push %eax" << std::endl;
        // no constructor, return
        return;
    }



    // save %eax %ecx %edx
    std::cout << "push %eax" << std::endl;
    std::cout << "push %ecx" << std::endl;
    std::cout << "push %edx" << std::endl;

    std::cout << "movl %eax, %esi" << std::endl;

    // push parameter in reverse order, offset 12
    if(node->expression_list) {
        for(std::list<ExpressionNode*>::reverse_iterator iter = node->expression_list->rbegin();
            iter != node->expression_list->rend(); iter++) {
          (*iter)->accept(this);
        }
    }

    // self pointer, offset 8
    std::cout << "push %esi" << std::endl;
    // return address, offset 4
    // save by call
    // jump to function
    std::cout << "call " + className + "_" + className << std::endl;

    // remove parameters
    std::cout << "add $4, %esp" << std::endl;

    if(node->expression_list) {
        for(std::list<ExpressionNode*>::iterator iter = node->expression_list->begin();
            iter != node->expression_list->end(); iter++) {
          std::cout << "add $4, %esp" << std::endl;
      }
    }

    // restore %eax %ecx %edx
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %ecx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    //push the address to stack
    std::cout << "push %esi" << std::endl;
}




void CodeGenerator::visitIntegerTypeNode(IntegerTypeNode* node) {
    // empty
}

void CodeGenerator::visitBooleanTypeNode(BooleanTypeNode* node) {
    // empty
}

void CodeGenerator::visitObjectTypeNode(ObjectTypeNode* node) {
    // empty
}

void CodeGenerator::visitNoneNode(NoneNode* node) {
    // empty
}

void CodeGenerator::visitIdentifierNode(IdentifierNode* node) {
    // empty
}





void CodeGenerator::visitIntegerNode(IntegerNode* node) {

    std::string val = std::to_string(node->value);
    // push the value to stack
    std::cout << "mov $" + val + ", " + "%eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}
