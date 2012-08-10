/*
	author: pzc
	email: rabbitpzc#gmail.com

	Ŀ���Ǵ���Ƶ�м�����ͷ�ı仯������Ǿ�ͷ�ţ���ʼ�ͽ�����֡�š�����OpenCV����Ŀǰֻ��Windows�汾��OpenCV 2.1�ϲ��Թ���
*/

#include "Video.h"

int main()
{
	//����ȷ����Ƶ·��
	const char *filename= "d:\\test.avi";

	//��ʼ��Video��
	Video video(filename);

	//����Ĭ�Ϸ����ָͷ
	video.ShotDivision();
	
	//�ָ�Ľ����������m_pShotListΪ�׵ı��У�����ʾ�ķ������ʾ�ͷ
	int nTotalShotNumber = video.m_pShotList->GetTotalShotNum();
	for(int nShotIndex = 0; nShotIndex < nTotalShotNumber; nShotIndex ++)
	{
		Shot *pS = video.m_pShotList->GetShot(nShotIndex);
		//ShotIndex�Ǿ�ͷ��ţ�ShotBegin�Ǿ�ͷ�Ŀ�ʼ֡��ShotEnd�Ǿ�ͷ�Ľ���֡
		printf("index: %d\tbegin: %d\tend: %d\n", pS->ShotIndex, pS->ShotBegin, pS->ShotEnd);
	}
	return 0;
}