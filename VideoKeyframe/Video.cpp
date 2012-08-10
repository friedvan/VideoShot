
#include "Video.h"

Video::Video(const char *szVideoFilePath)
{
	//�����ļ�·��
	strcpy(m_szVideoFilePath, szVideoFilePath);

	//������Ƶ
	m_pCapture = cvCaptureFromFile(m_szVideoFilePath);
	if(m_pCapture == NULL)
		m_bLoadFile = FALSE;
	else
		m_bLoadFile = TRUE;

	//��ͷ����Ϊ��
	m_pShotList = new ShotList;

	threshold_hist = 0.9;
	threshold_pixel_1 = 20.0;
	threshold_pixel_2 = 0.0;
}

Video::~Video()
{
	if(m_pCapture != NULL)
	{
		cvReleaseCapture(&m_pCapture);
		m_pCapture = NULL;
	}
	if(m_pShotList != NULL)
	{
		//cvReleaseMemStorage(&m_pShotList->m_pMem);
		delete m_pShotList;
	}
}

//����: ����֡�ʵ�־�ͷ�߽���
//���أ�����Ǳ߽緵��1�����򷵻�0
int Video::DivisionPixel(IplImage *pCurrentFrame, IplImage *pPreviousFrame)
{  
	CvRect ROIRect;
	CvScalar diffArray;
	//һ��Ȩֵ���󣬰�ͼ�񻮷ֳ�3*3�ľ��Σ���ÿһ�����ε�֡�����һ��Ȩֵ
	//�ĸ��ǲ�������ͼ�����Ľ�Զ������ȡȨֵ��С����ȨֵΪ1
	double weight[3][3]={
		0.10,0.12,0.10,
		0.12,0.12,0.12,
		0.10,0.12,0.10
	};
	double flag[3][3]={
		0.0,0.0,0.0,
		0.0,0.0,0.0,
		0.0,0.0,0.0
	};

	IplImage *imgDiff=NULL;
	IplImage *pCurrentFrame_hsv = NULL;
	IplImage *pPreviousFrame_hsv = NULL;

	imgDiff = cvCreateImage(cvGetSize(pCurrentFrame), IPL_DEPTH_8U, 3);
	pCurrentFrame_hsv = cvCreateImage(cvGetSize(pCurrentFrame), IPL_DEPTH_8U, 3);
	pPreviousFrame_hsv = cvCreateImage(cvGetSize(pCurrentFrame), IPL_DEPTH_8U, 3);

	//ת����hsv�ռ����
	cvCvtColor(pCurrentFrame, pCurrentFrame_hsv, CV_RGB2HSV);
	cvCvtColor(pPreviousFrame, pPreviousFrame_hsv, CV_RGB2HSV);

	// 	//ͳ�ƾ�ͷ����
	// 	pix_count++;

	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			//ȡ3*3�ľ��εĵ�i*j�����β��趨ΪROI
			ROIRect=cvRect(i*pCurrentFrame_hsv->width/3,j*pCurrentFrame_hsv->height/3,pCurrentFrame_hsv->width/3,pCurrentFrame_hsv->height/3);
			cvSetImageROI(pCurrentFrame_hsv,ROIRect);
			cvSetImageROI(pPreviousFrame_hsv,ROIRect);
			cvSetImageROI(imgDiff, ROIRect);

			cvAbsDiff(pPreviousFrame_hsv, pCurrentFrame_hsv, imgDiff);   //��������ͼ���Ӧ���ص��ֵ�����浽ͼ��imgDiff����
			diffArray=cvSum(imgDiff);//�ֱ��imgDiff��ÿһ��ͨ������ֵ��ͱ�����diffArray��
			if(diffArray.val[0]>threshold_pixel_1*(pCurrentFrame_hsv->height/3)*(pCurrentFrame_hsv->width/3))//threshold_pixel_1�Ǳ����ο������ƽ������
				flag[i][j]=1;					//������ξ��ο��ڼ�⵽�仯������ֵ��flag����Ӧ���ֵΪ1	

			cvResetImageROI(pCurrentFrame_hsv);
			cvResetImageROI(pPreviousFrame_hsv);
			cvResetImageROI(imgDiff);
		}
	}

	//�������ƶ�
	double diffResult=0;
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{			
			flag[i][j]*=weight[i][j];//ͼ��ľŸ�����ֱ����Ȩֵ
			diffResult+=flag[i][j];//ͳ������ͼ������ƶ�
		}
	}
	cvReleaseImage(&imgDiff);
	cvReleaseImage(&pPreviousFrame_hsv);
	cvReleaseImage(&pCurrentFrame_hsv);

	if(diffResult>threshold_pixel_2)//&&(pix_count>=22))//�����ͷ����С��22֡�Ͳ�����һ����ͷ
		return 1;
	else
		return 0;
}
//���� : ����ֱ��ͼ��ʵ�־�ͷ�߽��� 
//���أ�����Ǳ߽緵��1�����򷵻�0
int Video::DivisionHistogram(IplImage *frame2,IplImage *frame1)
{
	//	his_count++;

	int Bin=256;	
	float ranges[2]={0,255};	
	IplImage *Grey2=NULL;
	IplImage *Grey1=NULL;	
	CvHistogram *Grey_hist2=NULL;
	CvHistogram *Grey_hist1=NULL;
	float* Range[1]={&ranges[0]};	

	if(Grey2==NULL)
		Grey2=cvCreateImage(cvGetSize(frame2),8,1);
	if(Grey1==NULL)
		Grey1=cvCreateImage(cvGetSize(frame2),8,1);

	cvCvtColor(frame2, Grey2, CV_BGR2GRAY);//ɫ�ʿռ�任   ����CV_BGR2GRAY ��RGB��gray, 
	cvCvtColor(frame1, Grey1, CV_BGR2GRAY);

	Grey_hist2= cvCreateHist( 1, &Bin, CV_HIST_ARRAY, Range);
	Grey_hist1= cvCreateHist( 1, &Bin, CV_HIST_ARRAY, Range);

	cvCalcHist( &Grey2, Grey_hist2);
	cvCalcHist( &Grey1, Grey_hist1);

	double temp=0,temp_1 = 0,temp_2 = 0,t_sum = 0,corre = 0,t_frame = 0,temp1_sum = 0;
	double sumhist=0;

	for(int i=0;i<Bin;i++)
	{		
		temp_1 = cvQueryHistValue_1D(Grey_hist2,i);
		temp_2 = cvQueryHistValue_1D(Grey_hist1,i); 
		sumhist+=fabs(temp_1-temp_2);
		t_frame = (temp_1>temp_2)?temp_2:temp_1;
		t_sum +=t_frame;
		temp1_sum += temp_1;
	}
	corre = t_sum/temp1_sum;	

	cvReleaseImage(&Grey2);
	cvReleaseImage(&Grey1);
	cvReleaseHist(&Grey_hist2);
	cvReleaseHist(&Grey_hist1);

	//	threshold_hist=0.9;
	//���ǰ����ֱ֡��ͼ���Ʒ���1�����򷵻�0
	if(corre<threshold_hist)//&&(his_count>=22))//��ֵ
		return 1;
	else
		return 0;
}


//���෨�ָͷ
// void ShotandKeyframe::k_means(IplImage* src,long frame_count)
// {
// 	frame_pos++;
// 	
// 	long r,g,b;//һ�����ص��RGBֵ
// 	int h,s,v;//һ�����ص��H,S,V��ֵ  
// 	
// 	int tmp;//�м�ֵ
// 	int min;//rgb��Сֵ
// 	int max;//rgb�ϴ�ֵ
// 
// 	//ÿ֡HSV�������ε�ֱ��ͼֵ
// 	double H[8];
// 	double S[3];
// 	double V[3];
// 
// 
// 	//��ʼ��
// 	S_H=0;
// 	S_S=0;
// 	S_V=0;
// 	for(int a=0;a<8;a++)
// 		arrayh[a]=0;
// 	for(int d=0;d<3;d++)
// 		arrays[d]=0;	
// 	for(int c=0;c<3;c++)
// 		arrayv[c]=0;
// 
// //		cvShowImage("11",src);
// 
// 	//��һ֡��hsv�������ϵ����ص�ĸ���
// 	for(int x=0;x<src->width;x++) 
// 	{
// 		for(int y=0;y<src->height;y++)
// 		{	
// 			//����һ�����ص��RGBֵ
// 			b=((uchar*)(src->imageData + src->widthStep*y))[x*3]; 
// 			g=((uchar*)(src->imageData + src->widthStep*y))[x*3+1];  
// 			r=((uchar*)(src->imageData + src->widthStep*y))[x*3+2]; 
// 
// 			tmp=(r<g)?r:g;
// 			min=(tmp<b)?tmp:b;
// 			
// 			tmp=(r>g)?r:g;
// 			max=(tmp>b)?tmp:b;
// 					
// 			if(max==min) 
// 				break;
// 
// 	        //����һ�����ص��HSVֵ
// 			if(max!=0) 
// 				s=(max-min)/max;//���Ͷ�
// 			else 
// 			{
// 				s=0;
// 				h=-1;
// 				break;
// 			}  		
// 			v = max/255; 	// ����			
// 			if( r == max )	//ɫ�ʶ�
// 				h = ( g - b ) / (max-min); // between yellow & magenta	
// 			else if( g == max )	
// 				h = 2 + ( b -r ) /(max-min); // between cyan & yellow	
// 			else		
// 				h = 4 + ( r - g ) /(max-min); // between magenta & cyan	
// 			h *= 60;  
// 			if( h < 0 )		
//                 h += 360;   
// 			
// 			//��ɫ��H�ռ�ֳ�8�ݣ� ���Ͷ�S������v�ռ�ֱ�ֳ�3��
// 			//H��S��Vȡֵ��ΧΪΪ[0,7]��[0,2]��[0,2]
// 
// 			//H�����������ص����
// 			if(h>=316&&h<=360||h>=0&&h<=20)
// 				arrayh[0]++;
// 			else if(h>=21&&h<=40)
// 				arrayh[1]++;
// 			else if(h>=41&&h<=75)
// 				arrayh[2]++;
// 			else if(h>=76&&h<=155)
// 				arrayh[3]++;
// 			else if(h>=156&&h<=190)
// 				arrayh[4]++;
// 			else if(h>=191&&h<=270)
// 				arrayh[5]++;
// 			else if(h>=271&&h<=295)
// 				arrayh[6]++;
// 			else if(h>=296&&h<=315)
// 				arrayh[7]++;
// 			
// 			//S�����������ص����
// 			if(s>=0&&s<=0.2)
// 				arrays[0]++;
// 			else if(s>=0.2&&s<=0.7)
// 				arrays[1]++;
// 			else if(s>=0.7&&s<=1)
// 				arrays[2]++;
// 			
// 			//V�����������ص����
// 			if(v>=0&&v<=0.2)
// 				arrayv[0]++;
// 			else if(v>=0.2&&v<=0.7)
// 				arrayv[1]++;
// 			else if(v>=0.7&&v<=1)
// 				arrayv[2]++;
// 		}   
// 	}
// 	
// 	//����һ֡��HSV�������ε�ֱ��ͼ
// 	int i,j,k;
// 	for (i=0;i<8;i++)
// 		H[i]=(double)(arrayh[i])/(src->width*src->height);
// 	for (j=0;j<3;j++)
// 		S[j]=(double)(arrays[j])/(src->width*src->height);
// 	for (k=0;k<3;k++)
// 		V[k]=(double)(arrayv[k])/(src->width*src->height);
// 	
// 
// 	if(frame_pos==1)//��һ֡���Ե�һ����ͷ��ʼ����������
// 	{
// 		shot_count++;
// 		for (i=0;i<8;i++)
// 		{
// 			shot_H[i]=H[i];
// 			sum_H[i]=H[i];
// 		}
// 		for (j=0;j<3;j++)
// 		{
// 			shot_S[j]=S[j];
// 			sum_S[j]=S[j];
// 		}
// 		for (k=0;k<3;k++)
// 		{
// 			shot_V[k]=V[k];
// 			sum_V[k]=V[k];
// 		}
// 	}
// 	else
// 	{
// 		//���㵱ǰ֡�뾵ͷ��HSV�������ε�������
// 		for (i=0;i<8;i++)
// 			S_H+=(H[i]<shot_H[i])?H[i]:shot_H[i];
// 		for (j=0;j<3;j++)
// 			S_S+=(S[j]<shot_S[j])?S[j]:shot_S[j];
// 		for (k=0;k<3;k++)
// 			S_V+=(V[k]<shot_V[k])?V[k]:shot_V[k];
// 
// 
// 		
// 		//��ǰ֡�뾵ͷ��������
// 		if(frame_pos==2)
// 		{
// 			PS_HSV=S_HSV=(0.9*S_H+0.3*S_S+0.1*S_V)/3;
// 			p_cha=cha=0;
// 		}
// 		else
// 		{ 
// 			p_cha=cha;
// 			PS_HSV=S_HSV;
// 			S_HSV=(0.9*S_H+0.3*S_S+0.1*S_V)/3;
// 		}
// 		cha=S_HSV-PS_HSV;
// 
// 		
// 		//��ǰ֡�뾵ͷ�����ƶȺ�ǰһ֡�뾵ͷ�����ƶ�֮��С����ֵ��
// 		//���������ֵ��ǰһ����ֵͬʱ����һ����ֵ�����¼����������
// 		//���þ�ͷ֡��С��8ʱ����������(�����������)
// 		if((fabs(cha)<0.025)||(fabs(p_cha)>=0.015)||shot_count<20)
// 		{
// 			shot_count++;
// 			for (i=0;i<8;i++)
// 			{
// 				sum_H[i]+=H[i];	
// 				shot_H[i]=(sum_H[i]/shot_count);
// 			}
// 			for (j=0;j<3;j++)
// 			{
// 				sum_S[j]+=S[j];	
// 				shot_S[j]=(sum_S[j]/shot_count);
// 			}
// 			for (k=0;k<3;k++)
// 			{
// 				sum_V[k]+=V[k];	
// 				shot_V[k]=(sum_V[k]/shot_count);
// 			}
// 
// 		}
// 		//���������һ����ͷ
// 		else 
// 		{
// 			if(shot_visit==1)//��һ����ͷ
// 			{
// 				VideoData* newnode = new VideoData;
// 				newnode->shot_head = 0;
// 				newnode->shot_id = 0;
// 				newnode->shot_tail = frame_pos-2;
// 				newnode->next = NULL;
// 				ptrf=ptrb=newnode;
// 		
// 				shot_visit++;
// 
// 			    //����һ����ͷ��ʼ����������
// 				shot_count=1;
// 				for (i=0;i<8;i++)
// 				{
// 					shot_H[i]=H[i];
// 					sum_H[i]=H[i];
// 				}
// 				for (j=0;j<3;j++)
// 				{
// 					shot_S[j]=S[j];
// 					sum_S[j]=S[j];
// 				}
// 				for (k=0;k<3;k++)
// 				{
// 					shot_V[k]=V[k];
// 					sum_V[k]=V[k];
// 				}
// 				
// 	//			cout<<"��ͷ"<<1<<":"<<ptrb->shot_head<<"--"<<ptrb->shot_tail<<endl;
// 			}
// 			else
// 			{
// 				VideoData* newnode = new VideoData;
// 				newnode->shot_head = (ptrb->shot_tail+1);
// 				newnode->shot_id = (shot_visit-1);
// 				newnode->shot_tail = frame_pos-2;
// 				newnode->next = NULL;
// 				ptrb->next = newnode;
// 				ptrb = newnode;
// 				
// 				shot_visit++;
// 
// 				//����һ����ͷ��ʼ����������
// 				shot_count=1;
// 				for (i=0;i<8;i++)
// 				{
// 					shot_H[i]=H[i];
// 					sum_H[i]=H[i];
// 				}
// 				for (j=0;j<3;j++)
// 				{
// 					shot_S[j]=S[j];
// 					sum_S[j]=S[j];
// 				}
// 				for (k=0;k<3;k++)
// 				{
// 					shot_V[k]=V[k];
// 					sum_V[k]=V[k];
// 				}
// 				
// 		//		cout<<"��ͷ"<<ptrb->shot_index+1<<":"<<ptrb->shot_head<<"--"<<ptrb->shot_tail<<endl;
// 			}
// 		}
// 	}
// 
// 
// 	if (frame_pos==frame_count)
// 	{
// 		if (shot_visit==1)
// 		{
// 			VideoData* newnode = new VideoData;
// 			newnode->shot_head = 0;
// 			newnode->shot_id = 0;
// 			newnode->shot_tail = frame_pos;
// 			newnode->next = NULL;
// 			ptrf=ptrb=newnode;
// 
// 	//		cout<<"��ͷ"<<1<<":"<<ptrb->shot_head<<"--"<<ptrb->shot_tail<<endl;
// 		}
// 		else
// 		{
// 			VideoData* newnode = new VideoData;
// 			newnode->shot_head = (ptrb->shot_tail+1);
// 			newnode->shot_id = (shot_visit-1);
// 			newnode->shot_tail = frame_pos-1;
// 			newnode->next = NULL;
// 			ptrb->next = newnode;
// 			ptrb = newnode;
// 			
// 	//		cout<<"��ͷ"<<ptrb->shot_index+1<<":"<<ptrb->shot_head<<"--"<<ptrb->shot_tail<<endl;
// 		}
// 		frame_pos=0;
// //		shotframeCount=0;
// 		shot_visit=1;
// 	}
// }



void Video::ShotDivision(int ShotMethodCheckValue = 0)
{	

	IplImage *pPreviousFrame =  NULL;
	IplImage *pCurrentFrame = NULL;

	//Initial();//��ʼ��


	long begin=GetTickCount();//���㴦��ʱ��Ŀ�ʼʱ���
	int ShotIndex = 0;//��ʾ��ͷ��ID
	int nframes = 0;//��ʾ��Ƶ֡��λ��



	/////////////////////////////////����Ƶ�м����ĳ�㲻�ܶ�ʱ������frame_count�ĺ���ʧЧ��k_mean��������
	long frame_count=(long)cvGetCaptureProperty(m_pCapture,CV_CAP_PROP_FRAME_COUNT);//���֡����


	//�����
	int HistogramResult = 0;
	int PixelResult = 0;

	//��վ�ͷ�������
	m_pShotList->Clear();

	//����Ƶ��λ����ʼ
	cvSetCaptureProperty(m_pCapture,CV_CAP_PROP_POS_FRAMES,0);
	while(pCurrentFrame = cvQueryFrame(m_pCapture))
	{
		if(nframes == 0)
		{
			pPreviousFrame = cvCloneImage(pCurrentFrame);
		}
		//���浽CvvImage�ṹ�У���mfc��ʾ���ؼ���
		imgShow.CopyOf(pCurrentFrame);

		//�ָ�����ѡ��
		if(ShotMethodCheckValue==0||ShotMethodCheckValue==-1)
		{
			PixelResult = DivisionPixel(pCurrentFrame, pPreviousFrame);
			HistogramResult =DivisionHistogram(pCurrentFrame, pPreviousFrame);
		}
		else if(ShotMethodCheckValue==1)
		{
			HistogramResult=DivisionHistogram(pCurrentFrame, pPreviousFrame);
			PixelResult = 1;
		}
		else if(ShotMethodCheckValue==2)
		{
			HistogramResult=1;
			PixelResult = DivisionPixel(pCurrentFrame, pCurrentFrame);	
		}
		// 		else if(ShotMethodCheckValue==3)
		// 		{
		// 		    k_means(pCurrentFrame,frame_count);	
		// 		}

		//ȡֱ��ͼ�����ز����ּ�����Ľ�����������ּ�ⷽ������⵽�˾�ͷ�仯���ж�Ϊ��ͷ�仯
		if(PixelResult*HistogramResult)
		{
			//����һ����ͷ��ʱ��
			if(ShotIndex == 0)
			{	
				Shot NewS;
				NewS.ShotBegin = 0;
				NewS.ShotEnd = nframes;
				NewS.ShotKeyframe = 0;
				NewS.ShotIndex = 0;
				m_pShotList->AddShot(&NewS);
			}
			else
			{
				Shot NewS;
				Shot *pS = m_pShotList->GetShot(ShotIndex - 1);//�õ���һ����ͷ
				NewS.ShotBegin = pS->ShotEnd + 1;//��һ����ͷ�Ľ�β����1���Ǳ�����ͷ�Ŀ�ʼ
				NewS.ShotIndex = ShotIndex;
				NewS.ShotEnd = nframes ;
				NewS.ShotKeyframe = NewS.ShotBegin;//�ؼ�֡Ĭ��Ϊ��ͷ��֡���������ؼ�֡��ȫ���ٸ���
				m_pShotList->AddShot(&NewS);
			}
			ShotIndex++;
		}

		//���汾֡Ϊ����һ֡�У�׼���´�ѭ��
		cvCopyImage(pCurrentFrame,pPreviousFrame);
//		video_time.current_frame_pos=nframes;//���µ�ǰ֡λ��
		nframes ++;
	}/*end of while*/

	//�������һ����ͷ(����ֻ��һ����ͷ���)
	{
		Shot LastShot, *pS;
		pS = m_pShotList->GetShot(ShotIndex - 1);
		if(pS != NULL)
		{
			LastShot.ShotBegin = pS->ShotEnd + 1;
			LastShot.ShotEnd = nframes - 1;//����ѭ����һ֡�ǲ��ɶ��ģ��ʼ�ȥ1���õ����ɶ���һ֡
			LastShot.ShotIndex = ShotIndex;
			LastShot.ShotKeyframe = LastShot.ShotBegin;
			m_pShotList->AddShot(&LastShot);
		}
	}

// 	//����ʱ��Ľ���ʱ���
// 	long end = GetTickCount();
// 	video_time.shot_time=end-begin;

	//�ͷ��ڴ�
	cvReleaseImage(&pPreviousFrame);
	cvReleaseImage(&pCurrentFrame);
	pPreviousFrame=NULL;
	pCurrentFrame=NULL;
}
