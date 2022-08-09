//
// Based on code from Author: Phong Vu
// Author: jaaka.se
//
#ifndef __MESH_RC_H__
#define __MESH_RC_H__

#include <espnow.h>

namespace MeshRC {
#define MESH_RC_DEBUG_ALL_MSG true

typedef std::function<void(void)> esp_rc_callback_t;
typedef std::function<void(u8 *, u8)> esp_rc_data_callback_t;

struct esp_rc_event_t {
	String prefix;
	esp_rc_callback_t callback;
	esp_rc_data_callback_t callback2;
} events[250];

u8 buffer[250];
u8 broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
u8 usedChannel = 0;
u8 events_num = 0;
u32 received;
u32 ignored;
u8 *sender = NULL;
u32 sendTime;
u16 duration;
u8 *master = NULL;
bool sending;
// This is the crypto replase with some thing other !!!! must be the same for all ESP's
u8 psk[] = {'1','2','3','4','5','6','7','8','9','0','a','b','c','d','e','f'};

void setMaster(u8 *addr) {
	if (esp_now_is_peer_exist(master))
		esp_now_del_peer(master);
	master = addr;
	esp_now_add_peer(master, ESP_NOW_ROLE_COMBO, usedChannel, psk, sizeof(psk));
}
void send(u8 *data, u8 size) {
	sending = true;
	sendTime = micros();
	esp_now_send(master ? master : broadcast, data, size);
}

void send(String data) {
	send((u8 *)data.c_str(), data.length());
	Serial.printf(data.c_str());
	Serial.printf("\n");
}

void send(String type, u8 *data, u8 size) {
	memcpy(&buffer[0], (u8 *)type.c_str(), type.length());
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
	while (MeshRC::sending) yield();  // Wait while sending
}

void delayMs(u32 time) {
    u32 m = micros();
    if(time){
        u32 e = (m + time);
//	u32 delayUntil = millis() + time;
// this is not safe due to wraparound //	while (millis() < delayUntil) yield();
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

bool equals(u8 *a, u8 *b, u8 size, u8 offset = 0) {
	for (auto i = offset; i < offset + size; i++)
		if (a[i] != b[i])
			return false;
	return true;
}

//-----------------------------init and callback definition
esp_now_send_cb_t sendHandler = [](u8 *addr, u8 err) {
	sending = false;
	duration = micros() - sendTime;
	if ( err != 0 ) {
		Serial.write("*** ESP_NOW_SEND_FAILED to ");
		Serial.printf(" Addr=");
		for (auto a = 0; a < 6;a++) {
			Serial.printf("%02X ", addr[a]);
		}
		Serial.println(" ");
#ifdef MESH_RC_DEBUG_ALL_MSG
	} else {
		Serial.printf("Send cb OK duration %u \n",duration);
#endif
	}
};

esp_now_recv_cb_t recvHandler = [](u8 *addr, u8 *data, u8 size) {
	static u8 offset, i;
#ifdef MESH_RC_DEBUG_ALL_MSG
	Serial.printf(":: MESH_RC RECV %u bytes: ", size);
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
		sender = addr;
		
		for (i = 0; i < events_num; i++) {
			offset = events[i].prefix.length();
			if (equals(data, (u8 *)events[i].prefix.c_str(), offset)) {
				if (events[i].callback) events[i].callback();
				if (events[i].callback2) events[i].callback2(&data[offset], size - offset);
#ifdef MESH_RC_DEBUG_ALL_MSG
				Serial.println("For me ");
#endif
				i=events_num; //exit only firt match
			} 
		}
	} else {
		ignored++;
#ifdef MESH_RC_DEBUG_ALL_MSG
		Serial.println("Ignored ");
#endif
	}
};
void begin(u8 channel) {
	if (esp_now_init() == OK) {
#ifdef MESH_RC_DEBUG_ALL_MSG
	Serial.println(":: MESH_RC espnow OK: ");
#endif
		if (esp_now_is_peer_exist(broadcast)) {
			esp_now_del_peer(broadcast);
		}
		esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
		usedChannel = channel;
		esp_now_add_peer(broadcast, ESP_NOW_ROLE_COMBO, usedChannel, 0, 0);
		esp_now_register_send_cb(sendHandler);
		esp_now_register_recv_cb(recvHandler);
#ifdef MESH_RC_DEBUG_ALL_MSG
	Serial.printf(":: MESH_RC espnow channel: %u \n",channel);
#endif
	} else {
	Serial.println(":: MESH_RC espnow init failed: ");
	}		
}
}  // namespace MeshRC

#endif	//__MESH_RC_H__
