// FlashNess.cpp : CFlashNess ��ʵ��
#include "stdafx.h"
#include "FlashNess.h"


// CFlashNess


STDMETHODIMP CFlashNess::CloseDevice(SHORT device)
{
	// TODO: �ڴ����ʵ�ִ���
	::MessageBox(NULL, L"CloseDevice", L"NPAPI", NULL);
	return S_OK;
}
