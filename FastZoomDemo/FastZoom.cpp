#include "FastZoom.h"


#include <mmintrin.h>
#include <xmmintrin.h>


CFastZoom::CFastZoom(void)
{
}

CFastZoom::~CFastZoom(void)
{
}


void CFastZoom::PicZoom0(const TPicRegion& Dst,const TPicRegion& Src) 
{ 
	if  (  (0==Dst.width)||(0==Dst.height) 
		||(0==Src.width)||(0==Src.height))  return ; 
	for  ( long  x=0;x<Dst.width;++x) 
	{ 
		for  ( long  y=0;y<Dst.height;++y) 
		{ 
			long  srcx=(x*Src.width/Dst.width); // ���ű����ᵽ��������Ż�
			long  srcy=(y*Src.height/Dst.height); 
			Pixels(Dst,x,y)=Pixels(Src,srcx,srcy); 
		} 
	} 
}

void  CFastZoom::PicZoom1(const TPicRegion& Dst,const TPicRegion& Src) 
{ 
	if  (  (0==Dst.width)||(0==Dst.height) 
		||(0==Src.width)||(0==Src.height))  return ; 
	for  ( long  y=0;y<Dst.height;++y) 
	{ 
		for  ( long  x=0;x<Dst.width;++x) 
		{ 
			long  srcx=(x*Src.width/Dst.width); 
			long  srcy=(y*Src.height/Dst.height); 
			Pixels(Dst,x,y)=Pixels(Src,srcx,srcy); 
		} 
	} 
}


void CFastZoom::PicZoom2(const TPicRegion& Dst,const TPicRegion& Src) 
{ 
	if  (  (0==Dst.width)||(0==Dst.height) 
		||(0==Src.width)||(0==Src.height))  return ; 
	//�����ܹ���������ͼƬ�ߴ�65536*65536 
	unsigned long xrIntFloat_16=(Src.width<<16)/Dst.width+1;  //16.16��ʽ������ 
	unsigned long yrIntFloat_16=(Src.height<<16)/Dst.height+1;  //16.16��ʽ������
	//��֤��: (Dst.width-1)*xrIntFloat_16<Src.width����
	for (unsigned long y=0;y<Dst.height;++y) 
	{ 
		for (unsigned long x=0;x<Dst.width;++x) 
		{ 
			unsigned long srcx=(x*xrIntFloat_16)>>16; 
			unsigned long srcy=(y*yrIntFloat_16)>>16; 
			Pixels(Dst,x,y)=Pixels(Src,srcx,srcy); 
		} 
	}
}

void CFastZoom::PicZoom3(const TPicRegion& Dst,const TPicRegion& Src) 
{ 
	if (  (0==Dst.width)||(0==Dst.height) 
		||(0==Src.width)||(0==Src.height))  return ; 
	unsigned long xrIntFloat_16=(Src.width<<16)/Dst.width+1;  
	unsigned long yrIntFloat_16=(Src.height<<16)/Dst.height+1;
	unsigned long dst_width=Dst.width; 
	TARGB32* pDstLine=Dst.pdata; 
	unsigned long srcy_16=0; 
	for (unsigned long y=0;y<Dst.height;++y) 
	{ 
		TARGB32* pSrcLine=((TARGB32*)((TUInt8*)Src.pdata+Src.byte_width*(srcy_16>>16))); // ԭͼƬ��ÿһ��
		unsigned long srcx_16=0; 
		for (unsigned long x=0;x<dst_width;++x) // �����ͼƬ��ÿ�еĵ�RGBA
		{ 
			int n = srcx_16>>16;
			pDstLine[x]=pSrcLine[srcx_16>>16]; 
			srcx_16+=xrIntFloat_16; 
		} 
		srcy_16+=yrIntFloat_16; 
		((TUInt8*&)pDstLine)+=Dst.byte_width; 
	} 
}

void CFastZoom::PicZoom4(const TPicRegion& Dst,const TPicRegion& Src)
{
	PicZoom_float(Dst,Src);
}

void CFastZoom::PicZoom5(const TPicRegion& Dst,const TPicRegion& Src)
{
	PicZoom_Table(Dst,Src);
}

void CFastZoom::PicZoom6(const TPicRegion& Dst,const TPicRegion& Src)
{
	PicZoom_SSE(Dst,Src);
}

void CFastZoom::PicZoom7(const TPicRegion& Dst,const TPicRegion& Src)
{
	PicZoom_SSE_mmh(Dst,Src);
}

void CFastZoom::PicZoom_float(const TPicRegion& Dst,const TPicRegion& Src) 
{ 
	//ע��: �ú�����ҪFPU֧�� 
	if  ((0==Dst.width)||(0==Dst.height) 
		||(0==Src.width)||(0==Src.height))  return ; 
	double xrFloat=1.000000001/((double)Dst.width/Src.width); 
	double yrFloat=1.000000001/((double)Dst.height/Src.height); 
	unsigned  short  RC_Old; 
	unsigned  short  RC_Edit; 
	asm   //����FPU��ȡ����ʽ  Ϊ��ֱ��ʹ��fist����ָ�� 
	{ 
		FNSTCW  RC_Old              // ����Э������������,�����ָ� 
			FNSTCW  RC_Edit             // ����Э������������,�����޸� 
			FWAIT 
			OR      RC_Edit, 0x0F00     // ��Ϊ RC=11  ʹFPU����ȡ��      
			FLDCW   RC_Edit             // ����Э������������,RC���Ѿ��޸� 
	} 
	unsigned  long  dst_width=Dst.width; 
	TARGB32* pDstLine=Dst.pdata; 
	double  srcy=0; 
	for  (unsigned  long  y=0;y<Dst.height;++y) 
	{ 
		TARGB32* pSrcLine=((TARGB32*)((TUInt8*)Src.pdata+Src.byte_width*(( long )srcy))); 
		/**//* 
			double srcx=0; 
			for (unsigned long x=0;x<dst_width;++x) 
			{ 
			pDstLine[x]=pSrcLine[(unsigned long)srcx];//��ΪĬ�ϵĸ���ȡ����һ������ 
			//�Ĳ���! ���Բ�ʹ����ֱ�Ӳ���FPU�����������롣 
			srcx+=xrFloat; 
			}*/ 
		asm fld       xrFloat             //st0==xrFloat 
			asm fldz                          //st0==0   st1==xrFloat 
			unsigned  long  srcx=0; 
		for  ( long  x=0;x<dst_width;++x) 
		{ 
			asm fist dword ptr srcx       //srcx=(long)st0 
				pDstLine[x]=pSrcLine[srcx]; 
			asm fadd  st,st(1)            //st0+=st1   st1==xrFloat 
		} 
		asm fstp      st 
			asm fstp      st 
			srcy+=yrFloat; 
		((TUInt8*&)pDstLine)+=Dst.byte_width; 
	} 
	asm   //�ָ�FPU��ȡ����ʽ 
	{ 
		FWAIT 
			FLDCW   RC_Old  
	} 
}


void  CFastZoom::PicZoom_Table(const TPicRegion& Dst,const TPicRegion& Src) 
{ 
	if  (  (0==Dst.width)||(0==Dst.height) 
		||(0==Src.width)||(0==Src.height))  return ; 
	unsigned  long  dst_width=Dst.width; 
	unsigned  long * SrcX_Table =  new  unsigned  long [dst_width]; 
	for  (unsigned  long  x=0;x<dst_width;++x) //���ɱ� SrcX_Table 
	{ 
		SrcX_Table[x]=(x*Src.width/Dst.width); 
	} 
	TARGB32* pDstLine=Dst.pdata; 
	for  (unsigned  long  y=0;y<Dst.height;++y) 
	{ 
		unsigned  long  srcy=(y*Src.height/Dst.height); 
		TARGB32* pSrcLine=((TARGB32*)((TUInt8*)Src.pdata+Src.byte_width*srcy)); 
		for  (unsigned  long  x=0;x<dst_width;++x) 
			pDstLine[x]=pSrcLine[SrcX_Table[x]]; 
		((TUInt8*&)pDstLine)+=Dst.byte_width; 
	} 
	delete [] SrcX_Table; 
}

void  CFastZoom::PicZoom_SSE(const TPicRegion& Dst,const TPicRegion& Src) 
{ 
	//����: ������ҪCPU֧��MMX��movntqָ�� 
	if  (  (0==Dst.width)||(0==Dst.height) 
		||(0==Src.width)||(0==Src.height))  return ; 
	unsigned long xrIntFloat_16=(Src.width<<16)/Dst.width+1;  
	unsigned long yrIntFloat_16=(Src.height<<16)/Dst.height+1; 
	unsigned  long  dst_width=Dst.width; 
	TARGB32* pDstLine=Dst.pdata; 
	unsigned  long  srcy_16=0; 
	for  (unsigned  long  y=0;y<Dst.height;++y) 
	{ 
		TARGB32* pSrcLine=((TARGB32*)((TUInt8*)Src.pdata+Src.byte_width*(srcy_16>>16))); 
		asm 
		{ 
			push      ebp 
				mov       esi,pSrcLine 
				mov       edi,pDstLine 
				mov       edx,xrIntFloat_16 
				mov       ecx,dst_width 
				xor       ebp,ebp            //srcx_16=0 

				and    ecx, (not 3)     //ѭ��4��չ�� 
				TEST   ECX,ECX    //nop 
				jle    EndWriteLoop 
				lea       edi,[edi+ecx*4] 
			neg       ecx 
				//todo: Ԥ�� 

WriteLoop: 
			mov       eax,ebp 
				shr       eax,16             //srcx_16>>16 
				lea       ebx,[ebp+edx] 
			movd      mm0,[esi+eax*4] 
			shr       ebx,16             //srcx_16>>16 
				PUNPCKlDQ mm0,[esi+ebx*4] 
			lea       ebp,[ebp+edx*2] 

			// movntq qword ptr [edi+ecx*4], mm0  //��ʹ�û����д��ָ�� 
			asm _emit 0x0F asm _emit 0xE7 asm _emit 0x04 asm _emit 0x8F   
				mov       eax,ebp 
				shr       eax,16             //srcx_16>>16 
				lea       ebx,[ebp+edx] 
			movd      mm1,[esi+eax*4] 
			shr       ebx,16             //srcx_16>>16 
				PUNPCKlDQ mm1,[esi+ebx*4] 
			lea       ebp,[ebp+edx*2] 

			// movntq qword ptr [edi+ecx*4+8], mm1 //��ʹ�û����д��ָ�� 
			asm _emit 0x0F asm _emit 0xE7 asm _emit 0x4C asm _emit 0x8F asm _emit 0x08 
				add ecx, 4 
				jnz WriteLoop 
				//sfence //ˢ��д�� 
				asm _emit 0x0F asm _emit 0xAE asm _emit 0xF8   
				emms 
EndWriteLoop: 
			mov    ebx,ebp 
				pop    ebp 
				//����߽�  ѭ������Ϊ0,1,2,3��(���ѭ������չ��,��һ����ת��,��) 
				mov    ecx,dst_width 
				and    ecx,3 
				TEST   ECX,ECX 
				jle    EndLineZoom 
				lea       edi,[edi+ecx*4] 
			neg       ecx 
StartBorder: 
			mov       eax,ebx 
				shr       eax,16             //srcx_16>>16 
				mov       eax,[esi+eax*4] 
			mov       [edi+ecx*4],eax 
				add       ebx,edx 
				inc       ECX 
				JNZ       StartBorder 
EndLineZoom: 
		} 
		// 
		srcy_16+=yrIntFloat_16; 
		((TUInt8*&)pDstLine)+=Dst.byte_width; 
	} 
}

void CFastZoom::PicZoom_SSE_mmh(const TPicRegion& Dst,const TPicRegion& Src) 
{ 
	//����: ������ҪCPU֧��MMX��movntqָ��
	if (  (0==Dst.width)||(0==Dst.height) 
		||(0==Src.width)||(0==Src.height)) return;
	unsigned long xrIntFloat_16=(Src.width<<16)/Dst.width+1;  
	unsigned long yrIntFloat_16=(Src.height<<16)/Dst.height+1;
	unsigned long dst_width=Dst.width; 
	TARGB32* pDstLine=Dst.pdata; 
	unsigned long srcy_16=0; 
	unsigned long for4count=dst_width/4*4; 
	for (unsigned long y=0;y<Dst.height;++y) 
	{ 
		TARGB32* pSrcLine=((TARGB32*)((TUInt8*)Src.pdata+Src.byte_width*(srcy_16>>16))); 
		unsigned long srcx_16=0; 
		unsigned long x; 
		for (x=0;x<for4count;x+=4)//ѭ��4��չ�� 
		{ 
			__m64 m0=_m_from_int(*(int*)(&pSrcLine[srcx_16>>16])); 
			srcx_16+=xrIntFloat_16; 
			m0=_m_punpckldq(m0, _m_from_int(*(int*)(&pSrcLine[srcx_16>>16])) ); 
			srcx_16+=xrIntFloat_16; 
			__m64 m1=_m_from_int(*(int*)(&pSrcLine[srcx_16>>16])); 
			srcx_16+=xrIntFloat_16; 
			m1=_m_punpckldq(m1, _m_from_int(*(int*)(&pSrcLine[srcx_16>>16])) ); 
			srcx_16+=xrIntFloat_16; 
			_mm_stream_pi((__m64 *)&pDstLine[x],m0); //��ʹ�û����д��ָ�� 
			_mm_stream_pi((__m64 *)&pDstLine[x+2],m1); //��ʹ�û����д��ָ�� 
		} 
		for (x=for4count;x<dst_width;++x)//����߽� 
		{ 
			pDstLine[x]=pSrcLine[srcx_16>>16]; 
			srcx_16+=xrIntFloat_16; 
		} 
		srcy_16+=yrIntFloat_16; 
		((TUInt8*&)pDstLine)+=Dst.byte_width; 
	} 
	_m_empty(); 
}