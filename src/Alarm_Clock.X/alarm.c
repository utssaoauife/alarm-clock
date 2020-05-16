// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ 20000000
#define MODE RC0
#define SET RC1
#define UP RC2
#define DOWN RC3
#define BUZZ RC4
#define COLON RC5

#include <xc.h>
#include <7seg.h>
#include <eeprom.h>

char alarm_h, alarm_m, alarm_no, hr, min, sec, al_state, snz_state, sec_count;
char alarm[] = {0, 0, 0x7F}, time[] = {0, 0};
int alarms[4], snz[4];


// -----------------------------------------------------------------------------------------------------------

void disp_dig(char n, char dig){
    PORTA = ~(1 << (n-1));  // Selects digit of the display, '~' for CC
    PORTB = disp_cc(dig);  // Displays 'dig'
    __delay_ms(5);
}

// One 'disp()' takes ~20ms;
//Period calculations are very much based on this time within the program
void disp(char _1, char _2){
    disp_dig(1, _1/10);  // First digit of _1
    disp_dig(2, _1%10);  // Second digit of _1
    disp_dig(3, _2/10);  // First digit of _2
    disp_dig(4, _2%10);  // Second digit of _2
}

// ----------------------------------------------------------------------------------------------------------

void time_counter(){  // Counts minutes as secs
    /* With FOSC/4 = 5 MHz and TMR1 pre-scaler of 1:8, 1sec == 625000
     * But TMR1's max is 16**4 (2**16), therefore counting 62500 ten times does the trick.
     */
    if(TMR1 >= 62500){
        TMR1 = 0;
        sec_count++;
        if(sec_count == 10){
            sec_count = 0;
            min++;
            if(min == 60){
                min = 0;
                hr++;
                if(hr == 24) hr = 0;
            }
        }
    }
}

// Can easily switch between the two by 'replacing all' (except the function name)

void real_time_counter(){  // Counts real secs and minutes and hours
    /* With FOSC/4 = 5 MHz and TMR1 pre-scaler of 1:8, 1sec == 625000
     * But TMR1's max is 16**4, therefore counting 62500 ten times does the trick.
     */
    if(TMR1 >= 62500){
        TMR1 = 0;
        sec_count += 1;
        if(sec_count == 10){
            sec_count = 0;
            sec += 1;
            if(sec == 60){
                sec = 0;
                min++;
                if(min == 60){
                    min = 0;
                    hr++;
                    if(hr == 24) hr = 0;
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------------------

void buzz(char t1, char t2){
    COLON = 0;
    BUZZ = 1;
    for(char j = 0; j < 10; j++){  // Beeps for ~200ms
        disp(t1, t2);
        real_time_counter();
    }
    BUZZ = 0;
    COLON = 1;
}

// Used array so that function can be more versatile i.e can be used for different 'times'.
// 'i' so that the function can work for both hr and min.
char set_time(char arr[], char i){
    COLON = 0;
    for(char j = 0; j < 2 && SET; j++){  // Blinks 'SET' 2 times in 2 secs, stops if you release the button
        for(char k = 0; k < 40 && SET; k++){  // Displays 'SET' 0.8 secs
            disp_dig(1, ' ');
            disp_dig(2, 5);
            disp_dig(3, 'E');
            disp_dig(4, 'T');
            real_time_counter();
        }
        for(char k = 0; k < 10 && SET; k++){  // Display goes blank 0.2 secs
            PORTB = 0;
            __delay_ms(20);
            real_time_counter();
        }
    }
    COLON = 1;
    
    // If SET is released before the end of the 2 secs, this function exits
    // else, continues to actually setting the time
    if(SET){
        buzz(arr[0], arr[1]);  // Beeps
        while(SET){
            disp(arr[0], arr[1]);
            real_time_counter();
        }

        while(1){
            disp(arr[0], arr[1]);
            real_time_counter();
            if(UP){
                arr[i]++;  // Increases once
                if(i && (arr[i] == 60)) arr[i] = 0;  // minute
                if(!i && (arr[i] == 24)) arr[i] = 0;  // hour
                for(char j = 0; j < 25; j++){  // Waits for 0.5 sec before continual increase
                    disp(arr[0], arr[1]);
                    real_time_counter();
                }
                while(UP){  // Increases 5 times a sec.
                    real_time_counter();
                    arr[i]++;
                    if(i && (arr[i] == 60)) arr[i] = 0;  // minute
                    if(!i && (arr[i] == 24)) arr[i] = 0;  // hour
                    for(char j = 0; j < 10; j++){  // Waits 200ms while displaying
                        disp(arr[0], arr[1]);
                        real_time_counter();
                    }
                }
            }

            if(DOWN){
                if(arr[i] > 0) arr[i]--;  // Decreases once
                else if(i && (arr[i] == 0)) arr[i] = 59;  // minute
                else if(!i && (arr[i] == 0)) arr[i] = 23;  // hour

                for(char j = 0; j < 25; j++){  // Waits for 0.5 sec before continual decrease
                    disp(arr[0], arr[1]);
                    real_time_counter();
                }

                while(DOWN){  // Decreases 5 times a sec.
                    real_time_counter();
                    if(arr[i] > 0) arr[i]--;
                    else if(i && (arr[i] == 0)) arr[i] = 59;  // minute
                    else if(!i && (arr[i] == 0)) arr[i] = 23;  // hour
                    for(char j = 0; j < 10; j++){  // Waits 200ms while displaying
                        disp(arr[0], arr[1]);
                        real_time_counter();
                    }
                }
            }

            if(MODE){
                while(MODE){
                    disp(arr[0], arr[1]);
                    real_time_counter();
                }
                i = !i;  // Switches between Hour and minute
            }

            if(SET){
                for(char j = 0; j < 100 && SET; j++){  // Waits for 2 secs
                    disp(arr[0], arr[1]);
                    real_time_counter();
                }
                if(!SET) return 0; // Cancels Time Setting.
                // If held down for up to 2 secs, Sets time.
                buzz(arr[0], arr[1]);  // Beeps
                while(SET){
                    disp_dig(1, ' ');
                    disp_dig(2, 5);
                    disp_dig(3, 'E');
                    disp_dig(4, 'T');
                    real_time_counter();
                }
                break;
            }
        }
        return 1;  // Sets time.
    }
    return 0;
}

void clock_init(){
    hr = 0; min = 0;
    while(!SET) disp(hr, min);
    if(SET){
        while(SET) disp(hr, min);
        set_time(time, 0);
    }
    hr = time[0]; min = time[1];
    TMR1 = 0; TMR1ON = 1;
}

void alarm_init(){
    // Tries to read previously set alarm times
    // If not set before, sets to zero
    for(alarm_no = 0; alarm_no < 4; alarm_no++){
        cont_data_read(alarm, alarm_no*2, 2, 0x7F);
        if(alarm[0] == 0xFF){
            alarm[0] = alarm[1] = 0;
            cont_data_write(alarm_no*2, alarm, 0x7F);
            alarms[alarm_no] = 0;
        }
        else{
            alarms[alarm_no] = alarm[0]*60 + alarm[1];
        }
    }
    alarm_no = 0; // Set to Alarm 1
    
    al_state = data_read(0x08);  // Reads previous alarm states
    if(al_state == 0xFF){  // If not set before, sets all OFF
        al_state = 0;
        data_write(0x08, 0);
    }
    
    // Sets defaults for snooze data
    snz_state = 0;
    for(char i = 0; i < 4; i++){
        snz[i] = 0;
    }
}

void disp_alarm_no(){
    COLON = 0;
    for(char i = 0; i < 30; i++){  // Display Alarm No for ~0.6s
        disp_dig(1, 'A');
        disp_dig(2, 'L');
        disp_dig(3, 0);
        disp_dig(4, alarm_no+1);
        real_time_counter();
    }
    COLON = 1;
}

void disp_alarm_state(){  // Display Alarm State for ~0.6s
    COLON = 0;
    if(al_state & (1 << alarm_no)){
        for(char i = 0; i < 30; i++){  // Display 'HI' if ON
            disp_dig(1, ' ');
            disp_dig(2, ' ');
            disp_dig(3, 'H');
            disp_dig(4, 1);
            real_time_counter();
        }
    }
    else{
        for(char i = 0; i < 30; i++){  // Display 'LO' if OFF
            disp_dig(1, ' ');
            disp_dig(2, ' ');
            disp_dig(3, 'L');
            disp_dig(4, 0);
            real_time_counter();
        }
    }
    COLON = 1;
}

void disp_alarm(){  // Displays Alarm No, then Alarm State
    disp_alarm_no();
    disp_alarm_state();
}

// -------------------------------------------------------------------------------------------------------------

void main(){
    TRISA = TRISD = TRISE = TRISB = 0;
    TRISC = 0B00001111;
    PORTA = PORTB = PORTC = PORTD = PORTE = 0;
    T1CON = 0b00110000; TMR1 = 0;
    data_init();
    __delay_ms(50);
//    data_clear(0x00, 0x20);  // Sets bytes 0 through 32 to 0xFF.
    
    /* alarm_no is 0 to 3
     * al_state & (1 << alarm_no) gives the state of that alarm_no
     * same goes for snz_state
     */
    
    // Check for previous alarm data in memory and set defaults
    alarm_init();
    COLON = 1;  // Switches on the colon
    
//    Set clock time at start-up
//    clock_init();
    hr = min = 0;
    TMR1ON = 1;  // Starts counting time
// -----------------------------------------------------------------------------------------------------------
        
    while(1){
        disp(hr, min);
        real_time_counter();
        
//        Deliberately left these as comments
//        Initial time counting method
//        for(char i = 0; i < 50; i++){
//            disp(hr, min);
//        }
//        min++;
//        if(min == 60){
//            hr++;
//            min = 0;
//        }
        
        int time_sum = hr*60 + min;
        char pre_al_no = alarm_no;
        // Alarm Ringing --------------------------------------------------------------------------------------------
        for(alarm_no = 0; alarm_no < 4; alarm_no++){
            real_time_counter();
            // I don't really like long expressions but...
            if(((al_state & (1 << alarm_no)) && (time_sum == alarms[alarm_no])) || ((snz_state & (1 << alarm_no)) && (time_sum == snz[alarm_no]))){
                alarm_h = time_sum / 60;
                alarm_m = time_sum % 60;
                // Simulates the common alarm tone
                // Rings 15 times (for 30 secs) or until user stops or snoozes
                for(char i = 0; i < 15; i++){
                    // Beep for 400ms
                    for(char j = 0; j < 2 && !(SET || MODE || UP); j++) buzz(alarm_h, alarm_m);
                    if(SET || MODE || UP) break;
                    // Beep OFF for 200ms
                    for(char j = 0; j < 10 && !(SET || MODE || UP); j++){
                        disp(alarm_h, alarm_m);
                        real_time_counter();
                    }
                    if(SET || MODE || UP) break;
                    // Beep for 400ms
                    for(char j = 0; j < 2 && !(SET || MODE || UP); j++) buzz(alarm_h, alarm_m);
                    if(SET || MODE || UP) break;
                    // Beep OFF for 1sec
                    for(char j = 0; j < 50 && !(SET || MODE || UP); j++){
                        disp(alarm_h, alarm_m);
                        real_time_counter();
                    }
                    if(SET || MODE || UP) break;
                }
                
                if(snz_state & (1 << alarm_no)) snz_state -= 1 << alarm_no;
                
                if(SET){
                    while(SET){  // Alarm state goes OFF.
                        disp(hr, min);
                        real_time_counter();
                    }
                    al_state -= 1 << alarm_no; // Set Alarm state OFF
                    real_time_counter();
                    data_write(0x08, al_state);
                }
                else if(MODE){  // Alarm state stays ON.
                    while(MODE){
                        disp(hr, min);
                        real_time_counter();
                    }
                }
                else if(UP){  // Snooze by 5 mins
                    while(UP){
                        disp(hr, min);
                        real_time_counter();
                    }
                    snz_state += 1 << alarm_no;
                    snz[alarm_no] = time_sum + 5;
                }
                else{  // Stops on its own; Snooze by 5 mins
                    /* Happens maximum of two times,
                     * i.e if the first snooze stops ringing on it's own,
                     * It snoozes by an extra 5 mins
                     * Then if the second also stops on it's own, that's the end
                     */
                    if(snz[alarm_no] < alarms[alarm_no] + 10){
                        snz_state += 1 << alarm_no;
                        snz[alarm_no] = time_sum + 5;
                    }
                    else snz[alarm_no] = 0;
                }
                break;
            }
        }
        real_time_counter();
        alarm_no = pre_al_no;
        
        // ---------------------------------------------------------------------------------------------------
        
        if(SET){  // Set clock time.
            real_time_counter();
            time[0] = hr; time[1] = min;
            if(set_time(time, 0)){  // If cancelled, does not set time
                TMR1ON = 0;
                hr = time[0];
                min = time[1];
                TMR1 = 0; TMR1ON = 1;
            }
        }
        
        else if(MODE){
            while(MODE){
                disp(hr, min);
                real_time_counter();
            }
            real_time_counter();
            // Enters Alarm mode ------------------------------------------------------------------------------
            char pre_al_state = al_state;  // Used when exiting Alarm mode
            disp_alarm();
            // Reads time corresponding to current alarm_no
            alarm_h = alarms[alarm_no] / 60;
            alarm_m = alarms[alarm_no] % 60;
            
            while(1){
                real_time_counter();
                disp(alarm_h, alarm_m);  // Display time set to current alarm_no.
                
                if(UP){  // Switch to next Alarm No, rolls over
                    while(UP){
                        disp(alarm_h, alarm_m);
                        real_time_counter();
                    }
                    if(alarm_no < 3) alarm_no++;
                    else alarm_no = 0;
                    disp_alarm();
                    alarm_h = alarms[alarm_no] / 60;
                    alarm_m = alarms[alarm_no] % 60;
                }
                
                else if(DOWN){  // // Switch to previous Alarm No, rolls over
                    while(DOWN){
                        disp(alarm_h, alarm_m);
                        real_time_counter();
                    }
                    if(alarm_no > 0) alarm_no--;
                    else alarm_no = 3;
                    disp_alarm();
                    alarm_h = alarms[alarm_no] / 60;
                    alarm_m = alarms[alarm_no] % 60;
                }
                
                else if(SET){
                    for(char j = 0; j < 25 && SET; j++){  // Waits for 0.5 secs
                        disp(alarm_h, alarm_m);
                        real_time_counter();
                    }
                    if(!SET){  // if the user is no longer pressing the key
                        // Flips state of current alarm.
                        if(al_state & (1 << alarm_no)){
                            al_state -= 1 << alarm_no;
                            snz[alarm_no] = 0;
                        }
                        else al_state += 1 << alarm_no;
                        real_time_counter();
                        BUZZ = 1;
                        disp_alarm_state();
                        BUZZ = 0;
                    }
                    
                    else{  // if the user is still pressing the key i.e Press and hold
                        // Set alarm time and record in memory.
                        real_time_counter();
                        alarm[0] = alarm_h; alarm[1] = alarm_m;
                        if(set_time(alarm, 0)){  // If user cancels, time is not set
                            real_time_counter();
                            // If time is different from initial, set time.
                            if(alarm[0] != alarm_h || alarm[1] != alarm_m){
                                // Switches alarm ON, if OFF.
                                if(!(al_state & (1 << alarm_no))) al_state += 1 << alarm_no;
                                real_time_counter();
                                alarm_h = alarm[0]; alarm_m = alarm[1];
                                alarms[alarm_no] = alarm_h*60 + alarm_m;
                                snz[alarm_no] = 0;
                                real_time_counter();
                                // !**Writing to memory creates a little lag in timing**!
                                cont_data_write(alarm_no*2, alarm, 0x7F);
                                real_time_counter();
                                disp_alarm_state();
                            }
                            // else, just display state
                            else{
                                disp_alarm_state();
                            }
                        }
                    }
                }
                else if(MODE){  // Exit Alarm mode
                    while(MODE){
                        disp(alarm_h, alarm_m);
                        real_time_counter();
                    }
                    // Storage creates a little lag in timing
                    if(al_state != pre_al_state) data_write(0x08, al_state);
                    break;
                }
            }
        }
    }
}
