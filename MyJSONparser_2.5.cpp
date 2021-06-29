#include <iostream> 
#include <fstream>		
#include <string>
#include <cctype>
#include <cstdlib>
#include <regex>
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
	
	std::fstream thefile( "Temp_File.txt" );
	std::fstream out_file("Output File.txt");
	
	std::streamoff index_temp = 0; 
	std::streamoff index_out = 0;
	std::streamoff of_pindex_curr, of_pindex_prev;
	of_pindex_curr = out_file.tellp();
	
	unsigned short int LSB_count, RSB_count, LCB_count, RCB_count; 
				LSB_count = RSB_count = LCB_count = RCB_count = 0;	
	unsigned short int Dot_count = 0;
	unsigned short int temp_count = 0; 
	
	unsigned int char_till_comma, char_till_RCB, char_till_RSB, char_till_delimiter;
					char_till_comma = char_till_RCB = char_till_RSB = char_till_delimiter= 0;
	unsigned short int getln_delim = '0';	
	char yet_another_one_indicator = '0';
	
	std::regex reg_num("-?(?:0|[1-9]\\d*)(?:\\.\\d+)?(?:[eE][+-]?\\d+)?");
	std::regex reg_TFN("true|false|null", std::regex_constants::icase);
	
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
	char chr_tmp = '0';
	
	Stack<char> AO_indicator(100);												// Maximum nesting level for both arrays and objects - 100.
	Stack<unsigned short int> Arr_indexes(100);									// Maximum (nested) arrays' indexes to be stored(max nest level) - 100. 
	unsigned short int Arr_index = 0;
	
	std::string str_buff(50, ' '), str_number(50, ' '), str_TFN(50, ' ');			
	std::string current_key_path(500, ' ');						
	
	thefile.seekg(0, std::ios::beg);
		
	while (thefile)
	{			
		str_buff.clear();
		str_number.clear();
		str_TFN.clear();
		
		thefile.tellg();
		chr_tmp = thefile.peek();		
		
		if(!out_file.is_open()){
			out_file.open("Output File.txt", std::ios::app);
		}else{
		}
		
		if(State_var==Mode_0){						// Initial Mode is Zero. Verifies initial LCB and QM. Passes the control to Mode One. 
			thefile.get(chr);	
			if(chr == '{'){
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
			
			thefile.get(chr);
			if(chr == ':'){				
				State_var = Mode_5;	
				State_previous = Mode_1;
				continue;				
			}else{
				Error_msg("Err_06");
				break;
			}		
		}
		
		if(State_var==Mode_5){						// Mode Five switches to appropriate Mode for Value handling according to input. 
			thefile.get(chr);	
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
			of_pindex_prev = of_pindex_curr;
			 
			if(chr == '\"'){							
				std::getline (thefile, str_buff, '\"');
				out_file << str_buff;
				yet_another_one_indicator = 's';
			}else if(isdigit(chr)||chr == '-'){				
				unsigned int saved_index = thefile.tellg();
				saved_index = saved_index - 1;
				char_till_delimiter = 0;
				
				while((chr!=',')&&(chr!=']')&&(chr!='}')){
					thefile.get(chr);
					char_till_delimiter++;
				}
			
				thefile.seekg(saved_index);
				
				while(char_till_delimiter>0){
					thefile.get(chr);
					str_number += chr;
					char_till_delimiter--;
				}
							
				if(regex_match(str_number, reg_num)){
					out_file << str_number;
				}else{
					Error_msg("Err_06");
					std::cerr<< "Improper number format in value field. ";
					break;					
				}
				yet_another_one_indicator = 'n';
			}else if(chr == 't'||chr == 'f'||chr == 'n'||chr == 'T'||chr == 'F'||chr == 'N'){										// Not tested
				unsigned int saved_index = thefile.tellg();
				char_till_delimiter = 0;
				
				while((chr!=',')&&(chr!=']')&&(chr!='}')){
					thefile.get(chr);
					char_till_delimiter++;
				}				
				thefile.seekg(saved_index);
				
				while(char_till_delimiter>0){
					thefile.get(chr);
					str_number += chr;
					char_till_delimiter--;
				}
				
				if(regex_match(str_TFN, reg_TFN)){
					out_file << str_TFN;
				}else{
					Error_msg("Err_06");
					std::cerr<< "Improper number format in value field. ";
					break;					
				}
				yet_another_one_indicator = 'l';
			}else{
				std::cout<< "Wrong Mode_4 invocation. Aborted. (Internal Error) "<< '\n';
				break;							
			}		
			out_file.seekp(0, std::ios::end);
			of_pindex_curr = out_file.tellp();
				
			switch(yet_another_one_indicator){
				case 's':
				thefile.get(chr);
				break;
				case 'n':					
				thefile.get(chr);
				break;
				case 'l':
				// ?? 
				break;
				default:
				std::cout<< "Could not to switch to any \"yet_another_one_indicator\"-branch. (Internal Error) ";
				break;	
			}
						
			if(chr == ','){						
				State_var = Mode_6;			
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
			out_file << std::endl;
			if((AO_indicator.getload()!=0)&&(AO_indicator.peek()=='A')){								// Comma in Array
				if((LSB_count-RSB_count)>0){				
					out_file.close();
					out_file.open("Output File.txt");					
					out_file.seekg(of_pindex_prev +2);				
					temp_count = (LSB_count-RSB_count);
					chr_tmp = '0';
					std::streamoff iii = 1;
													
					while(temp_count>=1){
						while(chr_tmp!='['){
							chr_tmp = out_file.peek();
							if((chr_tmp=='[')){
								if(temp_count==1){
									break;
								}else{
									str_buff += chr_tmp;
									index_temp = out_file.tellg();
									out_file.seekg(index_temp+2);
								}	
							}
							str_buff += chr_tmp;
							out_file.seekg(of_pindex_prev +2 +iii);	
							iii++;
						}						
						temp_count--;
					}
					out_file.seekp(0, std::ios::end);
					out_file << str_buff;
					State_var = Mode_2;	
				}else{
					Error_msg("Err_06");						// 	
					std::cerr << "  LSB_count <= RSB_count. Cannot be in array. (internal error) " << '\n';
					break;
				}		
			}else if((AO_indicator.getload()!=0)&&(AO_indicator.peek()=='O')){			 				// Comma in Object
				if((LCB_count-RCB_count)>0){															
					out_file.seekp(of_pindex_prev);
					temp_count = (LCB_count-RCB_count);
					while(temp_count>=1){
						out_file.get(chr);
						std::getline (out_file, str_buff, '.');	
						temp_count--;
						out_file << str_buff;						
					}
					State_var = Mode_1;
				}else{
					Error_msg("Err_06");						//   
					std::cout << " LCB_count - RCB_count <= 0. Сannot be in object. (internal error) " << '\n';
					break;
				}			
			}else{
				chr = thefile.peek();
				thefile.get(chr);
				if(chr == '\"'){
					State_var = Mode_1;
				}else{
					Error_msg("Err_06");						//  
					std::cout << "Expected Key name string after comma when outside array/object." << '\n';
					break;
				}
			}		 
		}	
			
		if(State_var==Mode_2){						// Mode Two types indexes to Output and controls "currently in an array" indication.
			if(chr == '['){
				LSB_count++;
				Arr_index = 0;
				Arr_indexes.push(Arr_index);
				AO_indicator.push('A');
				out_file << '[' << Arr_index << ']';
				State_var = Mode_4;					
				thefile.get(chr);					
			}else if(chr == ']'){
				RSB_count++;
				Arr_indexes.pop();
				AO_indicator.pop();					
				thefile.get(chr);					
				if(chr == ','){
					State_var = Mode_6;
					thefile.get(chr);
				}else if(chr == ']'){
					State_var = Mode_2;
				}else if(chr == '}'){
					State_var = Mode_3;
				}else{
					Error_msg("Err_06");			// " Expected RSB, RCB or Comma after array ended. "
					break;
				}			
			}else{
				Arr_index = (Arr_indexes.peek() + 1);
				Arr_indexes.push(Arr_index);
				out_file << '[' << Arr_index << ']';
				State_var = Mode_5;					
			}
		}
	
		if(State_var==Mode_3){						// Mode Three types dots to Output and controls "currently in an object" indication. 
			if(chr == '{'){
				LCB_count++;
				AO_indicator.push('O');
				out_file << '.';
				Dot_count++;
				State_var = Mode_1;
			}else if(chr == '}'){
				RCB_count++;
				Dot_count--;						
				AO_indicator.pop();		
				thefile.get(chr);
				if(chr == ','){
					State_var = Mode_6;
					thefile.get(chr);
				}else if(chr == ']'){
					State_var = Mode_2;
				}else if(chr == '}'){
					State_var = Mode_3;
				}else{
					Error_msg("Err_06");			// " Expected RSB, RCB or Comma after object ended. "
					break;
				}			
			}else{
				out_file << '.';
				State_var = Mode_4;
			}			
		}	
		out_file.close();
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
			  << "May use only file names(with extensions), if files are on same path with executable. \n"
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



