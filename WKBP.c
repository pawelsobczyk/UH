#include <math.h>
#include "UH.h"

//int zm;
//=============================================== M O R Ś W I N ====================================================
int ReadOperatingMorswinData(int res, char *bufor)
{
	static TJoyCalib JoyCalib={0};
	unsigned char i;
	short zmienna_pomocnicza;

	if ((bufor[0]==2) && (bufor[1]==5)) {	//gdy ramka do Ukwiała rzeczywistego
		
		//if (res==91) ArrayToStruct(0x21,bufor);	//wersja protokołu "bez zer" dla CMT			
		if (res==sizeof(TWKBPdoUH)) WKBPdoUH=*((TWKBPdoUH*)bufor);		//wersja normalna
		else return 0;
		//irom++;	//ilosc ramek odebranych dot. morswina
		
		UHdoWKBP.LiczRam=WKBPdoUH.LiczRam;
		
		//bajty zwrotne dla potrzeb CTM
		UHdoWKBP.Testy=WKBPdoUH.Testy;
		UHdoWKBP.WlaczanieHydro=WKBPdoUH.WlaczanieHydro;
		UHdoWKBP.Kamera1=WKBPdoUH.Kamera1;
		UHdoWKBP.Kamera2=WKBPdoUH.Kamera2;
		memcpy((void*)&UHdoWKBP.StopienOsw[0],(const void*)&WKBPdoUH.StopienOsw[0],4*sizeof(unsigned char));
		memcpy((void*)&UHdoWKBP.ManipulZwalniak[0],(const void*)&WKBPdoUH.ManipulZwalniak[0],2*sizeof(unsigned char));
		UHdoWKBP.SterRoboczych=WKBPdoUH.SterRoboczych;
		memcpy((void*)&UHdoWKBP.Joysticki[0],(const void*)&WKBPdoUH.Joysticki[0],6*sizeof(short));
		UHdoWKBP.ZadKurs=WKBPdoUH.ZadKurs;
		UHdoWKBP.ZadPrzeg=WKBPdoUH.ZadPrzeg;
		UHdoWKBP.ZadGleb=WKBPdoUH.ZadGleb;
		UHdoWKBP.ZadEcho=WKBPdoUH.ZadEcho;
		UHdoWKBP.ZadPedPion=WKBPdoUH.ZadPedPion;
		UHdoWKBP.ZadPredkosc=WKBPdoUH.ZadPredkosc;
		UHdoWKBP.ZadPozGleb=WKBPdoUH.ZadPozGleb;
		UHdoWKBP.ZadPozDl=WKBPdoUH.ZadPozDl;		
		UHdoWKBP.ZadPozSzer=WKBPdoUH.ZadPozSzer;		
		UHdoWKBP.Precyzja=WKBPdoUH.Precyzja;
		UHdoWKBP.JoystickTryb=WKBPdoUH.JoystickTryb;
		UHdoWKBP.Regulatory=WKBPdoUH.Regulatory;
		UHdoWKBP.AutoMisja=WKBPdoUH.AutoMisja;
		UHdoWKBP.SterAwaryjne=WKBPdoUH.SterAwaryjne;
		UHdoWKBP.SaabSter=WKBPdoUH.SaabSter;
		UHdoWKBP.ZadSilaKabl=WKBPdoUH.ZadSilaKabl;
		UHdoWKBP.ZadPredKabl=WKBPdoUH.ZadPredKabl;
		UHdoWKBP.ZadDlKabl=WKBPdoUH.ZadDlKabl;
		
		if (!IfOpInGK.take_control) 
		{
			IfOpInGK.cmd[0]=WKBPdoUH.Joysticki[0];  // joysticki
			IfOpInGK.cmd[1]=WKBPdoUH.Joysticki[1];
			IfOpInGK.cmd[2]=WKBPdoUH.Joysticki[2];
			IfOpInGK.cmd[3]=WKBPdoUH.Joysticki[3];
			IfOpInGK.cmd[4]=WKBPdoUH.Joysticki[4];
			IfOpInGK.cmd[5]=WKBPdoUH.Joysticki[5];
			
			IfOpInGK.tryb=WKBPdoUH.Regulatory;   // reg gleb, reg odlegl, reg kursu, reg pochylenia, reg kaskadowy;
			IfOpInGK.rkurs=WKBPdoUH.ZadKurs;   // nastawy
			IfOpInGK.rpoch=WKBPdoUH.ZadPrzeg;
			IfOpInGK.rgleb=WKBPdoUH.ZadGleb;
			IfOpInGK.rodl=WKBPdoUH.ZadEcho;
			IfOpInGK.rpion=WKBPdoUH.ZadPedPion;
			IfOpInGK.rpredk=WKBPdoUH.ZadPredkosc;
			
			//unsigned char balast;
			IfOpInGK.bieg=WKBPdoUH.Precyzja;  // precyzja sterowania
			
			//unsigned char latlong_on;
			IfOpInGK.Lat=WKBPdoUH.ZadPozSzer,
			IfOpInGK.Long=WKBPdoUH.ZadPozDl, 
			IfOpInGK.Z=WKBPdoUH.ZadPozGleb;
			//__attribute__ ((aligned(8))) double X_NED,Y_NED,Z_NED;    // NED
			IfOpInGK.misja=WKBPdoUH.AutoMisja; // sterowanie dojsciem do celu
			IfOpInGK.wlatlong=1; // DLA KOR na stale lat-long
			//unsigned char wlatlong; // nastawa w latlong
			IfOpInGK.SterAwaryjne=WKBPdoUH.SterAwaryjne;	
	
			//printf("SPPdoGl.Joystick1[0] = %d, SPPdoGl.Joystick1[1] = %d\n", SPPdoGl.Joystick1[0], SPPdoGl.Joystick1[1]);
		}
		return 1;	
	}	
	return 0;
}


int SendStateMorswinData(int res, int handle, struct sockaddr_in WKBPAddr)
{
	unsigned char kons_buf_out[400];
	
	if (res==91) {
		//StructToArray(0x22,kons_buf_out);
		//printf("ATCStatus %d\n",UHdoWKBP.ATCStatus);
		return sendto(handle,(const char*)&kons_buf_out,272,0,(struct sockaddr*)&WKBPAddr,sizeof(struct sockaddr_in));	
	}	
	
	if (res==sizeof(TWKBPdoUH)) {
		//printf("Wysylam %d bajtow do WKBP\n",sizeof(TUHdoWKBP));
		
		return sendto(handle,(const char*)&UHdoWKBP,sizeof(TUHdoWKBP),0,(struct sockaddr*)&WKBPAddr,sizeof(struct sockaddr_in));	
	}
}

/*	tu przechowuję sobie starą procedurkę do odbioru danych z komputera Ukwiała 
int ReceiveData(int handle, unsigned char ilosc)
{
	int i, result;
	//unsigned long cnt;
	result=read(handle,recdata,ilosc);

	if (result<ilosc) return -1;
	if (recdata[0]!=0x02 || recdata[22]!=0x03) return -2;

	int suma_kon=0;
	for (i=1;i<ilosc-2;i++) suma_kon^=recdata[i];
	if (suma_kon!=recdata[ilosc-2]) return -3;

	return result;
	//-1 - nie odebrano oczekiwanej ilosci bajtˇw
	//-2 - blad bajtu poczatku lub konca ramki
	//-3 - blad sumy kontrolnej
}*/
