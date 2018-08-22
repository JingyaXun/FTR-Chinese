#include "function.h"
#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <charwrap.h>

#define CMP_MAXLEN 5
    
int* g_pnHZ = NULL;
int* g_pnIdx = NULL;
unsigned short* g_pnDat = NULL;
int g_nTotal = 0;
int g_WindowSize = 20;

void WriteData(char* psBuff, int nSize, FILE* fpOut, int& nCharNum, int* pnHZ)
{
	nCharNum = 0;
	unsigned short Word;
	for (int i = 0; i<nSize; i++) {
		int nLen = 1;
		if ((unsigned char)psBuff[i] < 0x21) {
			continue;
		}

		if ((unsigned char)psBuff[i] > 0x80) {
			nLen = 2;
		}
        char psBuffTmp[4] = "";
        strncpy(psBuffTmp, &psBuff[i], nLen);
        psBuffTmp[nLen] = 0;
        gbk_to_ucs16((const unsigned char *)&psBuffTmp, (unsigned short*)&Word, 1);
		nCharNum++;
		if (nLen == 2)
			i++;
		pnHZ[Word]++;
		fwrite(&Word, sizeof(unsigned short), 1, fpOut);
	}
}

bool MergeFilesUnicode(char* psFiles[], int nFileNum, char* psData, int*& pnHZ, int& nTotalCharNum)
{
	nTotalCharNum = 0;
	FILE* fpOut;
	fpOut = fopen(psData, "wb");
	if (fpOut == NULL)
		return false;

	for (int i = 0; i<nFileNum; i++) {
		FILE* fpInp;
		fpInp = fopen(psFiles[i], "rb");
		if (fpInp == NULL)
			continue;
		fseek(fpInp, 0, SEEK_END);
		int nSize = ftell(fpInp);
		rewind(fpInp);
		char* psBuff = new char[nSize];
		fread(psBuff, sizeof(char), nSize, fpInp);
		fclose(fpInp);

		int nCharNum;
		WriteData(psBuff, nSize, fpOut, nCharNum, pnHZ);
		nTotalCharNum += nCharNum;

		delete psBuff;
	}
	fclose(fpOut);
	return true;
}


void GetReadyHZ(int* pnHZ)
{
	for (int i = 1; i<0x10000; i++) {
		pnHZ[i] += pnHZ[i - 1];
	}
}

void RecoveryHZ(int* pnHZ)
{
	for (int i = 0x10000 - 1; i>0; i--) {
		pnHZ[i] = pnHZ[i - 1];
	}
	pnHZ[0] = 0;

}


unsigned short* g_pBuffer;
int Compare(const void *arg1, const void *arg2)
{
	//return wcsncmp((unsigned short*)&g_pBuffer[*(int*)arg1], (unsigned short*)&g_pBuffer[*(int*)arg2], CMP_MAXLEN);
	return wcsncmp((const wchar_t*)&g_pBuffer[*(int*)arg1], (const wchar_t*)&g_pBuffer[*(int*)arg2], CMP_MAXLEN);
}


void SortBlock(int nStart, int nEnd, int* pnPOS, unsigned short* psBuff)
{
	g_pBuffer = psBuff;
	if (nEnd - nStart > 1) {
		int i = 0;
	}
	qsort(&pnPOS[nStart], nEnd - nStart, sizeof(int), Compare);
}



void SortIdx(int* pnHZ, int* pnPOS, unsigned short* psBuff)
{
	for (int i = 0; i<0x10000 - 1; i++) {
		SortBlock(pnHZ[i], pnHZ[i + 1], pnPOS, psBuff);
	}
}


bool CreateIdxDat(char* psDat, int*& pnHZ, int*& pnPOS, int nTotalCharNum)
{
	GetReadyHZ(pnHZ);

	FILE* fpInp;
	fpInp = fopen(psDat, "rb");
	if (fpInp == NULL)
		return false;
	unsigned short* psBuff = new unsigned short[nTotalCharNum];
	fread(psBuff, sizeof(unsigned short), nTotalCharNum, fpInp);
	fclose(fpInp);


	pnPOS = new int[nTotalCharNum];
	for (int i = 0; i<nTotalCharNum; i++) {
		pnPOS[pnHZ[psBuff[i] - 1]] = i;
		pnHZ[psBuff[i] - 1]++;
	}

	RecoveryHZ(pnHZ);
	SortIdx(pnHZ, pnPOS, psBuff);

	delete psBuff;
	return true;
}

bool WriteHZ(char* psHZ, int* pnHZ)
{
	FILE* fpOut;
	fpOut = fopen(psHZ, "wb");
	if (fpOut == NULL)
		return false;
	fwrite(pnHZ, sizeof(int), 0x10000, fpOut);
	fclose(fpOut);
	return true;
}

bool WriteIdx(int* pnPOS, int nTotalCharNum, char* psIdxDat)
{
	FILE* fpOut;
	fpOut = fopen(psIdxDat, "wb");
	if (fpOut == NULL)
		return false;

	fwrite(&nTotalCharNum, sizeof(int), 1, fpOut);
	fwrite(pnPOS, sizeof(int), nTotalCharNum, fpOut);
	fclose(fpOut);
	return true;
}



bool CreateIdx(char* psFiles[], int nFileNum, char* psData, char* psHZ, char* psIdxDat)
{
	int* pnHZ = NULL;
	int* pnPOS = NULL;
	int nTotalCharNum;
	pnHZ = new int[0x10000];
	memset(pnHZ, 0, sizeof(int) * 0x10000);

	if (!MergeFilesUnicode(psFiles, nFileNum, psData, pnHZ, nTotalCharNum)) {
		return false;
	}

	if (!CreateIdxDat(psData, pnHZ, pnPOS, nTotalCharNum)) {
		return false;
	}

	if (!WriteHZ(psHZ, pnHZ)) {
		return false;
	}

	if (!WriteIdx(pnPOS, nTotalCharNum, psIdxDat)) {
		return false;
	}


	delete[] pnPOS;
	delete[] pnHZ;
	return true;
}



int FTRInit(char* psHZ, char* psDat, char* psIdx)
{
	FILE* fpInp = fopen(psHZ, "rb"); 
	if (fpInp == NULL)
		goto GOTO_ERROR;

	g_pnHZ = new int[0x10000];
	fread(g_pnHZ, sizeof(int), 0x10000, fpInp);
	fclose(fpInp);


	fpInp = fopen(psIdx, "rb");
	if (fpInp == NULL)
		goto GOTO_ERROR;

	fread(&g_nTotal, sizeof(int), 1, fpInp);
	g_pnIdx = new int[g_nTotal];

	fread(g_pnIdx, sizeof(int), g_nTotal, fpInp);
	fclose(fpInp);


	fpInp = fopen(psDat, "rb");
	if (fpInp == NULL)
		goto GOTO_ERROR;
	g_pnDat = new unsigned short[g_nTotal];
	fread(g_pnDat, sizeof(unsigned short), g_nTotal, fpInp);
	fclose(fpInp);

	return true;

GOTO_ERROR:
	if (g_pnHZ != NULL)delete g_pnHZ;
	if (g_pnIdx != NULL)delete g_pnIdx;
	if (g_pnDat != NULL)delete g_pnDat;
	g_pnHZ = NULL;
	g_pnIdx = NULL;
	g_pnDat = NULL;

	return false;

}


void FTRExit()
{
	if (g_pnHZ != NULL)delete g_pnHZ;
	if (g_pnIdx != NULL)delete g_pnIdx;
	if (g_pnDat != NULL)delete g_pnDat;
	g_pnHZ = NULL;
	g_pnIdx = NULL;
	g_pnDat = NULL;
}



int bSearchLow(int nStart, int nEnd, unsigned short *psQuery)
{
	int  CompRes = 0, mid = 0, Ret;
	if (nStart >= nEnd)
		return -1;

	int nLen;
	nLen = wcslen((const wchar_t*)psQuery);
	if (nLen > CMP_MAXLEN)
		nLen = CMP_MAXLEN;

	//CompRes = wcsncmp((unsigned short*)&g_pnDat[g_pnIdx[nStart]], (const wchar_t*)psQuery, nLen);
	CompRes = wcsncmp((const wchar_t*)&g_pnDat[g_pnIdx[nStart]], (const wchar_t*)psQuery, nLen);

	if (CompRes == 0)
		return nStart;

	if (nEnd - nStart == 1)
		return -1;

	if (CompRes > 0)
		return -1;

	mid = int((nStart + nEnd) / 2);

	Ret = bSearchLow(nStart, mid, psQuery);
	if (Ret == -1)
		return bSearchLow(mid, nEnd, psQuery);
	return Ret;
}


int bSearchHigh(int nStart, int nEnd, unsigned short *psQuery)
{
	int  CompRes = 0, mid = 0, Ret = 0;
	if (nStart >= nEnd)
		return -1;
	int nLen;
	nLen = wcslen((const wchar_t*)psQuery);
	if (nLen > CMP_MAXLEN)
		nLen = CMP_MAXLEN;

	//CompRes = wcsncmp((unsigned short*)&g_pnDat[g_pnIdx[nEnd - 1]], (const wchar_t*)psQuery, nLen);
	CompRes = wcsncmp((const wchar_t*)&g_pnDat[g_pnIdx[nEnd - 1]], (const wchar_t*)psQuery, nLen);
	if (CompRes == 0)
		return nEnd;

	if (nEnd - nStart == 1)
		return -1;

	if (CompRes < 0)
		return -1;

	mid = int((nStart + nEnd) / 2);
	Ret = bSearchHigh(mid, nEnd, psQuery);
	if (Ret == -1)
		return bSearchHigh(nStart, mid, psQuery);
	return Ret;
}


bool SearchFTR(int nStart, int nEnd, unsigned short* szQuery, char** psRet, int nMaxRetLen, int* Num)
{
	int nPosLow = bSearchLow(nStart, nEnd, szQuery);
	if (nPosLow == -1) {
		*Num = 0;
		return false;
	}

	int nPosHigh = bSearchHigh(nStart, nEnd, szQuery);


	*Num = nPosHigh - nPosLow;
	if (*Num > nMaxRetLen) {
		*Num = nMaxRetLen;
	}

	unsigned short nWindow[1024];
	char szWindow[1024];
	int nFirst;
	int nLast;
	for (int i = nPosLow; i<nPosLow + *Num; i++) {
		if (g_pnIdx[i] < g_WindowSize) {
			nFirst = 0;
		}
		else {
			nFirst = g_pnIdx[i] - g_WindowSize;
		}


		if (g_pnIdx[i] + (int)wcslen((const wchar_t*)szQuery) + g_WindowSize > g_nTotal) {
			nLast = g_nTotal;
		}
		else {
			nLast = g_pnIdx[i] + wcslen((const wchar_t*)szQuery) + g_WindowSize;
		}
		wcsncpy((wchar_t*)nWindow, (const wchar_t*)&g_pnDat[nFirst], nLast - nFirst);
        nWindow[nLast - nFirst] = 0;

        int nInvert = ucs16_to_gbk(nWindow, (unsigned char *)szWindow, 1024);
		szWindow[nInvert] = 0;

		psRet[i - nPosLow] = new char[strlen(szWindow) + 1];
		strcpy(psRet[i - nPosLow], szWindow);
	}

	return true;
}


int FTR(char* psInpUTF, int nMaxRetLen, char** psOut){
    char psInp[64] = "";
    utf8_to_gbk(psInpUTF, (unsigned char*)psInp, 64);

    char**psRet = new char*[64];
    *psOut = new char[0x100000];
    int* Num = new int;
	unsigned short szQuery[64];
	int nLen;
    int outLen;
    nLen = gbk_to_ucs16((const unsigned char*)psInp, (unsigned short*)szQuery, 64);
	szQuery[nLen] = 0;  
	if (nLen == 0) {
		*Num = 0;
		return false;
	}
	if (!SearchFTR(g_pnHZ[szQuery[0] - 1], g_pnHZ[szQuery[0]], szQuery, psRet, nMaxRetLen, Num)){
        return false;
    }
    else{
        for (int i = 0; i < *Num; i++) {
            char* psUTF8 = new char[0x100000];
            int len = gbk_to_utf8((const char*) psRet[i], (unsigned char*)psUTF8, 0x100000);
            strcat(*psOut, psUTF8);
            strcat(*psOut, "<br/>");
            delete psUTF8;
	    }
        return true;
    }
}
