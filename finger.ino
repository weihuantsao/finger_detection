#include <SoftwareSerial.h>

#define ACK_SUCCESS 0x00 //操作成功
#define ACK_FAIL 0x01 //操作失败
#define ACK_FULL 0x04 //指纹数据库已满
#define ACK_NOUSER 0x05 //无此用户
#define ACK_USER_EXIST 0x07 //用户已存在
#define ACK_TIMEOUT 0x08 //采集超时

// command
byte CMD_GET_MEMBER_COUNT[] = {0xF5,0x09,0x00,0x00,0x00,0x00,0x09,0xF5};
byte CMD_GET_MEMBER_RECOGNITION[] ={0xF5,0x0C,0x00,0x00,0x01,0x00,0x0D,0xF5};
byte CMD_SET_MEMBER_REGISTER[] ={0xF5,0x01,0x00,0x00,0x01,0x00,0x00,0xF5};
byte CMD_SET_MEMBER_DELALL[] = {0xF5,0x05,0x00,0x00,0x01,0x00,0x04,0xF5};

SoftwareSerial finger_serial(10, 11); //pin10 is RX , pin11 is TX

byte read_data[8]={0}; 
byte index = 0;
byte byteRead;
int member_count = 0; 

void setup()  
{
  Serial.begin(9600);
  finger_serial.begin(19200);
  
  delay(100);
  //get_member_count();
  //register_member();
}

void loop()                     // run over and over again
{
   if (Serial.available() > 0) {
    byteRead = Serial.read();
    switch (byteRead) {
    case 'u':    
      Serial.println(get_member_count());
      break;
    case 's':    
      search_member();
      break;
    case 'i':    
      register_member();
      break;
    case 'd':    
      del_allmember();
      break;
    } 
  }
}
int register_member(void){
  int new_account_id  = get_member_count() +1 ;
  Serial.println(new_account_id);
  //byte3 , byte4 is id 
  CMD_SET_MEMBER_REGISTER[1] = 0x01;
  CMD_SET_MEMBER_REGISTER[2] = highByte(new_account_id),HEX;
  CMD_SET_MEMBER_REGISTER[3] = lowByte(new_account_id),HEX;
  CMD_SET_MEMBER_REGISTER[6] = 0x00;
  CMD_SET_MEMBER_REGISTER[6] = Cal_CheckSum(CMD_SET_MEMBER_REGISTER); //checksum
  //Serial.write(CMD_SET_MEMBER_REGISTER,sizeof(CMD_SET_MEMBER_REGISTER));
  
  finger_serial.write(CMD_SET_MEMBER_REGISTER,sizeof(CMD_SET_MEMBER_REGISTER));
  while(true){
    if (finger_serial.available()) {
        byteRead = finger_serial.read();
        read_data[index] = byteRead;
        index++;
        if(index == 8){
          index = 0;
          break;
        }
    }
  }
  if(Cal_CheckSum(read_data) == 0){
    if(read_data[4] == ACK_SUCCESS){

      //twice register
      CMD_SET_MEMBER_REGISTER[1] = 0x03;
      CMD_SET_MEMBER_REGISTER[2] = highByte(new_account_id),HEX;
      CMD_SET_MEMBER_REGISTER[3] = lowByte(new_account_id),HEX;
      CMD_SET_MEMBER_REGISTER[6] = 0x00;
      CMD_SET_MEMBER_REGISTER[6] = Cal_CheckSum(CMD_SET_MEMBER_REGISTER); //checksum
      finger_serial.write(CMD_SET_MEMBER_REGISTER,sizeof(CMD_SET_MEMBER_REGISTER));
      
      while(true){
        if (finger_serial.available()) {
          byteRead = finger_serial.read();
          read_data[index] = byteRead;
          index++;
          if(index == 8){
            index = 0;
            break;
          }
        }
      }

      if(Cal_CheckSum(read_data) == 0 && read_data[4] == ACK_SUCCESS){
        Serial.println("SUCCESS");
        return 1;
      }else if(read_data[4] == ACK_FAIL){
        Serial.println("FAIL");
        return 0;
      }
    }else if(read_data[4] == ACK_FAIL){
      Serial.println("FAIL");
      return 0;
    }else if(read_data[4] == ACK_FULL){
      Serial.println("FULL");
      return 0;
    }else if(read_data[4] == ACK_USER_EXIST){
      Serial.println("USER_EXIST");
      return 1;
    }
  }else{
    return 0;
  }
  return 0;
}
int get_member_count(void){
  int count = 0 ;
  finger_serial.write(CMD_GET_MEMBER_COUNT,sizeof(CMD_GET_MEMBER_COUNT));
  while(true){
    if (finger_serial.available()) {
        byteRead = finger_serial.read();
        read_data[index] = byteRead;
        index++;
        if(index == 8){
          index = 0;
          break;
        }
    }
  }

  if(Cal_CheckSum(read_data) == 0){
    //count = (read_data[3]+read_data[2]*256);
    
    count = read_data[2]<<8; // X msb
    count |= read_data[3]; // X lsb
    return count;
  }else{
    get_member_count();
  }
  return 0;
}

int search_member(void){
  int member_id = 0; 
  finger_serial.write(CMD_GET_MEMBER_RECOGNITION,sizeof(CMD_GET_MEMBER_RECOGNITION));
  while(true){
    if (finger_serial.available()) {
        byteRead = finger_serial.read();
        read_data[index] = byteRead;
        index++;
        if(index == 8){
          index = 0;
          break;
        }
    }
  }

  if(Cal_CheckSum(read_data) == 0){
    //Serial.write(read_data,sizeof(read_data));
    if(read_data[4] == 0x01){
      member_id = read_data[2]<<8; // X msb
      member_id |= read_data[3]; // X lsb
      Serial.println(member_id);
      return member_id;
    }else if(read_data[4] == ACK_NOUSER){
      Serial.println("NO USER");
    }
  }else{
    Serial.println("checksum error");
    search_member();
  }
  return 0;
}

int del_allmember(void){
  finger_serial.write(CMD_SET_MEMBER_DELALL,sizeof(CMD_SET_MEMBER_DELALL));
  while(true){
    if (finger_serial.available()) {
        byteRead = finger_serial.read();
        read_data[index] = byteRead;
        index++;
        if(index == 8){
          index = 0;
          break;
        }
    }
  }

  if(Cal_CheckSum(read_data) == 0){
    if(read_data[4] == ACK_SUCCESS){
      Serial.println("SUCCESS");
      return 1;
    }else{
      Serial.println("DEL_FAIL");
      return 0;
    }
  }else{
    del_allmember();
  }
  return 0;
}

int Cal_CheckSum(byte *string)
{
  int XOR = 0;	
  for (int i = 0; i < 8; i++) 
  {
    XOR = XOR ^ string[i];
  }
  return XOR;
}


