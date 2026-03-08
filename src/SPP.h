#pragma once

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<cmath>
#include<sstream>
#include<iomanip>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

#define PAI 3.1415926535898
#define PAI2 (2.0*PAI)                    /* 2pi */
#define Rad (PAI/180.0)                  /* Radians per degree */
#define Deg (180.0/PAI)                  /* Degrees per radian */
#define C_Light 299792458.0      /* Speed of light  [m/s]; IAU 1976  */

#define R_WGS84  6378137.0          /* Radius Earth [m]; WGS-84  */
#define F_WGS84  1.0/298.257223563  /* Flattening; WGS-84   */
#define Omega_WGS 7.2921151467e-5   /*[rad/s], the earth rotation rate地球自转角速度 */ 
#define GM_Earth   398600.5e+9      /* [m^3/s^2]; WGS-84 */
#define R_CGS2K  6378137.0          /* Radius Earth [m]; CGCS2000  */
#define F_CGS2K  1.0/298.257222101  /* Flattening; CGCS2000   */
#define Omega_BDS 7.2921150e-5      /*[rad/s], the earth rotation rate */
#define GM_BDS   398600.4418e+9     /* [m^3/s^2]; CGCS2000  */

/* some constants about GPS satellite signal */
#define  FG1_GPS  1575.42E6             /* L1信号频率 */
#define  FG2_GPS  1227.60E6             /* L2信号频率 */
#define  FG12R    (77/60.0)             /* FG1_Freq/FG2_Freq */
#define  FG12R2   (5929/3600.0)
#define  WL1_GPS  (C_Light/FG1_GPS)
#define  WL2_GPS  (C_Light/FG2_GPS)

/* some constants about Compass satellite signal */
#define  FG1_BDS  1561.098E6               /* B1信号的基准频率 */
#define  FG2_BDS  1207.140E6               /* B2信号的基准频率 */
#define  FG3_BDS  1268.520E6               /* B2信号的基准频率 */
#define  FC12R    (FG1_BDS/FG2_BDS)       /* FG1_BDS/FG2_BDS */
#define  FC12R2   (FC12R*FC12R)           /* FG1_BDS^2/FG2_BDS^2 */
#define  FC13R    (FG1_BDS/FG3_BDS)       /* FG1_BDS^2/FG3_BDS^2 */
#define  FC13R2   (FC13R*FC13R)
#define  WL1_BDS  (C_Light/FG1_BDS)                                                                                                                                              
#define  WL2_BDS  (C_Light/FG2_BDS)
#define  WL3_BDS  (C_Light/FG3_BDS)

#define MAXGPSNUM  32
#define MAXBDSNUM 63
#define MAXCHANNUM 36               //一个历元最多的可视卫星数？
#define MAXRAWLEN 40960             //缓冲区大小
#define POLYCRC32 0xEDB88320u       //CRC校验码除数

extern int FILEMODE;                   //1：二进制文件；0：串口

//定义导航卫星系统 
enum GNSSSys { UNKS = 0, GPS, BDS, GLONASS, GALILEO, QZSS };   //UNKS未定义系统

//定义通用时结构体
struct COMMONTIME {
	short Year;
	unsigned short Month;
	unsigned short Day;
	unsigned short Hour;
	unsigned short Minute;
	double Second;

	COMMONTIME()
	{
		Year = 0;
		Month = 0;
		Day = 0;
		Hour = 0;
		Minute = 0;
		Second = 0.0;
	}
};

//定义简化儒略日结构体
struct MJDTIME {
	int Days;//整数天
	double FracDay;//小数天

	MJDTIME()
	{
		Days = 0;
		FracDay = 0.0;
	}
};

//定义GPS时结构体
struct GPSTIME {
	unsigned short Week;
	double SecOfWeek;

	GPSTIME()
	{
		Week = 0;
		SecOfWeek = 0.0;
	}
};

//卫星广播星历
struct GPSEPHREC {
	short Prn;
	GNSSSys Sys;
	GPSTIME TOC, TOE;         //卫星钟参考时刻，星历（轨道）参考时刻（s）
	double ClkBias, ClkDrift, ClkDriftRate;       //卫星钟的偏差(s)、漂移、漂移速度
	double IODE, IODC;                            //星历发布时间，钟的数据龄期
	double SqrtA, M0, e, OMEGA, i0, omega;        //轨道长半径平方根(根号m)，平近点角(rad)，偏心率，升交点赤经(rad)，轨道倾角(rad)，近地点角距(rad)
	double Crs, Cuc, Cus, Cic, Cis, Crc;          //短周期摄动项系数(m,rad,rad,rad,rad,m)
	double DeltaN, OMEGADot, iDot;                //平均角速度改正数（rad/s),长期摄动项参数（rad/s)，iDot（rad/s)
	int SVHealth;              //BDS:0 is good
	double TGD1, TGD2;        //群延迟（s,GPS一个，BDS两个）

	GPSEPHREC() {
		Prn = 0;
		SVHealth = 1;
		Sys = UNKS;
		ClkBias = ClkDrift = ClkDriftRate = IODE = IODC = TGD1 = TGD2 = 0.0;
		SqrtA = e = M0 = OMEGA = i0 = omega = OMEGADot = iDot = DeltaN = 0.0;
		Crs = Cuc = Cus = Cic = Cis = Crc = 0.0;
	}
};

//定义每颗卫星的观测数据
struct SATOBSDATA {
	short Prn;
	GNSSSys System;
	double P[2], L[2], D[2];
	double cn[2], LockTime[2];  //载噪比，锁定时长
	unsigned char half[2];      //相位锁定？
	bool Valid;                 //标记值,这个用来干什么的，判断观测值的状态吗？这个valid是在粗差探测中修改吗？
	//检查该卫星的双频伪距和相位数据是否有效和完整，若不全或为0，将Valid标记为false，continue

	SATOBSDATA()
	{
		Prn = 0;
		System = UNKS;
		for(int i=0;i<2;i++)
		{
			P[i] = L[i] = D[i] = 0.0;
		}
		Valid = false;
	}
};

/* 每颗卫星位置、速度、钟差和钟速等的中间计算结果 */
struct SATMIDRES
{
	double SatPos[3], SatVel[3];
	double SatClkOft, SatClkSft;
	double Elevation, Azimuth;
	double TropCorr;
	double Tgd1, Tgd2;
	bool Valid;  //false=没有星历或星历过期,true=计算成功

	SATMIDRES()
	{
		SatPos[0] = SatPos[1] = SatPos[2] = 0.0;
		SatVel[0] = SatVel[1] = SatVel[2] = 0.0;
		Elevation = PAI / 2.0;
		SatClkOft = SatClkSft = 0.0;
		Tgd1 = Tgd2 = TropCorr = 0.0;
		Valid = false;
	}
};

struct MWGF
{
	short Prn;//卫星号
	GNSSSys Sys;
	double MW;
	double GF;
	double PIF;
	int n; //平滑计数

	MWGF()
	{
		Prn = n = 0;
		Sys = UNKS;
		MW = GF = PIF = 0.0;
	}
};

//定义每个历元的观测数据
struct EPOCHOBSDATA {
	GPSTIME Time;      //从头文件中获取
	short SatNum;
	SATOBSDATA SatObs[MAXCHANNUM];
	MWGF ComObs[MAXCHANNUM];
	SATMIDRES  SatPVT[MAXCHANNUM];
	//ComObs和SatPVT数组存储的卫星顺序，与SatObs数组相同，即用相同循环i，可以找到该卫星的观测值、卫星位置和可用性。

	EPOCHOBSDATA()
	{
		SatNum = 0;
	}
};


//定义每个历元的定位结果结构体
struct POSRES {
	GPSTIME Time;
	double Pos[3], Vel[3];
	double RcvClkOft[2];//接收机GPS、BDS信号钟差
	double RcvClkSft;//接收机钟差变化率
	double PDOP, SigmaPos, SigmaVel;        //精度指标
	int SatNum, GPS_SatNum, BDS_SatNum;

	POSRES()
	{
		for (int i = 0; i < 3; i++)
		{
			Pos[i] = Vel[i] = 0.0;
		}
		RcvClkOft[0] = RcvClkOft[1] = 0.0;
		PDOP = SigmaPos = SigmaVel = 999.9;
		GPS_SatNum = BDS_SatNum = SatNum = 0;
	}
};

void CopyArray(int size, double* dest, const double* src);
int MatrixInv(int n, double a[], double b[]);//矩阵求逆
int MatrixInv_SRS(const int n, double a[]);//正定矩阵求逆
void MatrixMultiply(int m1, int n1, int m2, int n2, const double M1[], const double M2[], double M3[]);//矩阵相乘

//声明通用时,简化儒略日和GPS时之间的相互转换函数
void CommonTimeToMJDTime(const COMMONTIME* CT, MJDTIME* MJDT);
void MJDTimeToCommonTime(const MJDTIME* MJDT, COMMONTIME* CT);
void GPSTimeToMJDTime(const GPSTIME* GT, MJDTIME* MJDT);
void MJDTimeToGPSTime(const MJDTIME* MJDT, GPSTIME* GT);
void CommonTimeToGPSTime(const COMMONTIME* CT, GPSTIME* GT);
void GPSTimeToCommonTime(const GPSTIME* GT, COMMONTIME* CT);

//空间直角坐标,大地坐标的相互转换函数
void XYZToBLH(const double xyz[3], double blh[3], const double R, const double F);
void BLHToXYZ(const double BLH[3], double XYZ[3], const double R, const double F);
void BLHToNEUMat(const double Blh[], double Mat[3][3]);
void CompSatElAz(const double Xr[], const double Xs[], const double R, const double F, double* Elev, double* Azim); //卫星高度角方位角计算函数
void Comp_dEnu(const double X0[], const double Xr[], const double R, const double F, double dNeu[]);  //定位误差计算函数

// NovAtel OEM7数据解码函数：要换成eigen库
short S2(char* p);
unsigned short US2(unsigned char* p);
int I4(char* p);
unsigned int UI4(unsigned char* p);
float F4(unsigned char* p);
double D8(unsigned char* p);
int DecodeNovOem7Dat(unsigned char Buff[], int& Len, EPOCHOBSDATA* obs, GPSEPHREC geph[], GPSEPHREC beph[], POSRES* pos);
int decode_rangeb_oem7(unsigned char* Buff,  EPOCHOBSDATA* obs);
int decode_gpsephem(unsigned char* buff, GPSEPHREC* geph);
int decode_bdsephem(unsigned char* buff, GPSEPHREC* beph);
int decode_psrpos(unsigned char* buff, POSRES* pos);
unsigned int crc32(const unsigned char* buff, int len);

// 卫星位置
void SatellitePosition(GPSEPHREC geph[], GPSEPHREC beph[], EPOCHOBSDATA* obs, double UserPos[3]);
bool CompSatClkOff(const int Prn, const GNSSSys Sys, const GPSTIME* t, GPSEPHREC* GPSEph, GPSEPHREC* BDSEph, SATMIDRES* Mid);
bool CompGPSSatPVT(const int Prn, const GPSTIME* t, const GPSEPHREC* Eph, SATMIDRES* Mid);
bool CompBDSSatPVT(const int Prn, const GPSTIME* t, const GPSEPHREC* Eph, SATMIDRES* Mid);
void RotationCorrection(SATMIDRES* Mid, double UserPos[3], double Omega_e);//对卫星位置进行地球自转改正

//误差改正
void DetectOutlier(EPOCHOBSDATA* obs); 
double hopfield(double hgt, double elev);

//SPP & SPV
bool SPP(EPOCHOBSDATA* Epoch, GPSEPHREC* GPSEph, GPSEPHREC* BDSEph, POSRES* Res);
bool SPV(EPOCHOBSDATA* obs, POSRES* Res);
bool SPP_LS(EPOCHOBSDATA* obs, POSRES* Res);