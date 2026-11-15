#include <cstdio>
#include <chrono>
#include <cstdint>
#include <windows.h>
#include <unordered_map>
#include <string_view>
#include <iostream>
#include <fstream>

#include "Astianide.h"


std::unordered_map<std::string_view, intptr_t> StrIntOps = {
	//call ret eof
	{"extern_call", EXTERN_CALL},
	{"extern_call_ret", EXTERN_CALL_RET},
	{"goto", GOTO},
	{"goto_r1r2", GOTO_R1R2},
	{"call", CALL},
	{"ret", RET},
	{"eof", eOF},
	//mov
	{"mov_ii_vc", MOV_II_VC},
	{"mov_ii_r0c", MOV_II_R0C},
	{"mov_ii_r1c", MOV_II_R1C},
	{"mov_ii_r2c", MOV_II_R2C},
	//if
	{"if_le_ii_vc", IF_LE_II_VC},
	{"if_le_ii_r0c", IF_LE_II_R0C},
	{"if_le_ii_r0r1", IF_LE_II_R0R1},
	{"if_le_ii_r0r2", IF_LE_II_R0R2},
	//add
	{"add_iii_vvc", ADD_III_VVC},
	{"add_ii_vc", ADD_II_VC},
	{"add_iii_vvv", ADD_III_VVV},
	{"sub_iii_vvv", SUB_III_VVV},
	{"sub_iii_vvc", SUB_III_VVC},

	{"lda_r1r2", LDA_R1R2},
	{"lda_xmm0", LDA_XMM0},
	{"lda_xmm1", LDA_XMM1},


	//spec
	{"inc_i",INC_I},
	{"inc_r0",INC_R0},
	{"inc_r0_xmm0",INC_R0_XMM0},
	{"inc_r0_xmm1",INC_R0_XMM1},


};


std::unordered_map<std::string_view, intptr_t> StrIntTypes = {
	{"var_i", VAR_I},
	{"var_s", VAR_S},

	{"r0_i", R0_I},
	{"r1_i", R1_I},
	{"r2_i", R2_I},

	{"lit_i", LIT_I},
	{"lit_s", LIT_S},

};
enum
{
	SECTION_NO,
	SECTION_STRING,
	SECTION_CODE
};

std::unordered_map<std::string_view, intptr_t> StrIntSection = {
	{"section_string", SECTION_STRING},
	{"section_code", SECTION_CODE},
};


std::unordered_map<std::string_view, intptr_t> Labels;


int main(int argc, char* argv[])
{

	if (argc < 2)
	{
		std::cout << "usage: " << argv[0] << " <file>\n";
		return 1;
	}

	std::ifstream infile(argv[1]);
	if (!infile)
	{
		std::cout << "cannot open " << argv[1] << "\n";
		return 1;
	}
	///////////////////////////////////////////////

	std::vector<std::vector<std::string>> t;
	std::string line;



	while (std::getline(infile, line))
	{
		std::vector<std::string> row;
		std::string buffer;

		bool is_open_quote = false;
		bool is_open_escape = false;
		for (size_t i = 0; i < line.size(); ++i)
		{


			if (line[i] == '\"')
			{
				is_open_quote = !is_open_quote;
				continue;
			}

			if (line[i] == '\\')
			{
				is_open_escape = true;
				continue;
			}
			if (is_open_escape)
			{
				is_open_escape = false;

				if (line[i] == '\\')
				{
					buffer += '\\';
				}
				else if (line[i] == 'n')
				{
					buffer += '\n';
				}
				else
				{
					continue;
				}
				continue;
			}



			if (is_open_quote)
			{
				buffer += line[i];
			}
			else
			{
				if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r')
				{
					buffer += line[i];
				}
				else
				{
					if (!buffer.empty())
					{
						row.push_back(buffer);
						buffer.clear();
					}
				}
			}
		}

		if (!buffer.empty())
			row.push_back(buffer);

		if (!row.empty())
			t.push_back(row);
	}


	std::vector<std::vector<std::string>> section_code_s;
	std::vector<std::string> section_string;


	intptr_t current_section = SECTION_NO;

	for (size_t i = 0; i < t.size(); i++)
	{
		if (StrIntSection.contains(t[i][0]))
		{
			current_section = StrIntSection[t[i][0]];
			continue;
		}

		if (current_section == SECTION_CODE)
		{
			section_code_s.push_back(t[i]);
		}
		else if (current_section == SECTION_STRING)
		{
			section_string.push_back(t[i][0]);
		}
		else
		{
			std::cout << "outside the section" << std::endl;
			exit(-2);
		}

	}

	/////// анализ меток 
	intptr_t label_offset = 0;
	for (size_t i = 0; i < section_code_s.size(); i++)
	{

		if (section_code_s[i][0][0] == '.')
		{
			if (Labels.contains(section_code_s[i][0]))
			{
				std::cout << "Duplicate label" << std::endl;
				exit(-2);

			}
			else
			{
				Labels[section_code_s[i][0]] = label_offset;
			}
		}
		else
		{
			label_offset += section_code_s[i].size();
		}
	}

	std::vector<intptr_t> section_code;

	for (size_t i = 0; i < section_code_s.size(); i++)
	{

		if (Labels.contains(section_code_s[i][0]))
		{
			continue;
		}
		else if (StrIntOps.contains(section_code_s[i][0]))
		{
			section_code.push_back(StrIntOps[section_code_s[i][0]]);
			for (size_t j = 1; j < section_code_s[i].size(); j++)
			{
				if (StrIntTypes.contains(section_code_s[i][j]))
				{
					section_code.push_back(StrIntTypes[section_code_s[i][j]]);
				}
				else if (Labels.contains(section_code_s[i][j]))
				{
					section_code.push_back(Labels[section_code_s[i][j]]);
				}
				else
				{
					section_code.push_back(std::atoi(section_code_s[i][j].c_str()));
				}

			}
		}
		else
		{
			std::cout << "invalid opcode " << section_code_s[i][0] << std::endl;
			exit(-3);
		}

	}


	int result = VM_run(section_code.data(), section_code.size(), section_string);

	return result;
}