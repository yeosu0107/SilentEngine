#include "stdafx.h"
#include "Texture.h"

using std::wstring;
using std::unique_ptr;
CTexture::CTexture() : m_sPATH("Resource\\TestResource\\Tedori.png")
{	
	/*		정의		*/
	wstring a = std::wstring(m_sPATH.begin() , m_sPATH.end());
	unique_ptr<ScratchImage> image(new ScratchImage);

	HRESULT hr;

	IWICImagingFactory* m_piwiImageingFactory = NULL;
	
	const wchar_t* wcpPATH = a.c_str();

	/*		초기화		*/
	CoInitialize(m_piwiImageingFactory);

	hr = LoadFromWICFile(wcpPATH, WIC_FLAGS_NONE, nullptr, *image);
	/*
		LoadFromWICFile( *szFile , flags , *metadata , image )

		szFile  : File 이름( .BMP, .PNG, .GIF, .TIFF, .JPEG )
		flag	: https://directxtex.codeplex.com/wikipage?title=WIC%20I%2fO%20Functions&referringTitle=DirectXTex
					Related Flags 항목 참조
		metadata: 이미지의 서술자 ( SaveWICFile 함수에 주로 쓰인다. )
		image	: 불러온 ScratchImage를 저장할 포인터.
	*/

	if (SUCCEEDED(hr))
		std::cout << "성공" << std::endl;

	else
		std::cout << "실패" << std::endl;
}


CTexture::~CTexture()
{
}
