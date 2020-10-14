int port_init(const char *port, const short predkosc);
void ms_delay(unsigned short ms);
int InitMulticastHandle(const char* MulticastGroupIP, unsigned short port, struct sockaddr_in *saddr1); 
int InitUnicastHandle(char* IPadresata, unsigned short port, struct sockaddr_in *saddr1);
void timer_18ms(void);
void timer_init(void);
void abort_perror(const char *msg);
unsigned char automatyka_init(unsigned char time);
void CRC16(unsigned char ramka[], unsigned char m,
 unsigned char *crc_h, unsigned char *crc_l);
void licz_CRC(unsigned char ramka[], unsigned char m, 
unsigned char *crc_h, unsigned char *crc_l);
void write_i2c_register(int fd, unsigned char r, unsigned char val);
int CzekajNaZnaki(int uchwyt_portu, unsigned char adres_urzadzenia, unsigned short il_zn_do_odebrania, unsigned short timeout, unsigned char *buf_in);
