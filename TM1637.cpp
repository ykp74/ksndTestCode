//  Author:Fred.Chu
//  Date:9 April,2013
//
//  Applicable Module:
//        4-Digit Display by catalex
//        4-Digit Display (D4056A) by catalex
//   Store: http://www.aliexpress.com/store/1199788
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//  Modified record:
//
/*******************************************************************************/
#include "TM1637.h"

static int8_t TubeTab[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
                           0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71,0x40,0x00};
						   //0~9,A,b,C,d,E,F,"-"," "  

TM1637::TM1637(uint8_t Clk, uint8_t Data)
{
  Clkpin = Clk;
  Datapin = Data;
//  pinMode(Clkpin,OUTPUT);
//  pinMode(Datapin,OUTPUT);
  
  printf("TM1637 clk:%d data:%d \n",Clk,Data);
}

void TM1637::init(uint8_t DispType)
{
  _DispType = DispType;
  BlankingFlag = 1;
  DecPoint = 3;
  
  pinMode(Clkpin,OUTPUT);
  pinMode(Datapin,OUTPUT);
  
  clearDisplay();
}

void TM1637::writeByte(int8_t data)
{
	int c = 0;

	for(int i=0;i<8;i++){	//send 8bit dataE
		digitalWrite(Clkpin,LOW);      
		digitalWrite(Datapin, (data & 0x01) ? HIGH : LOW);	//LSB first
		delayMicroseconds(3);
		data >>= 1;      
		digitalWrite(Clkpin,HIGH);
		delayMicroseconds(3);
	}  
	digitalWrite(Clkpin,LOW); //wait for the ACK
	delayMicroseconds(5);   // After the falling edge of the eighth clock delay 5us, ACK signals the beginning of judgment  
	digitalWrite(Datapin,HIGH);
	digitalWrite(Clkpin,HIGH); 
	pinMode(Datapin,INPUT);		// switch datapin for Input

	while(digitalRead(Datapin)){ 
		if(++c == 200){
			pinMode(Datapin,OUTPUT);	// switch datapin for output
			digitalWrite(Datapin,LOW);
			c = 0;
			pinMode(Datapin,INPUT);
		}
	}
	pinMode(Datapin,OUTPUT);
}

//send start signal to TM1637
void TM1637::start(void)
{
  digitalWrite(Clkpin,HIGH);//send start signal to TM1637
  digitalWrite(Datapin,HIGH);
  //delayMicroseconds(2);
  digitalWrite(Datapin,LOW); 
  digitalWrite(Clkpin,LOW); 
} 

//End of transmission
void TM1637::stop(void)
{
  digitalWrite(Clkpin,LOW);
  //delayMicroseconds(2);
  digitalWrite(Datapin,LOW);
  //delayMicroseconds(2);
  digitalWrite(Clkpin,HIGH);
  //delayMicroseconds(2);
  digitalWrite(Datapin,HIGH); 
}

//display function.Write to full-screen.
void TM1637::display(int8_t data[])					// show 4 digits
{
	int i;
	//int8_t SegData[4];
	  
	//for(i=0;i < 4;i++){
	//	SegData[i] = data[i];
	//}
	start();
	writeByte(ADDR_AUTO);
	stop();
	start();
	writeByte(0xc0);
	for(i=0;i < 4;i++){
		writeByte(coding(data[i]));		
	}
	stop();
	start();
	writeByte(Cmd_DispCtrl);
	stop();
	
	//printf("display %d %d %d %d \n",data[0],data[1],data[2],data[3]);
}

//******************************************
void TM1637::display(uint8_t BitAddr,int8_t DispData)
{
  int8_t SegData;
  SegData = coding(DispData);
  start();          //start signal sent to TM1637 from MCU
  writeByte(ADDR_FIXED);//
  stop();           //
  start();          //
  writeByte(BitAddr|0xc0);//
  writeByte(SegData);//
  stop();            //
  start();          //
  writeByte(Cmd_DispCtrl);//
  stop();           //
}

void TM1637::display(double Decimal)
{
  int16_t temp;
  uint8_t i = 3;

  if(Decimal > 9999)
	  return;
  else if(Decimal < -999)
	  return;

  if(Decimal > 0){
	for( ;i > 0; i --){
	  if(Decimal < 1000)
		  Decimal *= 10;
	  else 
		  break;
	}
	temp = (int)Decimal;
	if((Decimal - temp)>0.5)
		temp++;
  } else {
	for( ;i > 1; i --){
	  if(Decimal > -100)
		  Decimal *= 10;
	  else 
		  break;
	}
	temp = (int)Decimal;
	if((temp - Decimal)>0.5)
		temp--;
  }
  DecPoint = i;

  BlankingFlag = 0;
  display(temp);
}

void TM1637::display(int16_t Decimal)
{
  int8_t temp[4];

  if((Decimal > 9999)||(Decimal < -999))
	  return;

  if(Decimal < 0){
	temp[0] = INDEX_NEGATIVE_SIGN;
	Decimal = abs(Decimal);
	temp[1] = Decimal/100;
    Decimal %= 100;
    temp[2] = Decimal / 10;
    temp[3] = Decimal % 10;

	if(BlankingFlag){
	  if(temp[1] == 0){ 
	    temp[1] = INDEX_BLANK;
	    if(temp[2] == 0) temp[2] = INDEX_BLANK;
	  }
	}
  } else {
    temp[0] = Decimal/1000;
	Decimal %= 1000;
    temp[1] = Decimal/100;
    Decimal %= 100;
    temp[2] = Decimal / 10;
    temp[3] = Decimal % 10;
	
	if(BlankingFlag){
	  if(temp[0] == 0){ 
	    temp[0] = INDEX_BLANK;
	    if(temp[1] == 0){
	      temp[1] = INDEX_BLANK;
		  if(temp[2] == 0){
			  temp[2] = INDEX_BLANK;
		  }
	    }
	  }
	}
  }
  BlankingFlag = 1;
  display(temp);
}

void TM1637::clearDisplay(void)
{
  display(0x00,0x7f);
  display(0x01,0x7f);
  display(0x02,0x7f);
  display(0x03,0x7f);  
  
  printf("clearDisplay \n");
}
//To take effect the next time it displays.
void TM1637::set(uint8_t brightness,uint8_t SetData,uint8_t SetAddr)
{
  Cmd_SetData = SetData;
  Cmd_SetAddr = SetAddr;
  Cmd_DispCtrl = 0x88 + brightness;//Set the brightness and it takes effect the next time it displays.
}

//Whether to light the clock point ":".
//To take effect the next time it displays.
void TM1637::point(boolean PointFlag)
{
  if(_DispType == D4036B) 
	  _PointFlag = PointFlag;
}

void TM1637::coding(int8_t DispData[])
{
  uint8_t PointData;

  if(_PointFlag == POINT_ON)
	  PointData = 0x80;
  else 
	  PointData = 0; 

  for(uint8_t i = 0;i < 4;i ++){
    if(DispData[i] == 0x7f)
		DispData[i] = 0x00;
    else 
		DispData[i] = TubeTab[DispData[i]] + PointData;
  }

  if((_DispType == D4056A)&&(DecPoint != 3)){
  	DispData[DecPoint] += 0x80;
	DecPoint = 3;
  }
}

int8_t TM1637::coding(int8_t DispData)
{
  uint8_t PointData;

  if(_PointFlag == POINT_ON)
	  PointData = 0x80;
  else 
	  PointData = 0; 
  if(DispData == 0x7f) 
	  DispData = 0x00 + PointData;//The bit digital tube off
  else 
	  DispData = TubeTab[DispData] + PointData;

  return DispData;
}
