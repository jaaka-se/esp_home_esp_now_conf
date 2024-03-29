//
// Author: jaaka.se
//
#ifndef __MESH32_RC_H__
#define __MESH32_RC_H__

#include <esp_now.h>
#include <WiFi.h>
namespace Mesh32RC {
#define MESH_RC_DEBUG_ALL_MSG true

typedef std::function<void(void)> esp_rc_callback_t;
typedef std::function<void(const uint8_t *, uint8_t)> esp_rc_data_callback_t;

struct esp_rc_event_t {
	String prefix;
	esp_rc_callback_t callback;
	esp_rc_data_callback_t callback2;
} events[250];

uint8_t buffer[250]; uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t channel = 0;
uint8_t events_num = 0;
uint32_t received;
uint32_t ignored;
uint32_t sendTime;
uint16_t duration;
uint8_t *master = NULL;
bool sending;// bussy
// This is the crypto replase with some thing other !!!! must be the same for all ESP's
uint8_t lmk[] = {'1','2','3','4','5','6','7','8','9','0','a','b','c','d','e','f'};

void setMaster(uint8_t *addr, uint8_t channel) {
	if (esp_now_is_peer_exist(master))
		esp_now_del_peer(master);
	master = addr;
	esp_now_peer_info_t peer;
	for (int ii = 0; ii < 6; ++ii) {
		peer.peer_addr[ii] = addr[ii];
	}
	peer.ifidx = WIFI_IF_STA;
	peer.channel = channel;
	peer.encrypt = false;
	const esp_now_peer_info_t *peer_p = & peer;
	esp_now_add_peer(peer_p);
}
void addReceiver(uint8_t *addr, uint8_t channel) {
	if (esp_now_is_peer_exist(addr)) {
		Serial.printf("Add Receiver exists %02X:%02X:%02X:%02X:%02X:%02X \n",addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	} else {
		esp_now_peer_info_t peer;
		for (int ii = 0; ii < 6; ++ii) {
			peer.peer_addr[ii] = addr[ii];
		}
		peer.ifidx = WIFI_IF_STA;
		peer.channel = channel;
		peer.encrypt = false;
		const esp_now_peer_info_t *peer_p = & peer;
		esp_err_t result = esp_now_add_peer(peer_p);
#ifdef MESH_RC_DEBUG_ALL_MSG
		if ( result == ESP_OK ) {
			Serial.println("ESPNOW esp_now_add_peer Success.");
		}
		else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
			// How did we get so far!!
			Serial.println("ESPNOW not init.");
		}	
		else if (result == ESP_ERR_ESPNOW_ARG) {
			Serial.println("esp_now_add_peer Invalid Argument");
		}
		else if (result == ESP_ERR_ESPNOW_FULL) {
			Serial.println("Peer List Full");
		}
		else if (result == ESP_ERR_ESPNOW_NO_MEM) {
			Serial.println("ESP_ERR_ESPNOW_NO_MEM");
		}
		else if (result == ESP_ERR_ESPNOW_EXIST) {
			Serial.println("Peer has existed.");
		}
#else
		if ( result != ESP_OK ) {
			Serial.println("ESPNOW esp_now_add_peer failed.");
		}
#endif
	}
}

void send(uint8_t *data, uint8_t size) { //updated
	sending = true;
	sendTime = micros();
	esp_err_t result = esp_now_send(NULL, data, size);
//	esp_err_t result = esp_now_send(master ? master : broadcast, data, size);
	if ( result != ESP_OK ) {
		if (result == ESP_ERR_ESPNOW_NOT_INIT) {
			// How did we get so far!!
			Serial.println("*** ESPNOW not Init.");
		}	
		else if (result == ESP_ERR_ESPNOW_ARG) {
			Serial.println("*** Invalid Argument");
		}
		else if (result == ESP_ERR_ESPNOW_INTERNAL) {
			Serial.println("*** Internal Error");
		}
		else if (result == ESP_ERR_ESPNOW_NO_MEM) {
			Serial.println("*** ESP_ERR_ESPNOW_NO_MEM");
		}
		else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
			Serial.println("*** Peer not found.");
		}
		else {
			Serial.println("*** Not sure what happened");
		}
	} else {
		Serial.printf("Sent %u bytes OK  \n",size);
//		Serial.printf("Sent %u bytes  to %02X:%02X:%02X:%02X:%02X:%02X OK  \n",size,addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	}
}

void send(String data) {
	send((uint8_t *)data.c_str(), data.length());
	Serial.printf(data.c_str());
	Serial.printf("\n");
}

void send(String type, uint8_t *data, uint8_t size) {
	memcpy(&buffer[0], (uint8_t *)type.c_str(), type.length());
	memcpy(&buffer[type.length()], data, size);
	send(buffer, type.length() + size);
}

//-----------------------support functions and register of conection to ESPHOME yml
//-----------------------register callback with or without parameters
void on(String prefix, esp_rc_callback_t callback) {
	events[events_num++] = (esp_rc_event_t){prefix, callback, NULL};
}

void on(String prefix, esp_rc_data_callback_t callback) {
	events[events_num++] = (esp_rc_event_t){prefix, NULL, callback};
}

void wait() {
	while (Mesh32RC::sending) yield();  // Wait while sending
}

void delayMs(uint32_t us) {
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
#ifdef MESH_RC_DEBUG_ALL_MSG
	Serial.printf(":: MESH32_RC  request %u delayintill %u \n: ",us , e);
#endif	
        if(m > e){ //overflow
            while(micros() > e){
                yield();
            }
        }
        while(micros() < e){
            yield();
        }
    }	
}

bool equals(const uint8_t *a, const uint8_t *b, uint8_t size, uint8_t offset = 0) {
	for (auto i = offset; i < offset + size; i++)
		if (a[i] != b[i])
			return false;
	return true;
}

void log_status() {
	Serial.println("My MacAddr "+WiFi.macAddress());
	Serial.println("My AP MacAddr " + WiFi.softAPmacAddress());
	
}



//-----------------------------init and callback definition
esp_now_send_cb_t sendHandler = [](const uint8_t *addr, esp_now_send_status_t err) { //updated
	sending = false;
	duration = micros() - sendTime;
	if ( err != ESP_NOW_SEND_SUCCESS ) {
		ESP_LOGD("Mesh32RC","*** ESP_NOW_SEND_FAILED Send cb to= %02X:%02X:%02X:%02X:%02X:%02X NOK  \n",addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
		ESP_LOGD("Mesh32RC","Send cb err= %i \n",err);
	} else {
		ESP_LOGD("Mesh32RC","Send cb from= %02X:%02X:%02X:%02X:%02X:%02X OK duration %u \n",addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],duration);
	}
};

esp_now_recv_cb_t recvHandler = [](const uint8_t *addr, const uint8_t *data, int size) { //updaterad
	static uint8_t offset, i;
#ifdef MESH_RC_DEBUG_ALL_MSG
	Serial.printf(":: MESH32_RC RECV %i bytes: ", size);
	for (i = 0; i < size; i++) {
		Serial.write(data[i]);
	}
	//Serial.printf(" [ ");
	//for (i = 0; i < size; i++) {
		//Serial.printf("%02X ", data[i]);
	//}
	//Serial.println("]");
	if (addr != NULL ) {
		Serial.printf(" Addr=");
		for (auto a = 0; a < 6;a++) {
			Serial.printf("%02X ", addr[a]);
		}
		Serial.println(" ");
	}
#endif
	// Only receives from master if set
	if (addr == NULL || master == NULL || equals(addr, master, 6)) {
		received++;
		//sender = addr;
		// check for yml-defined events => start all strings with unique event strings
		// call via events[] , should probably be changed to som event signal
		for (i = 0; i < events_num; i++) {
			offset = events[i].prefix.length();
			if (equals(data, (uint8_t *)events[i].prefix.c_str(), offset)) {
				if (events[i].callback) events[i].callback();
				if (events[i].callback2) events[i].callback2(&data[offset], size - offset);
#ifdef MESH_RC_DEBUG_ALL_MSG
				Serial.println("For me ");
#endif
			} 
		}
#ifdef MESH_RC_DEBUG_ALL_MSG
	} else {
		ignored++;
		Serial.println("Ignored ");
#endif
	}
};

void begin(uint8_t channel) { //updated
#ifdef MESH_RC_DEBUG_ALL_MSG
	Serial.println(":: MESH32_RC espnow begin: ");
#endif
	WiFi.mode(WIFI_MODE_APSTA);
	//esp_wifi_set_ps(WIFI_PS_NONE);

	esp_err_t result =esp_now_init();
	if (result == ESP_OK) {
#ifdef MESH_RC_DEBUG_ALL_MSG
	Serial.println(":: MESH32_RC espnow OK: ");
#endif
		//esp_now_set_self_role(ESP_NOW_ROLE_COMBO);//ej hittad??
		 uint8_t esp01C[6] = {0xE8, 0xDB, 0x84, 0x99, 0x4C, 0xA0};
		 //uint8_t esp01Cap[6] = {0xEA, 0xDB, 0x84, 0x99, 0x4C, 0xA0};// no ack
		 uint8_t esp01Cap[6] = {0x84, 0x0D, 0x8E, 0xA8, 0x18, 0x9C};// wemostemp
//const uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
addReceiver(broadcast,channel);
addReceiver(esp01C,channel);
addReceiver(esp01Cap,channel);
//addReceiver(,channel);

		esp_now_register_send_cb(sendHandler);
		esp_now_register_recv_cb(recvHandler);
#ifdef MESH_RC_DEBUG_ALL_MSG
	ESP_LOGD("Mesh32RC","C espnow channel: %u \n",channel);
#endif
	} else {
	ESP_LOGD("Mesh32RC","C espnow init failed: ");
	}
	
			
#ifdef MESH_RC_DEBUG_ALL_MSG
	Serial.println(":: MESH32_RC begin finished: ");
#endif
}
void Reboot() {
  ESP_LOGD("Mesh32RC","reboot orderd in 1s");
  delay(1000);
  ESP.restart();
};
 
}  // namespace Mesh32RC

#endif	//__MESH32_RC_H__
