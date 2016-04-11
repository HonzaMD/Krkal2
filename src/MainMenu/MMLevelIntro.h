#pragma once

class TiXmlDocument;
class TiXmlElement;
class CGUIFont;

class MMLevelIntro
{
public:
	MMLevelIntro(const char *levelDirPath, const char *levelUserName);
	~MMLevelIntro();
	bool WindowShown() {return cw != 0;}
private:
	class CGUIStdWindow *cw;
	void ShowXmlError(TiXmlDocument &doc);
	TiXmlElement * SelectRootByLanguage(TiXmlDocument &doc);
	void DisplayIntro(TiXmlElement * root, const char *levelUserName);
	void DisplayText(TiXmlElement * text);
	void DisplayImage(TiXmlElement * image);

	CGUIFont *font;
	CGUIFont *fontB;
	CGUIFont *fontN;
	const int headerSize = 90;

};
