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

        const std::string keywordOrIdentifer(){
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

class compileEngine{
    public:
        compileEngine(const std::string& filename) : m_tokenizer(filename){
            std::string fname = filename.substr(0, filename.size() - 5) + "1.xml";
            m_outfile.open(fname);
            if (!m_outfile.is_open()){
                std::exit(1);
            }
            do {
                m_tokenizer.advance();
            }
            while (m_tokenizer.currentTokenType() == INVALID);
        }

        ~compileEngine(){
            m_outfile.close();
        }

        void process(const std::string& word){
            if (m_tokenizer.currentTokenType() == KEYWORD){
                m_outfile << "<keyword> " << word << " </keyword>\n";
            }
            else if (m_tokenizer.currentTokenType() == SYMBOL){
                m_outfile << "<symbol> " << word << " </symbol>\n";
            }
            else if (m_tokenizer.currentTokenType() == IDENTIFIER){
                m_outfile << "<identifier> " << word << " </identifier>\n";
            }
            else if (m_tokenizer.currentTokenType() == STRING_CONST){
                m_outfile << "<stringConstant> " << word << " </stringConstant>\n";
            }
            else if (m_tokenizer.currentTokenType() == INT_CONST){
                m_outfile << "<integerConstant> " << word << " </integerConstant>\n";
            }
            do {
                m_tokenizer.advance();
            }
            while (m_tokenizer.currentTokenType() == INVALID);
        }

        void compileClass(){
            m_outfile << "<class>\n";
            process("class");
            process(m_tokenizer.keywordOrIdentifer());
            process("{");
            while (m_tokenizer.keyword() == _STATIC || m_tokenizer.keyword() == _FIELD){
                compileClassVarDec();
            }
            while (m_tokenizer.currentTokenType() == KEYWORD && (m_tokenizer.keyword() == _CONSTRUCTOR || m_tokenizer.keyword() == _FUNCTION || m_tokenizer.keyword() == _METHOD)){
                compileSubroutine();
            }
            process("}");
            m_outfile << "</class>\n";
        }

        void compileClassVarDec(){
            m_outfile << "<classVarDec>\n";
            process(m_tokenizer.keywordOrIdentifer());
            process(m_tokenizer.keywordOrIdentifer());
            process(m_tokenizer.keywordOrIdentifer());
            while (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == ','){
                process(",");
                process(m_tokenizer.keywordOrIdentifer());
            }
            process(";");
            m_outfile << "</classVarDec>\n";
        }

        void compileParameter(){
            process(m_tokenizer.keywordOrIdentifer());
            process(m_tokenizer.keywordOrIdentifer());
        }

        void compileSubroutine(){
            m_outfile << "<subroutineDec>\n";
            process(m_tokenizer.keywordOrIdentifer());
            process(m_tokenizer.keywordOrIdentifer());
            process(m_tokenizer.keywordOrIdentifer());
            process("(");
            compileParameterList();
            process(")");
            compileSubroutineBody();
            m_outfile << "</subroutineDec>\n";
        }

        void compileParameterList(){  
            m_outfile << "<parameterList>\n";
            if (m_tokenizer.currentTokenType() != SYMBOL){
                compileParameter();
                while (m_tokenizer.symbol() == ','){
                    process(",");
                    compileParameter();
                }
            }
            m_outfile << "</parameterList>\n";
        }

        void compileSubroutineBody(){
            m_outfile << "<subroutineBody>\n";
            process("{");
            while (m_tokenizer.currentTokenType() == KEYWORD && m_tokenizer.keywordOrIdentifer() == "var"){
                compileVarDec();
            }
            compileStatements();
            process("}");
            m_outfile << "</subroutineBody>\n";
        }

        void compileVarDec(){
            m_outfile << "<varDec>\n";
            process("var");
            process(m_tokenizer.keywordOrIdentifer());
            process(m_tokenizer.keywordOrIdentifer());
            while (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == ','){
                process(",");
                process(m_tokenizer.keywordOrIdentifer());
            }
            process(";");
            m_outfile << "</varDec>\n";
        }

        void compileStatements(){
            m_outfile << "<statements>\n";
            while (m_tokenizer.currentTokenType() == KEYWORD && (m_tokenizer.keyword() == _LET || m_tokenizer.keyword() == _IF 
                    || m_tokenizer.keyword() == _WHILE || m_tokenizer.keyword() == _DO || m_tokenizer.keyword() == _RETURN)){
                if (m_tokenizer.keyword() == _LET)          compileLet();
                else if (m_tokenizer.keyword() == _IF)      compileIf();
                else if (m_tokenizer.keyword() == _WHILE)   compileWhile();
                else if (m_tokenizer.keyword() == _DO)      compileDo();
                else                                        compileReturn();
            }
            m_outfile << "</statements>\n";
        }

        void compileLet(){
            m_outfile << "<letStatement>\n";
            process("let");
            process(m_tokenizer.keywordOrIdentifer());
            if (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == '['){
                process("[");
                compileExpression();
                process("]");
            }
            process("=");
            compileExpression();
            process(";");
            m_outfile << "</letStatement>\n";
        }

        void compileIf(){
            m_outfile << "<ifStatement>\n";
            process("if");
            process("(");
            compileExpression();
            process(")");
            process("{");
            compileStatements();
            process("{");
            if (m_tokenizer.currentTokenType() == KEYWORD && m_tokenizer.keywordOrIdentifer() == "else"){
                process("else");
                process("{");
                compileStatements();
                process("}");
            }
            m_outfile << "</ifStatement>\n";
        }

        void compileWhile(){
            m_outfile << "<whileStatement>\n";
            process("while");
            process("(");
            compileExpression();
            process(")");
            process("{");
            compileStatements();
            process("}");
            m_outfile << "</whileStatement>\n";
        }

        void compileDo(){
            m_outfile << "<doStatement>\n";
            process("do");
            compilesubroutineCall();
            process(";");
            m_outfile << "</doStatement>\n";
        }

        void compileReturn(){
            m_outfile << "<returnStatement>\n";
            process("return");
            if (m_tokenizer.currentTokenType() != SYMBOL || (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() != ';')){
                compileExpression();
            }
            process(";");
            m_outfile << "</returnStatement>\n";
        }

        void compileExpression(){
            m_outfile << "<expression>\n";
            compileTerm();
            while (m_tokenizer.currentTokenType() == SYMBOL && isOp(m_tokenizer.symbol())){
                process(std::string(1, m_tokenizer.symbol()));
                compileTerm();
            }
            m_outfile << "</expression>\n";
        }

        void compileTerm(){
            m_outfile << "<term>\n";
            if (m_tokenizer.currentTokenType() == INT_CONST)                process(std::to_string(m_tokenizer.intVal()));
            else if (m_tokenizer.currentTokenType() == STRING_CONST)        process(m_tokenizer.stringVal());
            else if (m_tokenizer.currentTokenType() == KEYWORD && isKeywordConstant(m_tokenizer.keywordOrIdentifer())){
                process(m_tokenizer.keywordOrIdentifer());
            }
            else if (m_tokenizer.currentTokenType() == IDENTIFIER){
                process(m_tokenizer.keywordOrIdentifer());
                if (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == '['){
                    process("[");
                    compileExpression();
                    process("]");
                }
                else if (m_tokenizer.currentTokenType() == SYMBOL && (m_tokenizer.symbol() == '(' || m_tokenizer.symbol() == '.')){  // handling subroutine call
                    if (m_tokenizer.symbol() == '('){
                        process("(");
                        compileExpressionList();
                        process(")");
                    }
                    else {
                        process(".");
                        process(m_tokenizer.keywordOrIdentifer());
                        process("(");
                        compileExpressionList();
                        process(")");
                    }
                }
            }
            else if (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == '('){
                process("(");
                compileExpression();
                process(")");
            }
            else if (m_tokenizer.currentTokenType() == SYMBOL && isUnaryOp(m_tokenizer.symbol())){
                process(std::string(1, m_tokenizer.symbol()));
                compileTerm();
            }
            m_outfile << "</term>\n";
        }

        void compileExpressionList(){
            m_outfile << "<expressionList>\n";
            if (m_tokenizer.currentTokenType() != SYMBOL || (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() != ')')){
                compileExpression();
                while (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == ','){
                    process(",");
                    compileExpression();
                }
            }
            m_outfile << "</expressionList>\n";
        }

        void compilesubroutineCall(){
            process(m_tokenizer.keywordOrIdentifer());
            if (m_tokenizer.currentTokenType() == SYMBOL && m_tokenizer.symbol() == '('){
                process("(");
                compileExpressionList();
                process(")");
            }
            else {
                process(".");
                process(m_tokenizer.keywordOrIdentifer());
                process("(");
                compileExpressionList();
                process(")");
            }
        }

    private:
        std::ofstream       m_outfile;
        tokenizer           m_tokenizer;
};

class analyzer{
    public:
        analyzer(const std::string& filename){
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
    analyzer new_analyzer(argv[1]);
    return 0;
}