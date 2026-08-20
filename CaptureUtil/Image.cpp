#include <windows.h>
#include <tchar.h>
#include <wincodec.h>
#include <vector>
#include "Image.h"

#pragma comment(lib, "windowscodecs.lib")

SIZE_T CalcDIBInfoSize(const BITMAPINFOHEADER* pbmih)
{
	SIZE_T Size;
	Size = sizeof(BITMAPINFOHEADER);
	if (pbmih->biBitCount <= 8)
		Size += ((SIZE_T)1 << pbmih->biBitCount) * sizeof(RGBQUAD);
	else if (pbmih->biCompression == BI_BITFIELDS)
		Size += 3 * sizeof(DWORD);
	return Size;
}
SIZE_T CalcDIBBitsSize(const BITMAPINFOHEADER* pbmih)
{
	return DIB_ROW_BYTES(pbmih->biWidth, pbmih->biBitCount) * abs(pbmih->biHeight);
}
SIZE_T CalcDIBSize(const BITMAPINFOHEADER* pbmih)
{
	return CalcDIBInfoSize(pbmih) + CalcDIBBitsSize(pbmih);
}

CImageCodec::CImageCodec()
{
	m_hLib = NULL;
	m_pSaveImage = NULL;
}
CImageCodec::~CImageCodec()
{
}

bool CImageCodec::Init()
{
	// WICを使うため、外部DLLの読み込みはもう不要
	return true;
}

bool CImageCodec::SaveImage(LPCTSTR pszFileName, int Format, LPCTSTR pszOption,
	const BITMAPINFO* pbmi, const void* pBits, LPCTSTR pszComment)
{
	HRESULT hrInit = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	bool needUninit = (hrInit == S_OK || hrInit == S_FALSE);

	bool result = false;
	IWICImagingFactory* pFactory = NULL;
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory, (void**)&pFactory);

	if (SUCCEEDED(hr)) {
		IWICStream* pStream = NULL;
		if (SUCCEEDED(pFactory->CreateStream(&pStream))) {
			if (SUCCEEDED(pStream->InitializeFromFilename(pszFileName, GENERIC_WRITE))) {
				GUID containerFormat;
				switch (Format) {
				case 1:  containerFormat = GUID_ContainerFormatJpeg; break;
				case 2:  containerFormat = GUID_ContainerFormatPng;  break;
				default: containerFormat = GUID_ContainerFormatBmp; break;
				}

				IWICBitmapEncoder* pEncoder = NULL;
				if (SUCCEEDED(pFactory->CreateEncoder(containerFormat, NULL, &pEncoder))) {
					if (SUCCEEDED(pEncoder->Initialize(pStream, WICBitmapEncoderNoCache))) {
						IWICBitmapFrameEncode* pFrame = NULL;
						IPropertyBag2* pPropertyBag = NULL;
						if (SUCCEEDED(pEncoder->CreateNewFrame(&pFrame, &pPropertyBag))) {

							// JPEGの場合、品質設定を反映
							if (Format == 1 && pPropertyBag != NULL && pszOption != NULL && pszOption[0] != TEXT('\0')) {
								PROPBAG2 option = {};
								option.pstrName = (LPOLESTR)L"ImageQuality";
								VARIANT varValue;
								VariantInit(&varValue);
								varValue.vt = VT_R4;
								int level = _wtoi(pszOption);
								if (level < 1) level = 1;
								if (level > 100) level = 100;
								varValue.fltVal = (FLOAT)level / 100.0f;
								pPropertyBag->Write(1, &option, &varValue);
							}

							if (SUCCEEDED(pFrame->Initialize(pPropertyBag))) {
								UINT width = (UINT)pbmi->bmiHeader.biWidth;
								UINT height = (UINT)abs(pbmi->bmiHeader.biHeight);

								pFrame->SetSize(width, height);
								WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat24bppBGR;
								pFrame->SetPixelFormat(&pixelFormat);

								// pBitsは下から上(BMP標準)に格納されているため、上から下に並べ替える
								UINT rowBytes = width * 3;
								std::vector<BYTE> topDownBuf((size_t)rowBytes * height);
								const BYTE* pSrc = (const BYTE*)pBits;
								for (UINT y = 0; y < height; y++) {
									memcpy(&topDownBuf[(size_t)y * rowBytes],
										pSrc + (size_t)(height - 1 - y) * rowBytes, rowBytes);
								}

								if (SUCCEEDED(pFrame->WritePixels(height, rowBytes,
									rowBytes * height, topDownBuf.data()))) {
									if (SUCCEEDED(pFrame->Commit())) {
										if (SUCCEEDED(pEncoder->Commit())) {
											result = true;
										}
									}
								}
							}
							if (pPropertyBag != NULL) pPropertyBag->Release();
							pFrame->Release();
						}
					}
					pEncoder->Release();
				}
			}
			pStream->Release();
		}
		pFactory->Release();
	}

	if (needUninit) CoUninitialize();
	return result;
}

LPCTSTR CImageCodec::EnumSaveFormat(int Index) const
{
	switch (Index) {
	case 0:	return TEXT("BMP");
	case 1:	return TEXT("JPEG");
	case 2:	return TEXT("PNG");
	}
	return NULL;
}
LPCTSTR CImageCodec::GetExtension(int Index) const
{
	switch (Index) {
	case 0:	return TEXT("bmp");
	case 1:	return TEXT("jpg");
	case 2:	return TEXT("png");
	}
	return NULL;
}
int CImageCodec::FormatNameToIndex(LPCTSTR pszName) const
{
	int i;
	LPCTSTR pszFormat;
	for (i = 0; (pszFormat = EnumSaveFormat(i)) != NULL; i++) {
		if (lstrcmpi(pszName, pszFormat) == 0)
			return i;
	}
	return -1;
}