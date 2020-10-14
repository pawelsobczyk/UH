#include <sys/time.h>
#include <stdio.h>


int TestPedMar(unsigned char *czy_test, int (*obr_zad)[6], int obr_mierz[6]) {

	static unsigned char wynik=0;
	static unsigned char etap=0;
	static time_t sek1;
	const unsigned char skok=6;	//czas trwania jednego etapu testu [s]
	const unsigned short granica_dolna=25;
	const unsigned short granica_gorna=35;
	const unsigned short wart_zad=30;	//wartość w procentach
	unsigned char wynik_out=0, i=0;;
	
	if ((*czy_test)==0) {
		etap=0;
	}
	
	//lewy przedni
	if ((*czy_test)==1 && (etap==0))
	{
		sek1=time(NULL);	
		for (i=0;i<6;i++) (*obr_zad)[i]=wart_zad;		
		etap=1;
		*czy_test=1;
		wynik=0;	//na początku testu zerowanie wyniku;
	}
	if ((etap==1) && (time(NULL)-sek1>=skok*etap)) 
	{
		//printf("wynik = %d\n",obr_mierz[0]);
		if ((abs(obr_mierz[0])<granica_dolna) || (abs(obr_mierz[0])>granica_gorna)) wynik|=(1<<0);
		if ((abs(obr_mierz[1])<granica_dolna) || (abs(obr_mierz[1])>granica_gorna)) wynik|=(1<<1);
		if ((abs(obr_mierz[2])<granica_dolna) || (abs(obr_mierz[2])>granica_gorna)) wynik|=(1<<2);
		if ((abs(obr_mierz[3])<granica_dolna) || (abs(obr_mierz[3])>granica_gorna)) wynik|=(1<<3);
		if ((abs(obr_mierz[4])<granica_dolna) || (abs(obr_mierz[4])>granica_gorna)) wynik|=(1<<4);
		if ((abs(obr_mierz[5])<granica_dolna) || (abs(obr_mierz[5])>granica_gorna)) wynik|=(1<<5);
		
		for (i=0;i<6;i++) (*obr_zad)[i]=-wart_zad;
		etap=2;
		*czy_test=1;
	}
	
	if ((etap==2) && (time(NULL)-sek1>=skok*etap)) 
	{
		//printf("wynik = %d\n",obr_mierz[0]);
		if ((abs(obr_mierz[0])<granica_dolna) || (abs(obr_mierz[0])>granica_gorna)) wynik|=(1<<0);
		if ((abs(obr_mierz[1])<granica_dolna) || (abs(obr_mierz[1])>granica_gorna)) wynik|=(1<<1);
		if ((abs(obr_mierz[2])<granica_dolna) || (abs(obr_mierz[2])>granica_gorna)) wynik|=(1<<2);
		if ((abs(obr_mierz[3])<granica_dolna) || (abs(obr_mierz[3])>granica_gorna)) wynik|=(1<<3);
		if ((abs(obr_mierz[4])<granica_dolna) || (abs(obr_mierz[4])>granica_gorna)) wynik|=(1<<4);
		if ((abs(obr_mierz[5])<granica_dolna) || (abs(obr_mierz[5])>granica_gorna)) wynik|=(1<<5);
		for (i=0;i<6;i++) (*obr_zad)[i]=0;
		//pednik_mar[1]=wart_zad;
		etap=0;
		*czy_test=0;
		
		if (wynik&0x01) wynik_out|=(0x01<<0); else wynik_out|=(0x02<<0);
		if (wynik&0x02) wynik_out|=(0x01<<2); else wynik_out|=(0x02<<2);
		if (wynik&0x04) wynik_out|=(0x01<<4); else wynik_out|=(0x02<<4);
		if (wynik&0x08) wynik_out|=(0x01<<6); else wynik_out|=(0x02<<6);
		if (wynik&0x10) wynik_out|=(0x01<<4); else wynik_out|=(0x02<<4);
		if (wynik&0x20) wynik_out|=(0x01<<6); else wynik_out|=(0x02<<6);
		
		
		//printf("wynik = 0x%02x, wynik_out = 0x%02x\n",wynik, wynik_out);
		return wynik_out;
	}
	
	//printf("%d %d %d %d %d %d %d %d\n",pednik_mar[3],ModoPG.marszowy[0].pred);
	
	return 0;
}
