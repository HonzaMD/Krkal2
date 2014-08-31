#pragma once

class TiXmlDocument;
class TiXmlElement;
class CGUIFont;

class MMLevelIntro
{
public:
	MMLevelIntro(const char *levelDirPath);
	~MMLevelIntro();
	bool WindowShown() {return cw != 0;}
private:
	class CGUIStdWindow *cw;
	void ShowXmlError(TiXmlDocument &doc);
	TiXmlElement * SelectRootByLanguage(TiXmlDocument &doc);
	void DisplayIntro(TiXmlElement * root);
	void DisplayHeader(TiXmlElement * header);
	void DisplayText(TiXmlElement * text);
	void DisplayImage(TiXmlElement * image);

	CGUIFont *font;
	CGUIFont *fontB;
	CGUIFont *fontN;

};
