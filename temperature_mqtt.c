/*
 * Copyright (c) 2015 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <sys/time.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

#include "jhd1313m1.h"
#include "temperature.h"
#include "upm.h"
#include "upm_utilities.h"
#include "signal.h"
#include "string.h"

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "MQTTExample"
#define TOPIC       "sensors/temperature/data"
#define QOS         1
#define TIMEOUT     10000L

bool shouldRun = true;

void sig_handler(int signo)
{
    if (signo == SIGINT)
        shouldRun = false;
}

int main(int argc, char **argv)
{
    signal(SIGINT, sig_handler);
    int fahrenheit;
    float celsius;

    MQTTClient_init_options init = MQTTClient_init_options_initializer;
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_SSLOptions sslopts = MQTTClient_SSLOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
  
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    // initialize a JHD1313m1 on I2C bus 0, LCD address 0x3e, RGB
    // address 0x62
    jhd1313m1_context lcd = jhd1313m1_init(512, 0x3e, 0x62);
    temperature_context temp = temperature_init(512);

    if (!lcd)
    {
        printf("jhd1313m1_i2c_init() failed\n");
        return 1;
    }

    int ndx = 0;
    char str1[20];
    char str2[20];
    char const *str_mqtt;
    uint8_t rgb[7][3] = {
        {0xd1, 0x00, 0x00},
        {0xff, 0x66, 0x22},
        {0xff, 0xda, 0x21},
        {0x33, 0xdd, 0x00},
        {0x11, 0x33, 0xcc},
        {0x22, 0x00, 0x66},
        {0x33, 0x00, 0x44}};

     while (shouldRun)
    {
        temperature_get_value(temp, &celsius);
        celsius = celsius * 0.6; //Arduino factor for 5V
        fahrenheit = (int) (celsius * 9.0/5.0 + 32.0);
        printf("%d degrees Celsius, or %d degrees Fahrenheit\n",
                (int)celsius, fahrenheit);

        snprintf(str1, sizeof(str1), "Temperature: ");
        snprintf(str2, sizeof(str2), "F: %d & C: %d", fahrenheit, (int)celsius);

	// Alternate rows on the LCD
        jhd1313m1_set_cursor(lcd, 0, 0);
        jhd1313m1_write(lcd, str1, strlen(str1));
        jhd1313m1_set_cursor(lcd, 1, 0);
        jhd1313m1_write(lcd, str2, strlen(str2));

	// Change the color
        uint8_t r = rgb[ndx%7][0];
        uint8_t g = rgb[ndx%7][1];
        uint8_t b = rgb[ndx%7][2];
        ndx++;
        jhd1313m1_set_color(lcd, r, g, b);

	// Create JSON Object for MQTT
	struct timeval time;
	gettimeofday(&time,NULL);
	char snum[2];
	sprintf(snum, "%d", (int) celsius);
	char cnum[13];
	sprintf(cnum, "%d", (int) time.tv_sec);
	char *sensor = "sensor_id";
	char *tempur = "temperature";
	char *value  = "value";
	char *timet   = "timestamp";
	struct json_object *jobj;
	jobj = json_object_new_object();
	json_object_object_add(jobj, sensor, json_object_new_string(tempur));
	json_object_object_add(jobj, value, json_object_new_string(snum));
	json_object_object_add(jobj, timet, json_object_new_string(cnum));
	str_mqtt = json_object_to_json_string(jobj);

	// Send JSON Object via MQTT
        pubmsg.payload = str_mqtt;
        pubmsg.payloadlen = strlen(str_mqtt);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);

	printf("Message with delivery token %d delivered\n", token);
	
	upm_delay(1);

    }
     
MQTTClient_disconnect(client, 10000);
MQTTClient_destroy(&client);
temperature_close(temp);

return 0;
}
