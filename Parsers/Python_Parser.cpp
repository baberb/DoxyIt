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

static TRex *tr_function;
static TRex *tr_parameters;

bool Initialize_PYTHON(void)
{
	const TRexChar *error = NULL;
	tr_function = trex_compile("def\\s+(\\w+)\\s*(\\([^)]*\\))", &error);
	tr_parameters = trex_compile("(\\w+)(\\s*=\\s*\\w+)?\\s*[,)]", &error);

	if(!tr_function || !tr_parameters) return false;
	return true;
}

void CleanUp_PYTHON(void)
{
	trex_free(tr_function);
	trex_free(tr_parameters);
}

std::string Parse_PYTHON(const ParserDefinition *pd, const char *text)
{
	const TRexChar *begin,*end;
	const char *eol;
	std::ostringstream doc_block;

	eol = getEolStr();

	if(trex_search(tr_function, text, &begin, &end))
	{
		TRexMatch func_match;
		TRexMatch params_match;
		const TRexChar *cur_params;
		const TRexChar *p_begin, *p_end;

		trex_getsubexp(tr_function, 1, &func_match);
		trex_getsubexp(tr_function, 2, &params_match);

		doc_block << pd->doc_start << eol;
		doc_block << pd->doc_line << pd->command_prefix << "brief " << FT("[Brief]") << eol;
		doc_block << pd->doc_line << eol;

		// For each param
		cur_params = params_match.begin;
		while(trex_searchrange(tr_parameters, cur_params, end, &p_begin, &p_end))
		{
			TRexMatch param_match;
			trex_getsubexp(tr_parameters, 1, &param_match);
			std::string desc;

			desc += "[Param ";
			desc.append(param_match.begin, param_match.len);
			desc += " Description]";

			doc_block << pd->doc_line << pd->command_prefix << "param [in] ";
			doc_block.write(param_match.begin, param_match.len);
			doc_block << " " << FT(desc.c_str()) << eol;

			cur_params = p_end;
		}

		// Return value
		doc_block << pd->doc_line << pd->command_prefix << "return ";
		doc_block << FT("[Return Description]") << eol;
		doc_block << pd->doc_line << eol;

		doc_block << pd->doc_line << pd->command_prefix << "details " << FT("[Details]") << eol;
		doc_block << pd->doc_end;
	}

	return doc_block.str();
}