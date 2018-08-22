
// �ṩutf-8��utf-16��GBK�ַ�����ת������
// ��ֻ��һ����װ����ʹ��Windows��API��������ƽ̨�޹صĶ�������
// �ӿڿ��Ա�����ʹ��
// ������ֻ�ṩ��utf-16<->gbk��utf-8<->utf16���������û���ϳ�utf-8<->gbk
// ����utf-8<->gbk�ŵ������Ϊ����ʹ�ÿ�����ƽ̨��صĶ�̬�ڴ����

#include "string.h"
#include "charwrap.h"

#ifdef USE_WIN_FUN
#include "windows.h"
#else
#include "charutil.h"
#endif

static int ucslen(const unsigned short *ucs)
{
	const unsigned short *p;
	int ucs_len = 0;
	
	p = ucs;
	while (*p++) {
		ucs_len++;
	}
	return ucs_len;
}

// �ӿ�Լ��:
// ������ַ���������0��β��Ϊ�ַ���������־
// ���صĽ������ӽ�β��0

// ˵��:
// ʹ��MultiByteToWideCharʱ�����MultiByte�ĳ�����Ϊ-1,���ص��ַ���Ҳ����0��β
// ���ָ��MultiByte�ĳ���,���ص��ַ����Ͳ�������β��0
// WideCharToMultiByteҲ��������Ϊ
// �Ҽ�Ȼ����Ҫ��β��0,ʹ������ʱ�Ͷ�ָ���˳���

int gbk_to_ucs16(const unsigned char *gbks, unsigned short *wbuf, int wbuf_len)
{
#ifdef USE_WIN_FUN
	int gbks_len;
	gbks_len = strlen(gbks);
	return MultiByteToWideChar(0, 0, (const char *)gbks, gbks_len, wbuf, wbuf_len);
#else
	return gb2uni(gbks, wbuf, wbuf_len);
#endif
}



int ucs16_to_gbk(const unsigned short *ucs, unsigned char *cbuf, int cbuf_len)
{
#ifdef USE_WIN_FUN
	int  ucs_len;
	ucs_len = ucslen(ucs);
	return WideCharToMultiByte(0, 0, ucs, ucs_len, (char *)cbuf, cbuf_len, NULL, NULL);
#else
	return uni2gb(ucs, cbuf, cbuf_len);
#endif
}


int ucs16_to_utf8(const unsigned short *ucs, unsigned char *cbuf, int cbuf_len)
{
#ifdef USE_WIN_FUN
	int ucs_len;
	ucs_len = ucslen(ucs);
	return WideCharToMultiByte(CP_UTF8, 0, ucs, ucs_len, (char *)cbuf, cbuf_len, NULL, NULL);
#else
	return ucs2ToUtf8(ucs, cbuf, cbuf_len);
#endif
}

int utf8_to_ucs16(const unsigned char *s, unsigned short *wbuf, int wbuf_len)
#ifdef USE_WIN_FUN
{
	int slen;
	slen = strlen(s);
	return MultiByteToWideChar(CP_UTF8, 0, (const char *)s, slen, wbuf, wbuf_len);
}
#else
{
	return utf8ToUcs2(s, wbuf, wbuf_len);
}
#endif


int gbk_to_utf8(const char *gbks, unsigned char *cbuf, int cbuf_len)
{
	unsigned short wbuf[1024];
	wbuf[0]=0;
	int  nLen=gbk_to_ucs16((const unsigned char *)gbks, wbuf, 1024);
	wbuf[nLen]=0;
	nLen=ucs16_to_utf8(wbuf, cbuf, cbuf_len);
	cbuf[nLen]=0;
	return 1;
}


int utf8_to_gbk(const char *gbks, unsigned char *cbuf, int cbuf_len)
{
	unsigned short wbuf[1024];
	int nLen=utf8_to_ucs16((const unsigned char* )gbks, wbuf, 1024);
	wbuf[nLen]=0;
	nLen=ucs16_to_gbk(wbuf, cbuf, cbuf_len);
	cbuf[nLen]=0;
	return 1;
}

// The End
