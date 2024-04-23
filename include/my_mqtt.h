#ifndef MY_MQTT_H_
#define MY_MQTT_H_

#include "globale.h"

void log(const char *x);
void logn(const char *x, size_t l);


#define MQTT_HOST "egreen.iotnet.ro" // Broker address
#define MQTT_PORT 1883


#define Pstr2char(X) snprintf_P(sBuff, sizeof(sBuff) - 1, X)
#define Pstr2char2(X,Y) snprintf_P(sBuff, sizeof(sBuff) - 1, X,Y)
#define Pstr2char3(X,Y,Z) snprintf_P(sBuff, sizeof(sBuff) - 1, X,Y,Z)

#define Pstr2mqttSW(X) snprintf_P(sBuff, sizeof(sBuff) - 1, X);mqttClient.publish(Topic("R_sw"), 1, false, sBuff)
#define Pstr2mqttSW2(X,Y) snprintf_P(sBuff, sizeof(sBuff) - 1, X,Y);mqttClient.publish(Topic("R_sw"), 1, false, sBuff)

const char *Topic(const char *nt);
void connectToMqtt();
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttConnect(bool sessionPresent);
void onMqttPublish(const uint16_t &packetId);
void onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos);
void onMqttUnsubscribe(const uint16_t &packetId);
void loose_time(uint32 ms);
bool async_send_file2mqtt(const char *name);
void mqttListDir(const char *dirname);
void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties,
                   const size_t &len, const size_t &index, const size_t &total);
void onFlashEnd(int f);
void onFlashEnd_s(int f);
void sendOfflineFiles(void);
void SimpleCrash_ino();
void SetMQTTcallbacks();
void stergeFisiereBaloti(void);
void MoveFisiereBaloti(void);

#endif