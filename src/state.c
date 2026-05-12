#include <xc.h>
#include "state.h"
#include "lcd.h"
#include "rfid.h"
#include "keypad.h"

SystemState state = STATE_IDLE;

// ===================== State handlers =====================

static void wait_for_card_removed(void) {
    unsigned char stableAbsent = 0;

    while (stableAbsent < 5) {
        if (MFRC522_Request()) {
            stableAbsent = 0;
        } else {
            stableAbsent++;
        }
        __delay_ms(100);
    }
}
 //#=results *=rst
void handle_IDLE() {
    unsigned char idleLoops = 0;

    MFRC522_Init();
    LCD_Clear();
    LCD_SetCursor(0, 0); LCD_Print(" WELCOME VOTER! ");
    LCD_SetCursor(1, 0); LCD_Print("  PLEASE SCAN ");

    while (1) {
        unsigned char uid[5] = {0, 0, 0, 0, 0};

        if (MFRC522_Request()) {
            if (MFRC522_Anticoll(uid)) {
                unsigned char alreadyVoted = 0;
                int idx = check_card(uid, &alreadyVoted);
                currentUser = idx;
                if (idx == -1 || alreadyVoted)
                    state = STATE_DENIED;
                else
                    state = STATE_AUTH_CHECK;
                return;
            }
        }

        char key = getKey();
        if (key == '#') { state = STATE_RESULTS_MODE; return; }
        if (key == '*') { state = STATE_RESET_MODE;   return; }

        __delay_ms(50); // small delay between polls

        idleLoops++;
        if (idleLoops >= 40) {
            idleLoops = 0;
            MFRC522_Init();
        }
    }
}

void handle_AUTH_CHECK() {
    LCD_Clear();
    LCD_SetCursor(0, 0); LCD_Print(" ACCESS GRANTED ");
    LCD_SetCursor(1, 0); LCD_Print(" REMOVE CARD    ");
    wait_for_card_removed();

    LCD_Clear();
    LCD_SetCursor(0, 0); LCD_Print(" ACCESS GRANTED ");
    LCD_SetCursor(1, 0); LCD_Print(" PRESS ANY KEY  ");
    while (!getKey());
    state = STATE_VOTING;
}

void handle_VOTING() {
    while (1) {
        LCD_Clear();
        LCD_SetCursor(0, 0); LCD_Print("CAST YOUR VOTE:  ");
        LCD_SetCursor(1, 0); LCD_Print("  A OR B OR C  ");

        char key = 0;
        while (!key) key = getKey();

        if (key == 'A') { votesA++; }
        else if (key == 'B') { votesB++; }
        else if (key == 'C') { votesC++; }
        else {
            // invalid input
            char ack = 0;
            while (!ack) {
                LCD_Clear();
                LCD_SetCursor(0, 0); LCD_Print("Invalid input   ");
                LCD_SetCursor(1, 0); LCD_Print("Press anything ");
                for (unsigned char i = 0; i < 10 && !ack; i++) {
                    ack = getKey();
                    __delay_ms(100);
                }

                LCD_Clear();
                LCD_SetCursor(0, 0); LCD_Print("Invalid input   ");
                LCD_SetCursor(1, 0); LCD_Print("to go back      ");
                for (unsigned char i = 0; i < 10 && !ack; i++) {
                    ack = getKey();
                    __delay_ms(100);
                }
            }
            continue; // back to vote screen
        }

        // valid vote recorded
        hasVoted[currentUser] = 1;
        save_state();
        currentUser = -1;
        LCD_Clear();
        LCD_SetCursor(0, 0); LCD_Print("  VOTE RECORDED ");
        LCD_SetCursor(1, 0); LCD_Print(" REMOVE CARD    ");
        wait_for_card_removed();

        LCD_Clear();
        LCD_SetCursor(0, 0); LCD_Print("  VOTE RECORDED ");
        LCD_SetCursor(1, 0); LCD_Print(" PRESS ANY KEY  ");
        while (!getKey());
        state = STATE_IDLE;
        return;
    }
}

void handle_DENIED() {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    if (currentUser == -1) {
        LCD_Print(" ACCESS DENIED  ");
        LCD_SetCursor(1, 0); LCD_Print(" UNKNOWN CARD   ");
    } else {
        LCD_Print(" ALREADY VOTED  ");
        LCD_SetCursor(1, 0); LCD_Print("     DENIED     ");
    }
    __delay_ms(1200);
    LCD_Clear();
    LCD_SetCursor(0, 0); LCD_Print(" REMOVE CARD    ");
    wait_for_card_removed();
    LCD_SetCursor(1, 0); LCD_Print(" PRESS ANY KEY  ");
    while (!getKey());
    currentUser = -1;
    state = STATE_IDLE;
}

void handle_RESULTS_MODE() {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("A:"); LCD_PrintInt(votesA);
    LCD_Print(" B:"); LCD_PrintInt(votesB);
    LCD_Print(" C:"); LCD_PrintInt(votesC);
    LCD_SetCursor(1, 0); LCD_Print("*=reset  #=back ");

    char key = 0;
    while (!key) {
        key = getKey();
        if (key != '#' && key != '*') key = 0;
    }
    state = (key == '*') ? STATE_RESET_MODE : STATE_IDLE;
}

void handle_RESET_MODE() {
    LCD_Clear();
    LCD_SetCursor(0, 0); LCD_Print("RESET ALL VOTES?");
    LCD_SetCursor(1, 0); LCD_Print(" 1=YES  Any=NO  ");

    char key = 0;
    while (!key) key = getKey();

    if (key == '1') {
        votesA = votesB = votesC = 0;
        for (int i = 0; i < MAX_USERS; i++) hasVoted[i] = 0;
        save_state();
        currentUser = -1;
        LCD_Clear();
        LCD_SetCursor(0, 0); LCD_Print("  VOTES CLEARED ");
        LCD_SetCursor(1, 0); LCD_Print(" PRESS ANY KEY  ");
        while (!getKey());
    }
    state = STATE_IDLE;
}

void State_Run() {
    switch (state) {
        case STATE_IDLE:         handle_IDLE();         break;
        case STATE_AUTH_CHECK:   handle_AUTH_CHECK();   break;
        case STATE_VOTING:       handle_VOTING();       break;
        case STATE_DENIED:       handle_DENIED();       break;
        case STATE_RESULTS_MODE: handle_RESULTS_MODE(); break;
        case STATE_RESET_MODE:   handle_RESET_MODE();   break;
    }
}