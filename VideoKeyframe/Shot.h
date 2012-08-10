#pragma comment(lib,"cv210.lib")
#pragma comment(lib,"cxcore210.lib")
#pragma comment(lib,"highgui210.lib")

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

typedef struct Shot
{
	int ShotIndex;				//¾µÍ·±àºÅ
	int ShotBegin;			//¾µÍ·Í·Ö¡
	int ShotEnd;				//¾µÍ·Î²Ö¡
	int ShotKeyframe;		//¾µÍ·¹Ø¼üÖ¡
}Shot;
//Ä£·ÂCvBlobSeq
class ShotList
{
public:
	ShotList()
	{
		m_pMem = cvCreateMemStorage();
		m_pSeq = cvCreateSeq(0,sizeof(CvSeq),sizeof(Shot),m_pMem);
		strcpy(m_pElemFormat,"ffffi");
	}
	virtual ~ShotList()
	{
		cvReleaseMemStorage(&m_pMem);
	};
	virtual Shot* GetShot(int ShotIndex)
	{
		return (Shot*)cvGetSeqElem(m_pSeq, ShotIndex);
	};
	virtual void DelShot(int ShotIndex)
	{
		cvSeqRemove(m_pSeq, ShotIndex);
	};
	virtual void Clear()
	{
		cvClearSeq(m_pSeq);
	};
	virtual void AddShot(Shot *pS)
	{
		cvSeqPush(m_pSeq, pS);
	};
	virtual int GetTotalShotNum()
	{
		return m_pSeq->total;
	};
	virtual void Write(CvFileStorage* fs, const char* name)
	{
		const char*  attr[] = {"dt",m_pElemFormat,NULL};
		if(fs)
		{
			cvWrite(fs,name,m_pSeq,cvAttrList(attr,NULL));
		}
	}
	virtual void Load(CvFileStorage* fs, CvFileNode* node)
	{
		if(fs==NULL) return;
		CvSeq* pSeq = (CvSeq*)cvRead(fs, node);
		if(pSeq)
		{
			int i;
			cvClearSeq(m_pSeq);
			for(i=0;i<pSeq->total;++i)
			{
				void* pB = cvGetSeqElem( pSeq, i );
				cvSeqPush( m_pSeq, pB );
			}
		}
	}
	void AddFormat(const char* str){strcat(m_pElemFormat,str);}
protected:
	CvMemStorage*   m_pMem;
	CvSeq*          m_pSeq;
	char            m_pElemFormat[1024];
};
