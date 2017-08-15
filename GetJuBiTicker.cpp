// GetJuBiTicker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <winhttp.h>
#include "Json/cJSON.h"

#pragma comment(lib,"winhttp.lib")

// 安全释放对象
#define			SafeDelete(p)			if(p!=NULL){delete[] p;p=NULL;}
#define			SafeFree(p)				if(p!=NULL){free(p);p = NULL;}

// 总缓冲区大小
#define			MAXBUFFSIZE				(1024 * 400)

// 定义单向链表，保存每一次读出来的返回数据的地址
typedef struct TICKERCHAIN
{
	TICKERCHAIN*	pNext;
	char*			szTicker;
	DWORD			dwSize;
}*PTICKERCHAIN;


// 每一种虚拟币包含的信息
typedef struct COININFO
{
	double		buy;	// 买一价
	double		sell;	// 卖一价
	double		high;	// 最高价
	double		low;	// 最低价
	double		last;	// 最近一次成交价
	double		vol;	// 成交量
	double		volume;	// 成交额
}*PCOININFO;

// 全币种信息
typedef struct ALLTICKER
{
	COININFO	ANS;	// 小蚁股
	COININFO	BCC;	// BCC
	COININFO	BLK;	// 黑币
	COININFO	BTC;	// 比特币
	COININFO	BTK;	// B-Token
	COININFO	BTM;	// 比原链
	COININFO	BTS;	// 比特股
	COININFO	DNC;	// 暗网币
	COININFO	DOGE;	// 狗狗币
	COININFO	EAC;	// 地球币
	COININFO	ELC;	// 选举链
	COININFO	EOS;	// EOS
	COININFO	ETC;	// 以太经典
	COININFO	ETH;	// 以太坊
	COININFO	FZ;		// 冰河币
	COININFO	GAME;	// 游戏点
	COININFO	GOOC;	// 谷壳币
	COININFO	HCC;	// 医疗链
	COININFO	HLB;	// 活力币
	COININFO	ICO;	// ICO
	COININFO	IFC;	// 无限币
	COININFO	JBC;	// 聚宝币
	COININFO	KTC;	// 肯特币
	COININFO	LKC;	// 幸运币
	COININFO	LSK;	// LISK
	COININFO	LTC;	// 莱特币
	COININFO	MAX;	// 最大币
	COININFO	MCC;	// 行云币
	COININFO	MET;	// 美通币
	COININFO	MRYC;	// 美人鱼币
	COININFO	MTC;	// 猴宝币
	COININFO	NXT;	// 未来币
	COININFO	PEB;	// 普银
	COININFO	PGC;	// 乐园通
	COININFO	PLC;	// 保罗币
	COININFO	PPC;	// 点点币
	COININFO	QEC;	// 企鹅链
	COININFO	QTUM;	// 量子链
	COININFO	RIO;	// 里约币
	COININFO	RSS;	// 红贝壳
	COININFO	SKT;	// 鲨之信
	COININFO	TFC;	// 传送币
	COININFO	TIC;	// 钛币
	COININFO	UGT;	// UG-Taken
	COININFO	VRC;	// 维理币
	COININFO	VTC;	// 绿币
	COININFO	WDC;	// 世界币
	COININFO	XAS;	// 阿希币
	COININFO	XPM;	// 质数币
	COININFO	XRP;	// 瑞波币
	COININFO	XSGS;	// 雪山古树
	COININFO	YTC;	// 一号币
	COININFO	ZCC;	// 招财币
	COININFO	ZET;	// 泽塔币
};


int WINAPI GetAllTickerFromJubi(char* szString2File)
{
	DWORD		dwSize = 0;
	DWORD		dwCount = 0;
	DWORD		dwOffset = 0;
	LPVOID		lpOutBuffer = NULL;
	DWORD		dwDownloaded = 0;
	char*		pszOutBuffer;
	char*		szJsonString;

	HINTERNET	hSession = NULL;
	HINTERNET	hConnect = NULL;
	HINTERNET	hRequest = NULL;
	BOOL		bResults = FALSE;
	SYSTEMTIME	stTime;

	cJSON		*stJson = NULL;
	cJSON		*stTicker = NULL;
	cJSON		*stCoin = NULL;
	cJSON		*buy = NULL;
	cJSON		*sell = NULL;
	cJSON		*high = NULL;
	cJSON		*low = NULL;
	cJSON		*last = NULL;
	cJSON		*vol = NULL;
	cJSON		*volume = NULL;
	ALLTICKER	stAllTicker;
	PTICKERCHAIN	pTickerChain = NULL;
	PTICKERCHAIN	pTickerTemp = NULL;
	PTICKERCHAIN	pTail = NULL;

	__try
	{
		hSession = WinHttpOpen(L"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Safari/537.36", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

		if (NULL == hSession)
		{
			printf("[L=%d]WinHttpOpen failed, err = %d\r\n", __LINE__, GetLastError());
			__leave;
		}

		hConnect = WinHttpConnect(hSession, L"www.jubi.com", INTERNET_DEFAULT_HTTP_PORT, 0);

		if (NULL == hConnect)
		{
			printf("[L=%d]WinHttpConnect failed, err = %d\n", __LINE__, GetLastError());
			__leave;
		}

		hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/api/v1/allticker/", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

		if (NULL == hRequest)
		{
			printf("[L=%d]WinHttpOpenRequest failed, err = %d\r\n", __LINE__, GetLastError());
			__leave;
		}

		bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

		if (!bResults)
		{
			printf("[L=%d]WinHttpSendRequest failed, err = %d\r\n", __LINE__, GetLastError());
			__leave;
		}

		bResults = WinHttpReceiveResponse(hRequest, NULL);

		// init chain with a header
		pTickerChain = (PTICKERCHAIN)malloc(sizeof(TICKERCHAIN));
		ZeroMemory(pTickerChain, sizeof(TICKERCHAIN));
		pTail = pTickerChain;

		// Keep checking for data until there is nothing left.
		if (bResults)
		{
			// 读取网站返回的数据
			do
			{
				// Check for available data.
				dwSize = 0;
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				{
					printf("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());
					__leave;
				}

				// No more available data.
				if (!dwSize)
					__leave;

				// Allocate space for the buffer.
				pszOutBuffer = new char[dwSize + 1];
				if (!pszOutBuffer)
				{
					printf("Out of memory\n");
					__leave;
				}

				// Read the Data.
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
				{
					printf("Error %u in WinHttpReadData.\n", GetLastError());
					__leave;
				}

				dwCount += dwSize;

				// insert address to chain
				pTickerTemp = (PTICKERCHAIN)malloc(sizeof(TICKERCHAIN));
				ZeroMemory(pTickerTemp, sizeof(TICKERCHAIN));

				pTickerTemp->dwSize = dwSize;
				pTickerTemp->szTicker = pszOutBuffer;
				pTickerTemp->pNext = pTail->pNext;
				pTail->pNext = pTickerTemp;
				pTail = pTail->pNext;

				dwSize = 0;

				// 一次可能读不完
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				{
					printf("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());
					__leave;
				}
			} while (dwSize > 0);

			// 整合字符串
			pTickerTemp = pTickerChain->pNext;

			// 申请一块数据用来保存所有的json字符串
			szJsonString = (char*)malloc(dwCount + 1);
			ZeroMemory(szJsonString, dwCount + 1);
			while (pTickerTemp != NULL)
			{
				lstrcatA(szJsonString, pTickerTemp->szTicker);
				pTickerTemp = pTickerTemp->pNext;
			}

			ZeroMemory(&stTime, sizeof(SYSTEMTIME));
			GetLocalTime(&stTime);
			dwOffset = sprintf(szString2File, "币种,买一价,卖一价,最高价,最低价,LAST,成交量,成交额,%u-%u-%u %u:%u:%u\r\n", stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond);

			// 开始解析json
			stTicker = cJSON_Parse(szJsonString);
			if (NULL == stTicker)
			{
				printf("[L=%d]Parse json string failed!\n", __LINE__);
				__leave;
			}

			// ans
			stCoin = cJSON_GetObjectItem(stTicker, "ans");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.ANS.buy = buy->valuedouble;
			stAllTicker.ANS.sell = sell->valuedouble;
			stAllTicker.ANS.high = high->valuedouble;
			stAllTicker.ANS.low = low->valuedouble;
			stAllTicker.ANS.last = last->valuedouble;
			stAllTicker.ANS.vol = vol->valuedouble;
			stAllTicker.ANS.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "ANS,%f,%f,%f,%f,%f,%f,%f\r\n",
								stAllTicker.ANS.buy,
								stAllTicker.ANS.sell,
								stAllTicker.ANS.high,
								stAllTicker.ANS.low,
								stAllTicker.ANS.last,
								stAllTicker.ANS.vol,
								stAllTicker.ANS.volume);

			// bcc
			stCoin = cJSON_GetObjectItem(stTicker, "bcc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.BCC.buy = buy->valuedouble;
			stAllTicker.BCC.sell = sell->valuedouble;
			stAllTicker.BCC.high = high->valuedouble;
			stAllTicker.BCC.low = low->valuedouble;
			stAllTicker.BCC.last = last->valuedouble;
			stAllTicker.BCC.vol = vol->valuedouble;
			stAllTicker.BCC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "BCC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.BCC.buy,
				stAllTicker.BCC.sell,
				stAllTicker.BCC.high,
				stAllTicker.BCC.low,
				stAllTicker.BCC.last,
				stAllTicker.BCC.vol,
				stAllTicker.BCC.volume);

			// blk
			stCoin = cJSON_GetObjectItem(stTicker, "blk");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.BLK.buy = buy->valuedouble;
			stAllTicker.BLK.sell = sell->valuedouble;
			stAllTicker.BLK.high = high->valuedouble;
			stAllTicker.BLK.low = low->valuedouble;
			stAllTicker.BLK.last = last->valuedouble;
			stAllTicker.BLK.vol = vol->valuedouble;
			stAllTicker.BLK.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "BLK,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.BLK.buy,
				stAllTicker.BLK.sell,
				stAllTicker.BLK.high,
				stAllTicker.BLK.low,
				stAllTicker.BLK.last,
				stAllTicker.BLK.vol,
				stAllTicker.BLK.volume);

			// BTC
			stCoin = cJSON_GetObjectItem(stTicker, "btc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.BTC.buy = buy->valuedouble;
			stAllTicker.BTC.sell = sell->valuedouble;
			stAllTicker.BTC.high = high->valuedouble;
			stAllTicker.BTC.low = low->valuedouble;
			stAllTicker.BTC.last = last->valuedouble;
			stAllTicker.BTC.vol = vol->valuedouble;
			stAllTicker.BTC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "BTC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.BTC.buy,
				stAllTicker.BTC.sell,
				stAllTicker.BTC.high,
				stAllTicker.BTC.low,
				stAllTicker.BTC.last,
				stAllTicker.BTC.vol,
				stAllTicker.BTC.volume);

			// BTK
			stCoin = cJSON_GetObjectItem(stTicker, "btk");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.BTK.buy = buy->valuedouble;
			stAllTicker.BTK.sell = sell->valuedouble;
			stAllTicker.BTK.high = high->valuedouble;
			stAllTicker.BTK.low = low->valuedouble;
			stAllTicker.BTK.last = last->valuedouble;
			stAllTicker.BTK.vol = vol->valuedouble;
			stAllTicker.BTK.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "BTK,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.BTK.buy,
				stAllTicker.BTK.sell,
				stAllTicker.BTK.high,
				stAllTicker.BTK.low,
				stAllTicker.BTK.last,
				stAllTicker.BTK.vol,
				stAllTicker.BTK.volume);

			// BTM
			stCoin = cJSON_GetObjectItem(stTicker, "btm");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.BTM.buy = buy->valuedouble;
			stAllTicker.BTM.sell = sell->valuedouble;
			stAllTicker.BTM.high = high->valuedouble;
			stAllTicker.BTM.low = low->valuedouble;
			stAllTicker.BTM.last = last->valuedouble;
			stAllTicker.BTM.vol = vol->valuedouble;
			stAllTicker.BTM.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "BTM,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.BTM.buy,
				stAllTicker.BTM.sell,
				stAllTicker.BTM.high,
				stAllTicker.BTM.low,
				stAllTicker.BTM.last,
				stAllTicker.BTM.vol,
				stAllTicker.BTM.volume);

			// BTS
			stCoin = cJSON_GetObjectItem(stTicker, "bts");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.BTS.buy = buy->valuedouble;
			stAllTicker.BTS.sell = sell->valuedouble;
			stAllTicker.BTS.high = high->valuedouble;
			stAllTicker.BTS.low = low->valuedouble;
			stAllTicker.BTS.last = last->valuedouble;
			stAllTicker.BTS.vol = vol->valuedouble;
			stAllTicker.BTS.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "BTS,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.BTS.buy,
				stAllTicker.BTS.sell,
				stAllTicker.BTS.high,
				stAllTicker.BTS.low,
				stAllTicker.BTS.last,
				stAllTicker.BTS.vol,
				stAllTicker.BTS.volume);

			// DNC
			stCoin = cJSON_GetObjectItem(stTicker, "dnc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.DNC.buy = buy->valuedouble;
			stAllTicker.DNC.sell = sell->valuedouble;
			stAllTicker.DNC.high = high->valuedouble;
			stAllTicker.DNC.low = low->valuedouble;
			stAllTicker.DNC.last = last->valuedouble;
			stAllTicker.DNC.vol = vol->valuedouble;
			stAllTicker.DNC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "DNC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.DNC.buy,
				stAllTicker.DNC.sell,
				stAllTicker.DNC.high,
				stAllTicker.DNC.low,
				stAllTicker.DNC.last,
				stAllTicker.DNC.vol,
				stAllTicker.DNC.volume);

			// DOGE
			stCoin = cJSON_GetObjectItem(stTicker, "doge");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.DOGE.buy = buy->valuedouble;
			stAllTicker.DOGE.sell = sell->valuedouble;
			stAllTicker.DOGE.high = high->valuedouble;
			stAllTicker.DOGE.low = low->valuedouble;
			stAllTicker.DOGE.last = last->valuedouble;
			stAllTicker.DOGE.vol = vol->valuedouble;
			stAllTicker.DOGE.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "DOGE,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.DOGE.buy,
				stAllTicker.DOGE.sell,
				stAllTicker.DOGE.high,
				stAllTicker.DOGE.low,
				stAllTicker.DOGE.last,
				stAllTicker.DOGE.vol,
				stAllTicker.DOGE.volume);

			// EAC
			stCoin = cJSON_GetObjectItem(stTicker, "eac");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.EAC.buy = buy->valuedouble;
			stAllTicker.EAC.sell = sell->valuedouble;
			stAllTicker.EAC.high = high->valuedouble;
			stAllTicker.EAC.low = low->valuedouble;
			stAllTicker.EAC.last = last->valuedouble;
			stAllTicker.EAC.vol = vol->valuedouble;
			stAllTicker.EAC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "EAC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.EAC.buy,
				stAllTicker.EAC.sell,
				stAllTicker.EAC.high,
				stAllTicker.EAC.low,
				stAllTicker.EAC.last,
				stAllTicker.EAC.vol,
				stAllTicker.EAC.volume);

			// ELC
			stCoin = cJSON_GetObjectItem(stTicker, "elc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.ELC.buy = buy->valuedouble;
			stAllTicker.ELC.sell = sell->valuedouble;
			stAllTicker.ELC.high = high->valuedouble;
			stAllTicker.ELC.low = low->valuedouble;
			stAllTicker.ELC.last = last->valuedouble;
			stAllTicker.ELC.vol = vol->valuedouble;
			stAllTicker.ELC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "ELC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.ELC.buy,
				stAllTicker.ELC.sell,
				stAllTicker.ELC.high,
				stAllTicker.ELC.low,
				stAllTicker.ELC.last,
				stAllTicker.ELC.vol,
				stAllTicker.ELC.volume);

			// EOS
			stCoin = cJSON_GetObjectItem(stTicker, "eos");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.EOS.buy = buy->valuedouble;
			stAllTicker.EOS.sell = sell->valuedouble;
			stAllTicker.EOS.high = high->valuedouble;
			stAllTicker.EOS.low = low->valuedouble;
			stAllTicker.EOS.last = last->valuedouble;
			stAllTicker.EOS.vol = vol->valuedouble;
			stAllTicker.EOS.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "EOS,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.EOS.buy,
				stAllTicker.EOS.sell,
				stAllTicker.EOS.high,
				stAllTicker.EOS.low,
				stAllTicker.EOS.last,
				stAllTicker.EOS.vol,
				stAllTicker.EOS.volume);

			// ETC
			stCoin = cJSON_GetObjectItem(stTicker, "etc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.ETC.buy = buy->valuedouble;
			stAllTicker.ETC.sell = sell->valuedouble;
			stAllTicker.ETC.high = high->valuedouble;
			stAllTicker.ETC.low = low->valuedouble;
			stAllTicker.ETC.last = last->valuedouble;
			stAllTicker.ETC.vol = vol->valuedouble;
			stAllTicker.ETC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "ETC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.ETC.buy,
				stAllTicker.ETC.sell,
				stAllTicker.ETC.high,
				stAllTicker.ETC.low,
				stAllTicker.ETC.last,
				stAllTicker.ETC.vol,
				stAllTicker.ETC.volume);

			// ETH
			stCoin = cJSON_GetObjectItem(stTicker, "eth");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.ETH.buy = buy->valuedouble;
			stAllTicker.ETH.sell = sell->valuedouble;
			stAllTicker.ETH.high = high->valuedouble;
			stAllTicker.ETH.low = low->valuedouble;
			stAllTicker.ETH.last = last->valuedouble;
			stAllTicker.ETH.vol = vol->valuedouble;
			stAllTicker.ETH.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "ETH,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.ETH.buy,
				stAllTicker.ETH.sell,
				stAllTicker.ETH.high,
				stAllTicker.ETH.low,
				stAllTicker.ETH.last,
				stAllTicker.ETH.vol,
				stAllTicker.ETH.volume);

			// FZ
			stCoin = cJSON_GetObjectItem(stTicker, "fz");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.FZ.buy = buy->valuedouble;
			stAllTicker.FZ.sell = sell->valuedouble;
			stAllTicker.FZ.high = high->valuedouble;
			stAllTicker.FZ.low = low->valuedouble;
			stAllTicker.FZ.last = last->valuedouble;
			stAllTicker.FZ.vol = vol->valuedouble;
			stAllTicker.FZ.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "FZ,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.FZ.buy,
				stAllTicker.FZ.sell,
				stAllTicker.FZ.high,
				stAllTicker.FZ.low,
				stAllTicker.FZ.last,
				stAllTicker.FZ.vol,
				stAllTicker.FZ.volume);

			// GAME
			stCoin = cJSON_GetObjectItem(stTicker, "game");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.GAME.buy = buy->valuedouble;
			stAllTicker.GAME.sell = sell->valuedouble;
			stAllTicker.GAME.high = high->valuedouble;
			stAllTicker.GAME.low = low->valuedouble;
			stAllTicker.GAME.last = last->valuedouble;
			stAllTicker.GAME.vol = vol->valuedouble;
			stAllTicker.GAME.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "GAME,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.GAME.buy,
				stAllTicker.GAME.sell,
				stAllTicker.GAME.high,
				stAllTicker.GAME.low,
				stAllTicker.GAME.last,
				stAllTicker.GAME.vol,
				stAllTicker.GAME.volume);

			// GOOC
			stCoin = cJSON_GetObjectItem(stTicker, "gooc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.GOOC.buy = buy->valuedouble;
			stAllTicker.GOOC.sell = sell->valuedouble;
			stAllTicker.GOOC.high = high->valuedouble;
			stAllTicker.GOOC.low = low->valuedouble;
			stAllTicker.GOOC.last = last->valuedouble;
			stAllTicker.GOOC.vol = vol->valuedouble;
			stAllTicker.GOOC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "GOOC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.GOOC.buy,
				stAllTicker.GOOC.sell,
				stAllTicker.GOOC.high,
				stAllTicker.GOOC.low,
				stAllTicker.GOOC.last,
				stAllTicker.GOOC.vol,
				stAllTicker.GOOC.volume);

			// HCC
			stCoin = cJSON_GetObjectItem(stTicker, "hcc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.HCC.buy = buy->valuedouble;
			stAllTicker.HCC.sell = sell->valuedouble;
			stAllTicker.HCC.high = high->valuedouble;
			stAllTicker.HCC.low = low->valuedouble;
			stAllTicker.HCC.last = last->valuedouble;
			stAllTicker.HCC.vol = vol->valuedouble;
			stAllTicker.HCC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "HCC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.HCC.buy,
				stAllTicker.HCC.sell,
				stAllTicker.HCC.high,
				stAllTicker.HCC.low,
				stAllTicker.HCC.last,
				stAllTicker.HCC.vol,
				stAllTicker.HCC.volume);

			// HLB
			stCoin = cJSON_GetObjectItem(stTicker, "hlb");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.HLB.buy = buy->valuedouble;
			stAllTicker.HLB.sell = sell->valuedouble;
			stAllTicker.HLB.high = high->valuedouble;
			stAllTicker.HLB.low = low->valuedouble;
			stAllTicker.HLB.last = last->valuedouble;
			stAllTicker.HLB.vol = vol->valuedouble;
			stAllTicker.HLB.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "HLB,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.HLB.buy,
				stAllTicker.HLB.sell,
				stAllTicker.HLB.high,
				stAllTicker.HLB.low,
				stAllTicker.HLB.last,
				stAllTicker.HLB.vol,
				stAllTicker.HLB.volume);

			// ICO
			stCoin = cJSON_GetObjectItem(stTicker, "ico");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.ICO.buy = buy->valuedouble;
			stAllTicker.ICO.sell = sell->valuedouble;
			stAllTicker.ICO.high = high->valuedouble;
			stAllTicker.ICO.low = low->valuedouble;
			stAllTicker.ICO.last = last->valuedouble;
			stAllTicker.ICO.vol = vol->valuedouble;
			stAllTicker.ICO.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "ICO,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.ICO.buy,
				stAllTicker.ICO.sell,
				stAllTicker.ICO.high,
				stAllTicker.ICO.low,
				stAllTicker.ICO.last,
				stAllTicker.ICO.vol,
				stAllTicker.ICO.volume);

			// IFC
			stCoin = cJSON_GetObjectItem(stTicker, "ifc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.IFC.buy = buy->valuedouble;
			stAllTicker.IFC.sell = sell->valuedouble;
			stAllTicker.IFC.high = high->valuedouble;
			stAllTicker.IFC.low = low->valuedouble;
			stAllTicker.IFC.last = last->valuedouble;
			stAllTicker.IFC.vol = vol->valuedouble;
			stAllTicker.IFC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "IFC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.IFC.buy,
				stAllTicker.IFC.sell,
				stAllTicker.IFC.high,
				stAllTicker.IFC.low,
				stAllTicker.IFC.last,
				stAllTicker.IFC.vol,
				stAllTicker.IFC.volume);

			// JBC
			stCoin = cJSON_GetObjectItem(stTicker, "jbc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.JBC.buy = buy->valuedouble;
			stAllTicker.JBC.sell = sell->valuedouble;
			stAllTicker.JBC.high = high->valuedouble;
			stAllTicker.JBC.low = low->valuedouble;
			stAllTicker.JBC.last = last->valuedouble;
			stAllTicker.JBC.vol = vol->valuedouble;
			stAllTicker.JBC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "JBC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.JBC.buy,
				stAllTicker.JBC.sell,
				stAllTicker.JBC.high,
				stAllTicker.JBC.low,
				stAllTicker.JBC.last,
				stAllTicker.JBC.vol,
				stAllTicker.JBC.volume);

			// KTC
			stCoin = cJSON_GetObjectItem(stTicker, "ktc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.KTC.buy = buy->valuedouble;
			stAllTicker.KTC.sell = sell->valuedouble;
			stAllTicker.KTC.high = high->valuedouble;
			stAllTicker.KTC.low = low->valuedouble;
			stAllTicker.KTC.last = last->valuedouble;
			stAllTicker.KTC.vol = vol->valuedouble;
			stAllTicker.KTC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "KTC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.KTC.buy,
				stAllTicker.KTC.sell,
				stAllTicker.KTC.high,
				stAllTicker.KTC.low,
				stAllTicker.KTC.last,
				stAllTicker.KTC.vol,
				stAllTicker.KTC.volume);

			// LKC
			stCoin = cJSON_GetObjectItem(stTicker, "lkc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.LKC.buy = buy->valuedouble;
			stAllTicker.LKC.sell = sell->valuedouble;
			stAllTicker.LKC.high = high->valuedouble;
			stAllTicker.LKC.low = low->valuedouble;
			stAllTicker.LKC.last = last->valuedouble;
			stAllTicker.LKC.vol = vol->valuedouble;
			stAllTicker.LKC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "LKC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.LKC.buy,
				stAllTicker.LKC.sell,
				stAllTicker.LKC.high,
				stAllTicker.LKC.low,
				stAllTicker.LKC.last,
				stAllTicker.LKC.vol,
				stAllTicker.LKC.volume);

			// LSK
			stCoin = cJSON_GetObjectItem(stTicker, "lsk");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.LSK.buy = buy->valuedouble;
			stAllTicker.LSK.sell = sell->valuedouble;
			stAllTicker.LSK.high = high->valuedouble;
			stAllTicker.LSK.low = low->valuedouble;
			stAllTicker.LSK.last = last->valuedouble;
			stAllTicker.LSK.vol = vol->valuedouble;
			stAllTicker.LSK.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "LSK,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.LSK.buy,
				stAllTicker.LSK.sell,
				stAllTicker.LSK.high,
				stAllTicker.LSK.low,
				stAllTicker.LSK.last,
				stAllTicker.LSK.vol,
				stAllTicker.LSK.volume);

			// LTC
			stCoin = cJSON_GetObjectItem(stTicker, "ltc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.LTC.buy = buy->valuedouble;
			stAllTicker.LTC.sell = sell->valuedouble;
			stAllTicker.LTC.high = high->valuedouble;
			stAllTicker.LTC.low = low->valuedouble;
			stAllTicker.LTC.last = last->valuedouble;
			stAllTicker.LTC.vol = vol->valuedouble;
			stAllTicker.LTC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "LTC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.LTC.buy,
				stAllTicker.LTC.sell,
				stAllTicker.LTC.high,
				stAllTicker.LTC.low,
				stAllTicker.LTC.last,
				stAllTicker.LTC.vol,
				stAllTicker.LTC.volume);

			// MAX
			stCoin = cJSON_GetObjectItem(stTicker, "max");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.MAX.buy = buy->valuedouble;
			stAllTicker.MAX.sell = sell->valuedouble;
			stAllTicker.MAX.high = high->valuedouble;
			stAllTicker.MAX.low = low->valuedouble;
			stAllTicker.MAX.last = last->valuedouble;
			stAllTicker.MAX.vol = vol->valuedouble;
			stAllTicker.MAX.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "MAX,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.MAX.buy,
				stAllTicker.MAX.sell,
				stAllTicker.MAX.high,
				stAllTicker.MAX.low,
				stAllTicker.MAX.last,
				stAllTicker.MAX.vol,
				stAllTicker.MAX.volume);

			// MCC
			stCoin = cJSON_GetObjectItem(stTicker, "mcc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.MCC.buy = buy->valuedouble;
			stAllTicker.MCC.sell = sell->valuedouble;
			stAllTicker.MCC.high = high->valuedouble;
			stAllTicker.MCC.low = low->valuedouble;
			stAllTicker.MCC.last = last->valuedouble;
			stAllTicker.MCC.vol = vol->valuedouble;
			stAllTicker.MCC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "MCC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.MCC.buy,
				stAllTicker.MCC.sell,
				stAllTicker.MCC.high,
				stAllTicker.MCC.low,
				stAllTicker.MCC.last,
				stAllTicker.MCC.vol,
				stAllTicker.MCC.volume);

			// MET
			stCoin = cJSON_GetObjectItem(stTicker, "met");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.MET.buy = buy->valuedouble;
			stAllTicker.MET.sell = sell->valuedouble;
			stAllTicker.MET.high = high->valuedouble;
			stAllTicker.MET.low = low->valuedouble;
			stAllTicker.MET.last = last->valuedouble;
			stAllTicker.MET.vol = vol->valuedouble;
			stAllTicker.MET.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "MET,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.MET.buy,
				stAllTicker.MET.sell,
				stAllTicker.MET.high,
				stAllTicker.MET.low,
				stAllTicker.MET.last,
				stAllTicker.MET.vol,
				stAllTicker.MET.volume);

			// MRYC
			stCoin = cJSON_GetObjectItem(stTicker, "mryc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.MRYC.buy = buy->valuedouble;
			stAllTicker.MRYC.sell = sell->valuedouble;
			stAllTicker.MRYC.high = high->valuedouble;
			stAllTicker.MRYC.low = low->valuedouble;
			stAllTicker.MRYC.last = last->valuedouble;
			stAllTicker.MRYC.vol = vol->valuedouble;
			stAllTicker.MRYC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "MRYC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.MRYC.buy,
				stAllTicker.MRYC.sell,
				stAllTicker.MRYC.high,
				stAllTicker.MRYC.low,
				stAllTicker.MRYC.last,
				stAllTicker.MRYC.vol,
				stAllTicker.MRYC.volume);

			// MTC
			stCoin = cJSON_GetObjectItem(stTicker, "mtc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.MTC.buy = buy->valuedouble;
			stAllTicker.MTC.sell = sell->valuedouble;
			stAllTicker.MTC.high = high->valuedouble;
			stAllTicker.MTC.low = low->valuedouble;
			stAllTicker.MTC.last = last->valuedouble;
			stAllTicker.MTC.vol = vol->valuedouble;
			stAllTicker.MTC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "MTC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.MTC.buy,
				stAllTicker.MTC.sell,
				stAllTicker.MTC.high,
				stAllTicker.MTC.low,
				stAllTicker.MTC.last,
				stAllTicker.MTC.vol,
				stAllTicker.MTC.volume);

			// NXT
			stCoin = cJSON_GetObjectItem(stTicker, "nxt");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.NXT.buy = buy->valuedouble;
			stAllTicker.NXT.sell = sell->valuedouble;
			stAllTicker.NXT.high = high->valuedouble;
			stAllTicker.NXT.low = low->valuedouble;
			stAllTicker.NXT.last = last->valuedouble;
			stAllTicker.NXT.vol = vol->valuedouble;
			stAllTicker.NXT.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "NXT,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.NXT.buy,
				stAllTicker.NXT.sell,
				stAllTicker.NXT.high,
				stAllTicker.NXT.low,
				stAllTicker.NXT.last,
				stAllTicker.NXT.vol,
				stAllTicker.NXT.volume);

			// PEB
			stCoin = cJSON_GetObjectItem(stTicker, "peb");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.PEB.buy = buy->valuedouble;
			stAllTicker.PEB.sell = sell->valuedouble;
			stAllTicker.PEB.high = high->valuedouble;
			stAllTicker.PEB.low = low->valuedouble;
			stAllTicker.PEB.last = last->valuedouble;
			stAllTicker.PEB.vol = vol->valuedouble;
			stAllTicker.PEB.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "PEB,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.PEB.buy,
				stAllTicker.PEB.sell,
				stAllTicker.PEB.high,
				stAllTicker.PEB.low,
				stAllTicker.PEB.last,
				stAllTicker.PEB.vol,
				stAllTicker.PEB.volume);

			// PGC
			stCoin = cJSON_GetObjectItem(stTicker, "pgc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.PGC.buy = buy->valuedouble;
			stAllTicker.PGC.sell = sell->valuedouble;
			stAllTicker.PGC.high = high->valuedouble;
			stAllTicker.PGC.low = low->valuedouble;
			stAllTicker.PGC.last = last->valuedouble;
			stAllTicker.PGC.vol = vol->valuedouble;
			stAllTicker.PGC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "PGC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.PGC.buy,
				stAllTicker.PGC.sell,
				stAllTicker.PGC.high,
				stAllTicker.PGC.low,
				stAllTicker.PGC.last,
				stAllTicker.PGC.vol,
				stAllTicker.PGC.volume);

			// PLC
			stCoin = cJSON_GetObjectItem(stTicker, "plc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.PLC.buy = buy->valuedouble;
			stAllTicker.PLC.sell = sell->valuedouble;
			stAllTicker.PLC.high = high->valuedouble;
			stAllTicker.PLC.low = low->valuedouble;
			stAllTicker.PLC.last = last->valuedouble;
			stAllTicker.PLC.vol = vol->valuedouble;
			stAllTicker.PLC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "PLC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.PLC.buy,
				stAllTicker.PLC.sell,
				stAllTicker.PLC.high,
				stAllTicker.PLC.low,
				stAllTicker.PLC.last,
				stAllTicker.PLC.vol,
				stAllTicker.PLC.volume);

			// PPC
			stCoin = cJSON_GetObjectItem(stTicker, "ppc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.PPC.buy = buy->valuedouble;
			stAllTicker.PPC.sell = sell->valuedouble;
			stAllTicker.PPC.high = high->valuedouble;
			stAllTicker.PPC.low = low->valuedouble;
			stAllTicker.PPC.last = last->valuedouble;
			stAllTicker.PPC.vol = vol->valuedouble;
			stAllTicker.PPC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "PPC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.PPC.buy,
				stAllTicker.PPC.sell,
				stAllTicker.PPC.high,
				stAllTicker.PPC.low,
				stAllTicker.PPC.last,
				stAllTicker.PPC.vol,
				stAllTicker.PPC.volume);

			// QEC
			stCoin = cJSON_GetObjectItem(stTicker, "qec");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.QEC.buy = buy->valuedouble;
			stAllTicker.QEC.sell = sell->valuedouble;
			stAllTicker.QEC.high = high->valuedouble;
			stAllTicker.QEC.low = low->valuedouble;
			stAllTicker.QEC.last = last->valuedouble;
			stAllTicker.QEC.vol = vol->valuedouble;
			stAllTicker.QEC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "QEC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.QEC.buy,
				stAllTicker.QEC.sell,
				stAllTicker.QEC.high,
				stAllTicker.QEC.low,
				stAllTicker.QEC.last,
				stAllTicker.QEC.vol,
				stAllTicker.QEC.volume);

			// QTUM
			stCoin = cJSON_GetObjectItem(stTicker, "qtum");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.QTUM.buy = buy->valuedouble;
			stAllTicker.QTUM.sell = sell->valuedouble;
			stAllTicker.QTUM.high = high->valuedouble;
			stAllTicker.QTUM.low = low->valuedouble;
			stAllTicker.QTUM.last = last->valuedouble;
			stAllTicker.QTUM.vol = vol->valuedouble;
			stAllTicker.QTUM.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "QTUM,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.QTUM.buy,
				stAllTicker.QTUM.sell,
				stAllTicker.QTUM.high,
				stAllTicker.QTUM.low,
				stAllTicker.QTUM.last,
				stAllTicker.QTUM.vol,
				stAllTicker.QTUM.volume);

			// RIO
			stCoin = cJSON_GetObjectItem(stTicker, "rio");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.RIO.buy = buy->valuedouble;
			stAllTicker.RIO.sell = sell->valuedouble;
			stAllTicker.RIO.high = high->valuedouble;
			stAllTicker.RIO.low = low->valuedouble;
			stAllTicker.RIO.last = last->valuedouble;
			stAllTicker.RIO.vol = vol->valuedouble;
			stAllTicker.RIO.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "RIO,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.RIO.buy,
				stAllTicker.RIO.sell,
				stAllTicker.RIO.high,
				stAllTicker.RIO.low,
				stAllTicker.RIO.last,
				stAllTicker.RIO.vol,
				stAllTicker.RIO.volume);

			// RSS
			stCoin = cJSON_GetObjectItem(stTicker, "rss");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.RSS.buy = buy->valuedouble;
			stAllTicker.RSS.sell = sell->valuedouble;
			stAllTicker.RSS.high = high->valuedouble;
			stAllTicker.RSS.low = low->valuedouble;
			stAllTicker.RSS.last = last->valuedouble;
			stAllTicker.RSS.vol = vol->valuedouble;
			stAllTicker.RSS.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "RSS,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.RSS.buy,
				stAllTicker.RSS.sell,
				stAllTicker.RSS.high,
				stAllTicker.RSS.low,
				stAllTicker.RSS.last,
				stAllTicker.RSS.vol,
				stAllTicker.RSS.volume);

			// SKT
			stCoin = cJSON_GetObjectItem(stTicker, "skt");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.SKT.buy = buy->valuedouble;
			stAllTicker.SKT.sell = sell->valuedouble;
			stAllTicker.SKT.high = high->valuedouble;
			stAllTicker.SKT.low = low->valuedouble;
			stAllTicker.SKT.last = last->valuedouble;
			stAllTicker.SKT.vol = vol->valuedouble;
			stAllTicker.SKT.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "SKT,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.SKT.buy,
				stAllTicker.SKT.sell,
				stAllTicker.SKT.high,
				stAllTicker.SKT.low,
				stAllTicker.SKT.last,
				stAllTicker.SKT.vol,
				stAllTicker.SKT.volume);

			// TFC
			stCoin = cJSON_GetObjectItem(stTicker, "tfc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.TFC.buy = buy->valuedouble;
			stAllTicker.TFC.sell = sell->valuedouble;
			stAllTicker.TFC.high = high->valuedouble;
			stAllTicker.TFC.low = low->valuedouble;
			stAllTicker.TFC.last = last->valuedouble;
			stAllTicker.TFC.vol = vol->valuedouble;
			stAllTicker.TFC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "TFC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.TFC.buy,
				stAllTicker.TFC.sell,
				stAllTicker.TFC.high,
				stAllTicker.TFC.low,
				stAllTicker.TFC.last,
				stAllTicker.TFC.vol,
				stAllTicker.TFC.volume);

			// TIC
			stCoin = cJSON_GetObjectItem(stTicker, "tic");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.TIC.buy = buy->valuedouble;
			stAllTicker.TIC.sell = sell->valuedouble;
			stAllTicker.TIC.high = high->valuedouble;
			stAllTicker.TIC.low = low->valuedouble;
			stAllTicker.TIC.last = last->valuedouble;
			stAllTicker.TIC.vol = vol->valuedouble;
			stAllTicker.TIC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "TIC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.TIC.buy,
				stAllTicker.TIC.sell,
				stAllTicker.TIC.high,
				stAllTicker.TIC.low,
				stAllTicker.TIC.last,
				stAllTicker.TIC.vol,
				stAllTicker.TIC.volume);

			// UGT
			stCoin = cJSON_GetObjectItem(stTicker, "ugt");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.UGT.buy = buy->valuedouble;
			stAllTicker.UGT.sell = sell->valuedouble;
			stAllTicker.UGT.high = high->valuedouble;
			stAllTicker.UGT.low = low->valuedouble;
			stAllTicker.UGT.last = last->valuedouble;
			stAllTicker.UGT.vol = vol->valuedouble;
			stAllTicker.UGT.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "UGT,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.UGT.buy,
				stAllTicker.UGT.sell,
				stAllTicker.UGT.high,
				stAllTicker.UGT.low,
				stAllTicker.UGT.last,
				stAllTicker.UGT.vol,
				stAllTicker.UGT.volume);

			// VRC
			stCoin = cJSON_GetObjectItem(stTicker, "vrc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.VRC.buy = buy->valuedouble;
			stAllTicker.VRC.sell = sell->valuedouble;
			stAllTicker.VRC.high = high->valuedouble;
			stAllTicker.VRC.low = low->valuedouble;
			stAllTicker.VRC.last = last->valuedouble;
			stAllTicker.VRC.vol = vol->valuedouble;
			stAllTicker.VRC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "VRC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.VRC.buy,
				stAllTicker.VRC.sell,
				stAllTicker.VRC.high,
				stAllTicker.VRC.low,
				stAllTicker.VRC.last,
				stAllTicker.VRC.vol,
				stAllTicker.VRC.volume);

			// VTC
			stCoin = cJSON_GetObjectItem(stTicker, "vtc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.VTC.buy = buy->valuedouble;
			stAllTicker.VTC.sell = sell->valuedouble;
			stAllTicker.VTC.high = high->valuedouble;
			stAllTicker.VTC.low = low->valuedouble;
			stAllTicker.VTC.last = last->valuedouble;
			stAllTicker.VTC.vol = vol->valuedouble;
			stAllTicker.VTC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "VTC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.VTC.buy,
				stAllTicker.VTC.sell,
				stAllTicker.VTC.high,
				stAllTicker.VTC.low,
				stAllTicker.VTC.last,
				stAllTicker.VTC.vol,
				stAllTicker.VTC.volume);

			// WDC
			stCoin = cJSON_GetObjectItem(stTicker, "wdc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.WDC.buy = buy->valuedouble;
			stAllTicker.WDC.sell = sell->valuedouble;
			stAllTicker.WDC.high = high->valuedouble;
			stAllTicker.WDC.low = low->valuedouble;
			stAllTicker.WDC.last = last->valuedouble;
			stAllTicker.WDC.vol = vol->valuedouble;
			stAllTicker.WDC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "WDC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.WDC.buy,
				stAllTicker.WDC.sell,
				stAllTicker.WDC.high,
				stAllTicker.WDC.low,
				stAllTicker.WDC.last,
				stAllTicker.WDC.vol,
				stAllTicker.WDC.volume);

			// XAS
			stCoin = cJSON_GetObjectItem(stTicker, "xas");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.XAS.buy = buy->valuedouble;
			stAllTicker.XAS.sell = sell->valuedouble;
			stAllTicker.XAS.high = high->valuedouble;
			stAllTicker.XAS.low = low->valuedouble;
			stAllTicker.XAS.last = last->valuedouble;
			stAllTicker.XAS.vol = vol->valuedouble;
			stAllTicker.XAS.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "XAS,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.XAS.buy,
				stAllTicker.XAS.sell,
				stAllTicker.XAS.high,
				stAllTicker.XAS.low,
				stAllTicker.XAS.last,
				stAllTicker.XAS.vol,
				stAllTicker.XAS.volume);

			// XPM
			stCoin = cJSON_GetObjectItem(stTicker, "xpm");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.XPM.buy = buy->valuedouble;
			stAllTicker.XPM.sell = sell->valuedouble;
			stAllTicker.XPM.high = high->valuedouble;
			stAllTicker.XPM.low = low->valuedouble;
			stAllTicker.XPM.last = last->valuedouble;
			stAllTicker.XPM.vol = vol->valuedouble;
			stAllTicker.XPM.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "XPM,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.XPM.buy,
				stAllTicker.XPM.sell,
				stAllTicker.XPM.high,
				stAllTicker.XPM.low,
				stAllTicker.XPM.last,
				stAllTicker.XPM.vol,
				stAllTicker.XPM.volume);

			// XRP
			stCoin = cJSON_GetObjectItem(stTicker, "xrp");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.XRP.buy = buy->valuedouble;
			stAllTicker.XRP.sell = sell->valuedouble;
			stAllTicker.XRP.high = high->valuedouble;
			stAllTicker.XRP.low = low->valuedouble;
			stAllTicker.XRP.last = last->valuedouble;
			stAllTicker.XRP.vol = vol->valuedouble;
			stAllTicker.XRP.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "XRP,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.XRP.buy,
				stAllTicker.XRP.sell,
				stAllTicker.XRP.high,
				stAllTicker.XRP.low,
				stAllTicker.XRP.last,
				stAllTicker.XRP.vol,
				stAllTicker.XRP.volume);

			// XSGS
			stCoin = cJSON_GetObjectItem(stTicker, "xsgs");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.XSGS.buy = buy->valuedouble;
			stAllTicker.XSGS.sell = sell->valuedouble;
			stAllTicker.XSGS.high = high->valuedouble;
			stAllTicker.XSGS.low = low->valuedouble;
			stAllTicker.XSGS.last = last->valuedouble;
			stAllTicker.XSGS.vol = vol->valuedouble;
			stAllTicker.XSGS.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "XSGS,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.XSGS.buy,
				stAllTicker.XSGS.sell,
				stAllTicker.XSGS.high,
				stAllTicker.XSGS.low,
				stAllTicker.XSGS.last,
				stAllTicker.XSGS.vol,
				stAllTicker.XSGS.volume);

			// YTC
			stCoin = cJSON_GetObjectItem(stTicker, "ytc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.YTC.buy = buy->valuedouble;
			stAllTicker.YTC.sell = sell->valuedouble;
			stAllTicker.YTC.high = high->valuedouble;
			stAllTicker.YTC.low = low->valuedouble;
			stAllTicker.YTC.last = last->valuedouble;
			stAllTicker.YTC.vol = vol->valuedouble;
			stAllTicker.YTC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "YTC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.YTC.buy,
				stAllTicker.YTC.sell,
				stAllTicker.YTC.high,
				stAllTicker.YTC.low,
				stAllTicker.YTC.last,
				stAllTicker.YTC.vol,
				stAllTicker.YTC.volume);

			// ZCC
			stCoin = cJSON_GetObjectItem(stTicker, "zcc");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.ZCC.buy = buy->valuedouble;
			stAllTicker.ZCC.sell = sell->valuedouble;
			stAllTicker.ZCC.high = high->valuedouble;
			stAllTicker.ZCC.low = low->valuedouble;
			stAllTicker.ZCC.last = last->valuedouble;
			stAllTicker.ZCC.vol = vol->valuedouble;
			stAllTicker.ZCC.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "ZCC,%f,%f,%f,%f,%f,%f,%f\r\n",
				stAllTicker.ZCC.buy,
				stAllTicker.ZCC.sell,
				stAllTicker.ZCC.high,
				stAllTicker.ZCC.low,
				stAllTicker.ZCC.last,
				stAllTicker.ZCC.vol,
				stAllTicker.ZCC.volume);

			// ZET
			stCoin = cJSON_GetObjectItem(stTicker, "zet");
			buy = cJSON_GetObjectItem(stCoin, "buy");
			sell = cJSON_GetObjectItem(stCoin, "sell");
			high = cJSON_GetObjectItem(stCoin, "high");
			low = cJSON_GetObjectItem(stCoin, "low");
			last = cJSON_GetObjectItem(stCoin, "last");
			vol = cJSON_GetObjectItem(stCoin, "vol");
			volume = cJSON_GetObjectItem(stCoin, "volume");
			stAllTicker.ZET.buy = buy->valuedouble;
			stAllTicker.ZET.sell = sell->valuedouble;
			stAllTicker.ZET.high = high->valuedouble;
			stAllTicker.ZET.low = low->valuedouble;
			stAllTicker.ZET.last = last->valuedouble;
			stAllTicker.ZET.vol = vol->valuedouble;
			stAllTicker.ZET.volume = volume->valuedouble;
			dwOffset += sprintf(szString2File + dwOffset, "ZET,%f,%f,%f,%f,%f,%f,%f\r\n\r\n\r\n",
				stAllTicker.ZET.buy,
				stAllTicker.ZET.sell,
				stAllTicker.ZET.high,
				stAllTicker.ZET.low,
				stAllTicker.ZET.last,
				stAllTicker.ZET.vol,
				stAllTicker.ZET.volume);


			//printf("%s\n\n\n", szString2File);
		}
		else
		{
			// Report any errors.
			__leave;
		}
	}

	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[GetAllTickerFromJubi]Exception code = 0x%x\n", GetExceptionCode());
	}

	// Free the memory allocated to the buffer.
	pTickerTemp = pTickerChain;
	pTickerChain = pTickerChain->pNext;
	while (pTickerTemp != NULL)
	{
		SafeDelete(pTickerTemp->szTicker);
		SafeFree(pTickerTemp);
		pTickerTemp = pTickerChain;
		if (NULL != pTickerChain)
		{
			pTickerChain = pTickerChain->pNext;
		}
	}

	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return 0;
}


int WINAPI SaveAllTicker2File(float flTime)
{
	char		szString2File[MAXBUFFSIZE];
	char		szFilePath[MAX_PATH];
	HANDLE		hTicker = INVALID_HANDLE_VALUE;
	DWORD		dwWriteByte = 0;


	__try
	{
		// 创建日志文件
		GetCurrentDirectoryA(MAX_PATH, szFilePath);
		lstrcatA(szFilePath, "\\TickerRecord.csv");
		hTicker = CreateFileA(szFilePath, FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hTicker)
		{
			printf("[L=%d]Create file failed, err = %d\n", GetLastError());
			__leave;
		}

		printf("文件创建成功，正在记录······\r\n");

		while (true)
		{
			ZeroMemory(szString2File, MAXBUFFSIZE);
			GetAllTickerFromJubi(szString2File);
			SetFilePointer(hTicker, 0, 0, FILE_END);
			WriteFile(hTicker, szString2File, lstrlenA(szString2File), &dwWriteByte, NULL);

			Sleep((flTime) * 1000);
		}
	}

	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[SaveAllTicker2File]Exception code = 0x%x\n", GetExceptionCode());
	}

	if (INVALID_HANDLE_VALUE != hTicker)
	{
		CloseHandle(hTicker);
		hTicker = INVALID_HANDLE_VALUE;
	}

	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	float		flTime = 0;

	printf("输入时间间隔(秒)：");
	scanf("%f", &flTime);
	while (flTime <= 0)
	{
		fflush(stdin);
		printf("时间间隔错误，输入时间间隔(秒)：");
		scanf("%f", &flTime);
	}

	printf("\r\n****************获取所有币种最新行情****************\r\n");
	SaveAllTicker2File(flTime);

	system("pause");
	return 0;
}