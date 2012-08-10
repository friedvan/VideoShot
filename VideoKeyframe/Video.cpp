
#include "Video.h"

Video::Video(const char *szVideoFilePath)
{
	//设置文件路径
	strcpy(m_szVideoFilePath, szVideoFilePath);

	//读入视频
	m_pCapture = cvCaptureFromFile(m_szVideoFilePath);
	if(m_pCapture == NULL)
		m_bLoadFile = FALSE;
	else
		m_bLoadFile = TRUE;

	//镜头链表为空
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

//功能: 利用帧差法实现镜头边界检测
//返回：如果是边界返回1，否则返回0
int Video::DivisionPixel(IplImage *pCurrentFrame, IplImage *pPreviousFrame)
{  
	CvRect ROIRect;
	CvScalar diffArray;
	//一个权值矩阵，把图像划分成3*3的矩形，对每一个矩形的帧差乘以一个权值
	//四个角部分由于图像中心较远，所以取权值较小，总权值为1
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

	//转换到hsv空间计算
	cvCvtColor(pCurrentFrame, pCurrentFrame_hsv, CV_RGB2HSV);
	cvCvtColor(pPreviousFrame, pPreviousFrame_hsv, CV_RGB2HSV);

	// 	//统计镜头长度
	// 	pix_count++;

	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			//取3*3的矩形的第i*j个矩形并设定为ROI
			ROIRect=cvRect(i*pCurrentFrame_hsv->width/3,j*pCurrentFrame_hsv->height/3,pCurrentFrame_hsv->width/3,pCurrentFrame_hsv->height/3);
			cvSetImageROI(pCurrentFrame_hsv,ROIRect);
			cvSetImageROI(pPreviousFrame_hsv,ROIRect);
			cvSetImageROI(imgDiff, ROIRect);

			cvAbsDiff(pPreviousFrame_hsv, pCurrentFrame_hsv, imgDiff);   //计算两幅图像对应像素点差值，保存到图像imgDiff里面
			diffArray=cvSum(imgDiff);//分别对imgDiff的每一个通道像素值求和保存在diffArray中
			if(diffArray.val[0]>threshold_pixel_1*(pCurrentFrame_hsv->height/3)*(pCurrentFrame_hsv->width/3))//threshold_pixel_1是本矩形框区域的平均亮度
				flag[i][j]=1;					//如果本次矩形框内检测到变化大于阈值则将flag中相应的项赋值为1	

			cvResetImageROI(pCurrentFrame_hsv);
			cvResetImageROI(pPreviousFrame_hsv);
			cvResetImageROI(imgDiff);
		}
	}

	//计算相似度
	double diffResult=0;
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{			
			flag[i][j]*=weight[i][j];//图像的九个区域分别乘以权值
			diffResult+=flag[i][j];//统计整个图像的相似度
		}
	}
	cvReleaseImage(&imgDiff);
	cvReleaseImage(&pPreviousFrame_hsv);
	cvReleaseImage(&pCurrentFrame_hsv);

	if(diffResult>threshold_pixel_2)//&&(pix_count>=22))//如果镜头长度小于22帧就不算做一个镜头
		return 1;
	else
		return 0;
}
//功能 : 利用直方图法实现镜头边界检测 
//返回：如果是边界返回1，否则返回0
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

	cvCvtColor(frame2, Grey2, CV_BGR2GRAY);//色彩空间变换   参数CV_BGR2GRAY 是RGB到gray, 
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
	//如果前后两帧直方图相似返回1，否则返回0
	if(corre<threshold_hist)//&&(his_count>=22))//阈值
		return 1;
	else
		return 0;
}


//聚类法分割镜头
// void ShotandKeyframe::k_means(IplImage* src,long frame_count)
// {
// 	frame_pos++;
// 	
// 	long r,g,b;//一个像素点的RGB值
// 	int h,s,v;//一个像素点的H,S,V的值  
// 	
// 	int tmp;//中间值
// 	int min;//rgb较小值
// 	int max;//rgb较大值
// 
// 	//每帧HSV分量各段的直方图值
// 	double H[8];
// 	double S[3];
// 	double V[3];
// 
// 
// 	//初始化
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
// 	//求一帧的hsv各分量上的像素点的个数
// 	for(int x=0;x<src->width;x++) 
// 	{
// 		for(int y=0;y<src->height;y++)
// 		{	
// 			//计算一个像素点的RGB值
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
// 	        //计算一个像素点的HSV值
// 			if(max!=0) 
// 				s=(max-min)/max;//饱和度
// 			else 
// 			{
// 				s=0;
// 				h=-1;
// 				break;
// 			}  		
// 			v = max/255; 	// 亮度			
// 			if( r == max )	//色彩度
// 				h = ( g - b ) / (max-min); // between yellow & magenta	
// 			else if( g == max )	
// 				h = 2 + ( b -r ) /(max-min); // between cyan & yellow	
// 			else		
// 				h = 4 + ( r - g ) /(max-min); // between magenta & cyan	
// 			h *= 60;  
// 			if( h < 0 )		
//                 h += 360;   
// 			
// 			//把色调H空间分成8份， 饱和度S和亮度v空间分别分成3份
// 			//H、S和V取值范围为为[0,7]、[0,2]、[0,2]
// 
// 			//H分量各段像素点个数
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
// 			//S分量各段像素点个数
// 			if(s>=0&&s<=0.2)
// 				arrays[0]++;
// 			else if(s>=0.2&&s<=0.7)
// 				arrays[1]++;
// 			else if(s>=0.7&&s<=1)
// 				arrays[2]++;
// 			
// 			//V分量各段像素点个数
// 			if(v>=0&&v<=0.2)
// 				arrayv[0]++;
// 			else if(v>=0.2&&v<=0.7)
// 				arrayv[1]++;
// 			else if(v>=0.7&&v<=1)
// 				arrayv[2]++;
// 		}   
// 	}
// 	
// 	//计算一帧的HSV分量各段的直方图
// 	int i,j,k;
// 	for (i=0;i<8;i++)
// 		H[i]=(double)(arrayh[i])/(src->width*src->height);
// 	for (j=0;j<3;j++)
// 		S[j]=(double)(arrays[j])/(src->width*src->height);
// 	for (k=0;k<3;k++)
// 		V[k]=(double)(arrayv[k])/(src->width*src->height);
// 	
// 
// 	if(frame_pos==1)//第一帧，对第一个镜头初始化聚类中心
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
// 		//计算当前帧与镜头在HSV分量各段的相似性
// 		for (i=0;i<8;i++)
// 			S_H+=(H[i]<shot_H[i])?H[i]:shot_H[i];
// 		for (j=0;j<3;j++)
// 			S_S+=(S[j]<shot_S[j])?S[j]:shot_S[j];
// 		for (k=0;k<3;k++)
// 			S_V+=(V[k]<shot_V[k])?V[k]:shot_V[k];
// 
// 
// 		
// 		//当前帧与镜头的相似性
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
// 		//当前帧与镜头的相似度和前一帧与镜头的相似度之差小于阈值，
// 		//或者这个差值和前一个差值同时大于一个阈值，重新计算聚类中心
// 		//当该镜头帧数小于8时，继续聚类(针对闪光问题)
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
// 		//否则进入下一个镜头
// 		else 
// 		{
// 			if(shot_visit==1)//第一个镜头
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
// 			    //对下一个镜头初始化聚类中心
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
// 	//			cout<<"镜头"<<1<<":"<<ptrb->shot_head<<"--"<<ptrb->shot_tail<<endl;
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
// 				//对下一个镜头初始化聚类中心
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
// 		//		cout<<"镜头"<<ptrb->shot_index+1<<":"<<ptrb->shot_head<<"--"<<ptrb->shot_tail<<endl;
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
// 	//		cout<<"镜头"<<1<<":"<<ptrb->shot_head<<"--"<<ptrb->shot_tail<<endl;
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
// 	//		cout<<"镜头"<<ptrb->shot_index+1<<":"<<ptrb->shot_head<<"--"<<ptrb->shot_tail<<endl;
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

	//Initial();//初始化


	long begin=GetTickCount();//计算处理时间的开始时间戳
	int ShotIndex = 0;//标示镜头的ID
	int nframes = 0;//标示视频帧的位置



	/////////////////////////////////当视频中间存在某点不能读时，引用frame_count的函数失效，k_mean中引用了
	long frame_count=(long)cvGetCaptureProperty(m_pCapture,CV_CAP_PROP_FRAME_COUNT);//获得帧数量


	//检测结果
	int HistogramResult = 0;
	int PixelResult = 0;

	//清空镜头检测结果表
	m_pShotList->Clear();

	//将视频定位到开始
	cvSetCaptureProperty(m_pCapture,CV_CAP_PROP_POS_FRAMES,0);
	while(pCurrentFrame = cvQueryFrame(m_pCapture))
	{
		if(nframes == 0)
		{
			pPreviousFrame = cvCloneImage(pCurrentFrame);
		}
		//保存到CvvImage结构中，供mfc显示到控件用
		imgShow.CopyOf(pCurrentFrame);

		//分隔方法选项
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

		//取直方图和像素差两种检测结果的交集，如果两种检测方法都检测到了镜头变化则判断为镜头变化
		if(PixelResult*HistogramResult)
		{
			//当第一个镜头的时候
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
				Shot *pS = m_pShotList->GetShot(ShotIndex - 1);//得到上一个镜头
				NewS.ShotBegin = pS->ShotEnd + 1;//上一个镜头的结尾加上1就是本个镜头的开始
				NewS.ShotIndex = ShotIndex;
				NewS.ShotEnd = nframes ;
				NewS.ShotKeyframe = NewS.ShotBegin;//关键帧默认为镜头首帧，后面计算关键帧完全后再更新
				m_pShotList->AddShot(&NewS);
			}
			ShotIndex++;
		}

		//保存本帧为到上一帧中，准备下次循环
		cvCopyImage(pCurrentFrame,pPreviousFrame);
//		video_time.current_frame_pos=nframes;//更新当前帧位置
		nframes ++;
	}/*end of while*/

	//处理最后一个镜头(包括只有一个镜头情况)
	{
		Shot LastShot, *pS;
		pS = m_pShotList->GetShot(ShotIndex - 1);
		if(pS != NULL)
		{
			LastShot.ShotBegin = pS->ShotEnd + 1;
			LastShot.ShotEnd = nframes - 1;//跳出循环的一帧是不可读的，故减去1，得到最后可读的一帧
			LastShot.ShotIndex = ShotIndex;
			LastShot.ShotKeyframe = LastShot.ShotBegin;
			m_pShotList->AddShot(&LastShot);
		}
	}

// 	//计算时间的结束时间戳
// 	long end = GetTickCount();
// 	video_time.shot_time=end-begin;

	//释放内存
	cvReleaseImage(&pPreviousFrame);
	cvReleaseImage(&pCurrentFrame);
	pPreviousFrame=NULL;
	pCurrentFrame=NULL;
}
