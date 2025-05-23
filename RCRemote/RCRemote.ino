#include "Configuration.h"
#include <RF24.h>
#include <Joystick_if.h>

#include "UiManagement.h"





// Declare and configure each input on the remote controller.
// As of now, this configuration can be changed on the fly via the UI. 
// TODO: Make sure the configuration can be saved in the EEPROM/Non Volatile memory.
RemoteChannelInput_t RemoteInputs[N_CHANNELS] = 
                                    // Pin,                     Val,RVal,  Trim,                Min,                Max,               Invert,  isAnalog,, exp  Channel Name  
                                   {{JOYSTICK_LEFT_AXIS_X_PIN,  0u, 0u,  ANALOG_HALF_VALUE,   ANALOG_MIN_VALUE,   ANALOG_MAX_VALUE,  false,    true,  true, "JLX"}, 
                                    {JOYSTICK_LEFT_AXIS_Y_PIN,  0u, 0u,  ANALOG_HALF_VALUE,   200u,               750u,              false,    true,  true, "JLY"}, 
                                    {JOYSTICK_RIGHT_AXIS_X_PIN, 0u, 0u,  ANALOG_HALF_VALUE,   200u,               750u,              true,     true,  true, "JRX"}, 
                                    {JOYSTICK_RIGHT_AXIS_Y_PIN, 0u, 0u,  ANALOG_HALF_VALUE,   ANALOG_MIN_VALUE,   ANALOG_MAX_VALUE,  false,    true,  true, "JRY"}, 
                                    {POT_LEFT_PIN,              0u, 0u,  0u,                  ANALOG_MIN_VALUE,   ANALOG_MAX_VALUE,  false,    true,  true, "PL"},  
                                    {POT_RIGHT_PIN,             0u, 0u,  0u,                  ANALOG_MIN_VALUE,   ANALOG_MAX_VALUE,  false,    true,  true, "PR"},  
                                    {SWITCH_SP_LEFT_PIN,        0u, 0u,  0u,                  ANALOG_MIN_VALUE,   ANALOG_MAX_VALUE,  false,    false, true, "SWL"}, 
                                    {SWITCH_SP_RIGHT_PIN,       0u, 0u,  0u,                  ANALOG_MIN_VALUE,   ANALOG_MAX_VALUE,  false,    false, true, "SWR"}};

RemoteCommunicationState_t RemoteCommunicationState = {false, 0l};
UiM_t_Inputs  uiInputs;
UiM_t_rPorts  uiInputData = {&uiInputs, RemoteInputs, &RemoteCommunicationState};
UiM_t_pPorts  uiResponseData = {false};



#if BATTERY_INDICATION == ON
#include <BatteryIndication.h>
#define R1 10000
#define R2 10000
BatteryIndication battery(BATTERY_INDICATION_PIN, R1, R2, BATTERY_9V);
#endif

// Remote Transmitter_Remote;
RFPayload payload;
RF24 Radio;

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


// TODO: Improve naming to enforce the concept of "remote CHANNEL input" and ordinary "remote input" (such as buttons)

void v_initRemoteInputs(RemoteChannelInput_t* pRemoteInputs)
{
  // Remote CHANNEL inputs
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    pinMode(pRemoteInputs[i].u8_Pin,    pRemoteInputs[i].b_Analog ? INPUT : INPUT_PULLUP);
  }

  // UI Management input initialization
  pinMode(INPUT_BUTTON_LEFT_PIN,   INPUT_PULLUP);
  pinMode(INPUT_BUTTON_RIGHT_PIN,  INPUT_PULLUP);
  pinMode(INPUT_BUTTON_SELECT_PIN, INPUT_PULLUP);

}

boolean b_initRadio(RF24* pRadio)
{
  *pRadio = RF24(RF24_CE_PIN, RF24_CSN_PIN);
  bool b_Success = pRadio->begin();
  if(b_Success)
  {
    // Radio.setAutoAck(false); // Making sure auto ack isn't ON to ensure we can properly calcualte timeouts
    pRadio->setPALevel(RF24_PA_LOW);
    pRadio->setPayloadSize(sizeof(RFPayload));
    pRadio->openWritingPipe(RF_Address); 
    pRadio->stopListening(); // Turn on TX Mode
  }
  else
  {
    Serial.println("Failed init radio");
  }

  return b_Success;
}




void v_computeButtonVoltageDividers(UiM_t_Inputs* pButtons)
{
  int i_Analog_Read = analogRead(BUTTON_ANALOG_PIN);
  // Reset buttons by default
  pButtons->inputButtonLeft    = LOW; 
  pButtons->inputButtonRight   = LOW; 
  pButtons->inputButtonSelect  = LOW; 


  if(i_Analog_Read < ANALOG_BUTTON_VDIV_THRESHOLD_B1)
  {
    pButtons->inputButtonRight = HIGH;
  }
  else if(i_Analog_Read > ANALOG_BUTTON_VDIV_THRESHOLD_B1 && i_Analog_Read < ANALOG_BUTTON_VDIV_THRESHOLD_B2)
  {
    pButtons->inputButtonLeft = HIGH;
  }
  else if(i_Analog_Read > ANALOG_BUTTON_VDIV_THRESHOLD_B2 && i_Analog_Read < ANALOG_BUTTON_VDIV_THRESHOLD_B3)
  {
    pButtons->inputButtonSelect = HIGH;
  }
}

void v_readButtons(UiM_t_Inputs* pInputs)
{
  pInputs->inputButtonLeft   = !digitalRead(INPUT_BUTTON_LEFT_PIN);
  pInputs->inputButtonRight  = !digitalRead(INPUT_BUTTON_RIGHT_PIN);
  pInputs->inputButtonSelect = !digitalRead(INPUT_BUTTON_SELECT_PIN);
}



void v_readChannelInputs(RemoteChannelInput_t *const pRemoteChannelInput)
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    if(pRemoteChannelInput[i].b_Analog)
    {
      pRemoteChannelInput[i].u16_Value = (uint16_t)analogRead(pRemoteChannelInput[i].u8_Pin);
      v_smoothAnalogEMA(&pRemoteChannelInput[i], i); // For now, raw value is also smoothend
      pRemoteChannelInput[i].u16_RawValue = pRemoteChannelInput[i].u16_Value; // Save raw value before any processing 
      if(pRemoteChannelInput[i].b_expControl)
      {
        v_applyExponential(&pRemoteChannelInput[i].u16_Value);
      }
      v_invertInput(&pRemoteChannelInput[i]); // Invertion of analog channels msut be processed before trimming and endpoint
      v_processTrimming(&pRemoteChannelInput[i]); // Trimming is processed before adjustment to ensure trim offset doesn't overload the min-max values
      v_processEndpointAdjustment(&pRemoteChannelInput[i]);
    }
    else
    {
      pRemoteChannelInput[i].u16_Value = map((uint16_t)digitalRead(pRemoteChannelInput[i].u8_Pin), LOW, HIGH, ANALOG_MIN_VALUE, ANALOG_MAX_VALUE);
      v_invertInput(&pRemoteChannelInput[i]); // Non-Analog channels can also be inverted.
    }
    
  }
}

/* Processes endpoint adjustment and overrides provided value if value is outside current configured endpoints */
void v_processEndpointAdjustment(RemoteChannelInput_t* pInput)
{
  pInput->u16_Value = (pInput->u16_Value > pInput->u16_MaxValue) ? pInput->u16_MaxValue : pInput->u16_Value; 
  pInput->u16_Value = (pInput->u16_Value < pInput->u16_MinValue) ? pInput->u16_MinValue : pInput->u16_Value; 
}

/* Process trimming and add the current trim offset to the actual value.*/
void v_processTrimming(RemoteChannelInput_t* pInput)
{
  uint8_t u8_trimOffset = (pInput->u16_Trim - ANALOG_HALF_VALUE);
  pInput->u16_Value += u8_trimOffset;
}

void v_invertInput(RemoteChannelInput_t* pInput)
{
  if(pInput->b_InvertInput)
  {
    pInput->u16_Value = map(pInput->u16_Value, ANALOG_MIN_VALUE, ANALOG_MAX_VALUE, ANALOG_MAX_VALUE, ANALOG_MIN_VALUE);
  }
}

void v_normalizeInput(uint16_t rawInput, float* normalizedOutput)
{
  *normalizedOutput = (rawInput / (float)ANALOG_HALF_VALUE) - 1.0;
}

void v_toRaw(float normalizedInput, uint16_t* rawOutput)
{
  *rawOutput = (normalizedInput + 1.0) * ANALOG_HALF_VALUE;
}

void v_applyExponential(uint16_t* rawInput) // TODO: have this take the remotechannelinput just like every other processing function
{
  float normalizedValue;
  v_normalizeInput(*rawInput, &normalizedValue);
  normalizedValue = normalizedValue * abs(normalizedValue) * (EXPONENTIAL_VALUE * (1.0 - abs(normalizedValue)) + abs(normalizedValue));
  v_toRaw(normalizedValue, rawInput);
} 

void v_smoothAnalogEMA(RemoteChannelInput_t* pInput, uint8_t channelIdx)
{
  static uint16_t average[N_CHANNELS];
  average[channelIdx] = EMA_ALPHA_VALUE * pInput->u16_Value + (1 - EMA_ALPHA_VALUE) * average[channelIdx]; // Still unsure if this is an improvement, but looks better with not much delay.
  pInput->u16_Value = average[channelIdx];
}

void v_buildPayload(const RemoteChannelInput_t* pRemoteChannelInput, RFPayload* pPayload)
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    pPayload->u16_Channels[i] = pRemoteChannelInput[i].u16_Value;
  }

}

#if DEBUG == ON
void printPayload(RFPayload* pl)
{
  uint8_t i;
  // for(i = 0; i < N_CHANNELS; i++)
  // {
  //   Serial.println(pl->u16_Channels[i]);
  // }
  Serial.println(pl->u16_Channels[1]);
  // Serial.println();
}
#endif


boolean b_sendPayload(RF24* pRadio, RFPayload* pPayload, unsigned long* lTransmissionTime)
{

#if DEBUG == ON
  printPayload(pPayload);
#endif
  // Time measure
  unsigned long lStartTimer = micros(); 
  boolean bPackageAcknowledged = pRadio->write(pPayload, sizeof(RFPayload));             
  unsigned long lEndTimer = micros();
  *lTransmissionTime = (lEndTimer - lStartTimer); // Total time to tx or timeout(configured internaly in rf24 as 60-70ms) if never acknowledged 
  return  bPackageAcknowledged; 
}

boolean b_transmissionTimeout(boolean bPackageAcknowledged)
{
  static unsigned long lPreviousSuccessfulTxTimestamp = 0l;
  bool bConnectionLost = false;


  if(bPackageAcknowledged)
  {
    lPreviousSuccessfulTxTimestamp = millis();
  }
  else
  {
    if(millis() - lPreviousSuccessfulTxTimestamp > TX_TIMEOUT)
    {
      bConnectionLost = true;
    }
  }
  return bConnectionLost;
}


void setup() 
{
  Serial.begin(115200);
  Serial.print(freeRam()); // TODO: Halt program, use u8x8 instead and display a msg on the screen
  Serial.print(F("Bytes\n"));
  v_initRemoteInputs(RemoteInputs);
  boolean b_initRadioSuccess = b_initRadio(&Radio);
  // TODO: Display a msg on screen if radio wasn't properly initialized
  
  v_UiM_init(&uiInputData, &uiResponseData);

}


void loop() 
{

  v_readChannelInputs(RemoteInputs);

  if(uiResponseData.analogSendAllowed)
  {
    v_buildPayload(RemoteInputs, &payload);
    boolean bSendSuccess = b_sendPayload(&Radio, &payload, &(RemoteCommunicationState.l_TransmissionTime));
    RemoteCommunicationState.b_ConnectionLost = b_transmissionTimeout(bSendSuccess);
    // TODO: Fix bug, oled not showing proper comm value
  }

#if BATTERY_INDICATION == ON
  bool battery_ready = battery.readBatteryVoltage(); // This is working but can't be seen with the arduino connected to pc. Otherwise will read the 5v instead of 9
  display_wrapper.printBatteryOLED(battery.getBatteryPercentage());
  // display_wrapper.printBatteryOLED(99.9);
#endif

  // Process UI inputs
  // v_computeButtonVoltageDividers(&uiInputs);
  v_readButtons(&uiInputs);

  uiInputs.scrollWheelRight = RemoteInputs[POT_RIGHT_CHANNEL_IDX].u16_RawValue; // Aditionally, let's map the scroll wheel here, for now
  uiInputs.scrollWheelLeft  = RemoteInputs[POT_LEFT_CHANNEL_IDX].u16_RawValue; // Aditionally, let's map the scroll wheel here, for now
  
  v_UiM_update();
  // TODO: use the response data to save configurations to eeprom. Later load configurations from eeprom at startup.
}
