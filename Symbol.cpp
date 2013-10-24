#include "SymbolTable.h"

using namespace std;

// class Symbol

Symbol::Symbol( const string &n ) : name(n) {
    
}

Symbol::~Symbol() {

}

// class VarType

vector<VarType*> VarType::stack;
vector<VarType*> VarType::atomTypes;

VarType::VarType( const VarType& vt ) : Symbol(vt.getName()), depth(vt.depth), compoundLevel(vt.compoundLevel),
    width(vt.width), memberList(vt.memberList), atomType(vt.atomType), size(vt.size) {

}


VarType::VarType( const string &n, int d, CompoundLevel cl, vector<int> *w, SymbolTable *ml )
    : Symbol(n), depth(d), compoundLevel(cl), width(w), memberList(ml), atomType(AtomTypesIndex::USER_TYPE) {
    int factor = 1;
    if( width != NULL ) {
        for( size_t i = 0 ; i < width->size() ; ++i ) {
            factor *= (*width)[i];
        }
    }

    if( d == 0 ) {
        size = 0;
        if ( cl == CompoundLevel::UNION ) { // find the size of the largest one
            for( int i = 0 ; i < ml->size() ; ++i ) {
                if ( size < dynamic_cast<VarType*>(ml->at(i))->size ) {
                    size = dynamic_cast<VarType*>(ml->at(i))->size;
                }
            }
        } else {    // calculate the sum of every elements' size
            for( int i = 0 ; i < ml->size() ; ++i ) {
                size += dynamic_cast<VarType*>(ml->at(i))->size;
            }
        }
    } else {    // is a pointer
        size = VarType::WORD_LENGTH;
    }

    size *= factor;    
}

VarType::VarType( const string &n, int s, AtomTypesIndex a )
    : Symbol(n), depth(0), compoundLevel(CompoundLevel::NONE), size(s), 
        width(NULL), memberList(NULL), atomType(a) {

}

VarType::~VarType() {
    // delete width;
    // delete memberList;   // they will be defined types and be deleted later
    if( stack.size() != 0 && this == stack.back() ) {
        stack.pop_back();
    }
}

Symbol::IDstate VarType::attach( SymbolTable *st ) const {
    this->scope = st;
    stack.push_back( const_cast<VarType*>(this) );
    st->addSymbol( const_cast<VarType*>(this) );
    return IDstate::OK;
}

bool VarType::isEqual( const Symbol *symbol ) const {
    if( typeid(*symbol) == typeid(*this) ) {
        const VarType *pvt = dynamic_cast<const VarType*>(symbol);
        bool ret = true;
        //ret &= ( this->getName() == pvt->getName() );   // same type does not need a same name
        ret &= ( this->depth == pvt->depth );
        ret &= ( this->compoundLevel == pvt->compoundLevel );
        ret &= ( ( this->width == NULL && pvt->width == NULL ) || 
            ( this->width != NULL && pvt->width != NULL && *(this->width) == *(pvt->width) ) );
        ret &= ( ( this->memberList == NULL && pvt->memberList == NULL ) || 
            ( this->memberList != NULL && pvt->memberList != NULL && this->memberList->isEqual( pvt->memberList ) ) );
        return ret;
    } else {
        return false;
    }
}

VarType::CompoundLevel VarType::getCompoundLevel() {
    return compoundLevel;
}


SymbolTable * VarType::getMemberList() {
    return memberList;
}

int VarType::getPointerDepth() const {
    return depth;
}

// Pointers will always be reguarded as USER_DEFINED_TYPE and won't be converted
bool VarType::atomType2ExprType( AtomTypesIndex atomType, ExprType &exprType ) {
    switch( atomType ) {
        case VarType::AtomTypesIndex::CHAR:
            exprType = ExprType::CHARACTER;
            break;
        case VarType::AtomTypesIndex::INT:
        case VarType::AtomTypesIndex::SHORT:
        case VarType::AtomTypesIndex::SIGNED:
        case VarType::AtomTypesIndex::LONG:
            exprType = ExprType::INTEGER;
            break;
        case VarType::AtomTypesIndex::UNSIGNED:
            exprType = ExprType::U_INTEGER;
            break;
        case VarType::AtomTypesIndex::FLOAT:
        case VarType::AtomTypesIndex::DOUBLE:
            exprType = ExprType::REAL;
        default:
            return false;
    }
    return true;
}

VarType * VarType::find( const std::string &name ) {
    for( int i = stack.size()-1 ; i >= 0 ; --i ) {
        if( name.compare( stack[i]->getName() ) == 0 ) {
            return stack[i];
        }
    }
    return NULL;
}

void VarType::initAtomTypes() {
    atomTypes.push_back( new VarType( "c1", 1, AtomTypesIndex::CHAR ) );
    atomTypes.push_back( new VarType( "i4", WORD_LENGTH, AtomTypesIndex::INT ) );
    atomTypes.push_back( new VarType( "i4", WORD_LENGTH, AtomTypesIndex::LONG ) );
    atomTypes.push_back( new VarType( "i2", WORD_LENGTH/2, AtomTypesIndex::SHORT ) );
    atomTypes.push_back( new VarType( "f4", WORD_LENGTH, AtomTypesIndex::FLOAT ) );
    atomTypes.push_back( new VarType( "f8", WORD_LENGTH*2, AtomTypesIndex::DOUBLE ) );
    atomTypes.push_back( new VarType( "i4", WORD_LENGTH, AtomTypesIndex::SIGNED ) );
    atomTypes.push_back( new VarType( "u4", WORD_LENGTH, AtomTypesIndex::UNSIGNED ) );
    atomTypes.push_back( new VarType( "v0", 0, AtomTypesIndex::VOID ) );
}

// class Variable

vector<Variable*> Variable::stack;

Variable::Variable( const string &n, VarType* t, Qualifier q )
    : Symbol(n), type(t), qualifier(q) {
    // calculate 'addr' after optimazation ?
}

Variable::~Variable() {
    // delete type;
    if( stack.size() != 0 && this == stack.back() ) {
        stack.pop_back();
    }
}

Variable * Variable::find( const std::string &name ) {
    for( int i = stack.size()-1 ; i >= 0 ; --i ) {
        if( name.compare( stack[i]->getName() ) == 0 ) {
            return stack[i];
        }
    }
    return NULL;
}

// VAR的int+VAR的double不兼容？可能是yacc中对临时变量构造Variable的问题？
bool Variable::isCompatConv( Expression s, Expression t ) {
    int pointerCount = 0;
    if( s.type == VarType::ExprType::VAR ) {
        VarType *pvt = s.pvar->type;
        // cast to atom type. if it is compound type, then it is not a compatible convert
        for( ; pvt->getMemberList() != NULL; pvt = dynamic_cast<VarType*>( pvt->getMemberList()->at(0) ) ) {
            if( pvt->getPointerDepth() > 0 ) {
                pointerCount &= 1;
                break;
            } else if( pvt->getCompoundLevel() != VarType::CompoundLevel::NONE ) {
                return false;
            }
        }
        VarType::atomType2ExprType( pvt->atomType, s.type );
    }

    if( t.type == VarType::ExprType::VAR ) {
        VarType *pvt = t.pvar->type;
        // cast to atom type. if it is compound type, then it is not a compatible convert
        for( ; pvt->getMemberList() != NULL; pvt = dynamic_cast<VarType*>( pvt->getMemberList()->at(0) ) ) {
            if( pvt->getPointerDepth() > 0 ) {
                pointerCount &= 2;
                break;
            } else if( pvt->getCompoundLevel() != VarType::CompoundLevel::NONE ) {
                return false;
            }
        }
        VarType::atomType2ExprType( pvt->atomType, t.type );
    }

    if( s.type == VarType::ExprType::STRING || t.type == VarType::ExprType::STRING ) {
        return false;
    } else if( s.type == t.type ) {
        if( s.type != VarType::ExprType::VAR || s.pvar->type->isEqual( t.pvar->type ) ) {
            return true;
        } else {
            return false;
        }
    } else if( s.type == VarType::ExprType::CHARACTER ) {
        s.type = VarType::ExprType::INTEGER;
        return isCompatConv( s, t );
    } else if( s.type == VarType::ExprType::INTEGER ) {
        s.type = VarType::ExprType::U_INTEGER;
        return isCompatConv( s, t );
    } else if( s.type == VarType::ExprType::U_INTEGER ) {
        s.type = VarType::ExprType::REAL;
        return isCompatConv( s, t );
    } else if( s.type == VarType::ExprType::REAL ) {
        return false;
    } else if( t.type == VarType::ExprType::VAR && s.type != VarType::ExprType::REAL ) {
        return true;    // if $t is a VAR, then it can only be a pointer and can operate with integer
    } else {
        return false;
    }
}


Symbol::IDstate Variable::attach( SymbolTable *st ) const {
    Variable *pf = find( this->getName() );
    this->scope = st;
    if( pf == NULL ) {
        stack.push_back( const_cast<Variable*>(this) );
        st->addSymbol( const_cast<Variable*>(this) );
        return IDstate::OK;
    } else if( !( this->isSameScope( pf ) ) ) {
        stack.push_back( const_cast<Variable*>(this) );
        st->addSymbol( const_cast<Variable*>(this) );
        return IDstate::OVERRIDE;
    } else {
        return IDstate::CONFLICT;
    }
}

bool Variable::isEqual( const Symbol *symbol ) const {
    if( typeid(*symbol) == typeid(*this) ) {
        const Variable *var = dynamic_cast<const Variable*>(symbol);
        bool ret = true;
        ret &= ( this->getName() == var->getName() );
        ret &= ( this->type->isEqual( var->type ) ); // type can not be NULL
        return ret;
    } else {
        return false;
    }
}

std::string Variable::getTypeName() const {
    return type->getName();
}

const VarType& Variable::getType() const {
    return *type;
}



// class Function

vector<Function*> Function::stack;

Function::Function( const string &n, VarType *rt, SymbolTable *al ) 
    : Symbol(n), returnType(rt), argList(al) {
    // addr = 0;    size = 0;
}

Function::~Function() {
    delete returnType;
    delete argList;
    if( stack.size() != 0 && this == stack.back() ) {
        stack.pop_back();
    }
}

Symbol::IDstate Function::attach( SymbolTable *st ) const {
    Function *pf = find( this->getName() );
    this->scope = st;
    if( pf == NULL ) {
        stack.push_back( const_cast<Function*>(this) );
        st->addSymbol( const_cast<Function*>(this) );
        return IDstate::OK;
    } else if( !( this->isSameScope( pf ) ) ) {
        stack.push_back( const_cast<Function*>(this) );
        st->addSymbol( const_cast<Function*>(this) );
        return IDstate::OVERRIDE;
    } else if( pf->isEqual( this ) ) {
        return IDstate::DUPLICATE;
    } else {
        return IDstate::CONFLICT;
    }
}

bool Function::isEqual( const Symbol *symbol ) const {
    if( typeid(*symbol) == typeid(*this) ) {
        const Function *pf = dynamic_cast<const Function*>(symbol);
        bool ret = true;
        ret &= ( this->getName() == pf->getName() );
        ret &= ( this->returnType->isEqual( pf->returnType ) ); // returnType can not be NULL
        ret &= ( ( this->argList == NULL && pf->argList == NULL ) || 
            ( this->argList != NULL && pf->argList != NULL && this->argList->isEqual( pf->argList ) ) );
        return ret;
    } else {
        return false;
    }
}

Function * Function::find( const std::string &name ) {
    for( int i = stack.size()-1 ; i >= 0 ; --i ) {
        if( name.compare( stack[i]->getName() ) == 0 ) {
            return stack[i];
        }
    }
    return NULL;
}



// class Label

vector<Label*> Label::stack;
const std::string Label::autoNamePrefix("CC_AUTO_GEN_LABEL");
string Label::autoName("CC_AUTO_GEN_LABEL0");
int Label::loopCount(0);

Label::Label( int a ) : Symbol( autoName ), addr(a) {
    // prepare next label name for loop structure
    stringstream ss( autoNamePrefix );
    ss << ++loopCount;
    autoName = ss.str();
}


Label::Label( const string &n, int a ) : Symbol(n), addr(a) {

}

Label::~Label() {
    if( stack.size() != 0 && this == stack.back() ) {
        stack.pop_back();
    }
}

Symbol::IDstate Label::attach( SymbolTable *st ) const {
    Label *label = find( this->getName() );
    this->scope = st;
    if( label == NULL ) {
        stack.push_back( const_cast<Label*>(this) );
        st->addSymbol( const_cast<Label*>(this) );
        return IDstate::OK;
    } else if( !( this->isSameScope( label ) ) ) {
        stack.push_back( const_cast<Label*>(this) );
        st->addSymbol( const_cast<Label*>(this) );
        return IDstate::OVERRIDE;
    } else {    // a label can be specified by name and scope, so isEqual() call is not needed
        return IDstate::CONFLICT;
    }
}

Label * Label::find( const std::string &name ) {
    for( int i = stack.size()-1 ; i >= 0 && stack[i]->scope == SymbolTable::getCurrentScope() ; --i ) {
        if( name.compare( stack[i]->getName() ) == 0 ) {
            return stack[i];
        }
    }
    return NULL;
}
