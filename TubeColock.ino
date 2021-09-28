#include <Wire.h>
#include <DS3231.h>

int SecSegment[2];
int MinSegment[2];
int HrSegment_[2];
int secVal, minVal, HrVal, cycleCount, mode = -1;
int doFastSet = 0, doSlowSet = 0, dfs = 0, dss = 0, do12Cnt;
bool isOnHigh, onSetMode, isStdTime = false;
char secDsp[3], minDsp[3], HrDsp[3], ret[10];
char timeDsp[10], buff[3], tubeOutput[10];

DS3231 clock;
RTCDateTime dt;

// ////////////////////////////////////////////////////////
// /////     /////     /////     /////     /////     /////
// //////////////////////////////////////////////////////
void setup()
{
  // put your setup code here, to run once:
  SecSegment[0] = A0;
  SecSegment[1] = A1;
  MinSegment[0] = A2;
  MinSegment[1] = A3;
  HrSegment_[0] = 9;
  HrSegment_[1] = 10;

  // start clock
  clock.begin();

  // SEGMENT
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);

  // MULTIPLEXER PINS
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  // AM/PM indicator
  pinMode(13, OUTPUT);

  // fast set Pin
  pinMode(11, INPUT);
  // slow set Pin
  pinMode(12, INPUT);

  // set clock with current system COMPILE time
  clock.setDateTime(__DATE__, __TIME__);

  // get time
  dt = clock.getDateTime();

  secVal = dt.second;
  minVal = dt.minute;
  HrVal = dt.hour;
  do12Cnt = 0;
}

// ////////////////////////////////////////////////////////
// /////     /////     /////     /////     /////     /////
// //////////////////////////////////////////////////////
void loop()
{

  // get button input
  dfs = digitalRead(11);
  dss = digitalRead(12);

  // set time after button depress
  if ((doFastSet == HIGH && dfs == LOW) || (doSlowSet == HIGH && dss == LOW))
  {
    minDsp[1] = timeDsp[3];
    minDsp[0] = timeDsp[2];
    HrDsp[1] = timeDsp[1];
    HrDsp[0] = timeDsp[0];
    HrVal = atoi(HrDsp);
    minVal = atoi(minDsp);
    clock.setDateTime(2525, 5, 5, HrVal, minVal, 00);
  }

  doFastSet = dfs;
  doSlowSet = dss;

  // -------------------------------------------
  // precess Button press
  if (doFastSet == HIGH && doSlowSet == LOW)
    mode = 1;
  else
  {
    if (doFastSet == LOW && doSlowSet == HIGH)
      mode = 2;
    else
    {
      if (doFastSet == LOW && doSlowSet == LOW)
        mode = -1;
      else
      {
        // set 24h or 12h display
        if (doFastSet == HIGH && doSlowSet == HIGH && mode >= -1)
        {
          mode = -2;
          isStdTime = !isStdTime;
        }
        else
          mode = -2;
      }
    }
  }

  // -------------------------------------------
  // perfom clock mode
  switch (mode)
  {
  case 1:
    // FAST SET
    sprintf(buff, "%c%c", timeDsp[0], timeDsp[1]);
    HrVal = atoi(buff);
    sprintf(buff, "%c%c", timeDsp[2], timeDsp[3]);
    minVal = atoi(buff);
    sprintf(timeDsp, "%s", clockSet(HrVal, minVal));
    break;
  case 2:
    // SLOW SET
    if ((cycleCount % 15) == 0)
    {
      sprintf(buff, "%c%c", timeDsp[0], timeDsp[1]);
      HrVal = atoi(buff);
      sprintf(buff, "%c%c", timeDsp[2], timeDsp[3]);
      minVal = atoi(buff);
      sprintf(timeDsp, "%s", clockSet(HrVal, minVal));
    }
    break;
  default:
    // STANDARD CLOCK MODE
    // get time
    dt = clock.getDateTime();

    secVal = dt.second;
    minVal = dt.minute;
    HrVal = dt.hour;

    // setup display string
    if (secVal < 10)
      sprintf(secDsp, "0%i", secVal);
    else
      sprintf(secDsp, "%i", secVal);

    if (minVal < 10)
      sprintf(minDsp, "0%i", minVal);
    else
      sprintf(minDsp, "%i", minVal);

    if (HrVal < 10)
      sprintf(HrDsp, "0%i", HrVal);
    else
      sprintf(HrDsp, "%i", HrVal);

    // combine string elements
    sprintf(timeDsp, "%s%s%s", HrDsp, minDsp, secDsp);
    break;
  }

  // -------------------------------------------
  // display 12h or 12h time
  if (isStdTime == true)
    sprintf(tubeOutput, "%s", timeNormilizer(timeDsp));
  else
    sprintf(tubeOutput, "%s", timeDsp);

  // -------------------------------------------
  // display the current count
  for (int i = 0; i < 6; i++)
  {
    numToDigit(tubeOutput, i);

    // on hours < 10 do not display  the ZERO
    // on the last tube insted leave the loop
    if (i == 6 && tubeOutput[0] == '0')
      break;
      
    delay(3);
  }

  // -------------------------------------------
  // turn on PM LED
  if (HrVal > 11 && isStdTime == true)
    digitalWrite(13, HIGH);
  else
    digitalWrite(13, LOW);

  // count cycles
  cycleCount = (cycleCount + 1) % 100;
}

// ////////////////////////////////////////////////////////
// /////     /////     /////     /////     /////     /////
// //////////////////////////////////////////////////////
void numToDigit(char timeString[], int plexer)
{
  char vfdDigit;

  // clear display
  digitalWrite(SecSegment[0], LOW);
  digitalWrite(SecSegment[1], LOW);
  digitalWrite(MinSegment[0], LOW);
  digitalWrite(MinSegment[1], LOW);
  digitalWrite(HrSegment_[0], LOW);
  digitalWrite(HrSegment_[1], LOW);

  // clear segments
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  
  switch (plexer)
  {
  case 0:
    // seconds_R: xx:xx:x#
    digitalWrite(SecSegment[0], HIGH);
    vfdDigit = timeString[5];
    break;
  case 1:
    // seconds_L: xx:xx:#x
    digitalWrite(SecSegment[1], HIGH);
    vfdDigit = timeString[4];
    break;
  case 2:
    // minutes_R: xx:x#:xx
    digitalWrite(MinSegment[0], HIGH);
    vfdDigit = timeString[3];
    break;
  case 3:
    // minutes_L: xx:#x:xx
    digitalWrite(MinSegment[1], HIGH);
    vfdDigit = timeString[2];
    break;
  case 4:
    // hours_R: x#:xx:xx
    digitalWrite(HrSegment_[0], HIGH);
    vfdDigit = timeString[1];
    break;
  case 5:
    // hours_L: #x:xx:xx
    // display only if the number is greater than 0
    digitalWrite(HrSegment_[1], HIGH);
    if (timeString[0] == '0')
    {
      digitalWrite(2, LOW);
      digitalWrite(3, LOW);
      digitalWrite(4, LOW);
      digitalWrite(5, HIGH);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(8, HIGH);
      return;
    }
    vfdDigit = timeString[0];
    break;
  }
  /*
      A,1    
      --- 
 F,6 | G | B,2 
      ---     
 E,5 | 7 | C,3       
      ---
      D,4
  */

  switch (vfdDigit)
  {
  case '0':
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(7, HIGH);
    break;
  case '1':
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    break;
  case '2':
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(8, HIGH);
    break;
  case '3':
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(8, HIGH);
    break;
  case '4':
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(8, HIGH);
    break;
  case '5':
    digitalWrite(2, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(8, HIGH);
    break;
  case '6':
    digitalWrite(2, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(8, HIGH);
    break;
  case '7':
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    break;
  case '8':
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(8, HIGH);
    break;
  case '9':
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(8, HIGH);
    break;
  }
}

// ////////////////////////////////////////////////////////
char *clockSet(int prmHr, int prmMin)
{
  char retHr[3], retMin[3];
  int tHr, tMin;

  // calc Miniuts
  tMin = (prmMin + 1) % 60;

  // calculate Hours
  if (tMin == 0)
    tHr = (prmHr + 1) % 24;
  else
    tHr = prmHr;

  // setup display string
  if (tMin < 10)
    sprintf(retMin, "0%i", tMin);
  else
    sprintf(retMin, "%i", tMin);

  if (tHr < 10)
    sprintf(retHr, "0%i", tHr);
  else
    sprintf(retHr, "%i", tHr);

  // combine and return
  // string RET is global
  sprintf(ret, "%s%s00", retHr, retMin);
  return ret;
}

// ////////////////////////////////////////////////////////
char *timeNormilizer(char input[])
{
  char hr[3], mmss[10];
  int hVal;

  // get time componets from string
  sprintf(hr, "%c%c", input[0], input[1]);
  sprintf(mmss, "%c%c%c%c", input[2], input[3], input[4], input[5]);

  // convert 24h values to 12h values
  hVal = atoi(hr);
  hVal = (hVal % 12);
  hVal = (hVal == 0) ? 12 : hVal;

  // convert new hour to string
  if (hVal < 10)
    sprintf(hr, "0%i", hVal);
  else
    sprintf(hr, "%i", hVal);

  // recombine and return new time string
  // string RET is global
  sprintf(ret, "%s%s", hr, mmss);
  return ret;
}
