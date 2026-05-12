#include <xc.h>
#include <string.h>
#include "rfid.h"

unsigned char auth_cards[MAX_USERS][4] = {
    {0xAB, 0x4F, 0xBB, 0x93},
    {0x10, 0x63, 0x47, 0x62},
    {0x21, 0x99, 0xCE, 0x06},
    {0xE5, 0x34, 0xCE, 0x06},
    {0xE5, 0x12, 0x6F, 0xAE}
};

unsigned char hasVoted[MAX_USERS];  // 0 or 1
int votesA = 0;
int votesB = 0;
int votesC = 0;
int currentUser = -1;

#define EEPROM_MAGIC_ADDR       0
#define EEPROM_VOTES_A_LOW      1
#define EEPROM_VOTES_A_HIGH     2
#define EEPROM_VOTES_B_LOW      3
#define EEPROM_VOTES_B_HIGH     4
#define EEPROM_VOTES_C_LOW      5
#define EEPROM_VOTES_C_HIGH     6
#define EEPROM_HAS_VOTED_BASE   7
#define EEPROM_CHECKSUM_ADDR    12
#define EEPROM_MAGIC            0xA5

void save_state(void);

static unsigned char eeprom_read_byte(unsigned char addr) {
    EEADR = addr;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}

static void eeprom_write_byte(unsigned char addr, unsigned char value) {
    if (eeprom_read_byte(addr) == value) return;

    EEADR = addr;
    EEDATA = value;
    EECON1bits.EEPGD = 0;
    EECON1bits.WREN = 1;

    unsigned char gie = INTCONbits.GIE;
    INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    INTCONbits.GIE = gie;

    while (EECON1bits.WR);
    EECON1bits.WREN = 0;
}

static unsigned char saved_state_checksum(void) {
    unsigned char sum = EEPROM_MAGIC;
    sum += (unsigned char)(votesA & 0xFF);
    sum += (unsigned char)((unsigned int)votesA >> 8);
    sum += (unsigned char)(votesB & 0xFF);
    sum += (unsigned char)((unsigned int)votesB >> 8);
    sum += (unsigned char)(votesC & 0xFF);
    sum += (unsigned char)((unsigned int)votesC >> 8);

    for (unsigned char i = 0; i < MAX_USERS; i++)
        sum += hasVoted[i];

    return sum;
}

void load_saved_state(void) {
    if (eeprom_read_byte(EEPROM_MAGIC_ADDR) != EEPROM_MAGIC) {
        save_state();
        return;
    }

    votesA = (int)eeprom_read_byte(EEPROM_VOTES_A_LOW);
    votesA |= ((int)eeprom_read_byte(EEPROM_VOTES_A_HIGH) << 8);
    votesB = (int)eeprom_read_byte(EEPROM_VOTES_B_LOW);
    votesB |= ((int)eeprom_read_byte(EEPROM_VOTES_B_HIGH) << 8);
    votesC = (int)eeprom_read_byte(EEPROM_VOTES_C_LOW);
    votesC |= ((int)eeprom_read_byte(EEPROM_VOTES_C_HIGH) << 8);

    for (unsigned char i = 0; i < MAX_USERS; i++)
        hasVoted[i] = eeprom_read_byte(EEPROM_HAS_VOTED_BASE + i) ? 1 : 0;

    if (eeprom_read_byte(EEPROM_CHECKSUM_ADDR) != saved_state_checksum()) {
        votesA = votesB = votesC = 0;
        for (unsigned char i = 0; i < MAX_USERS; i++) hasVoted[i] = 0;
        save_state();
    }
}

void save_state(void) {
    eeprom_write_byte(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
    eeprom_write_byte(EEPROM_VOTES_A_LOW, (unsigned char)(votesA & 0xFF));
    eeprom_write_byte(EEPROM_VOTES_A_HIGH, (unsigned char)((unsigned int)votesA >> 8));
    eeprom_write_byte(EEPROM_VOTES_B_LOW, (unsigned char)(votesB & 0xFF));
    eeprom_write_byte(EEPROM_VOTES_B_HIGH, (unsigned char)((unsigned int)votesB >> 8));
    eeprom_write_byte(EEPROM_VOTES_C_LOW, (unsigned char)(votesC & 0xFF));
    eeprom_write_byte(EEPROM_VOTES_C_HIGH, (unsigned char)((unsigned int)votesC >> 8));

    for (unsigned char i = 0; i < MAX_USERS; i++)
        eeprom_write_byte(EEPROM_HAS_VOTED_BASE + i, hasVoted[i] ? 1 : 0);

    eeprom_write_byte(EEPROM_CHECKSUM_ADDR, saved_state_checksum());
}

// ===================== Software SPI =====================

unsigned char SPI_Transfer(unsigned char data) {
    unsigned char out = 0;
    for (int i = 0; i < 8; i++) {
        RFID_MOSI = (data & 0x80) ? 1 : 0;
        data <<= 1;
        __delay_us(5);
        RFID_SCK = 1;
        __delay_us(5);
        out <<= 1;
        if (RFID_MISO) out |= 1;
        RFID_SCK = 0;
        __delay_us(2);
    }
    return out;
}

void MFRC522_Write(unsigned char addr, unsigned char val) {
    RFID_CS = 0;
    SPI_Transfer((addr << 1) & 0x7E);
    SPI_Transfer(val);
    RFID_CS = 1;
}

unsigned char MFRC522_Read(unsigned char addr) {
    unsigned char val;
    RFID_CS = 0;
    SPI_Transfer(((addr << 1) & 0x7E) | 0x80);
    val = SPI_Transfer(0x00);
    RFID_CS = 1;
    return val;
}

void MFRC522_ClearBitMask(unsigned char addr, unsigned char mask) {
    MFRC522_Write(addr, MFRC522_Read(addr) & ~mask);
}

void MFRC522_SetBitMask(unsigned char addr, unsigned char mask) {
    MFRC522_Write(addr, MFRC522_Read(addr) | mask);
}

// ===================== MFRC522 Init =====================

void MFRC522_Init() {
    RFID_RST = 0; __delay_ms(10);
    RFID_RST = 1; __delay_ms(50);

    MFRC522_Write(0x01, 0x0F); __delay_ms(10); // Soft reset
    MFRC522_Write(0x2A, 0x8D);
    MFRC522_Write(0x2B, 0x3E);
    MFRC522_Write(0x2D, 30);
    MFRC522_Write(0x2C, 0);
    MFRC522_Write(0x15, 0x40);
    MFRC522_Write(0x11, 0x3D);
    MFRC522_SetBitMask(0x14, 0x03);
}

// ===================== Card Communication =====================

unsigned char MFRC522_ToCard(unsigned char cmd,
                              unsigned char *sendData, unsigned char sendLen,
                              unsigned char *backData, unsigned char *backLen)
{
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;

    if (cmd == 0x0E) { irqEn = 0x12; waitFor = 0x10; }
    else if (cmd == 0x0C) { irqEn = 0x77; waitFor = 0x30; }

    MFRC522_Write(0x02, irqEn | 0x80);
    MFRC522_ClearBitMask(0x04, 0x80);
    MFRC522_Write(0x01, 0x00);
    MFRC522_SetBitMask(0x0A, 0x80);

    for (unsigned char i = 0; i < sendLen; i++)
        MFRC522_Write(0x09, sendData[i]);

    MFRC522_Write(0x01, cmd);

    if (cmd == 0x0C)
        MFRC522_SetBitMask(0x0D, 0x80);

    unsigned int timeout = 2000;
    unsigned char irqVal;
    do {
        irqVal = MFRC522_Read(0x04);
        timeout--;
    } while (timeout && !(irqVal & waitFor) && !(irqVal & 0x01));

    MFRC522_ClearBitMask(0x0D, 0x80);

    if (!timeout)                   return 0;
    if (irqVal & 0x01)              return 0;
    if (MFRC522_Read(0x06) & 0x1B) return 0;

    unsigned char n = MFRC522_Read(0x0A) & 0x3F;
    if (n == 0 || n > *backLen) return 0;

    *backLen = n;
    for (unsigned char i = 0; i < n; i++)
        backData[i] = MFRC522_Read(0x09);

    return 1;
}

unsigned char MFRC522_Request() {
    unsigned char tagType[1] = {0x26};
    unsigned char backData[2];
    unsigned char backLen = 2;
    MFRC522_Write(0x0D, 0x07);
    return MFRC522_ToCard(0x0C, tagType, 1, backData, &backLen);
}

unsigned char MFRC522_Anticoll(unsigned char *uid) {
    unsigned char sendData[2] = {0x93, 0x20};
    unsigned char backLen = 5;
    MFRC522_Write(0x0D, 0x00);
    if (!MFRC522_ToCard(0x0C, sendData, 2, uid, &backLen)) return 0;
    if (backLen != 5) return 0;
    unsigned char bcc = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
    if (bcc != uid[4]) return 0;
    return 1;
}

// ===================== Auth Check =====================
// Returns index if found, -1 if unknown.
// Sets alreadyVoted=1 if card found but already voted.

int check_card(unsigned char *uid, unsigned char *alreadyVoted) {
    *alreadyVoted = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        if (memcmp(uid, auth_cards[i], 4) == 0) {
            if (hasVoted[i]) *alreadyVoted = 1;
            return i;
        }
    }
    return -1;
}