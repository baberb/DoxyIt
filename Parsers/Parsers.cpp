//This file is part of DoxyIt.
//
//Copyright (C)2013 Justin Dailey <dail8859@yahoo.com>
//
//DoxyIt is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "Parsers.h"
#include <vector>

const char *default_format = "\
\r\n\
#brief Brief\r\n\
\r\n\
#param [in] $(PARAM) |Parameter_Description\r\n\
#return Return_Description\r\n\
\r\n\
#details Details\r\n\
";

// Very ugly macro
#define REGISTER_PARSER(lang, parser, language_name, doc_start, doc_line, doc_end, command_prefix, example) \
	{L_##lang, TEXT(#lang),TEXT(language_name), example, \
	{"", "", "", "", "", false}, \
	{doc_start, doc_line, doc_end, command_prefix, default_format, false}, \
	Initialize_##parser, CleanUp_##parser, Parse_##parser}

Parser parsers[] = 
{
	REGISTER_PARSER(C,      C,      "C",          "/**",  " *  ", " */",  "\\", "int function(const char *ptr, int index)"),
	REGISTER_PARSER(CPP,    C,      "C++",        "/**",  " *  ", " */",  "\\", "std::string function(const char *ptr, int &index)"),
	REGISTER_PARSER(JAVA,   C,      "Java",       "/**",  " *  ", " */",  "@",  "public boolean action(Event event, Object arg)"),
	REGISTER_PARSER(PYTHON, Python, "Python",     "## ",  "#  ",  "#  ",  "@",  "def foo(bar, string=None)"),
	REGISTER_PARSER(PHP,    C,      "PHP",        "/**",  " *  ", " */",  "@",  "function myFunction($abc, $defg)"),
	REGISTER_PARSER(JS,     C,      "JavaScript", "/**",  " *  ", " */",  "@",  "function myFunction(abc, defg)"),
	REGISTER_PARSER(CS,     C,      "C#",         "/// ", "/// ", "/// ", "\\", "public int Method(ref int abc, int defg)")
};

const Parser *getParserByName(std::wstring name)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		if(parsers[i].language_name == name)
			return &parsers[i];

	return NULL;
}

// 'update' causes the current parser pointer to be updated
// else just return the currently cached parser
const Parser *getCurrentParser(bool update)
{
	static const Parser *current = NULL;

	if(update)
	{
		int lang_type;
		int len = sizeof(parsers) / sizeof(parsers[0]);
		SendNpp(NPPM_GETCURRENTLANGTYPE, SCI_UNUSED, (LPARAM) &lang_type);

		for(int i = 0; i < len; ++i)
		{
			if(parsers[i].lang_type == lang_type)
			{
				current = &parsers[i];
				return current;
			}
		}

		// Parser wasn't found for current language
		current = NULL;
	}

	return current;
}

const ParserDefinition *getCurrentParserDefinition(void)
{
	const Parser *p;

	p = getCurrentParser();
	if(p) return &p->pd;
	else return NULL;
}



void InitializeParsers(void)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		if((*parsers[i].initializer)() == false)
			MessageBox(NULL, TEXT("DoxyIt initialization failed"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
}

void CleanUpParsers(void)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		(*parsers[i].cleanup)();
}



std::string formatBlock(const ParserDefinition *pd, std::map<std::string, std::vector<std::string>>& keywords)
{
	std::stringstream ss(pd->format);
	std::vector<std::string> lines;
	std::vector<std::string> params = keywords["$(PARAM)"];
	const char *eol = getEolStr();

	std::string aline;
	while(std::getline(ss, aline)) lines.push_back(aline);
	lines.push_back("");

	ss.clear(); // re use
	for(unsigned int i = 0; i < lines.size(); ++i)
	{
		if(lines[i].find("$(PARAM)") != std::string::npos)
		{
			unsigned int align_max = 0;
			std::vector<std::string> formatted_lines;
			for(unsigned int j = 0; j < params.size(); ++j)
			{
				std::string line;
				line = stringReplace(lines[i], "$(PARAM)", params[j]);

				unsigned int pipe = line.find('|');
				if(pipe != std::string::npos)
				{
					align_max = max(pipe, align_max);
				}
				
				formatted_lines.push_back(line);
			}
			if(align_max != 0)
			{
				for(unsigned int j = 0; j < formatted_lines.size(); ++j)
				{
					std::string formatted_line = formatted_lines[j];
					int a = formatted_line.find('|');
					formatted_line.replace(formatted_line.find('|'), 1, align_max - a, ' ');
					formatted_lines[j] = formatted_line;
				}
			}
			for(unsigned int j = 0; j < formatted_lines.size(); ++j)
			{
				if(i == 0) ss << pd->doc_start << formatted_lines[j] << eol;
				else if(i == lines.size() -1) ss << pd->doc_end << formatted_lines[j] << eol;
				else ss << pd->doc_line << formatted_lines[j] << eol;
			}
		}
		else
		{
			if(i == 0) ss << pd->doc_start << lines[i] << eol;
			else if(i == lines.size() -1) ss << pd->doc_end << lines[i];
			else ss << pd->doc_line << lines[i] << eol;
		}
	}

	return stringReplace(ss.str(), "#", pd->command_prefix);
}

std::string ParseFormatted(const Parser *p, const ParserDefinition *pd, const char *text)
{
	return formatBlock(pd, p->callback(pd, text));
}

// Get the current parser and text to parse
std::string Parse(void)
{
	std::string doc_block;
	const Parser *p;
	int found, curLine, foundLine;
	char *buffer;

	if(!(p = getCurrentParser()))
	{
		MessageBox(NULL, TEXT("Unrecognized language type."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	// Get the text until a closing parenthesis. Find '(' first
	if((found = findNext("(")) == -1)
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	// Do some sanity checking. Make sure curline <= found <= curline+2
	curLine = SendScintilla(SCI_LINEFROMPOSITION, SendScintilla(SCI_GETCURRENTPOS));
	foundLine = SendScintilla(SCI_LINEFROMPOSITION, found);
	if(foundLine < curLine || foundLine > curLine + 2)
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	// Find the matching closing brace
	if((found = SendScintilla(SCI_BRACEMATCH, found, 0)) == -1)
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	buffer = getRange(SendScintilla(SCI_GETCURRENTPOS), found + 1);
	doc_block = ParseFormatted(p, &p->pd, buffer);
	delete[] buffer;

	// I don't think there is currently a case where callback() will return a zero length string,
	// but check it just in case we decide to for the future.
	if(doc_block.length() == 0)
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	return doc_block;
}
