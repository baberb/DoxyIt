#include "PluginDefinition.h"

extern FuncItem funcItem[nbFunc];
extern NppData nppData;


BOOL APIENTRY DllMain( HANDLE hModule,
					   DWORD  reasonForCall,
					   LPVOID lpReserved )
{
	switch (reasonForCall)
	{
	case DLL_PROCESS_ATTACH:
		pluginInit(hModule);
		break;

	case DLL_PROCESS_DETACH:
		commandMenuCleanUp();
		pluginCleanUp();
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	NotifyHeader nh = notifyCode->nmhdr;
	int ch = notifyCode->ch;

	if(nh.code == SCN_CHARADDED)
	{
		if(ch == '\n' && do_active_commenting)
		{
			doxyItNewLine();
		}
		/*
		else if(do_active_wrapping) // && line starts with doc_line
		{
			int lineMax = 40;
			// Get the line length without counting line endings
			int lineStart = ::SendMessage(curScintilla, SCI_POSITIONFROMLINE, curLine, 0);
			int lineEnd = ::SendMessage(curScintilla, SCI_GETLINEENDPOSITION, curLine, 0);
			int lineLen = lineEnd - lineStart;

			if(lineLen > lineMax)
			{
				int char_width = ::SendMessage(curScintilla, SCI_TEXTWIDTH, STYLE_DEFAULT, (LPARAM) " ");
				::SendMessage(curScintilla, SCI_SETTARGETSTART, lineStart, 0);
				::SendMessage(curScintilla, SCI_SETTARGETEND, lineStart, 0);
				::SendMessage(curScintilla, SCI_LINESSPLIT, lineMax * char_width, 0);

				// Check the next few lines to insert the doc_line in front of them
				for(int i = 1; i < 5; ++i)
				{
					// Get the length and allocate a buffer
					int lineLen = ::SendMessage(curScintilla, SCI_LINELENGTH, curLine + i, 0);
					char *text = new char[lineLen + 1];

					// Get the text
					::SendMessage(curScintilla, SCI_GETLINE, curLine + i, (LPARAM) text);
					text[lineLen] = '\0';

					// if it doesn't start with doc_line or doc_start, insert the doc_line
					// else we are done
					if(strncmp(text, doc_line.c_str(), doc_line.length()) != 0 && strncmp(text, doc_end.c_str(), doc_end.length()) != 0)
					{
						int lineStart = ::SendMessage(curScintilla, SCI_POSITIONFROMLINE, curLine + i, 0);
						::SendMessage(curScintilla, SCI_INSERTTEXT, lineStart, (LPARAM) doc_line.c_str());
					}
					else
					{
						delete[] text;
						break;
					}

					delete[] text;
				}
			}
		}
		*/
	}
}


// Here you can process the Npp Messages
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}
#endif //UNICODE