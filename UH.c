#include "UH.h"

/*
 opis definicji zmiennych typu całkowitego ze znakiem
 long 		- 4 bajty
 long long 	- 8 bajtów
 double long - 12 bajtów
*/ 

#define bit_set(p,m) ((p) |= (1<<m))
#define bit_clr(p,m) ((p) &= ~(1<<m))

TWKBPdoUH WKBPdoUH;
TUHdoWKBP UHdoWKBP;

int Uk_handle, addrlen;
struct sockaddr_in WKBPAddr, addr_in;
int res;
unsigned char kons_buf_in[500], bufor[100];
int il_zn;
int nr_fal;		//numer falownika
int kam1;

unsigned char transdata[10], recdata[40], il_zna, suma_kon, komputer_odp, autofocus, kons_odp=0;

unsigned char wynik, test_ped_mar, test_ped_man, test_obr_kam, test_obr_son;

typedef struct {
	int uchwyt;
	char buf[100];
	char s_prad[10];
	unsigned char il_zn;
	float prad;
	int time_out;
	unsigned char koniec_ramki;
	unsigned char bcc;	//suma kontrolna
	unsigned char ramka_kompletna;
	char s_pred[10];
	float pred;
	unsigned short tripy;
} TFalownik;

TFalownik falownik[6];

int obroty_zadane[6], obroty_mierzone[6];

TPomiarGK PomiarGK={5,0,50}, PomiarGK_sym={5,0,50};
TifaceOpInGK IfOpInGK={5,0,31};
TOdSPP_Piotrka OdSPP_Piotrka;
int Piotr_handle;
struct sockaddr_in PiotrAddr;


void SterowanieObrazemKamery(int handle)
{
	static unsigned char kamera1_ms=0, kamera2_ms=0;		//czas na wykonanie jednego ruchu kamery
	
	//printf("Sterowanie kamerą: %d\n",WKBPdoUH.Kamera1);
	
	if (WKBPdoUH.Kamera1&0x02) {write(handle,"\x81\x01\x04\x08\x02\xFF",6); kamera1_ms=1;}	//focus daleko
	if (WKBPdoUH.Kamera1&0x01) {write(handle,"\x81\x01\x04\x08\x03\xFF",6); kamera1_ms=1;}	//focus blisko
	
	/*
	if ((PGdoMo.kamera1&0x01) || (WKBPdoMo.Kamera1&0x02)) {write(handle,"\x81\x01\x04\x0B\x02\xFF",6); kamera1_ms=1;}	//Iris Up
	if ((PGdoMo.kamera1&0x02) || (WKBPdoMo.Kamera1&0x01)) {write(handle,"\x81\x01\x04\x0B\x03\xFF",6); kamera1_ms=1;}	//Iris Down
	*/
	
	if (WKBPdoUH.Kamera1&0x04) {write(handle,"\x81\x01\x04\x07\x02\xFF",6); kamera2_ms=1;}	//zoom daleko
	if (WKBPdoUH.Kamera1&0x08) {write(handle,"\x81\x01\x04\x07\x03\xFF",6); kamera2_ms=1;}	//zoom blisko
	
	if (kamera1_ms>0) kamera1_ms++; 
	if (kamera1_ms>2) {write(handle,"\x81\x01\x04\x08\x00\xFF",6); kamera1_ms=0;} //focus stop
	
	if (kamera2_ms>0) kamera2_ms++; 
	if (kamera2_ms>2) {write(handle,"\x81\x01\x04\x07\x00\xFF",6); kamera2_ms=0;} //zoom stop
}


int SendData(int handle, unsigned char tablica[30], unsigned char ilosc)
{
	int i, result;
	
	tablica[ilosc-2]=0;
	for (i=1;i<ilosc-2;i++) tablica[ilosc-2]^=tablica[i];		//liczenie sumy kontrolnej
	result=write(handle,tablica,ilosc);
	
	return result;
};


void *timer_18ms(void)	//procedura się wykonuje co 50 ms
{
	int i;
	static unsigned char przeplot=0;
	
	if (przeplot==5) przeplot=0;
	
	//testy
	for (i=0;i<6;i++) obroty_mierzone[i]=(int)falownik[i].pred;	//zmierzone obroty falownika
	wynik=TestPedMar(&test_ped_mar, &obroty_zadane, obroty_mierzone);
	
	if ((wynik&0x03)==(1<<0) || (wynik&0x0C)==(1<<2) || (wynik&0x30)==(1<<4) || (wynik&0xC0)==(1<<6)) //test nieudany
	{bit_set(UHdoWKBP.TestOdp,2);  bit_clr(UHdoWKBP.TestOdp,1);}
	if ((wynik&0x03)==(2<<0) && (wynik&0x0C)==(2<<2) && (wynik&0x30)==(2<<4) && (wynik&0xC0)==(2<<6)) //test udany
	{bit_set(UHdoWKBP.TestOdp,1); bit_clr(UHdoWKBP.TestOdp,2);}
	
	if (test_ped_mar || test_ped_man || test_obr_kam || test_obr_son) bit_set(UHdoWKBP.TestOdp,3); else bit_clr(UHdoWKBP.TestOdp,3);	//zaznaczenie, że trwa jakiś test

	//printf("Prędkość zadana podczas testu = %d\n",obroty_zadane[0]);
	if (test_ped_mar>0) for (i=0;i<6;i++) WKBPdoUH.Joysticki[i]=obroty_zadane[i]*10;
	//koniec sekcji testów
	
	//printf("Przeplot = %d\n",przeplot);
	if (przeplot==0) 
	{
		if (OdSPP_Piotrka.ObrPedZad[0]!=0) ZalPrzetw(falownik[0].uchwyt); else WylPrzetw(falownik[0]);
		if (OdSPP_Piotrka.ObrPedZad[1]!=0) ZalPrzetw(falownik[1].uchwyt); else WylPrzetw(falownik[1]);
		if (OdSPP_Piotrka.ObrPedZad[2]!=0) ZalPrzetw(falownik[2].uchwyt); else WylPrzetw(falownik[2]);
		if (OdSPP_Piotrka.ObrPedZad[3]!=0) ZalPrzetw(falownik[3].uchwyt); else WylPrzetw(falownik[3]);
		if (OdSPP_Piotrka.ObrPedZad[4]!=0) ZalPrzetw(falownik[4].uchwyt); else WylPrzetw(falownik[4]);
		if (OdSPP_Piotrka.ObrPedZad[5]!=0) ZalPrzetw(falownik[5].uchwyt); else WylPrzetw(falownik[5]);
	}
	else if (przeplot==1) 
	{
		ReadTag(falownik[0].uchwyt,67);			//czytaj prąd [A]
		ReadTag(falownik[1].uchwyt,67);
		ReadTag(falownik[2].uchwyt,67);
		ReadTag(falownik[3].uchwyt,67);
		ReadTag(falownik[4].uchwyt,67);
		ReadTag(falownik[5].uchwyt,67);
	}
	else if (przeplot==2) //zadaj prędkości w %
	{	
		WriteTag(falownik[0].uchwyt,269,(float)-OdSPP_Piotrka.ObrPedZad[0]/100);
		WriteTag(falownik[1].uchwyt,269,(float)-OdSPP_Piotrka.ObrPedZad[1]/100);
		WriteTag(falownik[2].uchwyt,269,(float)OdSPP_Piotrka.ObrPedZad[2]/100);
		WriteTag(falownik[3].uchwyt,269,(float)OdSPP_Piotrka.ObrPedZad[3]/100);
		WriteTag(falownik[4].uchwyt,269,(float)-OdSPP_Piotrka.ObrPedZad[4]/100);
		WriteTag(falownik[5].uchwyt,269,(float)-OdSPP_Piotrka.ObrPedZad[5]/100);
	}
	else if (przeplot==3) 
	{
		ReadTag(falownik[0].uchwyt,45);			//czytaj obroty [%]
		ReadTag(falownik[1].uchwyt,45);	
		ReadTag(falownik[2].uchwyt,45);	
		ReadTag(falownik[3].uchwyt,45);	
		ReadTag(falownik[4].uchwyt,45);	
		ReadTag(falownik[5].uchwyt,45);	
	}
	else if (przeplot==4) 
	{
		ReadTag(falownik[0].uchwyt,4);			//czytaj tripy czyli informacje o usterkach
		ReadTag(falownik[1].uchwyt,4);	
		ReadTag(falownik[2].uchwyt,4);	
		ReadTag(falownik[3].uchwyt,4);	
		ReadTag(falownik[4].uchwyt,4);	
		ReadTag(falownik[5].uchwyt,4);	
		
		//wysyłanie danych do programu Piotrka
		PomiarGK.kurs=UHdoWKBP.Kurs;
		PomiarGK.pochylenie=UHdoWKBP.Przeg;
		PomiarGK.przechylenie=UHdoWKBP.Przechyl;
		PomiarGK.glebokosc=UHdoWKBP.Glebokosc;
		PomiarGK.od_od_dna=UHdoWKBP.Echo;	
		//PomiarGK.LLD[2]=GldoSPP.Glebokosc;
		for (i=0; i<6; i++) PomiarGK.RPM_meas[i]=UHdoWKBP.ObrPedMierz[i];  // obroty mierzone typ float i %
		
		sendto(Piotr_handle,(const char*)&PomiarGK,sizeof(PomiarGK),0,(struct sockaddr*)&PiotrAddr,sizeof(struct sockaddr_in));
	}
	
	przeplot++;
	
	for (i=0;i<6;i++) falownik[i].time_out++;
	
	SterowanieObrazemKamery(kam1);
	
	if (kons_odp<50) kons_odp++;
}

int main (void *cookie) 
{
	int i;
	
	int WKBP_handle=InitUnicastHandle("192.168.101.31",5000,&WKBPAddr);
	Uk_handle=port_init("/dev/ttyS4",B19200);
	kam1 = port_init("/dev/ttyS5", B9600);
	falownik[0].uchwyt=falow_port_init("/dev/ttyS10",B19200);
	falownik[1].uchwyt=falow_port_init("/dev/ttyS11",B19200);
	falownik[2].uchwyt=falow_port_init("/dev/ttyS6",B19200);
	falownik[3].uchwyt=falow_port_init("/dev/ttyS7",B19200);
	falownik[4].uchwyt=falow_port_init("/dev/ttyS8",B19200);
	falownik[5].uchwyt=falow_port_init("/dev/ttyS9",B19200);
	
	//Piotr_handle=InitUnicastHandle2("192.168.101.103",5303,5302,&PiotrAddr);		//dane z programem SPP Piotrka
	Piotr_handle=InitUnicastHandle2("127.0.0.1",5303,5302,&PiotrAddr);		//dane z programem SPP Piotrka
	
	Common_timer_init(30,timer_18ms);
	
	printf("Rozpoczynam działanie programu\n");
	
	transdata[0]=0x02;	//poczatek ramki
	transdata[6]=0x03;	//koniec ramki

	UHdoWKBP.Start=2;
	UHdoWKBP.Pojazd=5;
	UHdoWKBP.Ramka=0x22;
	UHdoWKBP.End=3;

	while (1) 
	{
		usleep(1000);		//1 ms, zeby się procek nie zajechał 
		//UNICAST
		
		addrlen=sizeof(struct sockaddr_in);
		res=recvfrom(Piotr_handle,kons_buf_in,300,MSG_DONTWAIT | MSG_NOSIGNAL,(struct sockaddr *)&addr_in,&addrlen);
		//if (res>0) printf("Dostałem od Piotrka %d znaków\n",res);
		if (sizeof(TOdSPP_Piotrka)==res) 
		{
			OdSPP_Piotrka=*((TOdSPP_Piotrka*)kons_buf_in);
			
			if (kons_odp>40) //jeśli nie ma komunikacji z WKBP
			{
				OdSPP_Piotrka.ObrPedZad[0]=0;	//Pionowy
				OdSPP_Piotrka.ObrPedZad[1]=0;	//LG
				OdSPP_Piotrka.ObrPedZad[2]=0;
				OdSPP_Piotrka.ObrPedZad[3]=0;
				OdSPP_Piotrka.ObrPedZad[4]=0;	//PD
				OdSPP_Piotrka.ObrPedZad[5]=0;
			}
			//printf("PedObrZad[0] = %d, PedObrZad[1] = %d, PedObrZad[2] = %d, PedObrZad[3] = %d, PedObrZad[4] = %d,\n",
			//OdSPP_Piotrka.ObrPedZad[0],OdSPP_Piotrka.ObrPedZad[1],OdSPP_Piotrka.ObrPedZad[2],OdSPP_Piotrka.ObrPedZad[3],OdSPP_Piotrka.ObrPedZad[4]);
		}

		addrlen=sizeof(struct sockaddr_in);
		if ((res=recvfrom(WKBP_handle,kons_buf_in,1000,MSG_DONTWAIT | MSG_NOSIGNAL,(struct sockaddr *)&addr_in,&addrlen))>0) 
		{
			//printf("Dostałem od Dorotki %d\n",res);
			if (ReadOperatingMorswinData(res,kons_buf_in)) 
			{
				//składanie ramki do komputera Ukwiała
				transdata[2]=((WKBPdoUH.Kamera1>>6)&0x01)+((WKBPdoUH.Kamera1>>4)&0x02)+((WKBPdoUH.SterRoboczych<<4)&0x30);
				//printf ("%d\n",transdata[2]);
				
				//sterowanie manipulatorem
				transdata[3]=WKBPdoUH.ManipulZwalniak[0];
				transdata[3]&=~0x0C;		//zamiana bitów cofanie/wysuwanie manipulatora
				transdata[3]|=((WKBPdoUH.ManipulZwalniak[0]>>1)&0x04)+((WKBPdoUH.ManipulZwalniak[0]<<1)&0x08);
				
				//stopień świecenia lamp
				transdata[4]=WKBPdoUH.StopienOsw[0]+(WKBPdoUH.StopienOsw[1]<<3);
				
				//sterowania autofacusem kamery
				if (autofocus!=(WKBPdoUH.Kamera2&(1<<3))) 
				{
					autofocus=WKBPdoUH.Kamera2&(1<<3);
					if (autofocus>0) write(kam1,"\x81\x01\x04\x38\x02\xFF",6);	//autofocus ON
					else write(kam1,"\x81\x01\x04\x38\x03\xFF",6);	//autofocus kamery OFF	
				}
				SendData(Uk_handle,transdata,7);		//wysyłanie do komputera Ukwiała
				komputer_odp++;
				if (komputer_odp>10) UHdoWKBP.DaneDodatkowe|=(1<<3);
				//koniec składania ramki do Ukwiała
				
				//printf("Joy[0] = %d, Joy[1] = %d, Joy[2] = %d, Joy[3] = %d, Joy[4] = %d,\n",
				//WKBPdoUH.Joysticki[0],WKBPdoUH.Joysticki[1],WKBPdoUH.Joysticki[2],WKBPdoUH.Joysticki[3],WKBPdoUH.Joysticki[4],WKBPdoUH.Joysticki[5]);
			
				//sterowanie testami
				if (WKBPdoUH.Testy&0x01) {test_ped_mar=1; bit_clr(UHdoWKBP.TestOdp,0); bit_clr(UHdoWKBP.TestOdp,1); bit_clr(UHdoWKBP.TestOdp,2);}
				//if (WKBPdoUH.Testy&0x10) {test_ped_man=1; bit_clr(ModoWKBP.TestOdp,0); bit_clr(ModoWKBP.TestOdp,1); bit_clr(ModoWKBP.TestOdp,2);}
				//if (WKBPdoMo.Testy&0x20) {test_obr_kam=1; bit_clr(ModoWKBP.TestOdp,0); bit_clr(ModoWKBP.TestOdp,1); bit_clr(ModoWKBP.TestOdp,2);}
				//if (WKBPdoMo.Testy&0x40) {test_obr_son=1; bit_clr(ModoWKBP.TestOdp,0); bit_clr(ModoWKBP.TestOdp,1); bit_clr(ModoWKBP.TestOdp,2);}
				if (WKBPdoUH.Testy&0x80) {test_ped_mar=0; test_ped_man=0; test_obr_kam=0; test_obr_son=0; bit_set(UHdoWKBP.TestOdp,0);}


				//składanie ramki z odpowiedzią do konsoli operatora
				UHdoWKBP.Niespr[1]=0;	//braki komunikacji z falownikami
				for (i=0;i<6;i++) 
					if (falownik[i].time_out>10) UHdoWKBP.Niespr[1]|=(1<<i); 
					
				UHdoWKBP.Niespr[2]=0;	//usterki pędników
				for (i=0;i<6;i++) {
					if (falownik[i].tripy>0) UHdoWKBP.Niespr[2]|=(1<<i);	
					if (falownik[i].tripy==0x0001) UHdoWKBP.Niespr[3]|=(1<<4);	//ustawienie informacji o błędzie przetwornicy (OVERVOLTS)	
				}
				UHdoWKBP.tripy_1_2[0]=falownik[0].tripy;
				UHdoWKBP.tripy_1_2[1]=falownik[1].tripy;
				UHdoWKBP.tripy_3_4[0]=falownik[2].tripy;
				UHdoWKBP.tripy_3_4[1]=falownik[3].tripy;
				UHdoWKBP.tripy_5_6[0]=falownik[4].tripy;
				UHdoWKBP.tripy_5_6[1]=falownik[5].tripy;
				
				for (i=0;i<6;i++) UHdoWKBP.ObrPedZad[i]=OdSPP_Piotrka.ObrPedZad[i];
				
				SendStateMorswinData(res,WKBP_handle,addr_in);	//odesłanie ramki do WKBP
				
				//wysłanie ramki do programu Piotrka
				sendto(Piotr_handle,(const char*)&IfOpInGK,sizeof(TifaceOpInGK),0,(struct sockaddr*)&PiotrAddr,sizeof(struct sockaddr_in));		
				
				kons_odp=0;		//zaznaczenie, że konsola odpowiada
			}
			else tcflush(WKBP_handle,TCIFLUSH);
		}
		
		
		//czytanie danych z komputera Ukwiała
		il_zn=read(Uk_handle,bufor,23);
		for (i=0; i<il_zn; i++) 
		{
			if (il_zna==0 && bufor[i]==0x02) 
			{
				recdata[0]=bufor[i];
				il_zna++;
			}
			else if (il_zna>0 && il_zna<30)	recdata[il_zna++]=bufor[i];
		}
		//if (il_zn>0) printf("Dostałem od Ukwiała %d znaków\n",il_zn);
		
		if (il_zna>=23)
		{
			if (recdata[22]==0x03) //printf("Otrzymałem prawidłową ramkę od komputera Ukwiała\n");
			{
				suma_kon=0;
				for (i=1;i<21;i++) suma_kon^=recdata[i];
				if (suma_kon==recdata[21]) 
				{		
					//printf("recdata[2] %02X, UHdoWKBP.ModulBojowy %02X\n",recdata[2],UHdoWKBP.ModulBojowy);
					
					//moduł bojowy
					UHdoWKBP.ModulBojowy=recdata[2]&0x1F;
					if (recdata[2]&0x20) UHdoWKBP.ModulBojowy|=0x80;
					if (recdata[20]&(1<<7)) UHdoWKBP.ModulBojowy|=0x40;
					
					//wartosci pomierzone
					UHdoWKBP.SkretyKabl=recdata[3];  //ilosc skretow kabloliny
					UHdoWKBP.Echo=(float)((recdata[4]<<8)+recdata[5])/100;   //echosonda
					UHdoWKBP.Glebokosc=(float)((recdata[6]<<8)+recdata[7])/100;	//glebokosc
					
					/*	tak miałem do tej pory
						UHdoWKBP.PolObrKamGD=recdata[8]-127;    //polozenie obracarki kamery; wartość bez znaku zamieniam na ze znakiem
						UHdoWKBP.PolObrKamGD=(char)((float)UHdoWKBP.PolObrKamGD*90/127);
					*/
					/*
						sr_kam=127;
						txt_kam=(sr_kam-r_polk);		
						txt_kam=(int)((txt_kam*180.0)/255.0);
						if ((txt_kam)<-99) txt_kam=-99;
					*/
					UHdoWKBP.PolObrKamGD=recdata[8]-127;    //polozenie obracarki kamery; wartość bez znaku zamieniam na ze znakiem
					UHdoWKBP.PolObrKamGD=(int)((float)UHdoWKBP.PolObrKamGD*180/255);
					if (UHdoWKBP.PolObrKamGD<-99) UHdoWKBP.PolObrKamGD=99;
					
					/*
					 * 	wyt_0=196;  wyt_pion=86;
					if (r_polw>=wyt_0) txt_wyt=(int)(((r_polw-wyt_0)* 40.0)/(254.0-wyt_0)); else
					if ((r_polw<wyt_0) && (r_polw>=wyt_pion)) txt_wyt=(int)((-(wyt_0-r_polw)* 90.0)/(wyt_0-wyt_pion)); else
					txt_wyt=(int)(-(-((r_polw* 90.0)/88.0)+180));
					*/
					if (recdata[9]>=196) UHdoWKBP.PolObrWytyku=(int)(((float)recdata[9]-196)*40/(254-196));		//polozenie obracarki wytyku
					else if ((recdata[9]<196) && (recdata[9]>=86)) UHdoWKBP.PolObrWytyku=(int)((-(196-recdata[9])*90)/(196-86));
					else UHdoWKBP.PolObrWytyku=(int)(-(-((recdata[9]*90)/88)+180));
					
					
					UHdoWKBP.Przeg=(float)(((char)recdata[10]<<8)+recdata[11])/10;		//przeglebienie
					UHdoWKBP.Przechyl=(float)(((char)recdata[12]<<8)+recdata[13])/10;	//przechylenie
					UHdoWKBP.Kurs=(float)((recdata[14]<<8)+recdata[15])/10;		//kurs
					UHdoWKBP.PodcKadl=recdata[16];		//podcisnienie LB
					UHdoWKBP.PodcMBojowy=recdata[17];		//podcisnienie PB
					UHdoWKBP.PolWindy=recdata[18]&~(1<<7);		//ilosc obrotow windy
					if (recdata[18]&(1<<7)) UHdoWKBP.PolWindy*=-1;		//najstarszy bit jest znakiem
					
					//informacje o alarmach
					if (recdata[2]&0x40) UHdoWKBP.PodcKadl|=0x80; 		//alarm woda LB
					if (recdata[2]&0x80) UHdoWKBP.PodcMBojowy|=0x80;	//alarm mowa PB

					//niesprawności
					UHdoWKBP.Niespr[0]=recdata[19];
					UHdoWKBP.Niespr[3]=recdata[20]&~(1<<7);	//wyrzucam ostatni bit bo on nie sygnalzuje błędu tylko zaparkowanie manipulatora
					
					for (i=0;i<6;i++) {
						UHdoWKBP.PradyPed[i]=falownik[i].prad;
						UHdoWKBP.ObrPedMierz[i]=(short)falownik[i].pred;	//float to short
					}
					
					komputer_odp=0;		//zaznaczenie, że komputer Ukwiała odpowiada
					UHdoWKBP.DaneDodatkowe&=~(1<<3);
				}
			}
			il_zna=0;
		}
		
		//czytanie ramek z falowników
		for (nr_fal=0;nr_fal<6;nr_fal++) 
		{
			il_zn=read(falownik[nr_fal].uchwyt,bufor,100);
			//if (il_zn>0) printf("Dostałem %d znaków od falownika: %d\n",il_zn,nr_fal);
			
			for (i=0; i<il_zn; i++) 
			{
				if (falownik[nr_fal].il_zn==0 && bufor[i]==0x02) 
				{
					falownik[nr_fal].buf[0]=bufor[i];
					falownik[nr_fal].il_zn++;
				}
				else if (falownik[nr_fal].il_zn>0 && falownik[nr_fal].il_zn<100)
					falownik[nr_fal].buf[falownik[nr_fal].il_zn++]=bufor[i];
					
					//detekcja znakow konca ramki
				if (falownik[nr_fal].buf[falownik[nr_fal].il_zn-1]==0x03) 
					falownik[nr_fal].koniec_ramki=1;
				else if (falownik[nr_fal].koniec_ramki==1)
				{
					falownik[nr_fal].ramka_kompletna=1;
					break;
				}
			}
			
			if (falownik[nr_fal].ramka_kompletna==1)
			{
				falownik[nr_fal].bcc=0;
				for (i=1;i<falownik[nr_fal].il_zn-1;i++) falownik[nr_fal].bcc^=falownik[nr_fal].buf[i];		//liczenie sumy kontrolnej
				if (falownik[nr_fal].bcc==falownik[nr_fal].buf[falownik[nr_fal].il_zn-1])  
				{
					//printf("Suma kontrolna prawidłowa\n");
					//falownik[nr_fal].buf[falownik[nr_fal].il_zn-2]=0;
					//printf("Dostałem %d znaków od falownika: %s\n",falownik[nr_fal].il_zn,falownik[nr_fal].buf+1);
					
					if (falownik[nr_fal].buf[1]=='1' && falownik[nr_fal].buf[2]=='v') //pomiar prądu [A]
					{		
						memcpy(&falownik[nr_fal].s_prad,&falownik[nr_fal].buf[3],falownik[nr_fal].il_zn-5);
						falownik[nr_fal].s_prad[falownik[nr_fal].il_zn-5]=0;
						//printf("prąd falownika %s A\n",falownik[0].s_prad); 
						sscanf(falownik[nr_fal].s_prad,"%f",&falownik[nr_fal].prad);
						//printf("Prad %04.1f A\n",falownik[0].prad); 
					}
					else if (falownik[nr_fal].buf[1]=='1' && falownik[nr_fal].buf[2]=='9')  //pomiar prędkości w %
					{
						memcpy(&falownik[nr_fal].s_pred,&falownik[nr_fal].buf[3],falownik[nr_fal].il_zn-5);
						falownik[nr_fal].s_pred[falownik[nr_fal].il_zn-5]=0;
						//printf("prąd falownika %s A\n",falownik[nr_fal].s_prad); 
						sscanf(falownik[nr_fal].s_pred,"%f",&falownik[nr_fal].pred);
						//printf("Predkosc %04.1f proc. od falownika %d\n",falownik[nr_fal].pred,nr_fal); 
						//for (i=0;i<falownik[nr_fal].il_zn;i++) printf("%02X, ",falownik[nr_fal].buf[i]); printf("\n");
					}
					else if (falownik[nr_fal].buf[1]=='0' && falownik[nr_fal].buf[2]=='4')	//tripy czyli usterki pracy falownika
					{
						//falownik[nr_fal].buf[4]='0'; falownik[nr_fal].buf[5]='1'; falownik[nr_fal].buf[6]='1'; falownik[nr_fal].buf[7]='0'; //some dummy values
						memcpy(&falownik[nr_fal].s_pred,&falownik[nr_fal].buf[4],4);
						sscanf(falownik[nr_fal].s_pred,"%04X",&falownik[nr_fal].tripy);
						//printf("0x%04X\n",falownik[nr_fal].tripy);
						//for (i=0;i<falownik[nr_fal].il_zn;i++) printf("%02X, ",falownik[nr_fal].buf[i]); printf("\n");
						
					}
					
					falownik[nr_fal].time_out=0;
				} 
				else printf("Suma kontrolna nieprawidłowa\n");
				
				falownik[nr_fal].koniec_ramki=0;
				falownik[nr_fal].ramka_kompletna=0;
				falownik[nr_fal].il_zn=0;
			}
		}
	}	//end while
	
	return 0;
}
