#include <iostream> 
#include <fstream>		
#include <string>
#include <cctype>
#include <cstdlib>
#include "Stack_Implement.h"

static void Usage_msg();
static void Error_msg(std::string err_code);
static unsigned short int Remove_spaces(std::string fpath);
static unsigned short int Provide_help(const int argc_intr, const char * const argv_intr[]);
static void Paired_check();


int main(int argc, char *argv[])	{
	
	if(Provide_help(argc, argv)) return 0;
	const char* input_file_path = argv[1];	
	if(Remove_spaces(input_file_path)) return 0;
	Paired_check();
	
	std::fstream thefile( "Temp_File.txt" );									// Temporary File without whitespaces. 
	std::fstream out_file("Output File.txt", std::ios::app);					// Intended Output File. 
		
	unsigned int index = 0; 
	unsigned short int LSB_count, RSB_count, LCB_count, RCB_count; 
				LSB_count = RSB_count = LCB_count = RCB_count = 0;	
	unsigned short int Dot_count = 0;
	unsigned int of_pindex_curr, of_pindex_prev;
	enum State_type {
		Mode_0,		
		Mode_1,		// name copy-paste
		Mode_2,		// handle array indexing and some output
		Mode_3,		// handle object and some output
		Mode_4,		// type value to Output
		Mode_5,		// handle value beginnig
		Mode_6 		// handle comma  
	};
	State_type State_var = Mode_0;
	State_type State_previous;
	
	char chr = '0';
	
	Stack<char> AO_indicator(100);												// Maximum nesting level for both arrays and objects - 100.
	Stack<unsigned short int> Arr_indexes(100);									// Maximum (nested) arrays' indexes to be stored(max nest level) - 100. 
	unsigned short int Arr_index = 0;
	
	std::string str_buff(50, ' ');
	std::string current_key_path(500, ' ');						

	while (thefile)
	{
		thefile.seekg(index);
		thefile.get(chr);
		str_buff.clear();
		
//		std::cout << "While() cycle index and chr values at start: " << index << '\t' << chr << '\n' << '\n';							// DEBUG INFO
		
		if(State_var==Mode_0){						// Initial Mode is Zero. Verifies initial LCB and QM. Passes the control to Mode One. 
			if(chr == '{'){
				LCB_count++;
				index++;
				thefile.get(chr);
				if(chr == '\"'){
					State_var = Mode_1;	
					continue;
				}else{
					Error_msg("Err_06");
					break;
				}				
			}else{
				Error_msg("Err_06");
				break;
			}
		}
		
		if(State_var==Mode_1){						// Mode One copy-pastes Key name strings and verifies the subsequent colon separator. 
			std::getline (thefile, str_buff, '\"');
			out_file << str_buff;
			index += str_buff.length();	
			index += 2;								 
			thefile.get(chr);
			if(chr == ':'){							
				State_var = Mode_5;	
				State_previous = Mode_1;
				index++;
				continue;				
			}else{
				Error_msg("Err_06");
				break;
			}		
		}
		
		if(State_var==Mode_5){						// Mode Five switches to appropriate Mode for Value handling according to input. 
			if(chr == '\"'||isdigit(chr)||chr == '-'){
				State_var = Mode_4;						
				continue;
			}else if(chr == 't'||chr == 'f'||chr == 'n'||chr == 'T'||chr == 'F'||chr == 'N'){
				State_var = Mode_4;						
				continue;					
			}else if(chr == '{'){
				State_var = Mode_3;						
				continue;				
			}else if(chr == '['){
				State_var = Mode_2;							
				continue;								
			}else{
				Error_msg("Err_08");
				break;				
			}			
		}

		if(State_var==Mode_4){						// Mode Four handles String or Numeric or Logical or Null values. And switches control according to subsequent characters. 
			out_file << '\t';
			if(chr == '\"'){							
				std::getline (thefile, str_buff, '\"');
				out_file << str_buff;
				index += str_buff.length();	
				index += 2;								// Need 2 chars to step over input word.  
			//	continue;
			}else if(isdigit(chr)||chr == '-'){																// Exponential numbers are not supported yet. 
				while(isdigit(chr)||chr=='.'||chr =='-'){															// No proper input validation here
					thefile.get(chr);		
					out_file.put(chr);
					index++;
				}							
			}else if(chr == 't'||chr == 'f'||chr == 'n'||chr == 'T'||chr == 'F'||chr == 'N'){						// Not implemented
				std::cout<< "Support for true/false/null is not implemented yet. "<< '\n';
				break;
				// for (std::string::size_type i=0; i<str.length(); ++i)
				// std::cout << std::toupper(str[i],loc);	
			}else{
				std::cout<< "Wrong Mode 4 invocation. Aborted. (Internal Error) "<< '\n';
				break;							
			}
			
			of_pindex_prev = out_file.tellp();
			
			thefile.get(chr);
			if(chr == ','){															
				State_var = Mode_6;			
				index++;	
				continue;
			}else if(chr == ']'){
				State_var = Mode_2;					
				continue;
			}else if(chr == '}'){
				State_var = Mode_3;					
				continue;
			}else{
				Error_msg("Err_09");
				break;
			}			
		}	
		
		if(State_var==Mode_6){						// Mode Six re-types existing Key path string and defines Key type that follows comma. 
			thefile.get(chr);
			if(AO_indicator.peek()=='A'){				
				if((LSB_count-RSB_count)>0){
					out_file.seekp(of_pindex_prev);
					while(LSB_count>=1){
						thefile.get(chr);						
						std::getline (out_file, str_buff, '[');	
						index++;
						LSB_count--;
					}
					out_file << str_buff;	
				}else if(LSB_count==RSB_count){
					
				}else{
					Error_msg("Err_06");
					break;
				}		
				//...?		
			}else if(AO_indicator.peek()=='O'){			 	
				if((LCB_count-RCB_count)>0){
					out_file.seekg(of_pindex_prev);
					while(Dot_count>=1){
						thefile.get(chr);						
						std::getline (out_file, str_buff, '.');	
						index++;
						Dot_count--;
					}
					out_file << str_buff;	
				}else{
					Error_msg("Err_06");
					break;
				}			
				//..?
			}else{
				index++;
				thefile.get(chr);
				if(chr == '\"'){
					State_var = Mode_1;
				}else{
					Error_msg("Err_06");				// "Expected Key name string after comma when outside array/object." 
				}
			}	
			out_file << std::endl;	 
		}	
			
		if(State_var==Mode_2){						// Mode Two types indexes to Output and controls "currently in an array" indication. 
			if(chr == '['){
				Arr_index = 0;
				Arr_indexes.push(Arr_index);
				AO_indicator.push('A');
				out_file << '[' << Arr_index << ']';
				State_var = Mode_4;
				index++;
			}else if(chr == ']'){
				Arr_indexes.pop();
				AO_indicator.pop();					
				index++;
				thefile.get(chr);
				if(chr == ','){
					State_var = Mode_6;
					index++;
				}else if(chr == ']'){
					State_var = Mode_2;
				}else if(chr == '}'){
					State_var = Mode_3;
				}else{
					Error_msg("Err_06");			// " Expected RSB, RCB or Comma after array ended. "
				}			
			}else{
				Arr_index = (Arr_indexes.peek() + 1);
				Arr_indexes.push(Arr_index);
				out_file << '[' << Arr_index << ']';
				State_var = Mode_4;
			}			
		}
	
		if(State_var==Mode_3){						// Mode Three types dots to Output and controls "currently in an object" indication. 
			if(chr == '{'){
				AO_indicator.push('O');
				out_file << '.';
				State_var = Mode_1;
				index++;
			}else if(chr == '}'){
				AO_indicator.pop();					
				index++;
				thefile.get(chr);
				if(chr == ','){
					State_var = Mode_6;
					index++;
				}else if(chr == ']'){
					State_var = Mode_2;
				}else if(chr == '}'){
					State_var = Mode_3;
				}else{
					Error_msg("Err_06");			// " Expected RSB, RCB or Comma after object ended. "
				}			
			}else{
				out_file << '.';
				State_var = Mode_4;
			}			
		}	
	
	}

std::cout << '\n' << "End." << '\n';
return 0;
}



static void Usage_msg(){
    std::cout << "Usage: [PROGRAM] [INPUT_FILE_PATH] <OUTPUT_FILE_PATH> <option> <--help> <-h> \n" 
              << "PROGRAM_NAME - current program. \n"
              << "INPUT_FILE_PATH - path to a .json file to be parsed. \n"
              << "OUTPUT_FILE_PATH - path to a file with required output. Can be used with <option>. \n"
			  << "Currently only one <option> is supported: \"-c\". Usage: \n"
			  << "\t Requires OUTPUT_FILE_PATH to be provided. \n"
			  << "\t When -c option is set, and the specified output file is absent, it will be created(if permitted by UAC). \n"
			  << "\t Wnen -c option is not set, and the specified output file is absent, an error will occur. \n"
			  << "If OUTPUT_FILE_PATH is not provided, program will output to console. \n"
			  << "Warning: current flow suggests that file on OUTPUT_FILE_PATH is empty. Program overrides (some or any) content this file may have. \n"
			  << "Key \"--help\" or \"-h\" suppresses any other provided input. \n\n"
			  << "Currently only ASCII characters are supported. Support of other characters is not guaranteed. \n"
			  << "Warning: all whitespaces(even in strings) will be truncated. Please, use underscores in strings. \n"
              << std::endl;
}

static void Error_msg(std::string err_code){	
const unsigned short int Err_codes_size = 10; 
	const std::string Err_codes[Err_codes_size] = {
		"Err_01",
		"Err_02",
		"Err_03",
		"Err_04",
		"Err_05",
		"Err_06",
		"Err_07",
		"Err_08",
		"Err_09",
		"Err_10"	
	};
	
	const std::string Err_messages[] = {
		"Err_01. Wrong program invocation format. Generic error response. \n 		Try executing program with \"--help\" key for reference.",
		"Err_02. Could not find/open file on a provided INPUT_FILE_PATH. \n",
		"Err_03. Option \"-c\" entered, but OUTPUT_FILE_PATH was not provided. \n",
		"Err_04. Could not find/open file on a provided OUTPUT_FILE_PATH. \n",
		"Err_05. A file already exists on a provided OUTPUT_FILE_PATH. \n",
		"Err_06. Provided input file is not JSON-compliant. Generic error response. \n",
		"Err_07. Number of paired punctuation marks is not even(missing or excessive punctuation mark(s)). \n",
		"Err_08. String, Numeric, (ARRAY), (OBJECT), (true), (false) or (null) expected as (KEY_VALUE). \n",
		"Err_09. Comma, closing square bracket or closing curly bracket expected after (KEY_VALUE). \n",
		" \n",
	};
	
	for (auto iter = 0; iter < Err_codes_size; iter++){
		if (Err_codes[iter] == err_code){
			std::cerr << Err_messages[iter] << std::endl;
			return;
		}
	}
	std::cerr << "Error code not found. (Internal error) \n";
}

static unsigned short int Remove_spaces(std::string fpath){		

	std::ifstream finput(fpath);
    if (!finput.is_open()) {
		Error_msg("Err_02"); 
		return 1;
	}  
	std::ofstream fcreate("Temp_File.txt");
	fcreate.close();
	std::fstream ftempinit( "Temp_File.txt" );
	std::string nospace;
		while( finput >> nospace ) 	ftempinit << nospace;
	ftempinit.close();
	return 0;
}

static unsigned short int Provide_help(int argc_intr, const char * const argv_intr[]){
	for (int iter = 1; iter < argc_intr; ++iter) {
		std::string cmdkey = argv_intr[iter];
		if ((cmdkey == "-h") || (cmdkey == "--help")) {
			Usage_msg();
			return 1;
		}
	} 
	return 0;
}

static void Paired_check(){
	std::fstream ftemp( "Temp_File.txt" );
	unsigned int LSB_count, RSB_count, LCB_count, RCB_count; 
	LSB_count = RSB_count = LCB_count = RCB_count = 0;	
	char chr;
	while (ftemp.get(chr)){
		if(chr == '[') LSB_count++ ;
		else if(chr == ']') RSB_count++ ;
		else if(chr == '{') LCB_count++ ;
		else if(chr == '}') RCB_count++ ;
	}
	if((LSB_count!=RSB_count)||(LCB_count!=RCB_count))	Error_msg("Err_07");
	ftemp.close(); 
}


	
	
	
	
	