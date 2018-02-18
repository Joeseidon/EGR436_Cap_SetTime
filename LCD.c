/*
 * LCD.c
 *
 *  Created on: Feb 10, 2018
 *      Author: joe
 */

/* DriverLib Includes */
#include "driverlib.h"

#include "LCD.h"
#include "ST7735.h"
#include "sysTick.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*Text File Includes*/
#include "dark.txt"
#include "overcast.txt"
#include "partlySunny.txt"
#include "sunny.txt"
#include "twilight.txt"

uint16_t grid_color = ST7735_CYAN;
uint16_t menu_text_color = ST7735_YELLOW;
uint16_t highlight_text_color = ST7735_CYAN;

uint8_t LCD_Rotation = 1;


//create array for menu item names
char *menu_names[5] = {
                     {"0.5Hz"},
                     {"1.0Hz"},
                     {"2.0Hz"},
                     {"5.0Hz"},
                     {"LED 0/1"}
};




display_cell inside={
                0,      //x_start
                0,      //x_finish
                0,      //y_start
                0,      //y_finish
                " ",    //display title
                71.5,    //temp
                22.5,    //humidity
                5000     //Pressure
};

display_cell outside={
                0,      //x_start
                0,      //x_finish
                0,      //y_start
                0,      //y_finish
                " ",    //display title
                33.5,    //temp
                22.0,    //humidity
                4000     //Pressure
};

forecast light_status_items[5]={
    {DARK,dark , 32,32 ,60 ,60 ,0 ,0},
    {TWILIGHT,twilight,32 ,32 ,60 ,60 ,0 ,0},
    {OVERCAST,overcast,32 ,32 ,60 ,60 ,0 ,0},
    {PARTLY_SUNNY,partlySunny,32 ,32 ,60 ,60 ,0 ,0},
    {SUNNY,sunny,32 ,32 ,60 ,60 ,0 ,0}
};

menu_item menu_options[5]={
    {item1,"0.5Hz",8,4,1,0,12000000,4},
    {item2,"1.0Hz",8,5,0,0,12000000,2},
    {item3,"2.0Hz",8,6,0,0,12000000,1},
    {item4,"5.0Hz",8,7,0,0,4800000,1},
    {item5,"LED 0/1",8,8,0,0,0,0}
};

menu_items num_to_menu_item(int x){
    menu_items main_menu;
    switch(x){
    case 0:
        main_menu = item1;
        break;
    case 1:
        main_menu = item2;
        break;
    case 2:
        main_menu = item3;
        break;
    case 3:
        main_menu = item4;
        break;
    case 4:
        main_menu = item5;
        break;
    }
    return main_menu;
}

void LCD_init(void){
    ST7735_InitR(INITR_REDTAB); // initialize LCD controller IC
    ST7735_SetRotation(LCD_Rotation);
}
Light_Status num_to_enum(int x){
    Light_Status current_status;
    switch(x){
    case 0:
        current_status = DARK;
        break;
    case 1:
        current_status = TWILIGHT;
        break;
    case 2:
        current_status = OVERCAST;
        break;
    case 3:
        current_status = PARTLY_SUNNY;
        break;
    case 4:
        current_status = SUNNY;
        break;
    }
    return current_status;
}

void print_current_status_pic(Light_Status current_status){

    int i;
    for(i=0; i<5;i++){
        if(light_status_items[i].light_quality == current_status){
            ST7735_DrawBitmap(light_status_items[i].x, light_status_items[i].y, light_status_items[i].image, light_status_items[i].width, light_status_items[i].height);
        }
    }
}

void updateForecast(Light_Status newForecast){
    return;
}

void create_data_display(void){
    //Draw inside display
    ST7735_DrawString2(110,50,"Out",menu_text_color,ST7735_BLACK);
    //ST7735_DrawFastHLine(100,65,50,grid_color);
    //ST7735_DrawString2(90,70,"T:",menu_text_color,ST7735_BLACK);
    //ST7735_DrawString2(90,90,"H:",menu_text_color,ST7735_BLACK);

    //Draw outside display
    ST7735_DrawString2(15,50,"In",menu_text_color,ST7735_BLACK);
    ST7735_DrawFastHLine(0,65,160,grid_color);
    ST7735_DrawString2(0,70,"T:",menu_text_color,ST7735_BLACK);
    ST7735_DrawString2(0,90,"H:",menu_text_color,ST7735_BLACK);


    ST7735_DrawString2(10,110,"Bp",menu_text_color,ST7735_BLACK);
    ST7735_DrawFastHLine(0,108,160,grid_color);

    ST7735_DrawString2(0,5,"HH:MM",menu_text_color,ST7735_BLACK);
    ST7735_DrawString2(100,5,"MM/DD",menu_text_color,ST7735_BLACK);
}

void updateDataDisplay(void){
    char data[6];
    //print temp
    sprintf(data,"%2.1f%cF",inside.temperature,247);
    SysTick_delay(1);
    ST7735_DrawString2(15,70,data,menu_text_color,ST7735_BLACK);

    //print humidity
    sprintf(data,"%2.1f%%",inside.humidity);
    SysTick_delay(1);
    ST7735_DrawString2(15,90,data,menu_text_color,ST7735_BLACK);

    SysTick_delay(1);

    //print temp
    sprintf(data,"%2.1f%cF",outside.temperature,247);
    SysTick_delay(1);
    ST7735_DrawString2(95,70,data,menu_text_color,ST7735_BLACK);

    //print humidity
    sprintf(data,"%2.1f%%",outside.humidity);
    SysTick_delay(1);
    ST7735_DrawString2(95,90,data,menu_text_color,ST7735_BLACK);

    //print bp
    sprintf(data,"%2.1fmmHg",outside.pressure);
    SysTick_delay(1);
    ST7735_DrawString2(40,110,data,menu_text_color,ST7735_BLACK);
}

void updateTimeDisplay(RTC_C_Calendar *newTime){
    //Format time
    uint16_t hours,mins,day,month;
    hours = RTC_C_convertBCDToBinary(newTime->hours);
    mins = RTC_C_convertBCDToBinary(newTime->minutes);
    day = RTC_C_convertBCDToBinary(newTime->dayOfmonth);
    month = RTC_C_convertBCDToBinary(newTime->month);
    //Display time
    char datetime[5];
    sprintf(datetime,"%02d:%02d",hours,mins);
    ST7735_DrawString2(0,5,datetime,menu_text_color,ST7735_BLACK);
    sprintf(datetime,"%02d/%02d",month,day);
    ST7735_DrawString2(100,5,datetime,menu_text_color,ST7735_BLACK);
}

