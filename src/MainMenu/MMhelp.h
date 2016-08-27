///////////////////////////////////////////////
//
// MMhelp.h
//
// help&credits
//
// A: Petr Altman
//
///////////////////////////////////////////////

#ifndef MAINMENU_HELP_H
#define MAINMENU_HELP_H


class CMainMenuHelp
{
public:
	void ShowCredits();
	void ShowHelp();
private:
	const char *ConvertKeys(const char *text, char buffer[1024]);
};


#endif
