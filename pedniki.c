#include "UH.h"

int ZalPrzetw(int uchwyt)
{
	//zalaczenie przetwornicy
	//int odp;
	unsigned char ramka1[20];
	int result;
	
	ramka1[0]=0x04;
	ramka1[1]='0';
	ramka1[2]='0';
	ramka1[3]='0';
	ramka1[4]='0';
	ramka1[5]=0x02;
	ramka1[6]='7';
	ramka1[7]='j';
	ramka1[8]='>';
	ramka1[9]='0';
	ramka1[10]='4';
	ramka1[11]='7';
	ramka1[12]='F';
	ramka1[13]=0x03;
	ramka1[14]=21;
	result=write(uchwyt,ramka1,15);
	//ReadFile(uchwyt,&odp,1,&cnt,NULL);
	return result;
}

int WylPrzetw(int uchwyt)
{
	//wylaczenie przetwornicy
	//char odp;
	unsigned char ramka1[20];
	int result;
	
	ramka1[0]=0x04;
	ramka1[1]='0';
	ramka1[2]='0';
	ramka1[3]='0';
	ramka1[4]='0';
	ramka1[5]=0x02;
	ramka1[6]='7';
	ramka1[7]='j';
	ramka1[8]='>';
	ramka1[9]='0';
	ramka1[10]='4';
	ramka1[11]='7';
	ramka1[12]='E';
	ramka1[13]=0x03;
	ramka1[14]=22;
	result=write(uchwyt,ramka1,15);
	//ReadFile(uchwyt,&odp,1,&cnt,NULL);
	return result;
}

int CzytPrad(int uchwyt)
{

	unsigned char ramka1[20];
	int result;
	
	ramka1[0]=0x04;
	ramka1[1]='0';
	ramka1[2]='0';
	ramka1[3]='0';
	ramka1[4]='0';
	ramka1[5]='1';      //prąd
	ramka1[6]='v';
	ramka1[7]=0x05;
	result=write(uchwyt,ramka1,8);
	return result;
	
	//ReadFile(uchwyt,odp,8,&cnt,NULL);
	/*gdy przetwornica jest WYLACZONA wowczas sterownik przysyla 7 znakow,
	gdy przetwornica jest wlaczona, wowczas przysyla 8 znakow*/
	//Label53->Caption=cnt;
/*
	odpo.printf("%s",odp);
	odpo.Delete(1,3);
	for (i=1;i<odpo.Length();i++)
	if (odpo.operator [](i)==0x03) {znacznik=i; break;}
	odpo.Delete(i,odpo.Length()-i+1);
	if (cnt<7) odpo="Blad";
	return odpo;
	*/
}

int CzytObroty(int uchwyt)
{

	unsigned char ramka1[20];
	int result;
	
	ramka1[0]=0x04;
	ramka1[1]='0';
	ramka1[2]='0';
	ramka1[3]='0';
	ramka1[4]='0';
	ramka1[5]='1';      //obroty
	ramka1[6]='9';
	ramka1[7]=0x05;
	result=write(uchwyt,ramka1,8);
	return result;
}



int ZadajPredkosc(int uchwyt,short predkosc)
{
	char s_pred[10];
	unsigned char ramka2[30], i;
	int result;
	
	sprintf(s_pred,"%04d",predkosc);	//predkosc jest rozumiana jako wypełnienie 0 - 300%
	
	ramka2[0]=0x04;
	ramka2[1]='0';
	ramka2[2]='0';
	ramka2[3]='0';
	ramka2[4]='0';
	ramka2[5]=0x02;
	ramka2[6]='7';
	ramka2[7]='h';
	memcpy(&ramka2[8],s_pred,4);
	ramka2[12]=0x03;
	
	//liczenie sumy kontrolnej
	ramka2[13]=0;
	for (i=6;i<13;i++) ramka2[13]^=ramka2[i]; //liczenie sumy kontrolnej BCC
	
	//ramka1[17]=0;	//tylko do wyświetlania tekstu
	//printf("%s\n",s_pred);
	result=write(uchwyt,ramka2,14);
	//ReadFile(uchwyt,&odp1,1,&cnt,NULL);
	return result;
}

int WriteTag(int uchwyt, int tag, float value)
{
	char s_val[10]={0}, ramka[30]={0x04,'0','0','0','0',0x02};
	unsigned char i;
	int result, m, n;
	
	m = tag/36;		n = tag - m*36;
	if (m>9) ramka[6] = 'a' + (m - 10);	else ramka[6] = '0' + m;
	if (n>9) ramka[7] = 'a' + (n - 10);	else ramka[7] = '0' + n;
	
	//printf("ch1 = %c, ch2 = %c\n",ramka[6],ramka[7]);
	//printf("wartość = %07.2f\n",value);
	sprintf(s_val,"%.2f",value);	//predkosc jest rozumiana jako wypełnienie 0 - 300%
	//printf("Łańcuch = %s, długość = %d\n",s_val,strlen(s_val));
	int dl = strlen(s_val);
	memcpy(&ramka[8],s_val,dl);
	ramka[8+dl]=0x03;
	
	//liczenie sumy kontrolnej
	ramka[9+dl]=0;
	for (i=6;i<9+dl;i++) ramka[9+dl]^=ramka[i]; //liczenie sumy kontrolnej BCC
	result=write(uchwyt,ramka,10+dl);
	
	return 0;
}

int ReadTag(int uchwyt, int tag)
{
	char ramka[30]={0x04,'0','0','0','0'};
	int result, m, n;
	
	m = tag/36;		n = tag - m*36;
	if (m>9) ramka[5] = 'a' + (m - 10);	else ramka[5] = '0' + m;
	if (n>9) ramka[6] = 'a' + (n - 10);	else ramka[6] = '0' + n;
	
	ramka[7]=5;
	result=write(uchwyt,ramka,8);
	
	return result;
}
