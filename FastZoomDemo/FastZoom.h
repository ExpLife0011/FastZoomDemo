/*
	32λλͼ�Ŀ������Ŵ���
	// �㷨0~7�ĺ�ʱ
	//31 31 32 0 0 0 0 0
*/
#pragma once

#include <Windows.h>

#define  asm __asm 
typedef unsigned  char  TUInt8;  // [0..255] 

struct  TARGB32       //32 bit color 
{ 
	TUInt8  B,G,R,A;           // A is alpha 
}; 

struct  TPicRegion   //һ����ɫ�����������������ڲ������� 
{ 
	TARGB32*    pdata;          //��ɫ�����׵�ַ 
	long         byte_width;     //һ�����ݵ�������(�ֽڿ��)�� 
	//abs(byte_width)�п��ܴ��ڵ���width*sizeof(TARGB32); 
	long         width;          //���ؿ�� 
	long         height;         //���ظ߶� 
}; 

//����һ����ĺ���
inline TARGB32& Pixels(const TPicRegion& pic,const  long  x,const  long  y) 
{ 
	return  ( (TARGB32*)((TUInt8*)pic.pdata+pic.byte_width*y) )[x]; 
} 

class CFastZoom
{
public:
	CFastZoom(void);
	~CFastZoom(void);
public:
	void	PicZoom0(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom1(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom2(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom3(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom4(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom5(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom6(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom7(const TPicRegion& Dst,const TPicRegion& Src);
private:
	void	PicZoom_float(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom_Table(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom_SSE(const TPicRegion& Dst,const TPicRegion& Src);
	void	PicZoom_SSE_mmh(const TPicRegion& Dst,const TPicRegion& Src);
};
