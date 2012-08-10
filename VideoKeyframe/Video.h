

#include "Shot.h"

class Video
{
	//��ͷ
private:
	BOOL m_bLoadFile;
	double threshold_hist; //ֱ��ͼ��ֵ  
	double threshold_pixel_1;//����  ƽ��������ֵ
	double threshold_pixel_2;//����  �ı�ٷֱ���ֵ
	int  DivisionHistogram(IplImage *src2,IplImage *src1);  //ֱ��ͼ���췽��
	int  DivisionPixel(IplImage *src2,IplImage *src1); //���ز�		

	CvCapture *m_pCapture;
	char m_szVideoFilePath[FILENAME_MAX];
public:
	Video(const char *szVideoFilePath);  //���캯��
	~Video();  //��������

	void setThresholdHist(double newThresholdHist);
	void setThresholdPixel1(double newThresholdPixel1);
	void setThresholdPixel2(double newThresholdPixel2);
//	bool LoadVideoFile(char szVideoFileDirName[]);
	void ShotDivision(int ShotMethodCheckValue = 0);
//	void ShowFrame(long frame_pos);

	CvvImage imgShow;
	ShotList *m_pShotList;
};


