/////////////////////////////////////////////////////////////////////////////
//
// fileReader.h
//
// fileReader pro Audiere
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////


#include "audiere.h"

namespace audiere{

	class CKrkalFile: public RefImplementation<File>{
	public:
		CKrkalFile(char *filename, FileFormat &ff );
		~CKrkalFile();

		int ADR_CALL read(void* buffer, int size) {
			if(filepos+size>filesize) 
				size=filesize-filepos;
			memcpy(buffer,filedata+filepos,size);
			filepos+=size;

			//fread(buffer,1,size,f);

			return size;
		}

		bool ADR_CALL seek(int position, SeekMode mode) {

			int st;

			switch (mode) {
				case BEGIN:   
					st=SEEK_SET;
					filepos = position; break;
				case CURRENT: 
					st=SEEK_CUR;
					filepos+= position; break;
				case END:     
					st=SEEK_END;
					filepos = filesize+position; break;
				default: 
					return 0;

			}
			//return fseek(f,position,st)==0;
			if (filepos < 0 || filepos > filesize) {
				filepos = 0;
				return 0;
			}

			return 1;
		}

		int ADR_CALL tell() {
			//return ftell(f);
			return filepos;
		}

	private:
		char *filedata,*filebuf;
		int filesize,filepos;

	};
 
};