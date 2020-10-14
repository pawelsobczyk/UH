#include <sys/time.h>	//timeval
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>		//tu jest def exit();
#include <fcntl.h>
#include <termios.h> 
#include <signal.h>
#include <arpa/inet.h>

unsigned long LocalTime()	//zwraca ms
{
	struct timeval czas; 
	gettimeofday(&czas,NULL);
	return czas.tv_sec*1000+czas.tv_usec/1000;
}


int port_init(const char *port, const short predkosc)
{   
	int uchwyt;
	struct termios newtio; 
    
	if ((uchwyt = open(port,O_RDWR | O_NOCTTY))<0) { 
		printf("Nie udalo sie otworzyc portu %s\n",port); 
		return uchwyt;
	}
	//printf("Uchwyt portu %d\n",uchwyt);
  	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = predkosc|CS8|CLOCAL|CREAD;		//CLOCAL - Ignore modem control lines, CREAD - Enable receiver
  	newtio.c_iflag = IGNPAR;	//Ustawienie to oznacza, że znaki, które spowodowały błąd kontroli parzystości są pomijane.
  	newtio.c_oflag = 0;
  	newtio.c_lflag = 0;
  	newtio.c_cc[VTIME] = 0;
  	newtio.c_cc[VMIN]  = 0;
	
  	tcflush(uchwyt,TCIFLUSH);
  	tcsetattr(uchwyt,TCSANOW,&newtio);
	return uchwyt;
}

int falow_port_init(const char *port, const short predkosc)
{   
	int uchwyt;
	struct termios newtio; 
    
	if ((uchwyt = open(port,O_RDWR | O_NOCTTY))<0) { 
		printf("Nie udalo sie otworzyc portu %s\n",port); 
		return uchwyt;
	}
	
  	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = predkosc|CS7|PARENB|CLOCAL|CREAD;		//CS7 - 7 znakow danych, PARENB - even parity check, CLOCAL - Ignore modem control lines, CREAD - Enable receiver
  	newtio.c_iflag = IGNPAR;	//Ustawienie to oznacza, że znaki, które spowodowały błąd kontroli parzystości są pomijane.
  	newtio.c_oflag = 0;
  	newtio.c_lflag = 0;
  	newtio.c_cc[VTIME] = 0;
  	newtio.c_cc[VMIN]  = 0;
	
  	tcflush(uchwyt,TCIFLUSH);
  	tcsetattr(uchwyt,TCSANOW,&newtio);
	return uchwyt;
}

void ms_delay(unsigned short ms)
{
	struct timeval t1, t2;
    unsigned int msTime=0;	//globalny licznik milisekund
	
	gettimeofday(&t1,NULL);
	while (msTime<ms) {
		gettimeofday(&t2,NULL);
		msTime = (t2.tv_sec - t1.tv_sec)*1000;      // sec to ms
		msTime += (t2.tv_usec - t1.tv_usec)/1000;   // ms
	}
}

int InitMulticastHandle(const char* MulticastGroupIP, unsigned short port, struct sockaddr_in *saddr) 
{
	unsigned int 		yes=1;    
	int 				uchwyt;  
	struct sockaddr_in  interfaceAddr;
	struct ip_mreq 		mreq;

	// create what looks like an ordinary UDP socket
	if ((uchwyt=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
		perror("Wystąpił błąd z otwarciem portu UDP Multicast");
		return uchwyt;
	}
	// allow multiple sockets to use the same PORT number
	if (setsockopt(uchwyt,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
		perror("Wystąpił błąd z otwarciem portu UDP Multicast");
		return 0;
	}
	
	//ustawienia siebie
	memset(&interfaceAddr,0,sizeof(struct sockaddr_in));
	interfaceAddr.sin_family=AF_INET;
	interfaceAddr.sin_addr.s_addr=INADDR_ANY; //swój adres
	interfaceAddr.sin_port=htons(port);	//port serwera multicast
	
	// bind to receive address
	if (bind(uchwyt,(struct sockaddr*)&interfaceAddr,sizeof(struct sockaddr_in)) < 0) {
		perror("Wystąpił błąd z otwarciem portu UDP Multicast");
		return 0;
	}

	// use setsockopt() to request that the kernel join a multicast group
	mreq.imr_multiaddr.s_addr=inet_addr(MulticastGroupIP);	//adres serwera multicast
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);	//swój adres
	if (setsockopt(uchwyt,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&mreq,sizeof(mreq)) < 0) {
		perror("Wystąpił błąd z otwarciem portu UDP Multicast");
		return 0;
	}

	(*saddr).sin_family = AF_INET;
	(*saddr).sin_addr.s_addr = inet_addr(MulticastGroupIP);
	(*saddr).sin_port = htons(port);
	
	//memcpy(saddr1,&saddr,sizeof(struct sockaddr_in));
	
	int loop=0;
	if (setsockopt(uchwyt,IPPROTO_IP,IP_MULTICAST_LOOP,&loop, sizeof(loop))<0)	{//wyłączenie LOOP_BACK
		perror("Wystąpił błąd z wyłączenie Loopback");
		return 0;
	}
	
	printf("Został otwarty port typu MULTICAST, IP: %s, port: %d\n",inet_ntoa((*saddr).sin_addr),port);
	tcflush(uchwyt,TCIFLUSH);	
	return uchwyt;
}

int InitMulticastHandle2(const char* MulticastGroupIP, unsigned short port_in, unsigned short port_out, struct sockaddr_in *saddr) 
{
	unsigned int 		yes=1;    
	int 				uchwyt;  
	struct sockaddr_in  interfaceAddr;
	struct ip_mreq 		mreq;
	int ttl = 60;	

	// create what looks like an ordinary UDP socket
	if ((uchwyt=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
		perror("Wystąpił błąd z otwarciem portu UDP Multicast");
		return uchwyt;
	}
	// allow multiple sockets to use the same PORT number
	if (setsockopt(uchwyt,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
		perror("Wystąpił błąd z otwarciem portu UDP Multicast");
		return 0;
	}
	if (setsockopt(uchwyt, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("Wystąpił błąd z ustawieniem parametru TTL portu MULTICAST");
		return 0;
	}
		
	//ustawienia siebie
	memset(&interfaceAddr,0,sizeof(struct sockaddr_in));
	interfaceAddr.sin_family=AF_INET;
	interfaceAddr.sin_addr.s_addr=INADDR_ANY; //swój adres
	interfaceAddr.sin_port=htons(port_in);	//port serwera multicast, z którego się nasłuchuje
	
	// bind to receive address
	if (bind(uchwyt,(struct sockaddr*)&interfaceAddr,sizeof(struct sockaddr_in)) < 0) {
		perror("Wystąpił błąd z otwarciem portu UDP Multicast");
		return 0;
	}

	// use setsockopt() to request that the kernel join a multicast group
	mreq.imr_multiaddr.s_addr=inet_addr(MulticastGroupIP);	//adres serwera multicast
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);	//swój adres
	if (setsockopt(uchwyt,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&mreq,sizeof(mreq)) < 0) {
		perror("Wystąpił błąd z otwarciem portu UDP Multicast");
		return 0;
	}

	(*saddr).sin_family = AF_INET;
	(*saddr).sin_addr.s_addr = inet_addr(MulticastGroupIP);
	(*saddr).sin_port = htons(port_out);	//port na który się wysyła dane
	
	//memcpy(saddr1,&saddr,sizeof(struct sockaddr_in));
	
	int loop=0;
	if (setsockopt(uchwyt,IPPROTO_IP,IP_MULTICAST_LOOP,&loop, sizeof(loop))<0)	{//wyłączenie LOOP_BACK
		perror("Wystąpił błąd z wyłączenie Loopback");
		return 0;
	}
	
	printf("Został otwarty port typu MULTICAST, IP: %s, port wejściowy: %d, port wyjściowy: %d\n",inet_ntoa((*saddr).sin_addr),port_in,port_out);
	tcflush(uchwyt,TCIFLUSH);
	return uchwyt;
}

int InitUnicastHandle(char* IPadresata, unsigned short port, struct sockaddr_in *saddr)
{
	int uchwyt;
	struct sockaddr_in caddr;

	if ((uchwyt=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
		perror("socket");
		return uchwyt;
	}
	
	//ustawienia siebie
	memset(&caddr,0,sizeof(struct sockaddr_in));
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	caddr.sin_addr.s_addr = INADDR_ANY;  //adres siebie bierze z ustawień sieci

	//ustwienia adresata
	memset(saddr,0,sizeof(struct sockaddr_in));   //moge sobie wyczyscic
	(*saddr).sin_family = AF_INET;
	(*saddr).sin_port = htons(port); //port pod ktory się podłączę
	(*saddr).sin_addr.s_addr = inet_addr(IPadresata);

	//powiązanie uchwytu ze swoim adresem
	if (bind(uchwyt,(struct sockaddr *)&caddr,sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return uchwyt;
	}
	printf("Został otwarty port typu UNICAST, IP adresata: %s, port swój i adresata: %d\n",inet_ntoa((*saddr).sin_addr),port);
	tcflush(uchwyt,TCIFLUSH);
	return uchwyt;
}

int InitUnicastHandle2(char* IPadresata, unsigned short rcv_port, unsigned short send_port, struct sockaddr_in *saddr)
{
	int uchwyt;
	struct sockaddr_in caddr;

	if ((uchwyt=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
		perror("socket");
		return uchwyt;
	}
	
	//ustawienia siebie
	memset(&caddr,0,sizeof(struct sockaddr_in));
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(rcv_port);
	caddr.sin_addr.s_addr = INADDR_ANY;  //adres siebie bierze z ustawień sieci

	//ustwienia adresata
	memset(saddr,0,sizeof(struct sockaddr_in));   //moge sobie wyczyscic
	(*saddr).sin_family = AF_INET;
	(*saddr).sin_port = htons(send_port); //port pod ktory się podłączę
	(*saddr).sin_addr.s_addr = inet_addr(IPadresata);

	//powiązanie uchwytu ze swoim adresem
	if (bind(uchwyt,(struct sockaddr *)&caddr,sizeof(struct sockaddr_in))<0) {
		perror("bind");
		return uchwyt;
	}
	printf("Został otwarty port typu UNICAST, IP adresata: %s, port adresata: %d, port swój: %d\n",inet_ntoa((*saddr).sin_addr),send_port,rcv_port);
	return uchwyt;
}


void Common_timer_init(unsigned short ms, void* funkcja_timera) 
{
	struct sigaction sygnal;	//do obslugi zdarzenia w systemie (przerwanie od timera rzeczywistego)
	struct itimerval timer;		//struktura do konfiguracji timera 
	
	//konfiguracja timera zwyklego
	//sygnal.sa_handler=(void*)timer_18ms;	//nazwa funkcji jaka ma byc wykonywana 
	sygnal.sa_handler=funkcja_timera;
	sygnal.sa_flags=0;
	sygnal.sa_restorer=NULL;
	sigaction(SIGALRM,&sygnal,NULL);//nazwa alarmu wysylana przez czasomierz rzeczywisty
	
	//ustawienie timera rzeczywistego
	timer.it_interval.tv_sec=0;
	timer.it_interval.tv_usec=ms*1000;	
	timer.it_value.tv_sec=1;	//1 s do pierwszego wystapienia sygnalu
	timer.it_value.tv_usec=0;	
	setitimer(ITIMER_REAL,&timer,NULL);	//wlaczenie timera wątka głównego
}

void abort_perror(const char *msg)
{
	perror(msg);
	exit(1);
}

unsigned char automatyka_init(unsigned char time, sigset_t *pblocked) 
{
	struct itimerspec its;
	struct sigevent sig;
	timer_t mytimer;
	
	//create a timer that will be used to periodically send a signal
	sig.sigev_notify = SIGEV_SIGNAL;
	sig.sigev_signo = SIGUSR1;
	sig.sigev_value.sival_ptr = &sig;
	if (timer_create(CLOCK_REALTIME, &sig, &mytimer)) {
		abort_perror("timer_create");
		return 1;
	}
	
	//block the signal used by the timer
	sigemptyset(pblocked);
	sigaddset(pblocked, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, pblocked, NULL);
	
	//set the periodic timer
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 1; // as soon as poss 
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = time*1000000; //100 ms guaranteed;
	if (timer_settime(mytimer,0,&its,NULL)) {
		abort_perror("timer_settime");
		return 2;	
	}
}

unsigned short CRC16(unsigned char ramka[], unsigned char m)
{
	unsigned int crc= 0xFFFF;
	unsigned char x=0,  //aktualny numer bajtu wiadomosci
					p;		//zmienna pomocnicza dla petli

	while (m>0) {
		crc^=ramka[x];
		for (p=0;p<8;p++) {
			if (crc&1) {
				crc>>=1;
				crc^=0xA001;
			} else crc>>=1;
		}
		m--;
		x++;
	}
	/*
	*crc_h=(CRC>>8)&0xFF;
	*crc_l=CRC&0xFF;*/
	return crc;
}

void licz_CRC(unsigned char ramka[], unsigned char m, 
		unsigned char *crc_h, unsigned char *crc_l)
{
	unsigned int CRC= 0xFFFF;
	unsigned char x=0,  //aktualny numer bajtu wiadomosci
				p;		//zmienna pomocnicza dla petli	
    
	while (m>0) {
	  	CRC^=ramka[x];
	  	for (p=0;p<8;p++) {
			if (CRC&1) {
		    CRC>>=1;
		    CRC^=0xA001;
			} else CRC>>=1;
	  	}
	  	m--;
	  	x++;
	}
	*crc_h=(CRC>>8)&0xFF;
	*crc_l=CRC&0xFF;
}

void write_i2c_register(int fd, unsigned char r, unsigned char val)
{
	unsigned char buf[2];
	buf[0] = r;
	buf[1] = val;
	if (write(fd,buf,2)!=2) {
		perror("write_i2c_register, write");
		exit(1);
	}
}

int CzekajNaZnaki(int uchwyt_portu, unsigned char adres_urzadzenia, unsigned short il_zn_do_odebrania, unsigned short timeout, unsigned char buf_in[300])
{
	unsigned short il_odeb_znak=0;		//ilość odebranych znaków
	unsigned char bufor[300], res, i;
	struct timeval t1, t2;
    unsigned int msTime=0;	//globalny licznik milisekund
	//unsigned long licznik;
	
	//start timer
	gettimeofday(&t1,NULL);

	while ((il_odeb_znak<il_zn_do_odebrania) && (msTime<timeout)) 
	{
		ms_delay(1);
		//usleep(1);
		res=read(uchwyt_portu,bufor,il_zn_do_odebrania);
		
		for (i=0;i<res;i++) {
		  	if ((bufor[i]==adres_urzadzenia) && (il_odeb_znak==0)) {
				buf_in[0]=bufor[i];
				il_odeb_znak=1;
			} else 
				if (il_odeb_znak>0) buf_in[il_odeb_znak++]=bufor[i];
		}//wyjscie z for
		
		gettimeofday(&t2,NULL);
		msTime = (t2.tv_sec - t1.tv_sec)*1000;      // sec to ms
		msTime += (t2.tv_usec - t1.tv_usec)/1000;   // ms
	}
	//for (i=0;i<il_odeb_znak;i++) printf("%d\n",buf_in[i]);
	return il_odeb_znak;	//funkcja zwraca ilosc odebranych znakow nawet jeśli wystąpił timeout
}


