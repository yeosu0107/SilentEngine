#include "stdafx.h"
#include "Texture.h"

using std::wstring;
using std::unique_ptr;
CTexture::CTexture() : m_sPATH("Resource\\TestResource\\Tedori.png")
{	
	/*		����		*/
	wstring a = std::wstring(m_sPATH.begin() , m_sPATH.end());
	unique_ptr<ScratchImage> image(new ScratchImage);

	HRESULT hr;

	IWICImagingFactory* m_piwiImageingFactory = NULL;
	
	const wchar_t* wcpPATH = a.c_str();

	/*		�ʱ�ȭ		*/
	CoInitialize(m_piwiImageingFactory);

	hr = LoadFromWICFile(wcpPATH, WIC_FLAGS_NONE, nullptr, *image);
	/*
		LoadFromWICFile( *szFile , flags , *metadata , image )

		szFile  : File �̸�( .BMP, .PNG, .GIF, .TIFF, .JPEG )
		flag	: https://directxtex.codeplex.com/wikipage?title=WIC%20I%2fO%20Functions&referringTitle=DirectXTex
					Related Flags �׸� ����
		metadata: �̹����� ������ ( SaveWICFile �Լ��� �ַ� ���δ�. )
		image	: �ҷ��� ScratchImage�� ������ ������.
	*/

	if (SUCCEEDED(hr))
		std::cout << "����" << std::endl;

	else
		std::cout << "����" << std::endl;
}


CTexture::~CTexture()
{
}
