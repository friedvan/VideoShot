

#include "Shot.h"

class Video
{
	//镜头
private:
	BOOL m_bLoadFile;
	double threshold_hist; //直方图阈值  
	double threshold_pixel_1;//像素  平均亮度阈值
	double threshold_pixel_2;//像素  改变百分比阈值
	int  DivisionHistogram(IplImage *src2,IplImage *src1);  //直方图差异方法
	int  DivisionPixel(IplImage *src2,IplImage *src1); //像素差		

	CvCapture *m_pCapture;
	char m_szVideoFilePath[FILENAME_MAX];
public:
	Video(const char *szVideoFilePath);  //构造函数
	~Video();  //析构函数

	void setThresholdHist(double newThresholdHist);
	void setThresholdPixel1(double newThresholdPixel1);
	void setThresholdPixel2(double newThresholdPixel2);
//	bool LoadVideoFile(char szVideoFileDirName[]);
	void ShotDivision(int ShotMethodCheckValue = 0);
//	void ShowFrame(long frame_pos);

	CvvImage imgShow;
	ShotList *m_pShotList;
};


