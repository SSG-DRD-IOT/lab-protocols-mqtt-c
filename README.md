# Objectives

### Read the Objectives

By the end of this module, you should be able to:

* Create a C program that sends temperature & humidity data via MQTT and also displays it on LCD

# Creating an MQTT Service to Publish the Temperature & Humidity Sensor Data

## Plug in the Grove shield, temperature & humidity sensor and the LCD

In the Sensors and Actuators lab, we connected the temperature & humidity sensor (I2C) and LCD display (I2C) to your sensor hub device. We wrote C code to measure the temperature in Celsius and Relative Humidity using upm library, convert it to Fahrenheit, then display it on the LCD.

1. Install the Grove Base Shield onto the sensor computer device (For this lab, this will be either a Up2 Board or a NUCi7).
2. Connect **Grove Temperature & Humidity Sensor** to an I2C pin **I2C1** of the Grove Base Shield.
3. Connect **Grove LCD** display to one of the **I2C2** pins.

## Start a new C program to add MQTT feature to our earlier temperature module

1. Create a new sketch in Arduino Create
2. Rename the sketch to temperature_mqtt.ino.
3. Delete the base code in the sketch
4. Copy the contents of the lab-temperature-sensor-c

This sensor project will be used in all the later labs to supply a steady stream of data.

### Write the C program that will send temperature and humidity data over MQTT

Update **temperature_mqtt.c** with following changes

1.  We will be using Paho MQTT in the Arduino Create Library for this lab. Under Libraries find Paho MQTT and include it in your program. Here MQTTClient.h is the header file of Paho MQTT C client library implemenation of MQTT protocol. Your includes should look like this.

  ```c
  // Eclipse Paho MQTT - Version: Latest
  #include <ArduinoPahoASync.h>
  #include <MQTTAsync.h>
  #include <MQTTClient.h>
  #include <MQTTClientPersistence.h>

  #include "th02.hpp"
  #include "upm_utilities.h"
  #include "jhd1313m1.h"
  ```

2.  Define the MQTT parameters we will use to connect using MQTT protocol. Here ADDRESS is our localhost or ip address of the computer device on port 1883, TOPIC_ONE is sensors/temperature/data on which we will publish the temperature values, and TOPIC_TWO is sensors/humidity/data on which we will publish the humidity values. Let's add a conditional "finished" for use in our call backs.

    ```c
    #define ADDRESS     "tcp://localhost:1883"
    #define CLIENTID    "MQTTExample"
    #define TOPIC_ONE   "sensors/temperature/data"
    #define TOPIC_TWO   "sensors/humidity/data"
    #define QOS         1
    #define TIMEOUT     10000L

    int finished = 0;
    ```

3.  In the main function of the program before the while loop first connect to MQTT client with the setup parameters. This is a MQTT asynchronous connection.
    
    ```c
    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(client, NULL, connLost, NULL, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) !=  MQTTASYNC_SUCCESS) {
      printf("Failed to start connect, return code %d\n", rc);
      exit(EXIT_FAILURE);
    }
    ```

4.  Next create the call back functions connLost, onConnect, and onConnectFailure for MQTT above the main function.
  
  ```c
    void connLost(void *context, char *cause) {
      MQTTAsync client = (MQTTAsync)context;
      MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
      int rc;

      printf("\nConnection lost\n");
      printf("     cause: %s\n", cause);

      printf("Reconnecting\n");
      conn_opts.keepAliveInterval = 20;
      conn_opts.cleansession = 1;
      if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
      }
    }
    void onConnectFailure(void* context, MQTTAsync_failureData* response) {
      printf("Connect failed, rc %d\n", response ? response->code : 0);
      finished = 1;
    }
    void onConnect(void* context, MQTTAsync_successData* response) {
      printf("Successful connection\n");
    }
	```
5.  Upload the sketch to the device and see if you get a successful connection in the Monitor serial port.

6.  We are going to use JSON to as our message format to publish via MQTT.  Include the JSON-C library in your sketch
	```c
    // JSON-C - Version: Latest
    #include <ArduinoJsonC.h>
    #include <json-c/json.h>

    // Eclipse Paho MQTT - Version: Latest
    #include <ArduinoPahoASync.h>
    #include <MQTTAsync.h>
    #include <MQTTClient.h>
    #include <MQTTClientPersistence.h>

    #include "th02.hpp"
    #include "upm_utilities.h"
    #include "jhd1313m1.h"
  ```
7.  We now need to fill in the messages with the proper data.  We will grab the system time and append it to the message for temperature and humidity.  After we write to the LCD let's add the JSON objects.
  ```c
  // Create JSON Objects for MQTT
  char cnum[13];
  sprintf(cnum, "%3.7f", celsius);
  char hnum[13];
  sprintf(hnum, "%3.7f", humidity);
  struct timeval time;
  gettimeofday(&time,NULL);
  char snum[13];
  sprintf(snum, "%d", (int) time.tv_sec);
  char *sensor = "sensor_id";
  char *temper = "temperature";
  char *humid = "humidity";
  char *value  = "value";
  char *timet   = "timestamp";
  //Create the json object for TOPIC_ONE i.e temperature
  struct json_object *jobj_1;
  jobj_1 = json_object_new_object();
  json_object_object_add(jobj_1, sensor, json_object_new_string(temper));
  json_object_object_add(jobj_1, value, json_object_new_string(cnum));
  json_object_object_add(jobj_1, timet, json_object_new_string(snum));

  //Create the json object for TOPIC_TWO i.e humidity
  struct json_object *jobj_2;
  jobj_2 = json_object_new_object();
  json_object_object_add(jobj_2, sensor, json_object_new_string(humid));
  json_object_object_add(jobj_2, value, json_object_new_string(hnum));
  json_object_object_add(jobj_2, timet, json_object_new_string(snum));

  ```
8.  Now let us publish the JSON objects.  Add the following code below the JSON object code.
  ```c
    char *str_mqtt;

    str_mqtt = (char *) json_object_to_json_string(jobj_1);

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    opts.onSuccess = onSend;
    opts.context = client;
    // Send JSON Object via MQTT
    pubmsg.payload = str_mqtt;
    pubmsg.payloadlen = strlen(str_mqtt);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    // Publish TOPIC_ONE
    if ((rc = MQTTAsync_sendMessage(client, TOPIC_ONE, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
      printf("Failed to start sendMessage, return code %d\n", rc);
    }

    // Wait until message is sent
    while (!finished) {
      usleep(10000L);
    }

    str_mqtt = (char *) json_object_to_json_string(jobj_2);

    // Send JSON Object via MQTT
    pubmsg.payload = str_mqtt;

    // Publish TOPIC_ONE
    if ((rc = MQTTAsync_sendMessage(client, TOPIC_TWO, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
      printf("Failed to start sendMessage, return code %d\n", rc);
    }

    // Wait until message is sent
    while (!finished) {
      usleep(10000L);
    }

  ```
9.  Let us add the onSend call back function before the main program

  ```c    
  void onSend(void* context, MQTTAsync_successData* response) {

    printf("Message with token value %d delivery confirmed\n", response->token);

    finished = 1;
    return;
  }

  ```

10. **Here is a link to the [final code](https://github.com/SSG-DRD-IOT/lab-protocols-mqtt-c/tree/master/_cmake/sketch/temperature_mqtt.ino.cpp).**

## Build and Run your C program

Upload your sketch to the device

You should see messages like below on your Monitor debug serial tab
```c
Message with token value 215 delivery confirmed
Message with token value 216 delivery confirmed
24.469 Celsius, or 76.044 Fahrenheit
21.438% Relative Humidity

```

To verify that we are successfully publishing the temperature data over MQTT topic **sensors/temperature/data** and **sensors/humidity/data** open two other SSH terminals to your Intel® IoT Gateway. Make sure this program is still running and publishing sensor data

In the new terminals use the below command to subscribe to MQTT topic

`mosquitto_sub -h localhost -p 1883 -t "sensors/temperature/data"`

and

`mosquitto_sub -h localhost -p 1883 -t "sensors/humidity/data"`

If the topic was published correctly you should start seeing the payload message as below:
```c
{ "sensor_id": "temperature", "value": "24.4687500", "timestamp": "1512599857" }
{ "sensor_id": "temperature", "value": "24.4687500", "timestamp": "1512599861" }
{ "sensor_id": "temperature", "value": "24.5000000", "timestamp": "1512599866" }
{ "sensor_id": "temperature", "value": "24.4687500", "timestamp": "1512599870" }
{ "sensor_id": "temperature", "value": "24.5000000", "timestamp": "1512599874" }
{ "sensor_id": "temperature", "value": "24.4687500", "timestamp": "1512599878" }
{ "sensor_id": "temperature", "value": "24.5000000", "timestamp": "1512599882" }
```

and

```c

{ "sensor_id": "humidity", "value": "21.6250000", "timestamp": "1512599866" }
{ "sensor_id": "humidity", "value": "21.5000000", "timestamp": "1512599870" }
{ "sensor_id": "humidity", "value": "21.5000000", "timestamp": "1512599874" }
{ "sensor_id": "humidity", "value": "21.5000000", "timestamp": "1512599878" }
{ "sensor_id": "humidity", "value": "21.4375000", "timestamp": "1512599882" }

```
You've completed the service that will publish temperature data to the gateway. See the Debugging MQTT section in the sidebar under Additional Information for information on how to verify that MQTT traffic is indeed being published.

## Solution

Click here to go to the [Lab Solution](https://create.arduino.cc/editor/danielholmlund/11194965-2c77-4d59-9849-8a79423c047f/preview).

https://create.arduino.cc/editor/danielholmlund/11194965-2c77-4d59-9849-8a79423c047f/preview

## Additional Resources

* [MQTT](http://mqtt.org/)
