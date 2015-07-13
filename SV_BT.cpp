#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial BTlink(10, 11); // (10 pin RX board -> TX BT), (11 pin TX board -> RX BT)

int waitingForSend = 99;
String toSend = "";
bool slotOpen = false;

void BT_init()
{
  BTlink.begin(9600);
}

int get_waitingForSend()
{
  return waitingForSend;
}

bool is_slotOpen()
{
  return slotOpen;
}

int BT_prepareToSend(String message)
{  
  if ((message[1]-'0')<waitingForSend) 
  {
    waitingForSend = message[1]-'0';
    toSend = message;
    return 0;
  }
  else
  {
    return -1;
  }
}

int BT_Send()
{
  
  if(true == slotOpen)
  {
    toSend = String('$') + toSend;
    toSend = toSend + String('#');
    BTlink.println(toSend);
    toSend = "";
    waitingForSend = 99;
    slotOpen = false;
  }
}

int BT_Receive(String &message)
{
  String inData = "";
  String BluetoothData = "";
  int ret = -1;
  while(BTlink.available()>0)
  {
    char recieved = BTlink.read();
    if( (inData == "" && recieved == '$') || (inData != "") ) 
    {
      inData += recieved;

      // Process message when new line character is recieved
      if (recieved == '#')
      {
        BluetoothData = inData;
        inData = "";
        break;
      }
    }
    else if(recieved >= '0' && recieved <= '9')
    {
      BluetoothData = String(recieved);
      break;
    }
  }
  
  if(BluetoothData[0] == '0')
  {
    slotOpen = true;
    ret = 0;
  }if(BluetoothData[0] == '9')
  {
    ret = 9;
  }else if(BluetoothData[0] == '2')
  {
    ret = 2;
  }else if(BluetoothData[0] == '3')
  {
    ret = 3;
  }else if(BluetoothData[1] == '4')
  {
    message = BluetoothData;
    ret = 4;
  }else if(BluetoothData[0] == '5')
  {
    ret = 5;
  }else if(BluetoothData[1] == '6')
  {
    ret = 6;    
  }
  
  inData = "";
  BluetoothData = "";
  return ret;
}
