#pragma once

class TiXmlDocument;
class TiXmlElement;
class CGUIFont;

class MMLevelIntro
{
public:
	MMLevelIntro(const char *levelDirPath, const char *fileName, const char *levelUserName, const char *author);
	~MMLevelIntro();
	bool WindowShown() {return cw != 0;}
private:
	class CGUIStdWindow *cw;
	void ShowXmlError(TiXmlDocument &doc);
	TiXmlElement * SelectRootByLanguage(TiXmlDocument &doc);
	void DisplayIntro(TiXmlElement * root, const char *levelUserName, const char *author);
	void DisplayText(TiXmlElement * text);
	void DisplayImage(TiXmlElement * image);

	CGUIFont *font;
	CGUIFont *fontB;
	CGUIFont *fontN;
	int headerSize = 90;

};
