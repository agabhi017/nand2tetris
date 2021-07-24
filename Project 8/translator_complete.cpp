#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>     // exit
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

enum InstructionType{
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_IF_GOTO,
    C_FUNCTION,
    C_RETURN,
    C_CALL,
    INVALID
};

static std::unordered_map<std::string, InstructionType> command_table;

void initCommandTable(){
    command_table["add"]        = C_ARITHMETIC;
    command_table["sub"]        = C_ARITHMETIC;
    command_table["neg"]        = C_ARITHMETIC;
    command_table["eq"]         = C_ARITHMETIC;
    command_table["gt"]         = C_ARITHMETIC;
    command_table["lt"]         = C_ARITHMETIC;
    command_table["and"]        = C_ARITHMETIC;
    command_table["or"]         = C_ARITHMETIC;
    command_table["not"]        = C_ARITHMETIC;
    command_table["push"]       = C_PUSH;
    command_table["pop"]        = C_POP;
    command_table["label"]      = C_LABEL;
    command_table["call"]       = C_CALL;
    command_table["return"]     = C_RETURN;
    command_table["function"]   = C_FUNCTION;
    command_table["goto"]       = C_GOTO;
    command_table["if"]         = C_IF;
    command_table["if-goto"]    = C_IF_GOTO;
    command_table[""]           = INVALID;
}

std::string getFilenameFromPath(std::string filename){
    int slash_index = 0;
    for (int i = 0; i < filename.size(); i++){
        if (filename[i] == '/'){
            slash_index = i;
        }
    }
    return filename.substr(slash_index + 1, filename.size() - slash_index - 1);
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

                if (m_current_instruction_type != C_RETURN){
                    if (m_current_instruction_type == C_ARITHMETIC){
                        m_arg1 = symbol;
                    }
                    else {
                        instruction_stream >> symbol;
                        m_arg1 = symbol;
                    }

                    if (m_current_instruction_type == C_PUSH || m_current_instruction_type == C_POP 
                        || m_current_instruction_type == C_FUNCTION || m_current_instruction_type == C_CALL){
                            instruction_stream >> symbol;
                            m_arg2 = symbol;
                        }
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
            std::string fname;
            if (filename.substr(filename.size() - 3, 3) == ".vm"){
                fname = filename.substr(0, filename.size() - 2) + "asm";
            }
            else {
                std::cout << filename << std::endl;
                fname = filename;
                fname = getFilenameFromPath(fname.substr(0, fname.size() - 1));
                fname = filename + fname + ".asm";
            }
            std::cout << fname << std::endl;
            m_outfile.open(fname);
            if (!m_outfile.is_open()){
                std::exit(1);
            }
        }

        ~CodeWriter(){
            m_outfile.close();
        }

        void setFileName(const std::string& filename){
            m_file_name = filename.substr(0, filename.size() - 3);
            m_return_index = 0;
            m_continue_index = 0;
        }

        void writeArithmetic(const std::string& command){
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
                              << "@" << m_file_name << "$CONTINUE." << m_continue_index << "\n";
                    if (command == "eq")        m_outfile << "D;JEQ\n";
                    else if (command == "gt")   m_outfile << "D;JGT\n";
                    else                        m_outfile << "D;JLT\n";
                    m_outfile << "@SP\n"
                              << "A=M-1\n"
                              << "M=0\n"
                              << "(" << m_file_name << "$CONTINUE." << m_continue_index << ")\n";
                    ++m_continue_index;
                }
            }
        }

        void writePushPop(InstructionType cmdtype, const std::string& segment, const std::string& index){
            if (cmdtype == C_POP){
                if (segment == "")          writePop();
                else {
                    if (index != "0"){
                        m_outfile << "@" << index << "\n"
                                  << "D=A\n";
                    }
                    m_outfile << "@" << segment << "\n"
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
                if (index != "0"){
                    m_outfile << "@" << index << "\n"
                              << "D=A\n";
                }
                if (segment != "" && segment != "constant"){
                    m_outfile << "@" << segment << "\n"
                              << "A=M+D\n"
                              << "D=M\n";
                }
                writePush();
            }
        }

        void writeLabel(const std::string& label){
            m_outfile << "(" << m_file_name << "$" << label << ")\n";
        }

        void writeGoto(const std::string& label){
            m_outfile << "@" << m_file_name << "$" << label << "\n";
            m_outfile << "0;JMP\n";
        }

        void writeIf(const std::string& label){
            writePop();
            m_outfile << "D=M\n";
            m_outfile << "@" << m_file_name << "$" << label << "\n";
            m_outfile << "D;JNE\n";
        }

        void writeFunction(const std::string& fun_name, int n_vars){
            writeLabel(fun_name);
            for (int i = 0; i < n_vars; i++)    m_outfile << "push 0\n";
        }

        void writeCall(const std::string& fun_name, int n_vars){
            std::string ret_addr = "@" + m_file_name + "." + fun_name + "$ret." + std::to_string(m_return_index);
            m_outfile << ret_addr << "\n"
                      << "D=A\n";
            writePush();
            writePushSegment("LCL");
            writePushSegment("ARG");
            writePushSegment("THIS");
            writePushSegment("THAT");   
            m_outfile << "@SP\n"
                      << "D=M\n"
                      << "@LCL\n"
                      << "M=D\n"
                      << "@ARG\n"
                      << "M=D-1\n";
            for (int i = 0; i < 5 + n_vars - 1; i++)    m_outfile << "M=M-1\n";
            m_outfile << "@" << m_file_name << "." << fun_name << "\n"
                      << "0;JMP\n"; 
            m_outfile << "(" + m_file_name + "." + fun_name + "$ret." + std::to_string(m_return_index) + ")\n";
            ++m_return_index;
        }

        void writeReturn(){
            --m_return_index;
            // temp frame
            m_outfile << "@LCL\n"
                      << "D=M\n"
                      << "@R13\n"
                      << "AM=D\n";
            for (int i = 0; i < 5; i++)     m_outfile << "A=A-1\n";
            // temp ret_addrs
            m_outfile << "D=M\n"
                      << "@R14\n"
                      << "M=D\n";
            // reposition arg and sp
            writePop();
            m_outfile << "D=M\n"
                      << "@ARG\n"
                      << "A=M\n"
                      << "M=D\n"
                      << "D=D+1\n"
                      << "@SP\n"
                      << "M=D\n"; 
            writeRestoreSegment("THAT");
            writeRestoreSegment("THIS");
            writeRestoreSegment("ARG");
            writeRestoreSegment("LCL");
            m_outfile << "@R14\n"
                      << "0;JMP\n";
        }

        void writePushSegment(const std::string& segment){
            m_outfile << "@" << segment << "\n"
                      << "D=M\n";
            writePush();
        }

        void writeRestoreSegment(const std::string& segment){
            m_outfile << "@R13\n"
                      << "AM=A-1\n"
                      << "D=M\n"
                      << "@" << segment << "\n"
                      << "M=D\n";
        }

        void writePop(){
            m_outfile << "@SP\n"
                      << "AM=M-1\n";
        }

        void writePush(){
            m_outfile << "@SP\n"
                      << "M=M+1\n"
                      << "A=M-1\n"
                      << "M=D\n";
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
        std::string             m_file_name;
        int                     m_return_index;
        int                     m_continue_index;
};

int main(int argc, char* argv[]){
    if (argc != 2){ // impose correct usage
        std::cout << "Incorrect usage" << "\n";
        return 1;
    }

    initCommandTable();
    CodeWriter writer(argv[1]);
    std::vector <std::string> filenames;
    std::string path(argv[1]);

    if (path.substr(path.size() - 2, 2) == "vm"){
        filenames.push_back(path);
    }
    else {
        std::string ext = ".vm";
        for (const auto& fpath : fs::directory_iterator(path)){
            if (fpath.path().extension() == ext){
                filenames.push_back(fpath.path());
            }
        }
    }

    for (std::string& fname : filenames){
        Parser parser(fname);
        fname = getFilenameFromPath(fname);
        writer.setFileName(fname);
        while (parser.hasMoreLines()){
            parser.parse();
            if (parser.commandType() == C_PUSH || parser.commandType() == C_POP){
                writer.writePushPop(parser.commandType(), parser.arg1(), parser.arg2());
            }
            else if (parser.commandType() == C_ARITHMETIC){
                writer.writeArithmetic(parser.arg1());
            }
            else if (parser.commandType() == C_LABEL){
                writer.writeLabel(parser.arg1());
            }
            else if (parser.commandType() == C_GOTO){
                writer.writeGoto(parser.arg1());
            }
            else if (parser.commandType() == C_IF_GOTO){
                writer.writeIf(parser.arg1());
            }
            else if (parser.commandType() == C_FUNCTION){
                writer.writeFunction(parser.arg1(), std::stoi(parser.arg2()));
            }
            else if (parser.commandType() == C_CALL){
                writer.writeCall(parser.arg1(), std::stoi(parser.arg2()));
            }
            else if (parser.commandType() == C_RETURN){
                writer.writeReturn();
            }
        }
    }
    writer.writeClosing();
    return 0;
}