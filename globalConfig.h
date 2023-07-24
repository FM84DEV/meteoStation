/*************************************************/
/* WIFI SETTINGS                                 */
/*************************************************/
const char* ssid = "SSIDNAME:TOCHANGE";
const char* password = "SSIDPSW:TOCHANGE";

const char* updateMD5paswd = "e3f6695d7e778038ec304bb6bc70ab64";  

String serverAddr = "IPADDR:TOCHANGE";                   //raspberry a cui passare i dati
String serverPage = "/pages/weather/dataUpdate.php";  //script aggiornamento dati centralina

// dovrei provare a comandare direttamente il shelly senza passare i dati al raspberry ???
// https://shelly-api-docs.shelly.cloud/gen1/#shelly2-5-relay-index

int httpUpdateReq = 0;

/*************************************************/
/*   DEBUG OPTION                                */
/*************************************************/
bool debugOutput = false;  // set to true for Debug print on serial port

unsigned long rebootTime = 0;                  //cyclic software reboot
unsigned long rebootTimeInterval = 0; //ms    

unsigned long wifiCheckTime = 0;               //wifi check reboot
unsigned long wifiCheckInterval = 0; //ms    


/*************************************************/
/* Application Settings                          */
/*************************************************/
/*
Ci sono tende certificate a braccia in queste 3 classi:
CLASSE 1 = velocità del vento 28 km/h - 40N/m - 4 vento moderato - “sollevamento di polvere e carta, rami agitati”
CLASSE 2 = velocità del vento 38 km/h - 70N/m - 5 vento teso - “oscillazione arbusti con foglie, si formano piccole onde nelle acque interne”
CLASSE 3 = velocità del vento 49 km/h - 110N/m - 6 vento fresco - “movimento di grossi rami, difficoltà ad usare l’ombrello”
*/
//WIND sensor config
int windSensorPin = 13;           //reed contact input  13=D7   
long wind_debouncing_time = 25;     //era 2ms ho messo 25ms tanto corrisponde a 40imp./secondo che sarebbe un vento di 154km/h per questo anemometro
const float number_reed = 1;  //number of reed = pulses per rotate
unsigned long windUpdateTime = 0;
unsigned long windUpdateInterval = 2500;   //ms    la centralina professionale davis sembra campionare a 2.5s
volatile unsigned long wind_counter = 0;
unsigned long wind_counter_copy = 0;   //per usarla nel codice la uso in flash anziche in ram e uso una copia della variabile, vedi codice.
float wind = 0;
float last_wind = 0;
volatile unsigned long wind_last_micros;
int windUpdateCounter = 0;
const int windUpdateCounterThr = 24; //update every windUpdateCounterThr*windUpdateInterval  24*2.5 = 1 min.

//RAIN sensor config
int rainSensorPin = 5;            //reed contact input  5=D1
float rain_mmOnePulse = 0.3;  //In teoria 0,3mm pioggia ogni impulso. Bisognerebbe provare con siringa acqua per caratterizzare il sensore. https://www.calctool.org/other/rainfall-volume
long rain_debouncing_time = 600;     //in ms --con 10ms provando a mano segnava piu impulsi a caso
unsigned long rainUpdateTime = 0;
unsigned long rainUpdateInterval = 60000;   //ms
volatile unsigned long rain_counter = 0;
unsigned long rain_counter_copy = 0;   //per usarla nel codice la uso in flash anziche in ram e uso una copia della variabile, vedi codice.
float rain=0;  
volatile unsigned long rain_last_micros;
 

//TEMPERATURE sensor config
//int temperatureSensorPin = 14;    //ds18b20 1wire  gpio14=D5
float temperature = 0;
unsigned long temperatureUpdateTime = 0; 
unsigned long temperatureUpdateInterval = 5000;   //ms   -  5 second






