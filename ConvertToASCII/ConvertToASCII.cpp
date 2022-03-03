#include <cmath>
#include <climits>
#include <mutex>
#include <stack>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <windows.h>
#include "ConvertToASCII.h"

std::string ConvertToASCII::ConvertToFileName::GetFileName()
{
	if(operator bool())
	{
		mutex.lock();
		std::string str=filename;
		auto pos=str.find('#');
		if(pos!=std::string::npos)
		{
			std::string ordinal=std::to_string(index+current);
			auto size=bits-ordinal.size();
			if(size<0)
				throw std::runtime_error("given bits less than ordinal bits.");
			str.replace(pos,1,std::string(size,'0')+ordinal);
		}
		++current;
		mutex.unlock();
		return str;
	}
	return "";
}

void ConvertToASCII::Convert(SaveAs as)
{
	while(converter)
	{
		std::string filename=converter.GetFileName();
		std::ifstream input(filename,std::ifstream::binary);
		if(input)
		{
			BITMAPFILEHEADER BitmapFileHeader;
			input.read((char*)&BitmapFileHeader,sizeof(BitmapFileHeader));
			//BM
			if(BitmapFileHeader.bfType!='MB')
				throw std::runtime_error("this file isn't a bitmap.");
			BITMAPINFOHEADER BitmapInfoHeader;
			input.read((char*)&BitmapInfoHeader,sizeof(BitmapInfoHeader));
			if(BitmapInfoHeader.biBitCount!=24&&BitmapInfoHeader.biBitCount!=32)
				throw std::runtime_error("only supprot 24 or 32 bits bitmap.");
			//还TM有不填充BITMAPINFOHEADER::biSizeImage这个域的!!!
			int size=BitmapInfoHeader.biHeight*BitmapInfoHeader.biWidth*4;
			char *buffer=new char[size];
			input.read(buffer,size);
			Save(as,filename,(unsigned char*)buffer,BitmapInfoHeader);
			delete[] buffer;
			input.close();
		}
		else throw std::runtime_error(std::string("failed to open file ")+filename+".");
	}
}

void ConvertToASCII::MutilThreadConvert(SaveAs as)
{
	std::vector<std::thread> set;
	while(converter)
		set.push_back(std::thread(&ConvertToASCII::Convert,this,as));
	for(auto &t:set)
		t.join();
}

void ConvertToASCII::Save(SaveAs as,std::string & filename,unsigned char * BitmapBits,BITMAPINFOHEADER &BitmapInfoHeader)
{
	if(as==HTML)
		filename.append(".html");
	else
		filename.append(".txt");
	std::ofstream output(filename,std::ofstream::binary);
	if(output)
	{
		std::vector<std::ostringstream> v;
		auto utf8_set=WcharToUTF8(wstr_element);
		Scale scale(BitmapInfoHeader,150);
		//Scale scale(BitmapInfoHeader,1.0);
		for(int i=0;i<std::abs(BitmapInfoHeader.biHeight);++i)
		{
			std::ostringstream osstream;
			for(int j=0;j<BitmapInfoHeader.biWidth;++j)
			{
				RGBTRIPLE triple;
				int ordinal=scale.GetBirghtness(BitmapBits,j,i,triple)/level;
				if(as==HTML)
				{
					std::ostringstream color_value;
					color_value<<std::hex<<(int)triple.rgbtRed<<(int)triple.rgbtGreen<<(int)triple.rgbtBlue;
					osstream<<"<span style=color:#"<<color_value.str()<<">";
					ordinal=0;
				}
				else if(ordinal==utf8_set.size())
						ordinal-=1;
				osstream<<utf8_set[ordinal];
				if(as==HTML)
					osstream<<"</span>";
			}
			v.push_back(std::move(osstream));
		}

		//UTF-8 header
		output<<(char)0xEF<<(char)0xBB<<(char)0xBF;
		int begin=(int)v.size()-1,end=-1,additions=-1;
		if(BitmapInfoHeader.biHeight<0)
		{
			begin=0,end=(int)v.size();
			additions=1;
		}
		if(as==HTML)
			output<<head;
		for(;begin!=end;begin+=additions)
			output<<v[begin].str()<<std::endl;
		if(as==HTML)
			output<<tail;
		output.close();
	}
	else throw std::runtime_error(std::string("failed to open file ")+filename+".");
}

std::vector<std::string> ConvertToASCII::WcharToUTF8(const std::wstring &str)
{
	std::vector<std::string> results;
	char *buffer=new char[4];
	for(auto &i:str)
	{
		std::size_t size=WideCharToMultiByte(CP_UTF8,0,&i,1,buffer,4,nullptr,nullptr);
		if(size==0)
			throw std::runtime_error("failed to call WideCharToMultiByte.");
		results.push_back({buffer,size});
	}
	delete[] buffer;
	return results;
}

unsigned int ConvertToASCII::Scale::GetBirghtness(unsigned char * &line,int & pos_x,int & pos_y,RGBTRIPLE &triple)
{
	bool LF=false;
	unsigned char *backup=line;
	int sum=0,count=0,width=BitmapInfoHeader.biBitCount/CHAR_BIT,sumR=0,sumG=0,sumB=0,
		offset=(&line[BitmapInfoHeader.biWidth*width]-line+3)&~3;
	for(int i=0;i<prepixel_y&&pos_y+i<std::abs(BitmapInfoHeader.biHeight);++i)
	{
		for(int j=0;j<prepixel_x&&pos_x+j<BitmapInfoHeader.biWidth;++j)
		{
			int index=(pos_x+j)*width;
			sumB+=line[index];
			sumG+=line[index+1];
			sumR+=line[index+2];
			sum+=line[index]+line[index+1]+line[index+2];
			++count;
		}
		if(pos_x+prepixel_x-1>=BitmapInfoHeader.biWidth-1)
			LF=true;
		line+=offset;
	}
	if(LF)
	{
		pos_x=BitmapInfoHeader.biWidth-1;
		pos_y+=prepixel_y-1;
	}
	else
	{
		pos_x+=prepixel_x-1;
		line=backup;
	}
	triple.rgbtBlue=sumB/count;
	triple.rgbtGreen=sumG/count;
	triple.rgbtRed=sumR/count;
	return sum/count;
}
