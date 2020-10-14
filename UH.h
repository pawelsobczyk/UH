#include <stdio.h>		//printf i scanf
#include <string.h>		//memcpy
//#include <pthread.h>
#include <termios.h> 
//#include <signal.h>
#include <netinet/in.h>
//#include <errno.h>
//#include <math.h>
//#include <stdbool.h>
//#include <sys/ioctl.h>	//zmienne do zmiany kierunku RS-485
//#include <fcntl.h>


//================================= U K W I A Ł ==============================
typedef struct {
	unsigned char Start;
	unsigned char Pojazd;
	unsigned char Ramka;
	unsigned short LiczRam;
	unsigned char Testy;
	unsigned char WlaczanieHydro;
	unsigned char Kamera1;
	unsigned char Kamera2;
	unsigned char StopienOsw[4];
	unsigned char ManipulZwalniak[2];
	unsigned char SterRoboczych;
	short Joysticki[6];
	float ZadKurs;
	float ZadPrzeg;
	float ZadGleb;
	float ZadEcho;
	float ZadPedPion;
	float ZadPredkosc;
	float ZadPozGleb;						
	__attribute__ ((aligned(8))) double ZadPozDl;			
	__attribute__ ((aligned(8))) double ZadPozSzer;			
	unsigned char Precyzja;
	unsigned char JoystickTryb;
	unsigned char Regulatory;
	unsigned char AutoMisja;
	unsigned char SterAwaryjne;
	unsigned char SaabSter;
	int ZadSilaKabl;
	int ZadPredKabl;
	int ZadDlKabl;
	unsigned char End;
} TWKBPdoUH;

typedef struct {
	unsigned char Start;
	unsigned char Pojazd;
	unsigned char Ramka;
	unsigned short LiczRam;
	unsigned char PodcKadl;
	unsigned short NrSer;
	float Kurs;
	float Przeg;
	float Przechyl;
	float Glebokosc;
	float Echo;
	float NapTrans;
	short SkretyKabl;
	short KadlTemp;
	__attribute__ ((aligned(8))) double DlugGeogr;	
	__attribute__ ((aligned(8))) double SzerGeogr;	
	short PredkoscX;
	short PredkoscY;
	char PolObrKamGD;
	char PolObrKamLP;
	char PolObrSon;
	unsigned char ModulManipul;
	short ObrPedZad[8];
	short ObrPedMierz[8];
	float PradyPed[8];
	unsigned char Niespr[4];
	unsigned char DaneDodatkowe;
	unsigned char ModulBojowy;
	unsigned char NiesprModBoj;
	unsigned char PodcMBojowy;
	short PolWindy;
	short PolObrWytyku;
	float PredDzwiek;
	unsigned char Misja;
	unsigned char PCUStatus;
	int tripy_1_2[2];
	int tripy_3_4[2];
	int tripy_5_6[2];
	unsigned char ATCStatus;
	int KablSila;
	int KablDl;
	int KablDlPoz;
	unsigned char TestOdp;
	unsigned short TrafoNap;
	unsigned short TrafoPrad;
	char TrafoTemp;
	unsigned char TrafoHamulec;

	//==============dane powtarzane z ramki steruj╣cej
    unsigned char Testy;
	unsigned char WlaczanieHydro;
	unsigned char Kamera1;
	unsigned char Kamera2;
	unsigned char StopienOsw[4];
	unsigned char ManipulZwalniak[2];
	unsigned char SterRoboczych;
	short Joysticki[6];
	float ZadKurs;
	float ZadPrzeg;
	float ZadGleb;
	float ZadEcho;
	float ZadPedPion;
	float ZadPredkosc;
	float ZadPozGleb;
	__attribute__ ((aligned (8))) double ZadPozDl;		
	__attribute__ ((aligned (8))) double ZadPozSzer;		
	unsigned char Precyzja;
	unsigned char JoystickTryb;
	unsigned char Regulatory;
	unsigned char AutoMisja;
	unsigned char SterAwaryjne;
	unsigned char SaabSter;
	int ZadSilaKabl;
	int ZadPredKabl;
	int ZadDlKabl;
	//==========================================
	unsigned char End;
} TUHdoWKBP;
   
typedef struct {
	short joy_lewy_x;
	short joy_lewy_y;
	short joy_prawy_x;
	short joy_prawy_y;
} TJoyCalib;
   
   
typedef struct {         // ramka sterowania z konsoli PG start=5 ramka=1
	unsigned char Start;
	unsigned char Pojazd;
	unsigned char Ramka;
	unsigned char Pusty;
	short cmd[6];  // joysticki
	float rkurs;   // nastawy
	float rpoch;
	float rgleb;
	float rodl;
	float rpion;
	float rpredk;
	float rprzech;
	float ThrCoef;
	unsigned char balast;
	unsigned char bieg;  // precyzja sterowania
	unsigned char tryb;   // reg gleb, reg odlegl, reg kursu, reg pochylenia, reg kaskadowy;
	unsigned char geo_mode; // uklad wsp. sterowania 0- uklad body, 1- NED (przod zamiast NE)
	unsigned char apr_nlin; // 0- apr. liniowa,   2 - nieliniowa aproksymacja charakterystyk pednikow
	unsigned char thr_kompensacja; // zalaczenie tablicowej kompensacji pednikow
	unsigned char take_control;  // sterowanie z konsoli PG
	unsigned char set_param;  // ustawianie parametrow z konsoli UUVi
	unsigned char latlong_on;
	__attribute__ ((aligned(8))) double Lat,Long, Z;
	__attribute__ ((aligned(8))) double X_NED,Y_NED,Z_NED;    // NED
	unsigned char misja; // sterowanie dojsciem do celu
	unsigned char reset_dr; // sterowanie dojsciem do celu
	unsigned char wlatlong; // nastawa w latlong
	unsigned char SterAwaryjne;
	unsigned char GL_POCH;  // zalaczanie kaskadowego regulatora glebokosci
} TifaceOpInGK;

typedef struct {
	unsigned char Start;
	unsigned char Pojazd;
	unsigned char Ramka;
	unsigned char Pusty;
    //pojazd
    float kurs;
    float pochylenie;
    float przechylenie;
    float glebokosc;
    float od_od_dna;
    float od_od_dna_filtr;
    float gyro[3];
    
    float slant_range;
    float bearing;
    double LLD[3];
    unsigned short USBL_frame_no;
    float USBL_x;
    float USBL_y;
    float USBL_depth;
    float USBL_accuracy;
    //dane okrętu
    unsigned short SHIP_frame_no;
    double SHIP_lat;
    double SHIP_long;
    float SHIP_course;
    float SHIP_heading;
    float SHIP_speed;
    
    short RPM_meas[5];
    unsigned char brak_kom : 1;
} TPomiarGK;

typedef struct {
	unsigned char Start;
	unsigned char Pojazd;
	unsigned char Ramka;
	unsigned char Pusty;
	short ObrPedZad[6];
	unsigned char Misja;
	unsigned char kzpp : 1;		//bit oznaczajacy kumunikację SPP z programem windowsowym Piotrka
} TOdSPP_Piotrka;

extern TWKBPdoUH WKBPdoUH;
extern TUHdoWKBP UHdoWKBP;

extern TPomiarGK PomiarGK, PomiarGK_sym;
extern TifaceOpInGK IfOpInGK;
extern TOdSPP_Piotrka OdSPP_Piotrka;

int WriteTag(int uchwyt, int tag, float value);
