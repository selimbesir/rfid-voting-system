#ifndef RFID_H
#define RFID_H

#include <xc.h>

#define _XTAL_FREQ 4000000

#define RFID_CS   PORTCbits.RC2
#define RFID_RST  PORTCbits.RC1
#define RFID_SCK  PORTCbits.RC6
#define RFID_MOSI PORTCbits.RC7
#define RFID_MISO PORTDbits.RD0

#define MAX_USERS 5

// Global data
extern unsigned char auth_cards[MAX_USERS][4];
extern unsigned char hasVoted[MAX_USERS];
extern int votesA;
extern int votesB;
extern int votesC;
extern int currentUser;

// Functions
void MFRC522_Init(void);
unsigned char MFRC522_Request(void);
unsigned char MFRC522_Anticoll(unsigned char *uid);
int check_card(unsigned char *uid, unsigned char *alreadyVoted);
void load_saved_state(void);
void save_state(void);

#endif