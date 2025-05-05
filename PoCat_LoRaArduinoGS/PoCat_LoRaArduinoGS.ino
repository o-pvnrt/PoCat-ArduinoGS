/**
 * @file PoCat_LoRaArduinoGS.ino
 * @brief Main application for LoRa communication with telecommand handling and interleaving.
 *
 * This file implements the main logic for sending and receiving telecommands over LoRa,
 * including packet interleaving, command handling, and serial interface.
 */
#include "LoRaWan_APP.h"
#include "Arduino.h"
#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

/**
 * @def RF_FREQUENCY
 * @brief LoRa radio frequency in Hz.
 */
#define RF_FREQUENCY                                868000000 // Hz
/**
 * @def TX_OUTPUT_POWER
 * @brief LoRa transmit output power in dBm.
 */
#define TX_OUTPUT_POWER                             10        // dBm
/**
 * @def LORA_BANDWIDTH
 * @brief LoRa bandwidth setting.
 * 0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved
 */
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
/**
 * @def LORA_SPREADING_FACTOR
 * @brief LoRa spreading factor (SF7..SF12).
 */
#define LORA_SPREADING_FACTOR                       11        // [SF7..SF12]
/**
 * @def LORA_CODINGRATE
 * @brief LoRa coding rate (1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8).
 */
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
/**
 * @def LORA_PREAMBLE_LENGTH
 * @brief LoRa preamble length for Tx and Rx.
 */
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
/**
 * @def LORA_SYMBOL_TIMEOUT
 * @brief LoRa symbol timeout in symbols.
 */
#define LORA_SYMBOL_TIMEOUT                         100         // Symbols
/**
 * @def LORA_FIX_LENGTH_PAYLOAD_ON
 * @brief LoRa fixed length payload mode.
 */
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
/**
 * @def LORA_IQ_INVERSION_ON
 * @brief LoRa IQ inversion mode.
 */
#define LORA_IQ_INVERSION_ON                        false
/**
 * @def RX_TIMEOUT_VALUE
 * @brief LoRa receive timeout value in ms.
 */
#define RX_TIMEOUT_VALUE                            4000
/**
 * @def BUFFER_SIZE
 * @brief Payload buffer size.
 */
#define BUFFER_SIZE                                 100 // Define the payload size here

/**
 * @enum telecommandIDS
 * @brief Telecommand identifiers for various system operations.
 */
typedef enum {
    ERR,                /**< Error */
    PING,               /**< Ping */
    TRANSIT_TO_NM,      /**< Transit to NM */
    TRANSIT_TO_CM,      /**< Transit to CM */
    TRANSIT_TO_SSM,     /**< Transit to SSM */
    TRANSIT_TO_SM,      /**< Transit to SM */
    UPLOAD_ADCS_CALIBRATION, /**< Upload ADCS calibration */
    UPLOAD_ADCS_TLE,    /**< Upload ADCS TLE */
    UPLOAD_COMMS_CONFIG,/**< Upload comms config */
    COMMS_UPLOAD_PARAMS,/**< Comms upload params */
    UPLOAD_UNIX_TIME,   /**< Upload UNIX time */
    UPLOAD_EPS_TH,      /**< Upload EPS thresholds */
    UPLOAD_PL_CONFIG,   /**< Upload payload config */
    DOWNLINK_CONFIG,    /**< Downlink config */
    EPS_HEATER_ENABLE,  /**< Enable EPS heater */
    EPS_HEATER_DISABLE, /**< Disable EPS heater */
    POL_PAYLOAD_SHUT,   /**< Shut down payload */
    POL_ADCS_SHUT,      /**< Shut down ADCS */
    POL_BURNCOMMS_SHUT, /**< Shut down burn comms */
    POL_HEATER_SHUT,    /**< Shut down heater */
    POL_PAYLOAD_ENABLE, /**< Enable payload */
    POL_ADCS_ENABLE,    /**< Enable ADCS */
    POL_BURNCOMMS_ENABLE,/**< Enable burn comms */
    POL_HEATER_ENABLE,  /**< Enable heater */
    CLEAR_PL_DATA,      /**< Clear payload data */
    CLEAR_FLASH,        /**< Clear flash memory */
    CLEAR_HT,           /**< Clear housekeeping telemetry */
    COMMS_STOP_TX,      /**< Stop comms transmission */
    COMMS_RESUME_TX,    /**< Resume comms transmission */
    COMMS_IT_DOWNLINK,  /**< Initiate comms downlink */
    COMMS_HT_DOWNLINK,  /**< Initiate housekeeping telemetry downlink */
    PAYLOAD_SCHEDULE,   /**< Schedule payload */
    PAYLOAD_DEACTIVATE, /**< Deactivate payload */
    PAYLOAD_SEND_DATA,  /**< Send payload data */
    OBC_HARD_REBOOT,    /**< Hard reboot of OBC */
    OBC_SOFT_REBOOT,    /**< Soft reboot of OBC */
    OBC_PERIPH_REBOOT,  /**< Reboot peripherals of OBC */
    OBC_DEBUG_MODE      /**< Enable debug mode */
} telecommandIDS;

/**
 * @var txpacket
 * @brief Buffer for outgoing packets.
 */
uint8_t txpacket[48];
/**
 * @var RxData
 * @brief Buffer for received data.
 */
uint8_t RxData[48];

/**
 * @var RadioEvents
 * @brief Structure holding radio event callbacks.
 */
static RadioEvents_t RadioEvents;

/**
 * @var rssi
 * @brief Received signal strength indicator.
 */
int16_t rssi,rxSize;
/**
 * @var tcinput
 * @brief String for telecommand input from serial.
 */
String tcinput;
/**
 * @var tcnumber
 * @brief Telecommand number parsed from input.
 */
int tcnumber=0;
/**
 * @var i
 * @brief General purpose index, used in loops.
 */
volatile int i=0;
/**
 * @var lora_idle
 * @brief Indicates if LoRa is idle and ready for new operation.
 */
bool lora_idle = true;
/**
 * @var senddata_TLC_sent
 * @brief Counter for sent TLC data.
 */
int senddata_TLC_sent=0;

/**
 * @var tcpacket
 * @brief Buffer for telecommand packet.
 */
uint8_t tcpacket[48]={0xC8,0x9D,0x00,0x00,0x00,0x00,0x03,0x83,0x1E,0x19,0xDC,0x63,0x53,0xC4};
/**
 * @var Encoded_Packet
 * @brief Buffer for encoded (interleaved) packet.
 */
uint8_t Encoded_Packet[48];

/**
 * @var totalpacketsize
 * @brief Size of the packet to be sent.
 */
uint8_t totalpacketsize=0;

/**
 * @brief Cubecell setup function. Initializes serial, radio, and event handlers.
 */
void setup() {
    Serial.begin(115200);
    rssi=0;
  
    RadioEvents.RxDone = OnRxDone;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( 868000000 );
  
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                              LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                              true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
    RadioEvents.TxDone = OnTxDone; // standby
    RadioEvents.RxDone = OnRxDone;
   }

/**
 * @brief Cubecell main loop. Handles serial input and LoRa idle state.
 */
void loop()
{
  tcnumber=0;
  if (Serial.available() > 0)
  {
    tcinput=Serial.readStringUntil('\n');
    tcnumber=tcinput.toInt();
    printf("Test");
    SendTC(tcnumber);
    delay(1800);
  
  }
  
  if(lora_idle)
  {
  	turnOffRGB();
    lora_idle = false;
    Radio.Rx(0);
  }
  }

/**
 * @brief Sends a telecommand packet over LoRa.
 * @param TC Telecommand ID to send.
 */
void SendTC(uint8_t TC) {
  tcpacket[2]=TC;
  switch (TC) {
      case PING:
          printf("Executing: ping\n");
          break;
      case TRANSIT_TO_NM:
          printf("Executing: transit to NM\n");
          break;
      case TRANSIT_TO_CM:
          printf("Executing: transit to CM\n");
          break;
      case TRANSIT_TO_SSM:
          printf("Executing: transit to SSM\n");
          break;
      case TRANSIT_TO_SM:
          printf("Executing: transit to SM\n");
          break;
      case UPLOAD_ADCS_CALIBRATION:
          printf("Executing: upload ADCS calibration TBD\n");
          //TBD
          break;
      case UPLOAD_ADCS_TLE:
          printf("Executing: upload ADCS TLE TBD\n");
          //TBD
          break;
      case UPLOAD_COMMS_CONFIG:
          printf("Executing: upload comms config TBD\n");
          tcpacket[3]=128; // 128 == 868MHz , 255 == 915 MHz
          tcpacket[4]=11; //Spreading factor
          tcpacket[5]=1; //Coding rate
          break;
      case COMMS_UPLOAD_PARAMS:
          printf("Executing: comms upload params\n");
          tcpacket[3]=10; //Define rxtime in ms/100
          tcpacket[4]=10; //Define sleeptime in ms/100
          tcpacket[5]=64; //CADMODE --> 0 to 128==ON 128 to 255==OFF
          tcpacket[6]=255; //Packet window for Payload data sending.
          break;
      case UPLOAD_UNIX_TIME:
          printf("Executing: upload UNIX time TBD\n");
          //TBD
          break;
      case UPLOAD_EPS_TH:
          printf("Executing: upload EPS thresholds TBD\n");
          //TBD
          break;
      case UPLOAD_PL_CONFIG:
          printf("Executing: upload payload config TBD\n");
          //TBD
          break;
      case DOWNLINK_CONFIG:
          printf("Executing: downlink config TBD\n");
          //TBD
          break;
      case EPS_HEATER_ENABLE:
          printf("Executing: enable EPS heater TBD\n");
          //TBD
          break;
      case EPS_HEATER_DISABLE:
          printf("Executing: disable EPS heater TBD\n");
          //TBD
          break;
      case POL_PAYLOAD_SHUT:
          printf("Executing: shut down payload TBD\n");
          break;
      case POL_ADCS_SHUT:
          printf("Executing: shut down ADCS TBD\n");
          break;
      case POL_BURNCOMMS_SHUT:
          printf("Executing: shut down burn comms TBD\n");
          break;
      case POL_HEATER_SHUT:
          printf("Executing: shut down heater TBD\n");
          break;
      case POL_PAYLOAD_ENABLE:
          printf("Executing: enable payload TBD\n");
          break;
      case POL_ADCS_ENABLE:
          printf("Executing: enable ADCS TBD\n");
          break;
      case POL_BURNCOMMS_ENABLE:
          printf("Executing: enable burn comms TBD\n");
          break;
      case POL_HEATER_ENABLE:
          printf("Executing: enable heater TBD\n");
          break;
      case CLEAR_PL_DATA:
          printf("Executing: clear payload data TBD\n");
          break;
      case CLEAR_FLASH:
          printf("Executing: clear flash memory TBD\n");
          break;
      case CLEAR_HT:
          printf("Executing: clear housekeeping telemetry TBD\n");
          break;
      case COMMS_STOP_TX:
          printf("Executing: stop comms transmission\n");
          break;
      case COMMS_RESUME_TX:
          printf("Executing: resume comms transmission\n");
          break;
      case COMMS_IT_DOWNLINK:
          printf("Executing: initiate comms downlink\n");
          break;
      case COMMS_HT_DOWNLINK:
          printf("Executing: initiate housekeeping telemetry downlink TBD\n");
          //TBD
          break;
      case PAYLOAD_SCHEDULE:
          printf("Executing: schedule payload TBD\n");
          //TBD
          break;
      case PAYLOAD_DEACTIVATE:
          printf("Executing: deactivate payload TBD\n");
          //TBD33
          break;
      case PAYLOAD_SEND_DATA:
          printf("Executing: send payload data\n");
          break;
      case OBC_HARD_REBOOT:
          printf("Executing: hard reboot of OBC TBD\n");
          //TBD
          break;
      case OBC_SOFT_REBOOT:
          printf("Executing: soft reboot of OBC TBRD\n");
          //TBRD
          break;
      case OBC_PERIPH_REBOOT:
          printf("Executing: reboot peripherals of OBC TBD\n");
          //TBD
          break;
      case OBC_DEBUG_MODE:
          printf("Executing: enable debug mode TBD\n");
          //TBD
          break;
      default:
          printf("Unknown command\n");
          break;
  }

  totalpacketsize=48;
  uint8_t *TxData = (uint8_t *) malloc(totalpacketsize);
  
  if (TxData == NULL) {
       exit(EXIT_FAILURE);
  }

  memcpy(TxData,tcpacket,totalpacketsize);
  Serial.println();
	interleave((uint8_t*) TxData, totalpacketsize);
	memcpy(Encoded_Packet,TxData,totalpacketsize);
	free(TxData);
  Serial.println();
  Radio.Send(Encoded_Packet,sizeof(Encoded_Packet));
}

/**
 * @brief Prints a byte as two-digit hexadecimal to Serial.
 * @param num Byte to print.
 */
void printHex(uint8_t num) {
  char hexCar[2];
  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}

/**
 * @brief Callback for when a LoRa packet is received.
 * @param payload Pointer to received data.
 * @param size Size of received data.
 * @param rssi Received signal strength indicator.
 * @param snr Signal-to-noise ratio.
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    turnOnRGB(COLOR_RECEIVED,0);

  	memset(RxData,0,sizeof(RxData));
    uint8_t *RxPacket =(uint8_t *) malloc(size);
    if (RxPacket == NULL) {
        exit(EXIT_FAILURE);
    }

    memcpy(RxPacket,payload,size);

    deinterleave((uint8_t*) RxPacket,size);
    memcpy(RxData,RxPacket,size);

    free(RxPacket);    
    Radio.Sleep();
    for(i=0; i<size; i++){
      printHex(RxData[i]);
    }
    Serial.println();
    lora_idle = true;
}

/**
 * @brief Callback for when a LoRa packet is sent.
 */
void OnTxDone()
{
  lora_idle = true;;
  
}

/**
 * @brief Interleaves the input array in groups of 6 for error resilience.
 * @param inputarr Pointer to input array.
 * @param size Size of the array (must be multiple of 6).
 */
void interleave(uint8_t *inputarr, int size) {
    // Check that the size is a multiple of 6.
    if (size % 6 != 0) {
        return;
    }

    int groupSize = size / 6;

    // Allocate temporary array to hold the interleaved result.
    uint8_t *temp =(uint8_t *) malloc(size * sizeof(uint8_t));
    if (temp == NULL) {
        exit(EXIT_FAILURE);
    }

    // For each index within the groups,
    // pick one element from each of the 6 groups.
    for (int i = 0; i < groupSize; i++) {
        for (int j = 0; j < 6; j++) {
            temp[i * 6 + j] = inputarr[j * groupSize + i];
        }
    }

    // Copy the interleaved elements back into the original array.
    memcpy(inputarr, temp, size * sizeof(uint8_t));

    free(temp);
}

/**
 * @brief Deinterleaves the input array in groups of 6 to restore original order.
 * @param inputarr Pointer to input array.
 * @param size Size of the array (must be multiple of 6).
 */
void deinterleave(uint8_t *inputarr, int size) {
    if (size % 6 != 0) {
        return;
    }

    int groupSize = size / 6;
    uint8_t *temp =(uint8_t *) malloc(size * sizeof(uint8_t));
    if (temp == NULL) {
        exit(EXIT_FAILURE);
    }

    // Reconstruct the original groups.
    // For each group index i and for each group j:
    // The interleaved array holds the jth element of group j at position i*6 + j.
    // We restore it to temp[ j * groupSize + i ].
    for (int i = 0; i < groupSize; i++) {
        for (int j = 0; j < 6; j++) {
            temp[j * groupSize + i] = inputarr[i * 6 + j];
        }
    }

    memcpy(inputarr, temp, size * sizeof(uint8_t));
    free(temp);
}

