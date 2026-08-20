#ifndef TVTEST_IMAGE_H
#define TVTEST_IMAGE_H


struct ImageSaveInfo {
	LPCWSTR pszFileName;
	LPCWSTR pszFormat;
	LPCWSTR pszOption;
	const BITMAPINFO *pbmi;
	const void *pBits;
	LPCWSTR pszComment;
};

typedef BOOL (WINAPI *SaveImageFunc)(const ImageSaveInfo *pInfo);


#endif
