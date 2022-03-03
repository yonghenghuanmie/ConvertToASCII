#pragma once

class ConvertToASCII
{
	class ConvertToFileName;
public:
	enum SaveAs
	{
		TEXT,HTML
	};

	ConvertToASCII(ConvertToFileName &c):converter(c){}
	ConvertToASCII(ConvertToFileName &&c):converter(c){}
	void Convert(SaveAs as=SaveAs::TEXT);
	void MutilThreadConvert(SaveAs as=SaveAs::TEXT);

private:
	void Save(SaveAs as,std::string &filename,unsigned char *BitmapBits,BITMAPINFOHEADER &BitmapInfoHeader);
	std::vector<std::string> WcharToUTF8(const std::wstring &str);

	class ConvertToFileName
	{
	public:
		ConvertToFileName(const char *filename,int count=1,int index=1,int bits=1)
		{
			this->filename=filename;
			this->count=count;
			this->current=0;
			this->index=index;
			this->bits=bits;
		}

		std::string GetFileName();

		operator bool()
		{
			mutex.lock();
			bool exist=current<count;
			mutex.unlock();
			return exist;
		}

	private:
		const char *filename;
		int count;
		int current;
		int index;
		int bits;
		std::mutex mutex;
	}&converter;

	class Scale
	{
	public:
		Scale(const BITMAPINFOHEADER &BitmapInfoHeader,int pixel_x,int pixel_y=-1)
		{
			prepixel_x=BitmapInfoHeader.biWidth/pixel_x;
			if(pixel_y==-1)
				prepixel_y=prepixel_x;
			else
				prepixel_y=BitmapInfoHeader.biHeight/pixel_y;
			this->BitmapInfoHeader=BitmapInfoHeader;
		}

		Scale(const BITMAPINFOHEADER &BitmapInfoHeader,double percent_x,double percent_y=0)
		{
			prepixel_x=(int)(1/percent_x);
			if(percent_y==0)
				percent_y=percent_x;
			prepixel_y=(int)(1/percent_y);
			this->BitmapInfoHeader=BitmapInfoHeader;
		}

		unsigned int GetBirghtness(unsigned char *&line,int &pos_x,int &pos_y,RGBTRIPLE &triple);

	private:
		BITMAPINFOHEADER BitmapInfoHeader;
		int prepixel_x,prepixel_y;
	};
	std::string head="<html><head><meta charset=\"UTF-8\"><style type=\"text/css\">*{font-size:1mm;letter-spacing:0.05;line-height:1;}</style></head>";
	static constexpr char *tail="</html>";
	const std::wstring wstr_element=L"█▇▆▅▄▃▂▁·";
	const int level=0x300/(int)wstr_element.size();
};