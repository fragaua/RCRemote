#include "Configuration.h"
#include <RF24.h>
#include <Joystick_if.h>

#include "UiManagement.h"





// Declare and configure each input on the remote controller.
// TODO: Later, the menus and inputs should be used to configure trimming and end-point adjustment on the fly.
// They shouldn't be configured here like some of the inputs are.
                                   
RemoteChannelInput_t RemoteInputs[N_CHANNELS] = 
                                    // Pin,                     Val,  Trim, Min,  Max,   Invert,  isAnalog,   Channel Name  
                                   {{JOYSTICK_LEFT_AXIS_X_PIN,  0u,   0u,   0u,   0u,      false,    true,    "JLX"}, 
                                    {JOYSTICK_LEFT_AXIS_Y_PIN,  0u,   0u,   255u, 255u,    false,    true,    "JLY"}, 
                                    {JOYSTICK_RIGHT_AXIS_X_PIN, 0u,   0u,   255u, 255u,    true,     true,    "JRX"}, 
                                    {JOYSTICK_RIGHT_AXIS_Y_PIN, 0u,   0u,   0u,   0u,      false,    true,    "JRY"}, 
                                    {POT_RIGHT_PIN,             0u,   0u,   0u,   0u,      false,    true,    "PR"},  
                                    {SWITCH_SP_LEFT_PIN,        0u,   0u,   0u,   0u,      true,     false,   "SWL"}, 
                                    {SWITCH_SP_RIGHT_PIN,       0u,   0u,   0u,   0u,      true,     false,   "SWR"}};

RemoteCommunicationState_t RemoteCommunicationState = {false, 0l};
UiM_t_Inputs  uiInputs;
UiM_t_rPorts  uiInputData = {&uiInputs, RemoteInputs, &RemoteCommunicationState};

#if RESPONSIVE_ANALOG_READ == ON
#include <ResponsiveAnalogRead.h>
ResponsiveAnalogRead ResponsiveAnalogs[N_ANALOG_CHANNELS];
#endif

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


// TODO: make a struct containing the responsive analog reads so that we can easily enable or disable it via if directives
// TODO: Improve naming to enforce the concept of "remote CHANNEL input" and ordinary "remote input" (such as buttons)
void v_initRemoteInputs(RemoteChannelInput_t* pRemoteInputs, ResponsiveAnalogRead* pRespAnalogRead)
{
  // Remote CHANNEL inputs
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    pinMode(RemoteInputs[i].u8_Pin,    RemoteInputs[i].b_Analog ? INPUT : INPUT_PULLUP);
#if RESPONSIVE_ANALOG_READ == ON
    if(RemoteInputs[i].b_Analog) // Only start responsive analogs for analog inputs.
    {
      ResponsiveAnalogs[i].begin(RemoteInputs[i].u8_Pin, true);
    }
#endif
  }
  // Remote input 
  pinMode(BUTTON_ANALOG_PIN, INPUT);
  
  // On the first version of Remote Controller, necessary for POT RIGHT to work.
  pinMode(POT_RIGHT_ACTIVATE_PIN, OUTPUT);
  digitalWrite(POT_RIGHT_ACTIVATE_PIN, HIGH);
}

boolean b_initRadio(RF24* pRadio)
{
  *pRadio = RF24(RF24_CE_PIN, RF24_CSN_PIN);
  bool b_Success = pRadio->begin();

  if(b_Success)
  {
    // Radio.setAutoAck(false); // Making sure auto ack isn't ON to ensure we can properly calcualte timeouts
    Radio.setPALevel(RF24_PA_LOW);
    Radio.setPayloadSize(sizeof(RFPayload));
    Radio.openWritingPipe(RF_Address); 
    Radio.stopListening(); // Turn on TX Mode
  }
  return b_Success;
}




void v_computeButtonVoltageDividers(UiM_t_Inputs* pButtons)
{
  // TODO: Debounce button input
  int i_Analog_Read = analogRead(BUTTON_ANALOG_PIN);
  // Reset buttons by default
  pButtons->inputButtonLeft = 0; 
  pButtons->inputButtonRight = 0; 
  pButtons->inputButtonSelect = 0; 


  if(i_Analog_Read < ANALOG_BUTTON_VDIV_THRESHOLD_DOWN)
  {
    pButtons->inputButtonRight = 1;
  }
  else if(i_Analog_Read > ANALOG_BUTTON_VDIV_THRESHOLD_DOWN && i_Analog_Read < ANALOG_BUTTON_VDIV_THRESHOLD_UP)
  {
    pButtons->inputButtonLeft = 1;
  }
  else if(i_Analog_Read > ANALOG_BUTTON_VDIV_THRESHOLD_UP && i_Analog_Read < 1000)
  {
    pButtons->inputButtonSelect = 1;
  }
}



void v_readChannelInputs(RemoteChannelInput_t *const pRemoteChannelInput, ResponsiveAnalogRead* pRespAnalogRead)
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    if(pRemoteChannelInput[i].b_Analog)
    {
      pRemoteChannelInput[i].u16_Value = (uint16_t)analogRead(pRemoteChannelInput[i].u8_Pin);
      v_processTrimming(&pRemoteChannelInput[i]); // Trimming is processed before adjustment to ensure trim offset doesn't overload the min-max values
      v_processEndpointAdjustment(&pRemoteChannelInput[i]);
#if RESPONSIVE_ANALOG_READ == ON
      pRespAnalogRead[i].update(pRemoteChannelInput[i].u16_Value);
      pRemoteChannelInput[i].u16_Value = pRespAnalogRead[i].getValue();
#endif
    }
    else
    {
      pRemoteChannelInput[i].u16_Value = map((uint16_t)digitalRead(pRemoteChannelInput[i].u8_Pin), LOW, HIGH, ANALOG_MIN_VALUE, ANALOG_MAX_VALUE);
    }
    
    v_invertInput(&pRemoteChannelInput[i]); // All channels, regardless of being analog or not, can be inverted
  }
}

/* Processes endpoint adjustment and overrides provided value if value is outside current configured endpoints */
void v_processEndpointAdjustment(RemoteChannelInput_t* pInput)
{
  // TODO: Scratch the concept of "offset" now that we have more memory to spare, otherwise endpoint setting is limited
  // to a max of 255.
  uint16_t u16_MaxValue = (ANALOG_MAX_VALUE - pInput->u8_MaxValueOffset);
  uint16_t u16_MinValue = (ANALOG_MIN_VALUE + pInput->u8_MinValueOffset);
  pInput->u16_Value = (pInput->u16_Value > u16_MaxValue) ? u16_MaxValue : pInput->u16_Value; 
  pInput->u16_Value = (pInput->u16_Value < u16_MinValue) ? u16_MinValue : pInput->u16_Value; 
}

/* Process trimming and add the current trim offset to the actual value.*/
void v_processTrimming(RemoteChannelInput_t* pInput)
{
  pInput->u16_Value += pInput->u8_Trim;
}

void v_invertInput(RemoteChannelInput_t* pInput)
{
  if(pInput->b_InvertInput)
  {
    pInput->u16_Value = map(pInput->u16_Value, ANALOG_MIN_VALUE, ANALOG_MAX_VALUE, ANALOG_MAX_VALUE, ANALOG_MIN_VALUE);
  }
}

void v_buildPayload(const RemoteChannelInput_t* pRemoteChannelInput, RFPayload* pPayload)
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    pPayload->u16_Channels[i] = pRemoteChannelInput[i].u16_Value;
  }

}


boolean b_sendPayload(RF24* pRadio, RFPayload* pPayload, unsigned long* lTransmissionTime)
{
  // Time measure
  unsigned long lStartTimer = micros(); 
  boolean bPackageAcknowledged = pRadio->write(&pPayload, sizeof(RFPayload));             
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
  
  v_initRemoteInputs(RemoteInputs, ResponsiveAnalogs);
  boolean b_initRadioSuccess = b_initRadio(&Radio);
  // TODO: Display a msg on screen if radio wasn't properly initialized
  
  v_UiM_init(&uiInputData);

}


void loop() 
{

  unsigned long lTxTime;
  v_readChannelInputs(RemoteInputs, ResponsiveAnalogs);
  v_buildPayload(RemoteInputs, &payload);

  boolean bSendSuccess = b_sendPayload(&Radio, &payload, &(RemoteCommunicationState.l_TransmissionTime));
  RemoteCommunicationState.b_ConnectionLost = b_transmissionTimeout(bSendSuccess);

#if BATTERY_INDICATION == ON
  bool battery_ready = battery.readBatteryVoltage(); // This is working but can't be seen with the arduino connected to pc. Otherwise will read the 5v instead of 9
  display_wrapper.printBatteryOLED(battery.getBatteryPercentage());
  // display_wrapper.printBatteryOLED(99.9);
#endif

  // Process UI inputs
  v_computeButtonVoltageDividers(&uiInputs);
  uiInputs.scrollWheel = RemoteInputs[POT_RIGHT_CHANNEL_IDX].u16_Value; // Aditionally, let's map the scroll wheel here, for now
  v_UiM_update();

  // v_updateOptionButtons(&display, ViewButtons, InternalRemoteInputs);
  // v_drawOptionButtons(&display, ViewButtons);
  // v_drawAnalogs(&display, RemoteInputs);
  // v_printConnectionStatusOLED(&display, i32TxTime, b_ConnectionLost);
}
