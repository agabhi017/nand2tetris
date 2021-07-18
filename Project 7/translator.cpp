#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>     // exit
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <sstream>

enum InstructionType{
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL,
    INVALID
};

static std::unordered_map<std::string, InstructionType> command_table;

void initCommandTable(){
    command_table["add"]    = C_ARITHMETIC;
    command_table["sub"]    = C_ARITHMETIC;
    command_table["neg"]    = C_ARITHMETIC;
    command_table["eq"]     = C_ARITHMETIC;
    command_table["gt"]     = C_ARITHMETIC;
    command_table["lt"]     = C_ARITHMETIC;
    command_table["and"]    = C_ARITHMETIC;
    command_table["or"]     = C_ARITHMETIC;
    command_table["not"]    = C_ARITHMETIC;
    command_table["push"]   = C_PUSH;
    command_table["pop"]    = C_POP;
    command_table[""]       = INVALID;
}

class Parser{
    public:
        Parser(const std::string& filename){
            m_infile.open(filename);
            if (!m_infile.is_open()){
                std::exit(1);
            }
        }

        ~Parser(){
            m_infile.close();
        }

        bool hasMoreLines(){
            if(!m_infile.eof())         {return true;}
            else                        {return false;}
        }

        void advance(){
            if (hasMoreLines()){
                do {
                    std::getline(m_infile, m_current_instruction);
                    cleanInstruction();
                }
                while (!isValidInstruction() && hasMoreLines());
            }
        }

        void cleanInstruction(){
            int slash_index = m_current_instruction.size();
            for (int i = 0; i < m_current_instruction.size(); i++){
                if (m_current_instruction[i] == '/'){
                    slash_index = i;
                    break;
                }
            }
            m_current_instruction = m_current_instruction.substr(0, slash_index);
        }

        bool isValidInstruction(){
            if (m_current_instruction.size() == 0 || m_current_instruction[0] == ' '){
                return false;
            }
            return true;
        }

        void parse(){
            advance();
            initStrings();
            if (isValidInstruction()){
                std::stringstream instruction_stream(m_current_instruction);
                std::string symbol;
                instruction_stream >> symbol;
                m_current_instruction_type = command_table[symbol]; 
                if (m_current_instruction_type == C_ARITHMETIC){
                    m_arg1 = symbol;
                }
                else {
                    instruction_stream >> symbol;
                    m_arg1 = symbol;
                    instruction_stream >> symbol;
                    m_arg2 = symbol;
                }
            }
            else {
                m_current_instruction_type = INVALID;
            }
        }

        void initStrings(){
            m_arg1 = "";
            m_arg2 = "";
        }

        std::string instruction(){
            return m_current_instruction;
        }

        InstructionType commandType(){
            return m_current_instruction_type;
        }

        std::string arg1(){
            return m_arg1;
        }

        std::string arg2(){
            return m_arg2;
        }

    private:
        std::ifstream           m_infile;
        std::string             m_current_instruction;
        InstructionType         m_current_instruction_type;
        std::string             m_arg1;
        std::string             m_arg2;
};

class CodeWriter{
    public:
        CodeWriter(const std::string& filename){
            std::string fname = filename.substr(0, filename.size() - 2) + "asm";
            m_outfile.open(fname);
            if (!m_outfile.is_open()){
                std::exit(1);
            }
        }

        ~CodeWriter(){
            m_outfile.close();
        }

        void writeArithmetic(const std::string& command){
            std::cout << "writing arithmetic\n";
            if (command == "not"){
                writePopOnly();
                m_outfile << "M=!M\n";
            }
            else if (command == "neg"){
                writePopOnly();
                m_outfile << "M=-M\n";
            }
            else {
                writePop();
                m_outfile << "D=M\n";
                writePopOnly();
                if (command == "add")           m_outfile << "M=D+M\n";
                else if (command == "sub")      m_outfile << "M=D-M\n";
                else if (command == "and")      m_outfile << "M=D&M\n";
                else if (command == "or")       m_outfile << "M=D|M\n";
                else {
                    m_outfile << "D=M-D\n"
                              << "M=-1\n"
                              << "@CONTINUE\n";
                    if (command == "eq")        m_outfile << "D;JEQ\n";
                    else if (command == "gt")   m_outfile << "D;JGT\n";
                    else                        m_outfile << "D;JLT\n";
                    m_outfile << "@SP\n"
                              << "A=M-1\n"
                              << "M=0\n"
                              << "(CONTINUE)";
                }
            }
        }

        void writePushPop(InstructionType cmdtype, const std::string& segment, const std::string& index){
            if (cmdtype == C_POP){
                std::cout << "writing pop\n";
                if (segment == "")          writePop();
                else {
                    m_outfile << "@" << index << "\n"
                              << "D=A\n"
                              << "@" << segment << "\n"
                              << "D=M+D\n"
                              << "@R13\n"
                              << "M=D\n";
                    writePop();
                    m_outfile << "D=M\n"
                              << "@R13\n"
                              << "A=M\n"
                              << "M=D\n";
                }      
            }
            else {
                std::cout << "writing push\n";
                m_outfile << "@" << index << "\n"
                          << "D=A\n";
                if (segment != "" && segment != "constant"){
                    m_outfile << "@" << segment << "\n"
                              << "A=M+D\n"
                              << "D=M\n";
                }
                m_outfile << "@SP\n"
                          << "M=M+1\n"
                          << "A=M-1\n"
                          << "M=D\n";
            }
        }

        void writePop(){
            m_outfile << "@SP\n"
                      << "AM=M-1\n";
        }

        void writePopOnly(){
            m_outfile << "@SP\n"
                      << "A=M-1\n";
        }

        void writeClosing(){
            m_outfile << "(END)\n"
                      << "@END\n"
                      << "0;JMP\n"; 
        }

    private:
        std::ofstream           m_outfile;
};

int main(int argc, char* argv[]){
    if (argc != 2){ // impose correct usage
        std::cout << "Incorrect usage" << "\n";
        return 1;
    }

    initCommandTable();
    Parser parser(argv[1]);
    CodeWriter writer(argv[1]); 

    while (parser.hasMoreLines()){
        parser.parse();
        if (parser.commandType() == C_PUSH || parser.commandType() == C_POP){
            writer.writePushPop(parser.commandType(), parser.arg1(), parser.arg2());
        }
        else if (parser.commandType() == C_ARITHMETIC){
            writer.writeArithmetic(parser.arg1());
        }
    }
    writer.writeClosing();
    return 0;
}