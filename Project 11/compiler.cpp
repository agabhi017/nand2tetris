#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <ctype.h>  //isspace
#include <unordered_map>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

enum tokenType{
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STRING_CONST,
    INVALID
};

enum keywordType{
    _CLASS,
    _METHOD,
    _FUNCTION,
    _CONSTRUCTOR,
    _INT,
    _BOOLEAN,
    _CHAR,
    _VOID,
    _VAR,
    _STATIC,
    _FIELD,
    _LET,
    _DO,
    _IF,
    _ELSE,
    _WHILE,
    _RETURN,
    _TRUE,
    _FALSE,
    _NULL,
    _THIS
};

enum varKind{
    VAR_STATIC,
    VAR_FIELD,
    VAR_VAR,
    VAR_ARG,
    VAR_NONE
};

std::unordered_map <std::string, keywordType> keyword_map;

static inline bool isKeywordConstant(const std::string& keyword){
    if (keyword == "true" || keyword == "false" || keyword == "null" || keyword == "this")      return true;
    return false;
}

static inline bool isUnaryOp(char symbol){
    if (symbol == '-' || symbol == '~')     return true;
    return false;
}

static inline bool isOp(char symbol){
    switch (symbol)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '&':
        case '|':
        case '<':
        case '>':
        case '=':
            return true;
            break;
        
        default:
            return false;
            break;
    }
}

static inline void initKeywordMap(){
    keyword_map["class"] = _CLASS;
    keyword_map["constructor"] = _CONSTRUCTOR;
    keyword_map["function"] = _FUNCTION;
    keyword_map["method"] = _METHOD;
    keyword_map["field"] = _FIELD;
    keyword_map["static"] = _STATIC;
    keyword_map["var"] = _VAR;
    keyword_map["int"] = _INT;
    keyword_map["char"] = _CHAR;
    keyword_map["boolean"] = _BOOLEAN;
    keyword_map["void"] = _VOID;
    keyword_map["true"] = _TRUE;
    keyword_map["false"] = _FALSE;
    keyword_map["null"] = _NULL;
    keyword_map["this"] = _THIS;
    keyword_map["let"] = _LET;
    keyword_map["do"] = _DO;
    keyword_map["if"] = _IF;
    keyword_map["else"] = _ELSE;
    keyword_map["while"] = _WHILE;
    keyword_map["return"] = _RETURN;
}



class tokenizer{
    public:
        tokenizer(const std::string& filename){
            m_infile.open(filename);
            if (!m_infile.is_open()){
                std::exit(1);
            }
            m_backup_token = '\0';
        }

        ~tokenizer(){
            m_infile.close();
        }
        
        bool hasMoreTokens(){
            if (!m_infile.eof())    return true;
            else                    return false;
        }

        void resetFields(){
            m_string_val = "";
            m_symbol = '\0';
            m_current_keyword_or_identifier = "";
        }

        void advance(){
            if (hasMoreTokens()){
                resetFields();
                char temp_token;
                if (m_backup_token == '\0')         m_infile.get(temp_token);
                else {
                    temp_token = m_backup_token;
                    m_backup_token = '\0';
                }
                while (hasMoreTokens() && std::isspace(temp_token))        m_infile.get(temp_token);          // skip white spaces and new lines
                
                if (isalpha(temp_token)){   // either a keyword or an identifier
                    std::string word; 
                    while (!std::isspace(temp_token) && (isalpha(temp_token) || isdigit(temp_token) || temp_token == '_')){
                        word += temp_token;
                        m_infile.get(temp_token);
                    }
                    m_backup_token = temp_token;
                    if (keyword_map.find(word) != keyword_map.end()){
                        m_current_token_type = KEYWORD;
                        m_current_keyword_type = keyword_map[word];
                    }
                    else {
                        m_current_token_type = IDENTIFIER;
                    }
                    m_current_keyword_or_identifier = word;
                }
                else if (isdigit(temp_token)){
                    std::string word;
                    while (!std::isspace(temp_token) && isdigit(temp_token)){
                        word += temp_token;
                        m_infile.get(temp_token);
                    }
                    m_backup_token = temp_token;
                    m_current_token_type = INT_CONST;
                    m_int_val = std::stoi(word);
                }
                else if (temp_token == '"'){
                    std::string word;
                    m_infile.get(temp_token);
                    while (temp_token != '"'){
                        word += temp_token;
                        m_infile.get(temp_token);
                    }
                    m_current_token_type = STRING_CONST;
                    m_string_val = word;
                }
                else if (temp_token == '/'){
                    m_infile.get(m_backup_token);
                    if (m_backup_token == '/'){
                        while (temp_token != '\n'){
                            m_infile.get(temp_token);
                        }
                        m_backup_token = '\0';
                        m_current_token_type = INVALID;
                    }
                    else if (m_backup_token == '*'){
                        std::string comment_end = "*/", t_str("  ");
                        m_infile.get(t_str[0]);
                        m_infile.get(t_str[1]);
                        while (t_str != comment_end){
                            t_str[0] = t_str[1];
                            m_infile.get(t_str[1]);
                        }
                        m_backup_token = '\0';
                        m_current_token_type = INVALID;
                    }
                    else {
                        // not checking for any unidentfied symbols here, it is easy tho
                        m_symbol = temp_token;
                        m_current_token_type = SYMBOL;
                    }
                }
                else if (!isspace(temp_token)){
                    // not checking for any unidentfied symbols here, it is easy tho
                    m_symbol = temp_token;
                    m_current_token_type = SYMBOL;
                }
            }
        }

        const std::string stringVal(){
            return m_string_val;
        }

        const char symbol(){
            return m_symbol;
        }

        const int intVal(){
            return m_int_val;
        }

        const keywordType keyword(){
            return m_current_keyword_type;
        }

        const tokenType currentTokenType(){
            return m_current_token_type;
        }

        const std::string keywordOrIdentifier(){
            return m_current_keyword_or_identifier;
        }

    private:
        std::ifstream       m_infile;
        tokenType           m_current_token_type;
        std::string         m_string_val;
        char                m_symbol;
        char                m_backup_token;
        int                 m_int_val;
        keywordType         m_current_keyword_type;
        std::string         m_current_keyword_or_identifier;
};

class symbolTable{
    public:
        symbolTable(){
            initIndices();
        }

        void initIndices(){
            m_running_index[VAR_ARG]    = 0;
            m_running_index[VAR_FIELD]  = 0;
            m_running_index[VAR_STATIC] = 0;
            m_running_index[VAR_VAR]    = 0;
        }

        void reset(){
            initIndices();
            m_var_type.clear();
            m_var_kind.clear();
            m_var_index.clear();
        }

        void define(const std::string& name, const std::string& type, varKind kind){
            m_var_type[name] = type;
            m_var_kind[name] = kind;
            m_var_index[name] = m_running_index[kind];
            m_running_index[kind]++;
        }

        int varCount(varKind kind){
            int var_count = 0;
            for (auto var : m_var_kind){
                if (var.second == kind)     ++var_count;
            }
            return var_count;
        }

        varKind kindOf(const std::string& name){
            if (m_var_kind.find(name) != m_var_kind.end())      return m_var_kind[name];
            return VAR_NONE;
        }

        std::string typeOf(const std::string& name){
            return m_var_type[name];
        }

        int indexOf(const std::string& name){
            return m_var_index[name];
        }
    private:
        std::unordered_map <std::string, std::string>           m_var_type;
        std::unordered_map <std::string, varKind>               m_var_kind;
        std::unordered_map <std::string, int>                   m_var_index;
        std::unordered_map <varKind, int>                       m_running_index;    
};

class VMWriter{
    public:
        VMWriter(const std::string& filename){
            std::string fname = filename.substr(0, filename.size() - 5) + ".vm";
            m_outfile.open(fname);
            if (!m_outfile.is_open()){
                std::exit(1);
            }
        }

        ~VMWriter(){
            m_outfile.close();
        }

        void writePush(const std::string& segment, int index){
            m_outfile << "push " << segment << " " << index << "\n";
        }

        void writePop(const std::string& segment, int index){
            m_outfile << "pop " << segment << " " << index << "\n";
        }

        void writeArithmetic(const std::string& command){
            m_outfile << command << "\n";
        }

        void writeLabel(const std::string& label){
            m_outfile << "label " << label << "\n";
        }

        void writeGoto(const std::string& label){
            m_outfile << "goto " << label << "\n";
        }

        void writeIf(const std::string& label){
            m_outfile << "if-goto " << label << "\n";
        }

        void writeCall(const std::string& name, int n_args){
            m_outfile << "call " << name << " " << n_args << "\n";
        }

        void writeFunction(const std::string& name, int n_vars){
            m_outfile << "function " << name << " " << n_vars << "\n";
        }

        void writeReturn(){
            m_outfile << "return\n";
        }
    private:
        std::ofstream               m_outfile;
};

class compileEngine{
    public:
        compileEngine(const std::string& filename) : m_tokenizer(filename), m_writer(filename){
            do {
                m_tokenizer.advance();
            }
            while (m_tokenizer.currentTokenType() == INVALID);
        }

        void process(){
            do {
                m_tokenizer.advance();
            }
            while (m_tokenizer.currentTokenType() == INVALID);
        }

        void compileClass(){    
            m_if_label_index = 0;
            m_while_label_index = 0;
            m_field_count = 0;
            process();          // "class"
            m_class_name = m_tokenizer.keywordOrIdentifier();
            process();          // class name
            process();          // "{"
            while (m_tokenizer.keyword() == _STATIC || m_tokenizer.keyword() == _FIELD){
                compileClassVarDec();   
            }
            while (m_tokenizer.currentTokenType() == KEYWORD && (m_tokenizer.keyword() == _CONSTRUCTOR || m_tokenizer.keyword() == _FUNCTION || m_tokenizer.keyword() == _METHOD)){
                compileSubroutine();
            }
            process();          // "}"
        }

        void compileClassVarDec(){     
            varKind var_kind;
            if (m_tokenizer.keywordOrIdentifier() == "static")      var_kind = VAR_STATIC;
            else                                                    var_kind = VAR_FIELD;
            process();     // var kind (static/ field)
            m_var_type = m_tokenizer.keywordOrIdentifier();
            process();     // var type
            m_var_name = m_tokenizer.keywordOrIdentifier();
            m_class_symbol_table.define(m_var_name, m_var_type, var_kind);
            process();     // var name
            ++m_field_count;
            while (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == ','){
                process();  // ","
                m_var_name = m_tokenizer.keywordOrIdentifier();
                m_class_symbol_table.define(m_var_name, m_var_type, var_kind);
                process();  // var name
                ++m_field_count;
            }
            process();      // ";"
        }

        void compileParameter(){        
            ++m_param_count;
            m_var_type = m_tokenizer.keywordOrIdentifier();
            process();      // type
            m_var_name = m_tokenizer.keywordOrIdentifier();
            process();      // name
            m_function_symbol_table.define(m_var_name, m_var_type, VAR_ARG);
        }

        void compileSubroutine(){       
            m_is_method = false;
            m_is_constructor = false;
            m_is_void_function = false;
            m_param_count = 0;
            m_var_count = 0;
            m_function_symbol_table.reset();

            if (m_tokenizer.keywordOrIdentifier() == "method")              m_is_method = true;
            else if (m_tokenizer.keywordOrIdentifier() == "constructor")    m_is_constructor = true;

            if (m_is_method){
                m_function_symbol_table.define("this", m_class_name, VAR_ARG);
            }

            process();      // method/constructor/routine
            if (m_tokenizer.keywordOrIdentifier() == "void")        m_is_void_function = true;
            process();      // void/return type
            m_current_function_name = m_class_name + "." + m_tokenizer.keywordOrIdentifier();
            process();      // subroutine name
            process();      // "("
            compileParameterList();
            process();      // ")"      
            compileSubroutineBody();               
        }

        void compileParameterList(){        
            if (m_tokenizer.currentTokenType() != SYMBOL){
                compileParameter();
                while (m_tokenizer.symbol() == ','){
                    process();      // ","
                    compileParameter();
                }
            }
        }

        void compileSubroutineBody(){       
            process();      // "{"
            while (m_tokenizer.currentTokenType() == KEYWORD && m_tokenizer.keywordOrIdentifier() == "var"){
                compileVarDec();
            }
            m_writer.writeFunction(m_current_function_name, m_var_count);
            if (m_is_method){
                m_writer.writePush("argument", 0);
                m_writer.writePop("pointer", 0);
            }
            else if (m_is_constructor){
                m_writer.writePush("constant", m_field_count);
                m_writer.writeCall("Memory.alloc", 1);
                m_writer.writePop("pointer", 0);
            }
            compileStatements();
            process();      // "}"
        }

        void compileVarDec(){       
            ++m_var_count;
            process();      // "var"
            m_var_type = m_tokenizer.keywordOrIdentifier();
            process();      // var type
            m_var_name = m_tokenizer.keywordOrIdentifier();
            m_function_symbol_table.define(m_var_name, m_var_type, VAR_VAR);
            process();      // var name
            while (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == ','){
                process();  // ","
                m_var_name = m_tokenizer.keywordOrIdentifier();
                m_function_symbol_table.define(m_var_name, m_var_type, VAR_VAR);
                process();  // var name
                ++m_var_count;
            }
            process();      // ";"
        }

        void compileStatements(){       
            while (m_tokenizer.currentTokenType() == KEYWORD && (m_tokenizer.keyword() == _LET || m_tokenizer.keyword() == _IF 
                    || m_tokenizer.keyword() == _WHILE || m_tokenizer.keyword() == _DO || m_tokenizer.keyword() == _RETURN)){
                if (m_tokenizer.keyword() == _LET)          compileLet();
                else if (m_tokenizer.keyword() == _IF)      compileIf();
                else if (m_tokenizer.keyword() == _WHILE)   compileWhile();
                else if (m_tokenizer.keyword() == _DO)      compileDo();
                else                                        compileReturn();
            }
        }

        void compileLet(){  
            process();      // "let"
            std::string identifer_name{m_tokenizer.keywordOrIdentifier()};
            process();      // name
            bool is_array = false;
            if (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == '['){
                m_writer.writePush(getMemorySegment(identifer_name), getVarIndex(identifer_name));
                process();  // "["
                compileExpression();
                process();  // "]"
                m_writer.writeArithmetic("add");
                is_array = true;
            }
            process();      // "="
            compileExpression();
            if (is_array){
                m_writer.writePop("temp", 0);
                m_writer.writePop("pointer", 1);
                m_writer.writePush("temp", 0);
                m_writer.writePop("that", 0);
            }
            else {
                m_writer.writePop(getMemorySegment(identifer_name), getVarIndex(identifer_name));
            }
            process();  // ";"
        }

        void compileIf(){   
            std::string label1{"IF_TRUE_" + std::to_string(m_if_label_index)};
            std::string label2{"IF_FALSE_" + std::to_string(m_if_label_index)};
            ++m_if_label_index;
            process();      // if
            process();      // "("
            compileExpression();
            process();      // ")"
            process();      // "{"
            m_writer.writeArithmetic("not");
            m_writer.writeIf(label1);
            compileStatements();
            m_writer.writeGoto(label2);
            process();      // "}"
            m_writer.writeLabel(label1);
            if (m_tokenizer.currentTokenType() == KEYWORD && m_tokenizer.keywordOrIdentifier() == "else"){
                process();      // else
                process();      // "{"
                compileStatements();
                process();      // "}"
            }
            m_writer.writeLabel(label2);
        }

        void compileWhile(){    
            std::string label1{"LOOP_START_" + std::to_string(m_while_label_index)};
            std::string label2{"LOOP_END_" + std::to_string(m_while_label_index)};
            ++m_while_label_index;
            process();  // while
            process();  // "("
            m_writer.writeLabel(label1);
            compileExpression();
            process();  // ")"
            m_writer.writeArithmetic("not");
            m_writer.writeIf(label2);
            process();  // "{"
            compileStatements();
            process();  // "}"
            m_writer.writeGoto(label1);
            m_writer.writeLabel(label2);
        }

        void compileDo(){   
            process();      // do
            compilesubroutineCall();
            process();      // ";"
            m_writer.writePop("temp", 0);
        }

        void compileReturn(){   
            process();      // return
            if (m_tokenizer.currentTokenType() != SYMBOL || (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() != ';')){
                compileExpression();
            }
            process();      // ";"
            if (m_is_void_function)     m_writer.writePush("constant", 0);
            m_writer.writeReturn();
        }

        void compileExpression(){       
            compileTerm();
            while (m_tokenizer.currentTokenType() == SYMBOL && isOp(m_tokenizer.symbol())){
                char symbol = m_tokenizer.symbol();
                process();      // symbol
                compileTerm();
                switch (symbol)
                {
                    case '+':   m_writer.writeArithmetic("add");            break;
                    case '-':   m_writer.writeArithmetic("sub");            break;
                    case '*':   m_writer.writeCall("Math.multiply", 2);     break;
                    case '/':   m_writer.writeCall("Math.divide", 2);       break;
                    case '&':   m_writer.writeArithmetic("and");            break;
                    case '|':   m_writer.writeArithmetic("or");             break;
                    case '<':   m_writer.writeArithmetic("lt");             break;
                    case '>':   m_writer.writeArithmetic("gt");             break;
                    case '=':   m_writer.writeArithmetic("eq");             break;
                    default:    break;
                }
            }
        }

        void compileTerm(){
            if (m_tokenizer.currentTokenType() == INT_CONST){
                m_writer.writePush("constant", m_tokenizer.intVal());
                process();      // int constant
            }                
            else if (m_tokenizer.currentTokenType() == STRING_CONST){
                std::string str_constant = m_tokenizer.stringVal();
                m_writer.writePush("constant", str_constant.size());
                m_writer.writeCall("String.new", 1);
                for (char ch : str_constant){
                    m_writer.writePush("constant", (int)ch);
                    m_writer.writeCall("String.appendChar", 2);
                }
                process();      // str constant
            }        
            else if (m_tokenizer.currentTokenType() == KEYWORD && isKeywordConstant(m_tokenizer.keywordOrIdentifier())){
                if (m_tokenizer.keywordOrIdentifier() == "null" || m_tokenizer.keywordOrIdentifier() == "false"){
                    m_writer.writePush("constant", 0);
                }
                else if (m_tokenizer.keywordOrIdentifier() == "true"){
                    m_writer.writePush("constant", 1);
                    m_writer.writeArithmetic("neg");
                }
                else {
                    m_writer.writePush("pointer", 0);
                }
                process();      // this/null/true/false
            }
            else if (m_tokenizer.currentTokenType() == IDENTIFIER){
                std::string identifier_name{m_tokenizer.keywordOrIdentifier()};
                process();      // identifier
                if (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == '['){
                    m_writer.writePush(getMemorySegment(identifier_name), getVarIndex(identifier_name));
                    process();  // "["
                    compileExpression();
                    process();  // "]"
                    m_writer.writeArithmetic("add");
                    m_writer.writePop("pointer", 1);
                    m_writer.writePush("that", 0);
                }
                else if (m_tokenizer.currentTokenType() == SYMBOL && (m_tokenizer.symbol() == '(' || m_tokenizer.symbol() == '.')){  // handling subroutine call
                    if (m_tokenizer.symbol() == '('){
                        process();      // "("
                        compileExpressionList();
                        process();      // ")"
                        identifier_name = m_class_name + "." + identifier_name;
                        m_expression_count++;
                        m_writer.writePush("pointer", 0);
                    }
                    else {
                        process();      // "."
                        bool is_function = false;
                        bool inc_exp_count = false;

                        if (m_function_symbol_table.kindOf(identifier_name) == VAR_NONE && m_class_symbol_table.kindOf(identifier_name) == VAR_NONE)    is_function = true;
                        if (identifier_name != m_class_name && !is_function){     //method 
                            inc_exp_count = true;
                            m_writer.writePush(getMemorySegment(identifier_name), getVarIndex(identifier_name));
                            std::string var_type;
                            if (m_function_symbol_table.kindOf(identifier_name) != VAR_NONE)      var_type = m_function_symbol_table.typeOf(identifier_name);
                            else                                                                  var_type = m_class_symbol_table.typeOf(identifier_name);
                            identifier_name = var_type + "." + m_tokenizer.keywordOrIdentifier();
                        }
                        else {
                            identifier_name += "." + m_tokenizer.keywordOrIdentifier();
                        }
                        process();      // subroutine name
                        process();      // "("
                        compileExpressionList();
                        process();      // ")"
                        if (inc_exp_count)      ++m_expression_count;
                    }
                    m_writer.writeCall(identifier_name, m_expression_count);
                }
                else {      // handling variable names only
                    m_writer.writePush(getMemorySegment(identifier_name), getVarIndex(identifier_name));
                }
            }
            else if (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == '('){
                process();      // "("
                compileExpression();
                process();      // ")"
            }
            else if (m_tokenizer.currentTokenType() == SYMBOL && isUnaryOp(m_tokenizer.symbol())){
                char symbol = m_tokenizer.symbol();
                process();      // unary op
                compileTerm();
                if (symbol == '-')      m_writer.writeArithmetic("neg");
                else                    m_writer.writeArithmetic("not");
            }
        }

        void compileExpressionList(){       
            m_expression_count = 0;
            if (m_tokenizer.currentTokenType() != SYMBOL || (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() != ')')){
                compileExpression();
                ++m_expression_count;
                while (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == ','){
                    process();      // ","
                    compileExpression();
                    ++m_expression_count;
                }
            }
        }

        void compilesubroutineCall(){
            std::string function_name{m_tokenizer.keywordOrIdentifier()};
            process();  // name
            if (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == '('){
                m_writer.writePush("pointer", 0);
                process();      // "("
                compileExpressionList();
                process();      // ")"
                function_name = m_class_name + "." + function_name;
                m_expression_count++;
            }
            else {
                process();      // "."
                bool is_function = false;
                bool inc_exp_count = false;
                if (m_function_symbol_table.kindOf(function_name) == VAR_NONE && m_class_symbol_table.kindOf(function_name) == VAR_NONE)    is_function = true;
                if (function_name != m_class_name && !is_function){     //method 
                    inc_exp_count = true;
                    m_writer.writePush(getMemorySegment(function_name), getVarIndex(function_name));
                    std::string var_type;
                    if (m_function_symbol_table.kindOf(function_name) != VAR_NONE)      var_type = m_function_symbol_table.typeOf(function_name);
                    else                                                                var_type = m_class_symbol_table.typeOf(function_name);
                    function_name = var_type + "." + m_tokenizer.keywordOrIdentifier();
                }
                else {
                    function_name += "." + m_tokenizer.keywordOrIdentifier();
                }
                process();      // name
                process();      // "("
                compileExpressionList();
                process();      // ")"
                if (inc_exp_count)      ++m_expression_count;
            }
            m_writer.writeCall(function_name, m_expression_count);
        }

        std::string getMemorySegment(const std::string& var_name){
            if (m_function_symbol_table.kindOf(var_name) != VAR_NONE){
                if (m_function_symbol_table.kindOf(var_name) == VAR_VAR)             return "local";
                else if (m_function_symbol_table.kindOf(var_name) == VAR_ARG)        return "argument";
            }
            else {
                if (m_class_symbol_table.kindOf(var_name) == VAR_STATIC)             return "static";
                else if (m_class_symbol_table.kindOf(var_name) == VAR_FIELD)         return "this";
            }
        }

        int getVarIndex(const std::string& var_name){
            if (m_function_symbol_table.kindOf(var_name) != VAR_NONE)                return m_function_symbol_table.indexOf(var_name);
            else                                                                     return m_class_symbol_table.indexOf(var_name);
        }

    private:
        tokenizer           m_tokenizer;
        symbolTable         m_class_symbol_table;
        symbolTable         m_function_symbol_table;
        VMWriter            m_writer;
        std::string         m_class_name;
        std::string         m_current_function_name;
        bool                m_is_void_function;
        bool                m_is_constructor;
        bool                m_is_method;
        int                 m_field_count;
        int                 m_param_count;
        int                 m_var_count;
        std::string         m_var_type;
        std::string         m_var_name;
        int                 m_expression_count;
        int                 m_if_label_index;
        int                 m_while_label_index;
};

class compiler{
    public:
        compiler(const std::string& filename){
            std::vector <std::string> filenames;
            if (filename.substr(filename.size() - 4, 4) == "jack"){
                filenames.push_back(filename);
            }
            else {
                std::string ext = ".jack";
                for (const auto& fpath : fs::directory_iterator(filename)){
                    if (fpath.path().extension() == ext){
                        filenames.push_back(fpath.path());
                    }
                }
            }
            for (std::string& fname : filenames){
                compileEngine compile_engine(fname);
                compile_engine.compileClass();
            }
        }
};

int main(int argc, char* argv[]){
    if (argc != 2){ // Impose correct usage
        std::cout << "Incorrect usage\n";
        return 1;
    }

    initKeywordMap();
    compiler new_compiler(argv[1]);
    return 0;
}