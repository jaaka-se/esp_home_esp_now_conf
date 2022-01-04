# esp_home_esp_now_conf
Test integration of HomeAssistant and EspHome with EspNow satelits on battery 

startwith "cp secrets.yaml_template secrets.yaml" and replace SSID and pwd strings
secrets.yaml shuold never be checkedin its in .gitignore you have to keep track of SSID and pwd by other meens

MeshRC.h and Mesh32RC.h contains macros for calls to standard-libs.
esp8266 and esp32 needs diffrent parameters in calls henns diffrent macro-files
