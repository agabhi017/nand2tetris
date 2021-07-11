#include <iostream>
#include <string>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <bitset>

#define ADDRESS_LEN (15)
#define VAR_ADDRESS (16)

static std::unordered_map <std::string, std::string> symbol_table;

enum instruction_type{
            A_INSTRUCTION, 
            C_INSTRUCTION, 
            L_INSTRUCTION,
            INVALID
        };

void symbolTableInit(){
    symbol_table["SCREEN"]  = "16384";
    symbol_table["KBD"]     = "24576";
    symbol_table["SP"]      = "0";
    symbol_table["LCL"]     = "1";
    symbol_table["ARG"]     = "2";
    symbol_table["THIS"]    = "3";
    symbol_table["THAT"]    = "4";
    symbol_table["R0"]      = "0";
    symbol_table["R1"]      = "1";
    symbol_table["R2"]      = "2";
    symbol_table["R3"]      = "3";
    symbol_table["R4"]      = "4";
    symbol_table["R5"]      = "5";
    symbol_table["R6"]      = "6";
    symbol_table["R7"]      = "7";
    symbol_table["R8"]      = "8";
    symbol_table["R9"]      = "9";
    symbol_table["R10"]     = "10";
    symbol_table["R11"]     = "11";
    symbol_table["R12"]     = "12";
    symbol_table["R13"]     = "13";
    symbol_table["R14"]     = "14";
    symbol_table["R15"]     = "15";
}

bool isNumber(const std::string& inst){
    for (char const& c : inst){
        if (std::isdigit(c) == 0){
            return false;
        }
    }
    return true;
}

class Parser{
    public:
        Parser(const std::string& filename){
            m_infile.open(filename);
        }

        ~Parser(){
            m_infile.close();
        }

        bool hasMoreLines(){
            if (!m_infile.eof())        {return true;}
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
            m_current_instruction.erase(std::remove_if(m_current_instruction.begin(), m_current_instruction.end(), ::isspace), m_current_instruction.end());
            int slash_index = m_current_instruction.size();
            for (int i = 0; i < m_current_instruction.size(); i++){
                if (m_current_instruction[i] == '/'){
                    slash_index = i;
                    break;
                }
            }
            m_current_instruction = m_current_instruction.substr(0, slash_index);       // remove comments from the instruction if any
        }

        bool isValidInstruction(){
            if (m_current_instruction.size() == 0){
                return false;
            }
            return true;
        }
        
        void parse(){
            if (m_infile.is_open()){
                advance();
                if (isValidInstruction()){
                    setInstructionType();
                    initStrings();
                    if (m_current_instruction_type == L_INSTRUCTION){
                        m_symbol = m_current_instruction.substr(1, m_current_instruction.size() - 2);
                    }
                    else if (m_current_instruction_type == A_INSTRUCTION){
                        m_symbol = m_current_instruction.substr(1, m_current_instruction.size() - 1);
                    }
                    else {  // figure out the dest, comp and jmp components
                        int dest_index = 0, jmp_index = -1;
                        for (int i = 0; i < m_current_instruction.size(); i++){
                            if (m_current_instruction[i] == '=')        {dest_index = i;}
                            else if (m_current_instruction[i] == ';')   {jmp_index = i;}
                        }
                        m_dest = m_current_instruction.substr(0, dest_index);
                        if (jmp_index != -1){   // jmp is present
                            if (dest_index != 0){   // dest is present
                                m_comp = m_current_instruction.substr(dest_index + 1, jmp_index - dest_index);
                            }
                            else {
                                m_comp = m_current_instruction.substr(0, jmp_index - dest_index);
                            }
                            m_jump = m_current_instruction.substr(jmp_index + 1, m_current_instruction.size() - jmp_index);
                        }
                        else {      // jmp absent
                            m_comp = m_current_instruction.substr(dest_index + 1, m_current_instruction.size() - dest_index);
                        }
                    }
                }
                else{   // encode the instruction as invalid to avoid any future ops on it; will mostly be the last line of the file
                    m_current_instruction_type = INVALID;
                }
            }
        }

        void setInstructionType(){
            if (m_current_instruction[0] == '(')          {m_current_instruction_type = L_INSTRUCTION;}
            else if (m_current_instruction[0] == '@')     {m_current_instruction_type = A_INSTRUCTION;}
            else                                          {m_current_instruction_type = C_INSTRUCTION;}
        }

        void initStrings(){
            m_symbol = "";
            m_comp = "";
            m_dest = "";
            m_jump = "";
        }

        void reset(const std::string& filename){    // prepare the parser for the 2nd pass
            m_infile.close();
            m_infile.open(filename);
        }

        instruction_type instructionType() const {
            return m_current_instruction_type;
        }

        std::string symbol() const {
            return m_symbol;
        }

        std::string dest() const {
            return m_dest;
        }

        std::string comp() const {
            return m_comp;
        }

        std::string jump() const {
            return m_jump;
        }

    private:
        instruction_type            m_current_instruction_type;
        std::ifstream               m_infile;
        std::string                 m_current_instruction;
        std::string                 m_symbol;
        std::string                 m_comp;
        std::string                 m_dest;
        std::string                 m_jump;
};

class Coder{
    public:
        Coder(const std::string& filename){
            std::string fname = filename.substr(0, filename.size() - 4);
            fname += ".hack";
            m_outfile.open(fname);
            initDestTable();
            initJumpTable();
            initCompTable();
        }

        ~Coder(){
            m_outfile.close();
        }

        void initDestTable(){
            m_destination_table[""]     = "000";
            m_destination_table["M"]    = "001";
            m_destination_table["D"]    = "010";
            m_destination_table["DM"]   = "011";
            m_destination_table["MD"]   = "011";
            m_destination_table["A"]    = "100";
            m_destination_table["AM"]   = "101";
            m_destination_table["MA"]   = "101";
            m_destination_table["AD"]   = "110";
            m_destination_table["DA"]   = "110";
            m_destination_table["ADM"]  = "111";
            m_destination_table["AMD"]  = "111";
            m_destination_table["MDA"]  = "111";
        }

        void initJumpTable(){
            m_jump_table[""]    = "000";
            m_jump_table["JGT"] = "001";
            m_jump_table["JEQ"] = "010";
            m_jump_table["JGE"] = "011";
            m_jump_table["JLT"] = "100";
            m_jump_table["JNE"] = "101";
            m_jump_table["JLE"] = "110";
            m_jump_table["JMP"] = "111";
        }

        void initCompTable(){
            m_comp_table["0"]       = "0101010";
            m_comp_table["1"]       = "0111111";
            m_comp_table["-1"]      = "0111010";
            m_comp_table["D"]       = "0001100";
            m_comp_table["A"]       = "0110000";
            m_comp_table["!D"]      = "0001101";
            m_comp_table["!A"]      = "0110001";
            m_comp_table["-D"]      = "0001111";
            m_comp_table["-A"]      = "0110011";
            m_comp_table["D+1"]     = "0011111";
            m_comp_table["A+1"]     = "0110111";
            m_comp_table["D-1"]     = "0001110";
            m_comp_table["A-1"]     = "0110010";
            m_comp_table["D+A"]     = "0000010";
            m_comp_table["D-A"]     = "0010011";
            m_comp_table["A-D"]     = "0000111";
            m_comp_table["D&A"]     = "0000000";
            m_comp_table["D|A"]     = "0010101";
            m_comp_table["M"]       = "1110000";
            m_comp_table["!M"]      = "1110001";
            m_comp_table["-M"]      = "1110011";
            m_comp_table["M+1"]     = "1110111";
            m_comp_table["M-1"]     = "1110010";
            m_comp_table["D+M"]     = "1000010";
            m_comp_table["D-M"]     = "1010011";
            m_comp_table["M-D"]     = "1000111";
            m_comp_table["D&M"]     = "1000000";
            m_comp_table["D|M"]     = "1010101";
        }

        void write(const std::string& instruction){
            m_outfile << instruction << "\n";
        }

        std::string dest(const std::string& destination){
            return m_destination_table[destination];
        }

        std::string comp(const std::string& computation){
            return m_comp_table[computation];
        }

        std::string jump(const std::string& jumpstr){
            return m_jump_table[jumpstr];
        }

    private:
        std::ofstream                                   m_outfile;
        std::unordered_map<std::string, std::string>    m_destination_table;
        std::unordered_map<std::string, std::string>    m_jump_table;
        std::unordered_map<std::string, std::string>    m_comp_table;
};

int main(int argc, char* argv[]){
    symbolTableInit();
    Parser parser(argv[1]);
    Coder coder(argv[1]);
    int line_number = 0, var_address = VAR_ADDRESS;
    while (parser.hasMoreLines()){
        parser.parse();
        if (parser.instructionType() == L_INSTRUCTION){
            --line_number;
            symbol_table[parser.symbol()] = std::to_string(line_number + 1);
        }
        ++line_number;
    }

    parser.reset(argv[1]);
    while (parser.hasMoreLines()){
        parser.parse();
        std::string instruction;
        if (parser.instructionType() == C_INSTRUCTION){
            instruction = "111" + coder.comp(parser.comp()) + coder.dest(parser.dest()) + coder.jump(parser.jump());
            coder.write(instruction);
        }
        else if (parser.instructionType() == A_INSTRUCTION){
            std::string symbol = parser.symbol();
            if (isNumber(symbol)){
                instruction = "0" + std::bitset<ADDRESS_LEN>(std::stoi(symbol)).to_string();
            }
            else {
                if (symbol_table.find(symbol) != symbol_table.end()){
                    instruction = "0" + std::bitset<ADDRESS_LEN>(std::stoi(symbol_table[symbol])).to_string();
                }
                else {  // symbol not present in the symbol table 
                    symbol_table[symbol] = std::to_string(var_address);
                    instruction = "0" + std::bitset<ADDRESS_LEN>(var_address).to_string();
                    ++var_address;
                }
            }
            coder.write(instruction);
        }
    }
    return 0;
}