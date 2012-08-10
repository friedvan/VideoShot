/*
	author: pzc
	email: rabbitpzc#gmail.com

	目的是从视频中检测出镜头的变化，并标记镜头号，开始和结束的帧号。基于OpenCV，但目前只在Windows版本的OpenCV 2.1上测试过。
*/

#include "Video.h"

int main()
{
	//首先确定视频路径
	const char *filename= "d:\\test.avi";

	//初始化Video类
	Video video(filename);

	//调用默认方法分割镜头
	video.ShotDivision();
	
	//分割的结果保存在以m_pShotList为首的表中，以所示的方法访问镜头
	int nTotalShotNumber = video.m_pShotList->GetTotalShotNum();
	for(int nShotIndex = 0; nShotIndex < nTotalShotNumber; nShotIndex ++)
	{
		Shot *pS = video.m_pShotList->GetShot(nShotIndex);
		//ShotIndex是镜头编号，ShotBegin是镜头的开始帧，ShotEnd是镜头的结束帧
		printf("index: %d\tbegin: %d\tend: %d\n", pS->ShotIndex, pS->ShotBegin, pS->ShotEnd);
	}
	return 0;
}