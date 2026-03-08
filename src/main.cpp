#include "SPP.h"
#include "socket.h"   

int FILEMODE = 0;
int main()
{
    int i;
    unsigned char Buff[MAXRAWLEN];
    int LenR;
    int LenD = 0;
    EPOCHOBSDATA obs;
    GPSEPHREC geph[MAXGPSNUM], beph[MAXBDSNUM];
    POSRES pos;//解码得到的每个历元的定位结果
    POSRES Res;//计算得到的每个历元的定位结果
    double dEnu[3];
    double blh[3];
    
    cout << "请确定模式：\n0：文件模式         1：实时模式\n";
    cin >> FILEMODE;

    if(FILEMODE ==0)
    {
        ofstream outfile_file("output_file.txt");
        FILE* fp = nullptr;
        errno_t err = fopen_s(&fp, "oem719-202504011900-1.bin", "rb");
        if (err != 0 || fp == nullptr)
        {
            printf("Cannot open file. Error code: %d\n", err);
            return 0;
        }
        while (!feof(fp))
        {
            if ((LenR = fread(Buff + LenD, 1, MAXRAWLEN - LenD, fp)) < 1)
            {
                fclose(fp);
                return 0;
            }
            LenD += LenR;
            DecodeNovOem7Dat(Buff, LenD, &obs, geph, beph, &pos);//暂时没有使用返回值
            DetectOutlier(&obs);
            if (SPP(&obs, geph, beph, &Res)) 
            {
                SPV(&obs, &Res);
            }
            else continue;
            XYZToBLH(Res.Pos, blh, R_WGS84, F_WGS84);
            Comp_dEnu(pos.Pos, Res.Pos, R_WGS84, F_WGS84, dEnu);
            for (i = 0; i < obs.SatNum; i++)
            {
                outfile_file << fixed << setprecision(3) << obs.SatObs[i].Prn << "  " << "X= " << obs.SatPVT[i].SatPos[0] << "  " << "Y= " << obs.SatPVT[i].SatPos[1] << "  " << "Z= " << obs.SatPVT[i].SatPos[2] << "  " << fixed << setprecision(6) << "Clk= " << obs.SatPVT[i].SatClkOft << "  "
                    << fixed << setprecision(4) << "Vx= " << obs.SatPVT[i].SatVel[0] << "  " << "Vy " << obs.SatPVT[i].SatVel[1] << "  " << "Vz= " << obs.SatPVT[i].SatVel[2] << "  " << fixed << setprecision(5) << "Clkd= " << obs.SatPVT[i].SatClkSft << "  "
                    << fixed << setprecision(4) << "PIF= " << obs.ComObs[i].PIF << fixed << setprecision(3) << "  " << "Trop= " << obs.SatPVT[i].TropCorr << "  " << "E= " << obs.SatPVT[i].Elevation << endl;
            }
            outfile_file << "SPP: " << Res.Time.Week << "  " << Res.Time.SecOfWeek << "  " << fixed << setprecision(4) << "X= " << Res.Pos[0] << "  " << "Y= " << Res.Pos[1] << "  " << "Z= " << Res.Pos[2] << "  "
                << fixed << setprecision(8) << "B= " << blh[1] << "  " << " L= " << blh[0] << "  " << fixed << setprecision(3) << "H= " << blh[2] << "  " << "GPS Clk= " << Res.RcvClkOft[0] << "  " << "BDS Clk= " << Res.RcvClkOft[1] << "  "
                << "PDOP= " << Res.PDOP << "  " << "SigmaPos= " << Res.SigmaPos << "  " << "SigmaVel= " << Res.SigmaVel << "  " << fixed << setprecision(0) << "GPSSats= " << Res.GPS_SatNum << "  " << "BDSSats= " << Res.BDS_SatNum << "  " << "Sats= " << Res.SatNum << endl;

        }
        fclose(fp);
        outfile_file.close();
    }
    
    else if(FILEMODE ==1)
    {
        ofstream outfile("output_net.txt");

        SOCKET NetGps;
        if (OpenSocket(&NetGps, "47.114.134.129", 7190) == false)
        {
            printf("This ip & port was not opened.\n");
            return -1;
        }
        printf("成功连接到GNSS基站 47.114.134.129:7190\n");
        while (true) {
            Sleep(980);
            if ((LenR = recv(NetGps, (char*)Buff, 40960 - LenD, 0)) > 0)
            {
                LenD += LenR;
                DecodeNovOem7Dat(Buff, LenD, &obs, geph, beph, &pos);
                DetectOutlier(&obs);
                if (SPP(&obs, geph, beph, &Res))
                {
                    SPV(&obs, &Res);
                }
                XYZToBLH(Res.Pos, blh, R_WGS84, F_WGS84);
                Comp_dEnu(pos.Pos, Res.Pos, R_WGS84, F_WGS84, dEnu);
                /*for (i = 0; i < obs.SatNum; i++)
                {
                    cout << fixed << setprecision(3) << obs.SatObs[i].Prn << "  " << "X= " << obs.SatPVT[i].SatPos[0] << "  " << "Y= " << obs.SatPVT[i].SatPos[1] << "  " << "Z= " << obs.SatPVT[i].SatPos[2] << "  " << fixed << setprecision(6) << "Clk= " << obs.SatPVT[i].SatClkOft << "  "
                        << fixed << setprecision(4) << "Vx= " << obs.SatPVT[i].SatVel[0] << "  " << "Vy " << obs.SatPVT[i].SatVel[1] << "  " << "Vz= " << obs.SatPVT[i].SatVel[2] << "  " << fixed << setprecision(5) << "Clkd= " << obs.SatPVT[i].SatClkSft << "  "
                        << fixed << setprecision(4) << "PIF= " << obs.ComObs[i].PIF << fixed << setprecision(3) << "  " << "Trop= " << obs.SatPVT[i].TropCorr << "  " << "E= " << obs.SatPVT[i].Elevation << endl;
                }*/
               cout << "SPP: " << Res.Time.Week << "  " << Res.Time.SecOfWeek << "  " << fixed << setprecision(4) << "X= " << Res.Pos[0] << "  " << "Y= " << Res.Pos[1] << "  " << "Z= " << Res.Pos[2] << "  "
                    << fixed << setprecision(8) << "B= " << blh[1] << "  " << " L= " << blh[0] << "  " << fixed << setprecision(3) << "H= " << blh[2] << "  " << "GPS Clk= " << Res.RcvClkOft[0] << "  " << "BDS Clk= " << Res.RcvClkOft[1] << "  "
                    <<"Vx= "<<Res.Vel[0]<<"  " << "Vy= " << Res.Vel[1] << "  " << "Vz= " << Res.Vel[2] << "  "<<"RcvSft= "<<Res.RcvClkSft<<"  "
                    << "PDOP= " << Res.PDOP << "  " << "SigmaPos= " << Res.SigmaPos << "  " <<"SigmaVel= " << Res.SigmaVel << "  " << fixed << setprecision(0) << "GPSSats= " << Res.GPS_SatNum << "  " << "BDSSats= " << Res.BDS_SatNum << "  " << "Sats= " << Res.SatNum << "  "
                    << fixed << setprecision(4) <<"dE= "<<dEnu[0] << "  " << "dN= " << dEnu[1] << "  " << "dU= " << dEnu[2] << "  " << pos.Pos[0] << "  " << pos.Pos[1] << "  " << pos.Pos[2] << "  " <<  endl;
               outfile << "SPP: " << Res.Time.Week << "  " << Res.Time.SecOfWeek << "  " 
                   << fixed << setprecision(3)  << "GPS Clk= " << Res.RcvClkOft[0] << "  " << "BDS Clk= " << Res.RcvClkOft[1] << "  " << "RcvSft= " << Res.RcvClkSft << "  "
                   << "PDOP= " << Res.PDOP << "  " << "SigmaPos= " << Res.SigmaPos << "  " << "SigmaVel= " << Res.SigmaVel << "  " 
                   << fixed << setprecision(0) << "GPSSats= " << Res.GPS_SatNum << "  " << "BDSSats= " << Res.BDS_SatNum << "  " << "Sats= " << Res.SatNum << "  "
                   << fixed << setprecision(4) << "dE= " << dEnu[0] << "  " << "dN= " << dEnu[1] << "  " << "dU= " << dEnu[2] << endl;
            }
        }
        outfile.close();
    }

    return 0;
}

