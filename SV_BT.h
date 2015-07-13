#include <SoftwareSerial.h>
#define BT_SLOT_OPEN       0
#define BT_SHOW_GEAR_CMD   1
#define BT_SHOW_GEAR       "c1;"
#define BT_SHOW_LOG_CMD    9
#define BT_SHOW_LOG        "c9;"
#define BT_LEARN_GEAR_CMD  2
#define BT_LEARN_GEAR      "c2;"
#define BT_LOAD_GEAR_CMD   3
#define BT_LOAD_GEAR       "c3;"
#define BT_SAVE_GEAR_CMD   4
#define BT_SAVE_EEPROM_CMD 5

int BT_Receive(String &message);
int BT_Send(void);
int BT_prepareToSend(String message);
int get_waitingForSend(void);
bool is_slotOpen(void);
void BT_init(void);
